#include "ESC/BLHeli/BLHeliESC.hpp"

namespace pcp {
    ESCState BLHeliESC::escState(void) const {
        switch (_state) {
            case BLHeliESCState::IdleFirstStart:
                return ESCState::Disarmed;
            case BLHeliESCState::Idle:
                return ESCState::Disarmed;
            case BLHeliESCState::InPWMControlScheme:
                return _pwmControlScheme->escState();
            case BLHeliESCState::InBootloaderUARTScheme:
                return _uartControlScheme->escState();
        }
        assert(false && "BLHeliESCState case not handled in switch");
        return ESCState::Disarmed;
    }

    void BLHeliESC::arm(Completion completion) {
        assert(_state == BLHeliESCState::IdleFirstStart || _state == BLHeliESCState::Idle);

        this->_state = BLHeliESCState::InPWMControlScheme;
        _pwmControlScheme = std::make_unique<ESCControlSchemePWM<1000u, 2000u>>();
        _pwmControlScheme->arm(completion);
    }

    void BLHeliESC::disarm(Completion completion) {
        assert(_state == BLHeliESCState::InPWMControlScheme);

        _pwmControlScheme->disarm([this]() {
            this->_pwmControlScheme = nullptr;
            _state = BLHeliESCState::Idle;
        });
    }

    void BLHeliESC::setThrottle(uint8_t percentage, MsTime duration) {
        assert(_state == BLHeliESCState::InPWMControlScheme);

        _pwmControlScheme->setThrottle(percentage, duration);
    }

    void BLHeliESC::decreaseThrottle(int8_t percentage, MsTime duration) {
        assert(_state == BLHeliESCState::InPWMControlScheme);

        _pwmControlScheme->decreaseThrottle(percentage, duration);
    }

    void BLHeliESC::increaseThrottle(int8_t percentage, MsTime duration) {
        assert(_state == BLHeliESCState::InPWMControlScheme);

        _pwmControlScheme->increaseThrottle(percentage, duration);
    }

    std::optional<uint8_t> BLHeliESC::throttle() const {
        switch (_state) {
            case BLHeliESCState::IdleFirstStart:
                return std::optional<uint8_t>();
            case BLHeliESCState::Idle:
                return std::optional<uint8_t>();
            case BLHeliESCState::InPWMControlScheme:
                return _pwmControlScheme->throttle();
            case BLHeliESCState::InBootloaderUARTScheme:
                return std::optional<uint8_t>();
        }

        assert(false && "Unhandled BLHeliESCState in switch");
        return std::optional<uint8_t>();
    }

    void BLHeliESC::enterProgrammingMode(std::function<void(std::optional<BLHeliESCConfig>)> completion) {
        assert(_state == BLHeliESCState::IdleFirstStart);

        _state = BLHeliESCState::InBootloaderUARTScheme;
        _uartControlScheme = std::make_unique<BLHeliControlSchemeUART>();
        _uartControlScheme->connect([this, completion](bool success) { completion(escConfig()); });
    }

    std::optional<BLHeliESCConfig> BLHeliESC::escConfig(void) {
        return (_uartControlScheme == nullptr) ? std::optional<BLHeliESCConfig>() : _uartControlScheme->escConfig();
    }
}  // namespace pcp
