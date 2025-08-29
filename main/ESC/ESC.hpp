#pragma once

#include "Utilities/MsTime.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace pcp {
    using Completion = std::function<void(void)>;

    enum class ESCState : uint8_t {
        Disarmed = 0,
        Arming = 1,
        Armed = 2,
        Running = 3,
        Disarming = 4,
        EnteringProgrammingMode = 5,
        Programming = 6,
        ExitingProgrammingMode = 7,
    };

    inline std::string to_string(ESCState state) {
        switch (state) {
            case ESCState::Disarmed: return "Disarmed";
            case ESCState::Arming: return "Arming";
            case ESCState::Armed: return "Armed";
            case ESCState::Running: return "Running";
            case ESCState::Disarming: return "Disarming";
            case ESCState::EnteringProgrammingMode: return "EnteringProgrammingMode";
            case ESCState::Programming: return "Programming";
            case ESCState::ExitingProgrammingMode: return "ExitingProgrammingMode";
        }
    }

    class ESC {
    public:
        virtual bool isArmed(void) const {
            const ESCState state = escState();
            return state == ESCState::Armed || state == ESCState::Running;
        }

        virtual ESCState escState(void) const = 0;

        virtual void arm(Completion completion = []() {}) = 0;
        virtual void disarm(Completion completion = []() {}) = 0;
        virtual void setThrottle(uint8_t percentage, MsTime duration = 0_ms) = 0;
        virtual void decreaseThrottle(int8_t percentage, MsTime duration) = 0;
        virtual void increaseThrottle(int8_t percentage, MsTime duration) = 0;
        virtual std::optional<uint8_t> throttle() const = 0;

        virtual std::string stateString(void) const {
            switch (escState()) {
                case ESCState::Disarmed: return "Disarmed";
                case ESCState::Arming: return "Arming...";
                case ESCState::Armed: return "Armed";
                case ESCState::Running: assert(throttle().has_value()); return std::to_string(throttle().value()) + "%";
                case ESCState::Disarming: return "Disarming...";
                case ESCState::EnteringProgrammingMode: return "Programming...";
                case ESCState::Programming: return "Programming...";
                case ESCState::ExitingProgrammingMode: return "Programming...";
            }

            assert(false && "Unhandled ESCState in switch");
            return "";
        }
    };
}  // namespace pcp
