#pragma once

#include "ESCOperation.hpp"
#include "Log.hpp"
#include "Pins.hpp"
#include "Utilities/Maths.hpp"
#include "Utilities/MsTime.hpp"
#include "Utilities/freeRTOSErrorString.hpp"

#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <algorithm>
#include <cstdint>
#include <deque>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace pcp {
    using PWMPulseWidth = int32_t;

    template <PWMPulseWidth minThrottlePWM = 1000, PWMPulseWidth maxThrottlePWM = 2000>
    class ESCControlSchemePWM {
    public:
        ESCControlSchemePWM();
        ~ESCControlSchemePWM();

        bool isArmed(void) const;

        ESCState escState(void) const { return _state; }

        void arm(Completion completion = []() {});
        void disarm(Completion completion = []() {});
        void setThrottle(int8_t percentage, MsTime duration = 0);
        void decreaseThrottle(int8_t percentage, MsTime duration);
        void increaseThrottle(int8_t percentage, MsTime duration);
        uint8_t throttle() const;

        std::string stateString(void) const;

    private:
        uint8_t _throttleForPWM(PWMPulseWidth pwm) const;
        PWMPulseWidth _pwmForThrottle(uint8_t throttle) const;

        ESCOperation<PWMPulseWidth> _changeThrottleOp(int8_t percentage, MsTime duration) const;
        void _runOperation(const ESCOperation<PWMPulseWidth>& op, Completion completion);

        void _setupTimers(void);

        std::optional<MsTime> _updateThrottle();
        void _setThrottlePWM(PWMPulseWidth throttlePWM);

        void _timeAdvance(uint32_t deltaT);

        PWMPulseWidth _lastQueuedPWM(void) const;
        uint8_t _lastQueuedThrottle(void) const;

        template <PWMPulseWidth min, PWMPulseWidth max>
        friend bool _timerFull(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t* edata, void* user_ctx);
        template <PWMPulseWidth min, PWMPulseWidth max>
        friend void _updateThrottleTask(void* userInfo);

        mcpwm_timer_handle_t _timerHandle = nullptr;
        mcpwm_oper_handle_t _operatorHandle = nullptr;
        mcpwm_gen_handle_t _generatorHandle = nullptr;
        mcpwm_cmpr_handle_t _comparatorHandle = nullptr;
        MsTime _time = 0;
        PWMPulseWidth _throttlePWM = 0;
        ESCState _state = ESCState::Disarmed;

        std::deque<std::pair<ESCOperation<PWMPulseWidth>, Completion>> _operationQueue;
        TaskHandle_t _updateTask = nullptr;
        SemaphoreHandle_t _taskSemaphore = nullptr;

        static ESCOperation<PWMPulseWidth> _armOp;
        static ESCOperation<PWMPulseWidth> _disarmOp;
    };

    static constexpr uint32_t kInteruptPriority = 3;
    static constexpr uint32_t kArmSpeed = 2;

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    ESCOperation<PWMPulseWidth> ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_armOp =
        ESCOperation<PWMPulseWidth>("Arm", {
                                               {0_s / kArmSpeed, 0},
                                               {1_s / kArmSpeed, 0},
                                               {1_s / kArmSpeed, minThrottlePWM / 2},
                                               {2_s / kArmSpeed, (minThrottlePWM + maxThrottlePWM) / 2},
                                               {3_s / kArmSpeed, minThrottlePWM / 2},
                                               {4_s / kArmSpeed, minThrottlePWM / 2},
                                           });

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    ESCOperation<PWMPulseWidth> ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_disarmOp = ESCOperation<PWMPulseWidth>("Disarm", {
                                                                                                                                           {0_s / kArmSpeed, 0},
                                                                                                                                           {1_s / kArmSpeed, 0},
                                                                                                                                       });

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    bool _timerFull(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t* edata, void* user_ctx);

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    bool _timerStopped(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t* edata, void* user_ctx);

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void _updateThrottleTask(void* userInfo);

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::ESCControlSchemePWM() {
        _setupTimers();
        _taskSemaphore = xQueueGenericCreate((UBaseType_t)1, semSEMAPHORE_QUEUE_ITEM_LENGTH, queueQUEUE_TYPE_BINARY_SEMAPHORE);
        BaseType_t err = xTaskCreate(_updateThrottleTask<minThrottlePWM, maxThrottlePWM>, "Throttle Update Task", 8192, this, 10, &_updateTask);
        if (err != pdPASS) {
            PCP_LOGE("Motor task creation failed: %s", freeRTOSErrorString(err));
        }
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::~ESCControlSchemePWM() {
        vTaskDelete(_updateTask);
        vSemaphoreDelete(_taskSemaphore);

        mcpwm_timer_disable(_timerHandle);
        mcpwm_del_generator(_generatorHandle);
        mcpwm_del_comparator(_comparatorHandle);
        mcpwm_del_operator(_operatorHandle);
        mcpwm_del_timer(_timerHandle);
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    std::optional<MsTime> ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_updateThrottle() {
        if (_comparatorHandle == nullptr) {
            return std::optional<MsTime>();
        }

        do {
            if (_operationQueue.empty()) {
                return std::optional<MsTime>();
            }
            const ESCOperation<PWMPulseWidth>& operation = _operationQueue.front().first;
            if (operation.empty()) {
                _operationQueue.pop_front();
            } else if (operation._speeds.back().first < _time) {
                _setThrottlePWM(operation._speeds.back().second);

                const Completion& completion = _operationQueue.front().second;
                if (completion) {
                    completion();
                }

                _time = 0;
                _operationQueue.pop_front();
            } else {
                break;
            }
        } while (true);

        const ESCOperation<PWMPulseWidth>& operation = _operationQueue.front().first;
        MsTime timeToNextUpdate;
        const PWMPulseWidth speed = operation.at(_time, timeToNextUpdate);

        _setThrottlePWM(speed);

        assert(timeToNextUpdate < 60'000_ms && "We shouldn't be asking to wait a whole minute, that's madness.");

        return timeToNextUpdate;
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_setThrottlePWM(PWMPulseWidth throttlePWM) {
        _throttlePWM = std::clamp(throttlePWM, (PWMPulseWidth)0, (PWMPulseWidth)maxThrottlePWM);
        esp_err_t err = mcpwm_comparator_set_compare_value(_comparatorHandle, _throttlePWM);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while updating comparator value: %s", esp_err_to_name(err));
        }

        if (isArmed()) {
            _state = _throttlePWM <= minThrottlePWM ? ESCState::Armed : ESCState::Running;
        }
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    uint8_t ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::throttle() const {
        return _throttleForPWM(_throttlePWM);
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    std::string ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::stateString(void) const {
        switch (escState()) {
            case ESCState::Disarmed: return "Disarmed";
            case ESCState::Arming: return "Arming...";
            case ESCState::Armed:  // fallthrough
            case ESCState::Running: return std::to_string(throttle()) + "%";
            case ESCState::Disarming: return "Disarming...";
        }
        assert(false && "Unhandled ESC state");
        return "";
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    uint8_t ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_throttleForPWM(PWMPulseWidth pwm) const {
        return invLerpPercentage(pwm, minThrottlePWM, maxThrottlePWM);
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    PWMPulseWidth ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_pwmForThrottle(uint8_t throttle) const {
        return lerpPercentage(minThrottlePWM, maxThrottlePWM, throttle);
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_setupTimers(void) {
        esp_err_t err = ESP_OK;

        mcpwm_timer_config_t timerConfig = {.group_id = 0,
                                            .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
                                            .resolution_hz = 1'000'000,
                                            .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
                                            .period_ticks = 20'000,
                                            .intr_priority = kInteruptPriority,
                                            .flags = {
                                                .update_period_on_empty = false,
                                                .update_period_on_sync = false,
                                                .allow_pd = false,
                                            }};
        err = mcpwm_new_timer(&timerConfig, &_timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating timer: %s", esp_err_to_name(err));
            return;
        }

        mcpwm_operator_config_t operatorConfig = {.group_id = 0,
                                                  .intr_priority = kInteruptPriority,
                                                  .flags = {
                                                      .update_gen_action_on_tez = false,
                                                      .update_gen_action_on_tep = false,
                                                      .update_gen_action_on_sync = false,
                                                      .update_dead_time_on_tez = false,
                                                      .update_dead_time_on_tep = false,
                                                      .update_dead_time_on_sync = false,
                                                  }};
        err = mcpwm_new_operator(&operatorConfig, &_operatorHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating operator: %s", esp_err_to_name(err));
            return;
        }
        err = mcpwm_operator_connect_timer(_operatorHandle, _timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while attaching operator to timer: %s", esp_err_to_name(err));
            return;
        }

        mcpwm_timer_event_callbacks_t timerCallbacks = {
            .on_full = _timerFull<minThrottlePWM, maxThrottlePWM>,
            .on_empty = nullptr,
            .on_stop = _timerStopped<minThrottlePWM, maxThrottlePWM>,
        };
        err = mcpwm_timer_register_event_callbacks(_timerHandle, &timerCallbacks, this);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while setting up timer callbacks: %s", esp_err_to_name(err));
            return;
        }

        mcpwm_comparator_config_t comparatorConfig = {.intr_priority = kInteruptPriority,
                                                      .flags{
                                                          .update_cmp_on_tez = false,
                                                          .update_cmp_on_tep = false,
                                                          .update_cmp_on_sync = false,
                                                      }};
        err = mcpwm_new_comparator(_operatorHandle, &comparatorConfig, &_comparatorHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating comparator: %s", esp_err_to_name(err));
            return;
        }
        err = mcpwm_comparator_set_compare_value(_comparatorHandle, 0);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while setting comparator value: %s", esp_err_to_name(err));
            return;
        }

        mcpwm_generator_config_t generatorCongif = {.gen_gpio_num = kMotorOutputGPIO,
                                                    .flags = {
                                                        .invert_pwm = false,
                                                        .io_loop_back = false,
                                                        .io_od_mode = false,
                                                        .pull_up = false,
                                                        .pull_down = true,
                                                    }};
        err = mcpwm_new_generator(_operatorHandle, &generatorCongif, &_generatorHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating generator: %s", esp_err_to_name(err));
            return;
        }
        err = mcpwm_generator_set_action_on_timer_event(
            _generatorHandle,
            (mcpwm_gen_timer_event_action_t){.direction = MCPWM_TIMER_DIRECTION_UP, .event = MCPWM_TIMER_EVENT_EMPTY, .action = MCPWM_GEN_ACTION_HIGH});
        if (err != ESP_OK) {
            PCP_LOGE("While setting generator to output high on timer reset: %s", esp_err_to_name(err));
            return;
        }
        err = mcpwm_generator_set_action_on_compare_event(
            _generatorHandle,
            (mcpwm_gen_compare_event_action_t){.direction = MCPWM_TIMER_DIRECTION_UP, .comparator = _comparatorHandle, .action = MCPWM_GEN_ACTION_LOW});
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while setting generator to output low on comparator match: %s", esp_err_to_name(err));
            return;
        }

        err = mcpwm_timer_enable(_timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while enabling timer: %s", esp_err_to_name(err));
            return;
        }
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    bool ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::isArmed(void) const {
        return _state == ESCState::Armed || _state == ESCState::Running;
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::arm(Completion completion) {
        assert(_state == ESCState::Disarmed);

        esp_err_t err = ESP_OK;
        err = mcpwm_timer_start_stop(_timerHandle, MCPWM_TIMER_START_NO_STOP);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while starting timer: %s", esp_err_to_name(err));
            return;
        }

        _state = ESCState::Arming;
        _runOperation(_armOp, [this, completion]() {
            this->_state = this->_throttlePWM <= minThrottlePWM ? ESCState::Armed : ESCState::Running;
            completion();
        });
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::disarm(Completion completion) {
        _state = ESCState::Disarming;
        _runOperation(_disarmOp, [this, completion]() {
            this->_state = ESCState::Disarmed;
            esp_err_t err = ESP_OK;
            err = mcpwm_timer_start_stop(_timerHandle, MCPWM_TIMER_STOP_EMPTY);
            if (err != ESP_OK) {
                PCP_LOGE("Error occurred while stopping timer: %s", esp_err_to_name(err));
                return;
            }
            completion();
        });
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    ESCOperation<PWMPulseWidth> ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_changeThrottleOp(int8_t percentage, MsTime duration) const {
        const uint32_t lastPWM = std::clamp(_lastQueuedPWM(), minThrottlePWM, maxThrottlePWM);
        uint32_t newPWM = (uint32_t)(lastPWM + ((int32_t)percentage * (int32_t)(maxThrottlePWM - minThrottlePWM)) / 100);
        return ESCOperation<PWMPulseWidth>("Change Throttle", {{0, lastPWM}, {duration, newPWM}});
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::setThrottle(int8_t percentage, MsTime duration) {
        _runOperation(_changeThrottleOp(percentage - _lastQueuedThrottle(), duration), []() {});
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::decreaseThrottle(int8_t percentage, MsTime duration) {
        _runOperation(_changeThrottleOp(-percentage, duration), []() {});
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::increaseThrottle(int8_t percentage, MsTime duration) {
        _runOperation(_changeThrottleOp(percentage, duration), []() {});
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_runOperation(const ESCOperation<PWMPulseWidth>& op, Completion completion) {
        bool isEmpty = _operationQueue.empty();
        _operationQueue.emplace_back(op, completion);
        if (isEmpty) {
            _time = 0;
            xSemaphoreGive(_taskSemaphore);
        }
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_timeAdvance(uint32_t deltaT) {
        _time += deltaT;
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    PWMPulseWidth ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_lastQueuedPWM(void) const {
        for (auto iter = _operationQueue.rbegin(); iter != _operationQueue.rend(); ++iter) {
            const ESCOperation<PWMPulseWidth>& op = iter->first;
            if (!op._speeds.empty()) {
                return op._speeds.back().second;
            }
        }
        return _throttlePWM;
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    uint8_t ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>::_lastQueuedThrottle(void) const {
        return _throttleForPWM(_lastQueuedPWM());
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    bool _timerFull(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t* edata, void* user_ctx) {
        ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>* motor = reinterpret_cast<ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>*>(user_ctx);
        motor->_timeAdvance(20);
        return false;
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    bool _timerStopped(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t* edata, void* user_ctx) {
        PCP_LOGW("Timer Stopped!");
        return false;
    }

    template <PWMPulseWidth minThrottlePWM, PWMPulseWidth maxThrottlePWM>
    void _updateThrottleTask(void* userInfo) {
        ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>* motor = reinterpret_cast<ESCControlSchemePWM<minThrottlePWM, maxThrottlePWM>*>(userInfo);

        while (!xSemaphoreTake(motor->_taskSemaphore, portMAX_DELAY)) {}

        while (true) {
            std::optional<MsTime> time = motor->_updateThrottle();
            if (time.has_value()) {
                vTaskDelay(time.value().get() / portTICK_PERIOD_MS);
            } else {
                while (!xSemaphoreTake(motor->_taskSemaphore, portMAX_DELAY)) {}
            }
        }
    }
}  // namespace pcp
