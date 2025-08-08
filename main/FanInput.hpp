#include "driver/mcpwm_cap.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include <limits>

namespace pcp {
    class FanInput {
        public:
            FanInput();
            ~FanInput();

            void start(void);
            void stop(void);

            uint8_t dutyCyclePercentage(void);

        private:
            bool _capture(mcpwm_cap_channel_handle_t captureChannel, const mcpwm_capture_event_data_t* eventData);
            void _timerFired();
            void _sendDutyCyclePercentage(uint8_t percentage);

            mcpwm_cap_timer_handle_t _captureTimer;
            mcpwm_cap_channel_handle_t _captureChannel;
            uint64_t _lastCapture = 0;
            esp_timer_handle_t _timer;
            bool _timerStarted = false;
            uint32_t _numRuns = 0;

            uint32_t _lastDescendingValue = std::numeric_limits<uint32_t>::max();
            uint32_t _lastAscendingValue = std::numeric_limits<uint32_t>::max();
            uint8_t _dutyCyclePercentage = 0;

            friend bool capture(mcpwm_cap_channel_handle_t cap_channel, const mcpwm_capture_event_data_t* edata, void* user_ctx);
            friend void timerFired(void* userData);
    };
}