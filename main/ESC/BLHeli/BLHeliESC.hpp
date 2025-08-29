#pragma once

#include "ESC/ESC.hpp"

#include "ESC/BLHeli/BLHeliControlSchemeUART.hpp"
#include "ESC/ESCControlSchemePWM.hpp"

#include <memory>

namespace pcp {
    enum class BLHeliESCState {
        IdleFirstStart = 0,
        Idle = 1,
        InPWMControlScheme = 2,
        InBootloaderUARTScheme = 3,
    };

    class BLHeliESC : public ESC {
    public:
        virtual ESCState escState(void) const override;

        virtual void arm(Completion completion = []() {}) override;
        virtual void disarm(Completion completion = []() {}) override;
        virtual void setThrottle(uint8_t percentage, MsTime duration = 0_ms) override;
        virtual void decreaseThrottle(int8_t percentage, MsTime duration) override;
        virtual void increaseThrottle(int8_t percentage, MsTime duration) override;
        virtual std::optional<uint8_t> throttle() const override;

        void enterProgrammingMode(std::function<void(std::optional<BLHeliESCConfig>)> completion);
        std::optional<BLHeliESCConfig> escConfig(void);

    private:
        BLHeliESCState _state = BLHeliESCState::IdleFirstStart;
        std::unique_ptr<ESCControlSchemePWM<1000u, 2000u>> _pwmControlScheme;
        std::unique_ptr<BLHeliControlSchemeUART> _uartControlScheme;
    };
}  // namespace pcp
