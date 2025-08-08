#pragma once

#include "Log.hpp"
#include "Pins.hpp"
#include "utils.hpp"

#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <algorithm>
#include <functional>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <vector>
#include <deque>

namespace pcp {
    using MsTime = uint32_t;
    using PWMPulseWidth = uint32_t;
    using Completion = std::function<void(void)>;

    struct MotorOperation {
        using DataPoint = std::pair<MsTime, PWMPulseWidth>;

        MotorOperation(const std::string& name, const std::initializer_list<DataPoint>& speeds) : _name(name), _speeds(speeds) {}
        std::string _name;
        std::vector<DataPoint> _speeds;
    };

    template <PWMPulseWidth minThrottlePWM = 1000, uint32_t maxThrottlePWM = 2000>
    class Motor {
        public:
            Motor();
            ~Motor();

            bool isArmed(void) const;
            void arm(Completion completion = Completion());
            void disarm(Completion completion = Completion());
            void setThrottle(int8_t percentage, MsTime duration = 0);
            void decreaseThrottle(int8_t percentage, MsTime duration);
            void increaseThrottle(int8_t percentage, MsTime duration);
            uint8_t throttle() const;
        
        private:
            uint8_t _throttleForPWM(uint32_t pwm) const;
            uint32_t _pwmForThrottle(uint8_t throttle) const;

            MotorOperation _changeThrottleOp(int8_t percentage, MsTime duration) const;
            void _runOperation(const MotorOperation& op, Completion completion);

            void _setupTimers(void);

            std::optional<MsTime> _updateThrottle();
            void _setThrottlePWM(uint32_t throttlePWM);

            void _timeAdvance(uint32_t deltaT);

            uint32_t _lastQueuedPWM(void) const;
            uint8_t _lastQueuedThrottle(void) const;

            template <PWMPulseWidth min, uint32_t max>
            friend bool _timerFull(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t* edata, void *user_ctx);
            template <PWMPulseWidth min, uint32_t max>
            friend void _updateThrottleTask(void* userInfo);

            mcpwm_timer_handle_t _timerHandle = nullptr;
            mcpwm_cmpr_handle_t _comparatorHandle = nullptr;
            MsTime _time = 0;
            PWMPulseWidth _throttlePWM = 0;
            bool _isArmed = false;

            std::deque<std::pair<MotorOperation, Completion>> _operationQueue;
            TaskHandle_t _updateTask = nullptr;
            SemaphoreHandle_t _taskSemaphore = nullptr;

            static MotorOperation _armOp;
            static MotorOperation _unarmOp;
    };

    static constexpr uint32_t kInteruptPriority = 3;
    static constexpr uint32_t kArmSpeed = 1;

    static constexpr uint32_t kMsPerSec = 1'000;

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    MotorOperation Motor<minThrottlePWM, maxThrottlePWM>::_armOp = MotorOperation("Arm", {
        MotorOperation::DataPoint(0 * kMsPerSec / kArmSpeed, 0),
        MotorOperation::DataPoint(3 * kMsPerSec / kArmSpeed, 0),
        MotorOperation::DataPoint(3 * kMsPerSec / kArmSpeed, minThrottlePWM / 2),
        MotorOperation::DataPoint(4 * kMsPerSec / kArmSpeed, (minThrottlePWM + maxThrottlePWM) / 2),
        MotorOperation::DataPoint(5 * kMsPerSec / kArmSpeed, minThrottlePWM / 2),
        MotorOperation::DataPoint(6 * kMsPerSec / kArmSpeed, minThrottlePWM / 2),
    });

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    MotorOperation Motor<minThrottlePWM, maxThrottlePWM>::_unarmOp = MotorOperation("Un-arm", {
        MotorOperation::DataPoint(0 * kMsPerSec / kArmSpeed, 0),
        MotorOperation::DataPoint(1 * kMsPerSec / kArmSpeed, 0),
    });

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    bool _timerFull(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx);

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    bool _timerStopped(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx);

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void _updateThrottleTask(void* userInfo);

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    Motor<minThrottlePWM, maxThrottlePWM>::Motor() {
        _setupTimers();
        _taskSemaphore = xQueueGenericCreate((UBaseType_t)1, semSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE );
        BaseType_t err = xTaskCreate(_updateThrottleTask<minThrottlePWM, maxThrottlePWM>, "Throttle Update Task", 8192, this, 10, &_updateTask);
        if (err != pdPASS) {
            PCP_LOGE("Motor task creation failed: %s", freeRTOSErrorString(err));
        }
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    Motor<minThrottlePWM, maxThrottlePWM>::~Motor() {
        vTaskDelete(_updateTask);
        vSemaphoreDelete(_taskSemaphore);
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    std::optional<MsTime> Motor<minThrottlePWM, maxThrottlePWM>::_updateThrottle() {
        if (_comparatorHandle == nullptr) {
            return std::optional<MsTime>();
        } 
        
        do {
            if (_operationQueue.empty()) {
                return std::optional<MsTime>();
            }
            const MotorOperation& operation = _operationQueue.front().first;
            if (operation._speeds.back().first < _time) {
                _setThrottlePWM(operation._speeds.back().second);

                const Completion& completion = _operationQueue.front().second;
                if (completion) {
                    completion();
                }

                _time = 0;
                _operationQueue.pop_front();
                if (!_operationQueue.empty()) {
                }
            } else {
                break;
            }
        } while (true);

        const MotorOperation& operation = _operationQueue.front().first;

        const MotorOperation::DataPoint &secondLast = *(operation._speeds.end() - 2);
        const MotorOperation::DataPoint &last = *(operation._speeds.end() - 1);

        MsTime preTime = secondLast.first;
        MsTime postTime = last.first;
        PWMPulseWidth preSpeed = secondLast.second;
        PWMPulseWidth postSpeed = last.second;

        for (auto iter = operation._speeds.begin(); iter != operation._speeds.end(); ++iter) {
            if (iter->first > _time) {
                if (iter == operation._speeds.begin()) {
                    preTime = iter->first;
                    postTime = iter->first;
                    preSpeed = iter->second;
                    postSpeed = iter->second;
                } else {
                    preTime = (iter - 1)->first;
                    preSpeed = (iter - 1)->second;
                    postTime = iter->first;
                    postSpeed = iter->second;
                }
                break;
            }
        }

        PWMPulseWidth speed = preSpeed;
        MsTime timeToNextUpdate = 20;
        if (preSpeed == postSpeed) {
            timeToNextUpdate = postTime - _time;
        } else {
            const MsTime timePeriod = postTime - preTime;
            const MsTime timeIn = std::clamp(_time - preTime, (uint32_t)0, timePeriod);
            const int32_t speedIncrease = (int32_t)postSpeed - (int32_t)preSpeed;
            const int32_t scaledSpeedIncrease = timePeriod == 0 ? 0 : ((int32_t)timeIn * speedIncrease) / (int32_t)timePeriod;
            speed = (PWMPulseWidth)((int32_t)preSpeed + scaledSpeedIncrease);
        }

        _setThrottlePWM(speed);

        return timeToNextUpdate;
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::_setThrottlePWM(uint32_t throttlePWM) {
        _throttlePWM = std::clamp(throttlePWM, (PWMPulseWidth)0, (PWMPulseWidth)maxThrottlePWM);
        esp_err_t err = mcpwm_comparator_set_compare_value(_comparatorHandle, _throttlePWM);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while updating comparator value: %s", esp_err_to_name(err));
        }
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    uint8_t Motor<minThrottlePWM, maxThrottlePWM>::throttle() const {
        return _throttleForPWM(_throttlePWM);
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    uint8_t Motor<minThrottlePWM, maxThrottlePWM>::_throttleForPWM(uint32_t pwm) const {
        return invLerpPercentage(pwm, minThrottlePWM, maxThrottlePWM);
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    uint32_t Motor<minThrottlePWM, maxThrottlePWM>::_pwmForThrottle(uint8_t throttle) const {
        return lerpPercentage(minThrottlePWM, maxThrottlePWM, throttle);
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::_setupTimers(void) {
        esp_err_t err = ESP_OK;
        
        mcpwm_timer_handle_t timerHandle;
        mcpwm_oper_handle_t operatorHandle;
        mcpwm_gen_handle_t generatorHandle;

        mcpwm_timer_config_t timerConfig = {
            .group_id = 0,
            .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
            .resolution_hz = 1'000'000,
            .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
            .period_ticks = 20'000,
            .intr_priority = kInteruptPriority,
            .flags = {
                .update_period_on_empty = false,
                .update_period_on_sync = false,
                .allow_pd = false,
            }
        };
        err = mcpwm_new_timer(&timerConfig, &timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating timer: %s", esp_err_to_name(err));
            return;
        }
        
        mcpwm_operator_config_t operatorConfig = {
            .group_id = 0,
            .intr_priority = kInteruptPriority,
            .flags = {
                .update_gen_action_on_tez = false,
                .update_gen_action_on_tep = false,
                .update_gen_action_on_sync = false,
                .update_dead_time_on_tez = false,
                .update_dead_time_on_tep = false,
                .update_dead_time_on_sync = false,
            }
        };
        err = mcpwm_new_operator(&operatorConfig, &operatorHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating operator: %s", esp_err_to_name(err));
            return;
        }
        err = mcpwm_operator_connect_timer(operatorHandle, timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while attaching operator to timer: %s", esp_err_to_name(err));
            return;
        }

        mcpwm_timer_event_callbacks_t timerCallbacks = {
            .on_full = _timerFull<minThrottlePWM, maxThrottlePWM>,
            .on_empty = nullptr,
            .on_stop = _timerStopped<minThrottlePWM, maxThrottlePWM>,
        };
        err = mcpwm_timer_register_event_callbacks(timerHandle, &timerCallbacks, this);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while setting up timer callbacks: %s", esp_err_to_name(err));
            return;
        }

        mcpwm_comparator_config_t comparatorConfig = {
            .intr_priority = kInteruptPriority,
            .flags  {
                .update_cmp_on_tez = false,
                .update_cmp_on_tep = false,
                .update_cmp_on_sync = false,
            }
        };
        err = mcpwm_new_comparator(operatorHandle, &comparatorConfig, &_comparatorHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating comparator: %s", esp_err_to_name(err));
            return;
        }
        err = mcpwm_comparator_set_compare_value(_comparatorHandle, 0);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while setting comparator value: %s", esp_err_to_name(err));
            return;
        }

        mcpwm_generator_config_t generatorCongif = {
            .gen_gpio_num = kMotorOutputGPIO,
            .flags = {
                .invert_pwm = false,
                .io_loop_back = false,
                .io_od_mode = false,
                .pull_up = false,
                .pull_down = true,
            }
        };
        err = mcpwm_new_generator(operatorHandle, &generatorCongif, &generatorHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating generator: %s", esp_err_to_name(err));
            return;
        }
        err = mcpwm_generator_set_action_on_timer_event(generatorHandle, (mcpwm_gen_timer_event_action_t){ .direction = MCPWM_TIMER_DIRECTION_UP, .event = MCPWM_TIMER_EVENT_EMPTY, .action = MCPWM_GEN_ACTION_HIGH });
        if (err != ESP_OK) {
            PCP_LOGE("While setting generator to output high on timer reset: %s", esp_err_to_name(err));
            return;
        }
        err = mcpwm_generator_set_action_on_compare_event(generatorHandle, (mcpwm_gen_compare_event_action_t){ .direction = MCPWM_TIMER_DIRECTION_UP, .comparator = _comparatorHandle, .action = MCPWM_GEN_ACTION_LOW });
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while setting generator to output low on comparator match: %s", esp_err_to_name(err));
            return;
        }

        err = mcpwm_timer_enable(timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while enabling timer: %s", esp_err_to_name(err));
            return;
        }

        err = mcpwm_timer_start_stop(timerHandle, MCPWM_TIMER_START_NO_STOP);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while starting timer: %s", esp_err_to_name(err));
            return;
        }
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    bool Motor<minThrottlePWM, maxThrottlePWM>::isArmed(void) const {
        return _isArmed;
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::arm(Completion completion) {
        _runOperation(_armOp, [this, completion]() {
            this->_isArmed = true;
            completion();
        });
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::disarm(Completion completion) {
        _isArmed = false;
        _runOperation(_unarmOp, completion);
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    MotorOperation Motor<minThrottlePWM, maxThrottlePWM>::_changeThrottleOp(int8_t percentage, MsTime duration) const {
        const uint32_t lastPWM = std::clamp(_lastQueuedPWM(), minThrottlePWM, maxThrottlePWM);
        uint32_t newPWM = (uint32_t)(lastPWM + ((int32_t)percentage * (int32_t)(maxThrottlePWM - minThrottlePWM)) / 100);
        return MotorOperation("Change Throttle", {
            MotorOperation::DataPoint(0, lastPWM),
            MotorOperation::DataPoint(duration, newPWM)
        });
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::setThrottle(int8_t percentage, MsTime duration) {
        _runOperation(_changeThrottleOp(percentage - _lastQueuedThrottle(), duration), [](){});
    }
    
    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::decreaseThrottle(int8_t percentage, MsTime duration) {
        _runOperation(_changeThrottleOp(-percentage, duration), [](){});
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::increaseThrottle(int8_t percentage, MsTime duration) {
        _runOperation(_changeThrottleOp(percentage, duration), [](){});
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::_runOperation(const MotorOperation& op, Completion completion) {
        bool isEmpty = _operationQueue.empty();
        _operationQueue.emplace_back(op, completion);
        if (isEmpty) {
            _time = 0;
            xSemaphoreGive(_taskSemaphore);
        }
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void Motor<minThrottlePWM, maxThrottlePWM>::_timeAdvance(uint32_t deltaT) {
        _time += deltaT;
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    uint32_t Motor<minThrottlePWM, maxThrottlePWM>::_lastQueuedPWM(void) const {
        for (auto iter = _operationQueue.rbegin(); iter != _operationQueue.rend(); ++iter) {
            const MotorOperation& op = iter->first;
            if (!op._speeds.empty()) {
                return op._speeds.back().second;
            }
        }
        return _throttlePWM;
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    uint8_t Motor<minThrottlePWM, maxThrottlePWM>::_lastQueuedThrottle(void) const {
        return _throttleForPWM(_lastQueuedPWM());
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    bool _timerFull(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx) {
        Motor<minThrottlePWM, maxThrottlePWM>* motor = reinterpret_cast<Motor<minThrottlePWM, maxThrottlePWM>*>(user_ctx);
        motor->_timeAdvance(20);
        return false;
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    bool _timerStopped(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t *edata, void *user_ctx) {
        PCP_LOGW("Timer Stopped!");
        return false;
    }

    template <uint32_t minThrottlePWM, uint32_t maxThrottlePWM>
    void _updateThrottleTask(void* userInfo) {
        Motor<minThrottlePWM, maxThrottlePWM>* motor = reinterpret_cast<Motor<minThrottlePWM, maxThrottlePWM>*>(userInfo);
        
        while (!xSemaphoreTake(motor->_taskSemaphore, portMAX_DELAY)) {}

        while (true) {
            std::optional<MsTime> time = motor->_updateThrottle();
            if (time.has_value()) {
                vTaskDelay(*time / portTICK_PERIOD_MS);
            } else {
                while (!xSemaphoreTake(motor->_taskSemaphore, portMAX_DELAY)) {}
            }
        }
    }
}
