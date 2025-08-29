#include "BLHeliControlSchemeUART.hpp"

#include "Log.hpp"
#include "Pins.hpp"
#include "Utilities/CRC.hpp"
#include "Utilities/freeRTOSErrorString.hpp"

#include "driver/gpio.h"

#include "esp_crc.h"
#include "esp_intr_alloc.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <arpa/inet.h>

#include <array>
#include <format>
#include <tuple>

namespace pcp {
    static constexpr size_t kReadBufferSize = 512;

    static constexpr uint32_t kInteruptPriority = 3;

    struct UserData {
        BLHeliControlSchemeUART* uartController;
        std::function<void(bool)> completion;
    };

    void _uartTaskF(void* userInfo) {
        BLHeliControlSchemeUART* uartController = reinterpret_cast<BLHeliControlSchemeUART*>(userInfo);
        uartController->_task();
    }

    bool _timerEmpty(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t* edata, void* userInfo) {
        BLHeliControlSchemeUART* uartController = reinterpret_cast<BLHeliControlSchemeUART*>(userInfo);
        uartController->_configureGeneratorForPulse();
        return true;
    }

    bool _timerStopped(mcpwm_timer_handle_t timer, const mcpwm_timer_event_data_t* edata, void* userInfo) {
        BLHeliControlSchemeUART* uartController = reinterpret_cast<BLHeliControlSchemeUART*>(userInfo);
        uartController->_preambleDidEnd();
        return true;
    }

    BLHeliControlSchemeUART::BLHeliControlSchemeUART() {
        gpio_config_t gpioConfig = {
            .pin_bit_mask = 0x1 << kMotorOutputGPIO,
            .mode = GPIO_MODE_INPUT_OUTPUT_OD,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&gpioConfig);

        _taskSemaphore = xSemaphoreCreateBinary();
        BaseType_t err = xTaskCreate(_uartTaskF, "ESC UART", 16384, this, 10, &_uartTask);
        if (err != pdPASS) {
            PCP_LOGE("Motor task creation failed: %s", freeRTOSErrorString(err));
        }
    }

    BLHeliControlSchemeUART::~BLHeliControlSchemeUART() {
        vTaskDelete(_uartTask);

        if (uart_is_driver_installed(kMotorUART)) {
            uart_driver_delete(kMotorUART);
        }
        if (_timerHandle != nullptr) {
            _cleanupTimer();
        }
    }

    void BLHeliControlSchemeUART::connect(std::function<void(bool)> completion) {
        PCP_LOGD("Connecting to ESC");

        _programModeEntryStep = ProgramModeEntryStep::ReadyForRebootSequence;
        _escState = ESCState::EnteringProgrammingMode;

        _connectionCompletions.push_back(completion);
        xSemaphoreGive(_taskSemaphore);
    }

    void BLHeliControlSchemeUART::_task(void) {
        while (true) {
            while (xSemaphoreTake(_taskSemaphore, portMAX_DELAY) != pdTRUE) {}

            switch (_escState) {
                case ESCState::Programming: {
                    for (Completion& completion : _connectionCompletions) {
                        completion(true);
                    }
                    _connectionCompletions.clear();
                    continue;
                }
                case ESCState::EnteringProgrammingMode:
                    switch (_programModeEntryStep) {
                        case ProgramModeEntryStep::ReadyForRebootSequence:
                            _attemptConnection();
                            break;
                        case ProgramModeEntryStep::ReadyForUART:
                            _beginUART();
                            break;
                        default:
                            break;
                    }
                    continue;
                default:
                    continue;
            }
        }
    }

    void BLHeliControlSchemeUART::_attemptConnection() {
        _programModeEntryStep = ProgramModeEntryStep::RebootingESC;

        bool success = _setupTimers();
        if (success) {
            _transmitPreamble();
        }
    }

    bool BLHeliControlSchemeUART::_setupTimers(void) {
        esp_err_t err = ESP_OK;

        mcpwm_timer_config_t timerConfig = {.group_id = 0,
                                            .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
                                            .resolution_hz = 1'000'000,
                                            .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
                                            .period_ticks = 64,
                                            .intr_priority = kInteruptPriority,
                                            .flags = {
                                                .update_period_on_empty = false,
                                                .update_period_on_sync = false,
                                                .allow_pd = false,
                                            }};
        err = mcpwm_new_timer(&timerConfig, &_timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating timer: %s", esp_err_to_name(err));
            _retryConnection();
            return false;
        }

        mcpwm_operator_config_t operatorConfig = {.group_id = 0,
                                                  .intr_priority = kInteruptPriority,
                                                  .flags = {
                                                      .update_gen_action_on_tez = true,
                                                      .update_gen_action_on_tep = true,
                                                      .update_gen_action_on_sync = false,
                                                      .update_dead_time_on_tez = false,
                                                      .update_dead_time_on_tep = false,
                                                      .update_dead_time_on_sync = false,
                                                  }};
        err = mcpwm_new_operator(&operatorConfig, &_operatorHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating operator: %s", esp_err_to_name(err));
            _retryConnection();
            return false;
        }
        err = mcpwm_operator_connect_timer(_operatorHandle, _timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while attaching operator to timer: %s", esp_err_to_name(err));
            _retryConnection();
            return false;
        }

        mcpwm_timer_event_callbacks_t timerCallbacks = {
            .on_full = nullptr,
            .on_empty = _timerEmpty,
            .on_stop = _timerStopped,
        };
        err = mcpwm_timer_register_event_callbacks(_timerHandle, &timerCallbacks, this);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while setting up timer callbacks: %s", esp_err_to_name(err));
            _retryConnection();
            return false;
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
            _retryConnection();
            return false;
        }
        err = mcpwm_comparator_set_compare_value(_comparatorHandle, 32);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while setting comparator value: %s", esp_err_to_name(err));
            _retryConnection();
            return false;
        }

        mcpwm_generator_config_t generatorConfig = {.gen_gpio_num = kMotorOutputGPIO,
                                                    .flags = {
                                                        .invert_pwm = false,
                                                        .io_loop_back = false,
                                                        .io_od_mode = true,
                                                        .pull_up = true,
                                                        .pull_down = false,
                                                    }};
        err = mcpwm_new_generator(_operatorHandle, &generatorConfig, &_generatorHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while creating generator: %s", esp_err_to_name(err));
            _retryConnection();
            return false;
        }
        err = mcpwm_generator_set_force_level(_generatorHandle, 1, false);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while forcing generator to initialy output high: %s", esp_err_to_name(err));
            _retryConnection();
            return false;
        }

        return true;
    }

    void BLHeliControlSchemeUART::_cleanupTimer(void) {
        if (_timerHandle != nullptr) {
            mcpwm_timer_start_stop(_timerHandle, MCPWM_TIMER_STOP_EMPTY);
            mcpwm_timer_disable(_timerHandle);
        }

        if (_generatorHandle != nullptr) {
            mcpwm_del_generator(_generatorHandle);
            _generatorHandle = nullptr;
        }
        if (_comparatorHandle != nullptr) {
            mcpwm_del_comparator(_comparatorHandle);
            _comparatorHandle = nullptr;
        }
        if (_operatorHandle != nullptr) {
            mcpwm_del_operator(_operatorHandle);
            _operatorHandle = nullptr;
        }
        if (_timerHandle != nullptr) {
            mcpwm_del_timer(_timerHandle);
            _timerHandle = nullptr;
        }
    }

    enum class PulseWidth : uint8_t { Short = 0, Long = 1 };

    // What even is this shit?  I can't figure out what protocol is being used here, but somehow this gets you
    // into the bootloader.
    static constexpr std::array<PulseWidth, 238> _preamblePulseTiming(
        {PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,
         PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,
         PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,
         PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,
         PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,
         PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,
         PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Long,
         PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Long,
         PulseWidth::Long,  PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Long,  PulseWidth::Long,  PulseWidth::Short, PulseWidth::Short, PulseWidth::Short,
         PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short, PulseWidth::Short});

    bool BLHeliControlSchemeUART::_configureGeneratorForPulse() {
        esp_err_t err = ESP_OK;

        if (_preamblePulseNumber < _preamblePulseTiming.size()) {
            const bool pulseStartsHigh = _preamblePulseNumber % 2 == 0;
            const mcpwm_generator_action_t firstAction = pulseStartsHigh ? MCPWM_GEN_ACTION_LOW : MCPWM_GEN_ACTION_HIGH;
            err = mcpwm_generator_set_action_on_timer_event(
                _generatorHandle,
                (mcpwm_gen_timer_event_action_t){.direction = MCPWM_TIMER_DIRECTION_UP, .event = MCPWM_TIMER_EVENT_EMPTY, .action = firstAction});
            if (err != ESP_OK) {
                PCP_LOGE("While setting generator to output on timer reset: %s", esp_err_to_name(err));
                _retryConnection();
                return false;
            }
            const PulseWidth pulseWidth = _preamblePulseTiming[_preamblePulseNumber];
            if (pulseWidth == PulseWidth::Short) {
                assert(_preamblePulseTiming[_preamblePulseNumber + 1] == PulseWidth::Short);
                _preamblePulseNumber += 2;
                err = mcpwm_generator_set_action_on_compare_event(
                    _generatorHandle, (mcpwm_gen_compare_event_action_t){
                                          .direction = MCPWM_TIMER_DIRECTION_UP, .comparator = _comparatorHandle, .action = MCPWM_GEN_ACTION_TOGGLE});
                if (err != ESP_OK) {
                    PCP_LOGE("Error occurred while setting generator to output invert on comparator match: %s", esp_err_to_name(err));
                    _retryConnection();
                    return false;
                }
            } else {
                _preamblePulseNumber++;
                err = mcpwm_generator_set_action_on_compare_event(
                    _generatorHandle, (mcpwm_gen_compare_event_action_t){
                                          .direction = MCPWM_TIMER_DIRECTION_UP, .comparator = _comparatorHandle, .action = MCPWM_GEN_ACTION_KEEP});
                if (err != ESP_OK) {
                    PCP_LOGE("Error occurred while setting generator to output equal on comparator match: %s", esp_err_to_name(err));
                    _retryConnection();
                    return false;
                }
            }
        } else if (!_timerStopping) {
            // If this is the last iteration, add an extra action - when the timer reaches the top
            // set the output high, and then stop.  _preambleDidEnd() will then get called, which
            // will disable the timer and start the uart communication.
            _timerStopping = true;
            err = mcpwm_generator_set_action_on_timer_event(
                _generatorHandle,
                (mcpwm_gen_timer_event_action_t){.direction = MCPWM_TIMER_DIRECTION_UP, .event = MCPWM_TIMER_EVENT_FULL, .action = MCPWM_GEN_ACTION_HIGH});
            if (err != ESP_OK) {
                PCP_LOGE("While setting generator to output on timer reset: %s", esp_err_to_name(err));
                _retryConnection();
                return false;
            }
            err = mcpwm_timer_start_stop(_timerHandle, MCPWM_TIMER_STOP_EMPTY);
            if (err != ESP_OK) {
                PCP_LOGE("While stopping timer at end of preamble pulse sequence: %s", esp_err_to_name(err));
                _retryConnection();
                return false;
            }
        }

        return true;
    }

    void BLHeliControlSchemeUART::_transmitPreamble() {
        PCP_LOGD("Transmitting preamble");

        esp_err_t err = ESP_OK;

        _preamblePulseNumber = 0;
        _timerStopping = false;
        bool success = _configureGeneratorForPulse();
        if (!success) {
            return;
        }

        err = mcpwm_timer_enable(_timerHandle);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while enabling timer: %s", esp_err_to_name(err));
            _retryConnection();
            return;
        }
        err = mcpwm_timer_start_stop(_timerHandle, MCPWM_TIMER_START_NO_STOP);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while starting timer: %s", esp_err_to_name(err));
            _retryConnection();
            return;
        }
    }

    void BLHeliControlSchemeUART::_preambleDidEnd(void) {
        esp_err_t err = ESP_OK;
        err = mcpwm_generator_set_force_level(_generatorHandle, 1, false);
        if (err != ESP_OK) {
            PCP_LOGE("Error occurred while forcing generator to initialy output high: %s", esp_err_to_name(err));
            _retryConnection();
            return;
        }

        _programModeEntryStep = ProgramModeEntryStep::ReadyForUART;
        xSemaphoreGive(_taskSemaphore);
    }

    void BLHeliControlSchemeUART::_beginUART(void) {
        _programModeEntryStep = ProgramModeEntryStep::BeginningUART;

        PCP_LOGD("Opening UART connection to ESC");

        _cleanupTimer();

        esp_err_t err = ESP_OK;

        static constexpr size_t kUartBufferSize = 2 * 1024;
        static constexpr size_t kUartQueueSize = 8;
        err = uart_driver_install(kMotorUART, kUartBufferSize, kUartBufferSize, kUartQueueSize, &_uartQueue, ESP_INTR_FLAG_LEVEL3);
        if (err != ESP_OK) {
            PCP_LOGE("Error installing uart driver: %s", esp_err_to_name(err));
            _retryConnection();
            return;
        }
        uart_config_t uartConfig = {.baud_rate = 19200,
                                    .data_bits = UART_DATA_8_BITS,
                                    .parity = UART_PARITY_DISABLE,
                                    .stop_bits = UART_STOP_BITS_1,
                                    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                                    .rx_flow_ctrl_thresh = 0,
                                    .source_clk = UART_SCLK_DEFAULT,
                                    .flags = {
                                        .allow_pd = false,
                                        .backup_before_sleep = false,
                                    }};
        err = uart_param_config(kMotorUART, &uartConfig);
        if (err != ESP_OK) {
            PCP_LOGE("Error configuring UART: %s", esp_err_to_name(err));
            _retryConnection();
            return;
        }

        // If we're using the same pin for transmit and receive we must configure the gpio to have an open drain and a pullup.  This stops us from burning out the pin.
        gpio_config_t gpioConfig = {
            .pin_bit_mask = 0x1 << kMotorOutputGPIO,
            .mode = GPIO_MODE_INPUT_OUTPUT_OD,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&gpioConfig);

        err = uart_set_pin(kMotorUART, kMotorOutputGPIO, kMotorInputGPIO, -1, -1);
        if (err != ESP_OK) {
            PCP_LOGE("Error configuring one wire UART: %s", esp_err_to_name(err));
            _retryConnection();
            return;
        }

        const uint8_t preamble[] = {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\r'};
        const uint8_t handshake[] = {'B', 'L', 'H', 'e', 'l', 'i'};
        const size_t kExpectedResponseLength = 8;
        const size_t kPreambleLength = sizeof(preamble) / sizeof(preamble[0]);
        const size_t kHandshakeLength = sizeof(handshake) / sizeof(handshake[0]);
        size_t transmittedBytes = _writeBytes(preamble, kPreambleLength, false);
        transmittedBytes += _writeBytes(handshake, kHandshakeLength, true);

        const size_t kReadBufferLength = 256;
        uint8_t readBuffer[kReadBufferLength];
        size_t bytesRead = _readBytes(readBuffer, kReadBufferLength - 1, 100 / portTICK_PERIOD_MS);
        readBuffer[bytesRead] = '\0';

        if (bytesRead < kPreambleLength + kHandshakeLength + kExpectedResponseLength) {
            PCP_LOGE("Did not get expected response from ESC after handshake");
            _retryConnection();
            return;
        }

        BootloaderResult<BLHeliESCConfig> device = _getDeviceConfig(readBuffer);
        if (!device) {
            PCP_LOGE("Device config does not look like anything I understand");
            _retryConnection();
            return;
        }

        _esc = device.data;
        _numRetries = 0;
        PCP_LOGD("Got device config for: %s", _esc.prettyLayout().c_str());

        _escState = ESCState::Programming;
        _connectionFinished(true);
    }

    void BLHeliControlSchemeUART::_retryConnection(void) {
        const size_t kMaxRetries = 2;

        PCP_LOGW("Connection failed, retrying.");

        if (_numRetries < kMaxRetries) {
            _numRetries++;

            _cleanupTimer();

            if (uart_is_driver_installed(kMotorUART)) {
                uart_driver_delete(kMotorUART);
            }

            gpio_set_level(kMotorOutputGPIO, 1);
            vTaskDelay(200 / portTICK_PERIOD_MS);

            _attemptConnection();
        } else {
            _escState = ESCState::Disarmed;
            _connectionFinished(false);
        }
    }

    void BLHeliControlSchemeUART::_connectionFinished(bool success) {
        for (Completion& completion : _connectionCompletions) {
            completion(true);
        }
        _connectionCompletions.clear();
    }

    BootloaderResult<BLHeliESCConfig> BLHeliControlSchemeUART::_getDeviceConfig(const uint8_t* handshake) {
        static constexpr uint16_t kDeviceLayoutLocation = 0x1a00;
        static constexpr uint16_t kDeviceLayoutLength = 0x0070;

        BootloaderResult<std::vector<uint8_t>> deviceConfigMemory = readMemory(kDeviceLayoutLocation, kDeviceLayoutLength, 1000 / portTICK_PERIOD_MS);
        if (deviceConfigMemory) {
            const std::vector<uint8_t>& deviceConfigBytes = deviceConfigMemory.data;

            BLHeliESCConfig device(handshake, deviceConfigBytes);

            return BootloaderResult<BLHeliESCConfig>(device);
        }
        PCP_LOGE("Could not read memory: %s", std::to_string(deviceConfigMemory).c_str());
        return BootloaderResult<BLHeliESCConfig>(deviceConfigMemory.resultCode);
    }

    BootloaderResult<std::vector<uint8_t>> BLHeliControlSchemeUART::readMemory(uint16_t address, uint8_t length, TickType_t timeout) {
        BootloaderResult<Void> success = _setAddress(address, timeout);
        if (!success) {
            PCP_LOGE("Could not set address: %s", std::to_string(success).c_str());
            return BootloaderResult<std::vector<uint8_t>>(success.resultCode);
        }
        return _readMemory(length, timeout);
    }

    constexpr uint16_t kCRCLength = 2;
    constexpr uint16_t kAckLength = 1;

    constexpr uint16_t ackLocation(uint16_t expectedDataLength) {
        return expectedDataLength == 0 ? 0 : expectedDataLength + kCRCLength;
    }

    BootloaderResultCode ack(const uint8_t* responseData, uint16_t expectedDataLength) {
        return static_cast<BootloaderResultCode>(responseData[ackLocation(expectedDataLength)]);
    }

    uint16_t crc(const uint8_t* responseData, uint16_t expectedDataLength) {
        assert(expectedDataLength != 0);
        return ntohs(*(reinterpret_cast<const uint16_t*>(responseData + expectedDataLength)));
    }

    constexpr uint16_t responseBufferSize(uint16_t expectedDataLength) {
        return ackLocation(expectedDataLength) + kAckLength;
    }

    template <>
    Void BLHeliControlSchemeUART::getReadBytes<Void>(const uint8_t* buffer, size_t length) {
        return Void();
    }

    template <>
    std::vector<uint8_t> BLHeliControlSchemeUART::getReadBytes<std::vector<uint8_t>>(const uint8_t* buffer, size_t length) {
        std::vector<uint8_t> bytes;
        bytes.resize(length);
        memcpy(bytes.data(), buffer, length);
        return bytes;
    }

    template <BootloaderCommandType cmd>
    BootloaderResult<typename BootloaderCommand<cmd>::ReturnType> BLHeliControlSchemeUART::_runCommand(BootloaderCommand<cmd> command, TickType_t timeout) {
        using return_type = BootloaderCommand<cmd>::ReturnType;

        std::vector<uint8_t> message = {to_uint8(cmd), 0x00};
        if constexpr (command.hasCommandData) {
            message[1] = command.commandData;
        }
        if constexpr (command.hasArgument) {
            uint8_t* argumentPtr = reinterpret_cast<uint8_t*>(&command.argument);
            message.push_back(argumentPtr[0]);
            message.push_back(argumentPtr[1]);
        }
        const size_t messageBufferSize = message.size();
        const size_t transmittedBytes = _writeBytes(message.data(), messageBufferSize, true);

        const size_t expectedResponseLength = command.expectedReturnBytes();
        const size_t expectedReadBytes = transmittedBytes + responseBufferSize(expectedResponseLength);
        uint8_t allReadBytes[expectedReadBytes];
        size_t bytesRead = 0;
        const uint16_t startTime = esp_timer_get_time();
        while (bytesRead < expectedReadBytes) {
            bytesRead += _readBytes(allReadBytes + bytesRead, expectedReadBytes - bytesRead, timeout);
            const uint16_t now = esp_timer_get_time();
            if (now - startTime > timeout * portTICK_PERIOD_MS * 1000) {
                break;
            }
        }
        if (bytesRead < expectedReadBytes) {
            return BootloaderResult<return_type>(BootloaderResultCode::ErrorTimeout);
        }
        const uint8_t* response = allReadBytes + transmittedBytes;

        BootloaderResultCode resultCode = ack(response, expectedResponseLength);
        if (resultCode == BootloaderResultCode::Success) {
            return BootloaderResult<return_type>(getReadBytes<return_type>(response, expectedResponseLength));
        } else {
            return BootloaderResult<return_type>(resultCode);
        }
    }

    BootloaderResult<Void> BLHeliControlSchemeUART::_setAddress(uint16_t address, TickType_t timeout) {
        BootloaderCommand<BootloaderCommandType::SetAddress> cmd;
        cmd.argument = htons(address);
        return _runCommand(cmd, timeout);
    }

    BootloaderResult<std::vector<uint8_t>> BLHeliControlSchemeUART::_readMemory(uint8_t length, TickType_t timeout) {
        BootloaderCommand<BootloaderCommandType::ReadFlash> cmd;
        cmd.commandData = length;
        return _runCommand(cmd, timeout);
    }

    size_t BLHeliControlSchemeUART::_writeBytes(const uint8_t* bytes, size_t length, bool writeCRC) {
        uart_write_bytes(kMotorUART, bytes, length);
        size_t transmittedBytes = length;
        if (writeCRC) {
            const uint16_t crc = crc_16_ibm(bytes, length);
            uart_write_bytes(kMotorUART, &crc, sizeof(uint16_t));
            transmittedBytes += 2;
        }
        return transmittedBytes;
    }

    size_t BLHeliControlSchemeUART::_readBytes(uint8_t* bytes, size_t length, TickType_t ticksToWait) {
        return uart_read_bytes(kMotorUART, bytes, length, ticksToWait);
    }
}  // namespace pcp
