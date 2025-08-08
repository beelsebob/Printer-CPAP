#include "FanInput.hpp"

#include "Log.hpp"

#include "Pins.hpp"

namespace pcp {
    static constexpr uint64_t kPWMTimeoutCheckFrequency = 1'000'000 / 30;
    static constexpr uint64_t kPWMTimeout = 1'000'000 / 12'500;

    bool capture(mcpwm_cap_channel_handle_t capChannel, const mcpwm_capture_event_data_t* eventData, void* userData);
    void timerFired(void* userData);

    FanInput::FanInput() {
        mcpwm_capture_timer_config_t captureTimerConfig = {
            .group_id = 1,
            .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT,
            .resolution_hz = 2'500'000,
            .flags = {
                .allow_pd = false,
            }
        };
        esp_err_t err = ESP_OK;
        err = mcpwm_new_capture_timer(&captureTimerConfig, &_captureTimer);
        if (err != ESP_OK) {
            PCP_LOGE("Error creating capture timer: %s", esp_err_to_name(err));
        }

        mcpwm_capture_channel_config_t captureChannelConfig = {
            .gpio_num = kFanInputGPIO,
            .intr_priority = 3,
            .prescale = 1,
            .flags = {
                .pos_edge = true,
                .neg_edge = true,
                .pull_up = false,
                .pull_down = true, 
                .invert_cap_signal = false,
                .io_loop_back = false,
                .keep_io_conf_at_exit = false,
            }
        };
        err = mcpwm_new_capture_channel(_captureTimer, &captureChannelConfig, &_captureChannel);
        if (err != ESP_OK) {
            PCP_LOGE("Error creating capture channel: %s", esp_err_to_name(err));
        }
        mcpwm_capture_event_callbacks_t callbacks = {
            .on_cap = capture
        };
        err = mcpwm_capture_channel_register_event_callbacks(_captureChannel, &callbacks, this);
        if (err != ESP_OK) {
            PCP_LOGE("Error setting capture callbacks: %s", esp_err_to_name(err));
        }
        err = mcpwm_capture_timer_enable(_captureTimer);
        if (err != ESP_OK) {
            PCP_LOGE("Error enabling capture timer: %s", esp_err_to_name(err));
        }
        err = mcpwm_capture_channel_enable(_captureChannel);
        if (err != ESP_OK) {
            PCP_LOGE("Error enabling capture channel: %s", esp_err_to_name(err));
        }
        esp_timer_create_args_t timerConfig = {
            .callback = timerFired,
            .arg = this,
            .dispatch_method = esp_timer_dispatch_t::ESP_TIMER_TASK,
            .name = "PWM Saturation timer",
            .skip_unhandled_events = false,
        };
        err = esp_timer_create(&timerConfig, &_timer);
        if (err != ESP_OK) {
            PCP_LOGE("Error creating timer: %s", esp_err_to_name(err));
        }
    }

    FanInput::~FanInput() {
        esp_timer_delete(_timer);
    }

    void FanInput::start(void) {
        esp_err_t err = ESP_OK;
        err = mcpwm_capture_timer_start(_captureTimer);
        if (err != ESP_OK) {
           PCP_LOGE("Error starting capture: %s", esp_err_to_name(err));
        }
        esp_timer_start_periodic(_timer, kPWMTimeoutCheckFrequency); 
    }

    void FanInput::stop(void) {
        esp_err_t err = ESP_OK;
        err = mcpwm_capture_timer_stop(_captureTimer);
        if (err != ESP_OK) {
            PCP_LOGE("Error stopping capture: %s", esp_err_to_name(err));
        }
        esp_timer_stop(_timer);
    }

    uint8_t FanInput::dutyCyclePercentage(void) {
        if (_numRuns % 1000 == 0) {
            esp_timer_dump(stdout);
        }
        return _dutyCyclePercentage;
    }

    bool FanInput::_capture(mcpwm_cap_channel_handle_t captureChannel, const mcpwm_capture_event_data_t* eventData) {
        switch (eventData->cap_edge) {
            case mcpwm_capture_edge_t::MCPWM_CAP_EDGE_NEG:
                _lastDescendingValue = eventData->cap_value;
                break;
            case mcpwm_capture_edge_t::MCPWM_CAP_EDGE_POS:
                if (_lastAscendingValue != std::numeric_limits<uint32_t>::max() && _lastDescendingValue != std::numeric_limits<uint32_t>::max() && _lastDescendingValue > _lastAscendingValue) {
                    const uint32_t totalTime = eventData->cap_value - _lastAscendingValue;
                    const uint32_t onTime = _lastDescendingValue - _lastAscendingValue;
                    _dutyCyclePercentage = (onTime * 100) / totalTime;
                }
                _lastAscendingValue = eventData->cap_value;
                break;
        }
        _lastCapture = esp_timer_get_time();
        return false;
    }

    void FanInput::_timerFired() {
        _numRuns++;
        if (esp_timer_get_time() > _lastCapture + kPWMTimeout) {
            int level = gpio_get_level(kFanInputGPIO);
            _dutyCyclePercentage = level > 0 ? 100 : 0;
        }
    }

    bool capture(mcpwm_cap_channel_handle_t captureChannel, const mcpwm_capture_event_data_t* eventData, void* userData) {
        FanInput* fanInput = reinterpret_cast<FanInput*>(userData);
        return fanInput->_capture(captureChannel, eventData);
    }

    void timerFired(void* userData) {
        FanInput* fanInput = reinterpret_cast<FanInput*>(userData);
        fanInput->_timerFired();
    }
}
