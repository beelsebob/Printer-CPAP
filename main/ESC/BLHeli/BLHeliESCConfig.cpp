#include "ESC/BLHeli/BLHeliESCConfig.hpp"

#include <string.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace std {
    std::string to_string(const pcp::BLHeliRotorType& rotorType) {
        switch (rotorType) {
            case pcp::BLHeliRotorType::Main:
                return "Main Rotor";
            case pcp::BLHeliRotorType::Tail:
                return "Tail Rotor";
            case pcp::BLHeliRotorType::Multi:
                return "Multi Rotor";
        }
        assert(false);
        return "UNKNOWN";
    }

    std::string to_string(const pcp::BLHeliESCSetting& setting) {
        switch (setting) {
            case pcp::BLHeliESCSetting::FirmwareMajorVersion:
                return "Firmware Major Version";
            case pcp::BLHeliESCSetting::FirmwareMinorVersion:
                return "Firmware Minor Version";
            case pcp::BLHeliESCSetting::LayoutRevision:
                return "Layout Revision";
            case pcp::BLHeliESCSetting::GovernorPGain:
                return "Governor P Gain";
            case pcp::BLHeliESCSetting::GovernorIGain:
                return "Governor I Gain";
            case pcp::BLHeliESCSetting::GovernorMode:
                return "Governor Mode";
            case pcp::BLHeliESCSetting::LowVoltageLimit:
                return "Low Voltage Limit";
            case pcp::BLHeliESCSetting::MotorGain:
                return "Motor Gain";
            case pcp::BLHeliESCSetting::MotorIdle:
                return "Motor Idle";
            case pcp::BLHeliESCSetting::StartupPower:
                return "Startup Power";
            case pcp::BLHeliESCSetting::PwmFrequency:
                return "PWM Frequency";
            case pcp::BLHeliESCSetting::Direction:
                return "Direction";
            case pcp::BLHeliESCSetting::InputPolarity:
                return "Input Polarity";
            case pcp::BLHeliESCSetting::SignatureLow:
                return "Signature Low";
            case pcp::BLHeliESCSetting::SignatureHigh:
                return "Signature High";
            case pcp::BLHeliESCSetting::EnableProgramByTx:
                return "Enable Program By Tx";
            case pcp::BLHeliESCSetting::MainRearmStart:
                return "Main Rearm Start";
            case pcp::BLHeliESCSetting::GovernerSetupTarget:
                return "Governer Setup Target";
            case pcp::BLHeliESCSetting::StartupRpm:
                return "Startup RPM";
            case pcp::BLHeliESCSetting::StartupAcceleration:
                return "Startup Acceleration";
            case pcp::BLHeliESCSetting::VoltageCompensation:
                return "Voltage Compensation";
            case pcp::BLHeliESCSetting::CommutationTiming:
                return "Commutation Timing";
            case pcp::BLHeliESCSetting::DampingForce:
                return "Damping Force";
            case pcp::BLHeliESCSetting::GovernorRange:
                return "Governor Range";
            case pcp::BLHeliESCSetting::StartupMethod:
                return "Startup Method";
            case pcp::BLHeliESCSetting::MinThrottlePpm:
                return "Min Throttle PPM";
            case pcp::BLHeliESCSetting::MaxThrottlePpm:
                return "Max Throttle PPM";
            case pcp::BLHeliESCSetting::BeepStrength:
                return "Beep Strength";
            case pcp::BLHeliESCSetting::BeaconStrength:
                return "Beacon Strength";
            case pcp::BLHeliESCSetting::BeaconDelay:
                return "Beacon Delay";
            case pcp::BLHeliESCSetting::ThrottleRate:
                return "Throttle Rate";
            case pcp::BLHeliESCSetting::DemagCompensation:
                return "Demag Compensation";
            case pcp::BLHeliESCSetting::BECVoltageHigh:
                return "BEC Voltage High";
            case pcp::BLHeliESCSetting::CentreThrottlePpm:
                return "Centre Throttle PPM";
            case pcp::BLHeliESCSetting::MainSpoolupTime:
                return "Main Spoolup Time";
            case pcp::BLHeliESCSetting::TemperatureProtection:
                return "Temperature Protection";
            case pcp::BLHeliESCSetting::EnablePowerProtection:
                return "Enable Power Protection";
            case pcp::BLHeliESCSetting::EnablePwmInput:
                return "Enable PWM Input";
            case pcp::BLHeliESCSetting::EnablePwmDither:
                return "Enable PWM Dither";
            case pcp::BLHeliESCSetting::EnableBrakeOnStop:
                return "Enable Brake On Stop";
            default:
                assert(false);
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliGovernorGain& setting) {
        switch (setting) {
            case pcp::BLHeliGovernorGain::Gain0p13:
                return "0.13x";
            case pcp::BLHeliGovernorGain::Gain0p17:
                return "0.17x";
            case pcp::BLHeliGovernorGain::Gain0p25:
                return "0.25x";
            case pcp::BLHeliGovernorGain::Gain0p38:
                return "0.38x";
            case pcp::BLHeliGovernorGain::Gain0p50:
                return "0.50x";
            case pcp::BLHeliGovernorGain::Gain0p75:
                return "0.75x";
            case pcp::BLHeliGovernorGain::Gain1p00:
                return "1.00x";
            case pcp::BLHeliGovernorGain::Gain1p50:
                return "1.50x";
            case pcp::BLHeliGovernorGain::Gain2p00:
                return "2.00x";
            case pcp::BLHeliGovernorGain::Gain3p00:
                return "3.00x";
            case pcp::BLHeliGovernorGain::Gain4p00:
                return "4.00x";
            case pcp::BLHeliGovernorGain::Gain6p00:
                return "6.00x";
            case pcp::BLHeliGovernorGain::Gain8p00:
                return "8.00x";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingGovernorMode& setting) {
        switch (setting) {
            case pcp::BLHeliSettingGovernorMode::Tx:
                return "Tx";
            case pcp::BLHeliSettingGovernorMode::Arm:
                return "Arm";
            case pcp::BLHeliSettingGovernorMode::Setup:
                return "Setup";
            case pcp::BLHeliSettingGovernorMode::Off:
                return "Off";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingLowVoltageLimit& setting) {
        switch (setting) {
            case pcp::BLHeliSettingLowVoltageLimit::Off:
                return "Off";
            case pcp::BLHeliSettingLowVoltageLimit::VoltPerCell3p0:
                return "3.0V";
            case pcp::BLHeliSettingLowVoltageLimit::VoltPerCell3p1:
                return "3.1V";
            case pcp::BLHeliSettingLowVoltageLimit::VoltPerCell3p2:
                return "3.2V";
            case pcp::BLHeliSettingLowVoltageLimit::VoltPerCell3p3:
                return "3.3V";
            case pcp::BLHeliSettingLowVoltageLimit::VoltPerCell3p4:
                return "3.4V";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingMotorGain& setting) {
        switch (setting) {
            case pcp::BLHeliSettingMotorGain::MotorGain0p75:
                return "0.75x";
            case pcp::BLHeliSettingMotorGain::MotorGain0p88:
                return "0.88x";
            case pcp::BLHeliSettingMotorGain::MotorGain1p00:
                return "1.00x";
            case pcp::BLHeliSettingMotorGain::MotorGain1p12:
                return "1.12x";
            case pcp::BLHeliSettingMotorGain::MotorGain1p25:
                return "1.25x";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingMotorIdle& setting) {
        switch (setting) {
            case pcp::BLHeliSettingMotorIdle::Low:
                return "Low";
            case pcp::BLHeliSettingMotorIdle::MediumLow:
                return "Medium Low";
            case pcp::BLHeliSettingMotorIdle::Medium:
                return "Medium";
            case pcp::BLHeliSettingMotorIdle::MediumHigh:
                return "Medium High";
            case pcp::BLHeliSettingMotorIdle::High:
                return "High";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingStartupPower& setting) {
        switch (setting) {
            case pcp::BLHeliSettingStartupPower::StartupPower0p031:
                return "0.031x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p047:
                return "0.047x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p063:
                return "0.063x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p094:
                return "0.094x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p125:
                return "0.125x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p188:
                return "0.188x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p250:
                return "0.250x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p380:
                return "0.380x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p500:
                return "0.500x";
            case pcp::BLHeliSettingStartupPower::StartupPower0p750:
                return "0.750x";
            case pcp::BLHeliSettingStartupPower::StartupPower1p000:
                return "1.000x";
            case pcp::BLHeliSettingStartupPower::StartupPower1p250:
                return "1.250x";
            case pcp::BLHeliSettingStartupPower::StartupPower1p500:
                return "1.500x";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingPwmFrequency& setting) {
        switch (setting) {
            case pcp::BLHeliSettingPwmFrequency::High:
                return "High";
            case pcp::BLHeliSettingPwmFrequency::Low:
                return "Low";
            case pcp::BLHeliSettingPwmFrequency::DampedLight:
                return "Damped Light";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingDirection& setting) {
        switch (setting) {
            case pcp::BLHeliSettingDirection::Forward:
                return "Forward";
            case pcp::BLHeliSettingDirection::Reverse:
                return "Reverse";
            case pcp::BLHeliSettingDirection::Bidirectional:
                return "Bidirectional";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingInputPolarity& setting) {
        switch (setting) {
            case pcp::BLHeliSettingInputPolarity::Positive:
                return "Positive";
            case pcp::BLHeliSettingInputPolarity::Negative:
                return "Negative";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliBool& setting) {
        switch (setting) {
            case pcp::BLHeliBool::Off:
                return "Off";
            case pcp::BLHeliBool::On:
                return "On";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::PercentageTarget& setting) {
        return std::format("{}%", setting.approximatePercentageTarget());
    }

    std::string to_string(const pcp::BLHeliSettingCommutationTiming& setting) {
        switch (setting) {
            case pcp::BLHeliSettingCommutationTiming::Low:
                return "Low";
            case pcp::BLHeliSettingCommutationTiming::MediumLow:
                return "Medium Low";
            case pcp::BLHeliSettingCommutationTiming::Medium:
                return "Medium";
            case pcp::BLHeliSettingCommutationTiming::MediumHigh:
                return "Medium High";
            case pcp::BLHeliSettingCommutationTiming::High:
                return "High";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingGovernorRange& setting) {
        switch (setting) {
            case pcp::BLHeliSettingGovernorRange::High:
                return "High";
            case pcp::BLHeliSettingGovernorRange::Middle:
                return "Middle";
            case pcp::BLHeliSettingGovernorRange::Low:
                return "Low";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliThrottleValue& setting) {
        return std::to_string(setting.ppm());
    }

    std::string to_string(const pcp::BLHeliSettingBeaconDelay& setting) {
        switch (setting) {
            case pcp::BLHeliSettingBeaconDelay::Minutes1:
                return "1 min";
            case pcp::BLHeliSettingBeaconDelay::Minutes2:
                return "2 min";
            case pcp::BLHeliSettingBeaconDelay::Minutes5:
                return "5 min";
            case pcp::BLHeliSettingBeaconDelay::Minutes10:
                return "10 min";
            case pcp::BLHeliSettingBeaconDelay::NoBeacon:
                return "No Beacon";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingDemagCompensation& setting) {
        switch (setting) {
            case pcp::BLHeliSettingDemagCompensation::Off:
                return "Off";
            case pcp::BLHeliSettingDemagCompensation::Low:
                return "Low";
            case pcp::BLHeliSettingDemagCompensation::High:
                return "High";
            default:
                return "<UNDEFINED>";
        }
    }

    std::string to_string(const pcp::BLHeliSettingPwmDither& setting) {
        switch (setting) {
            case pcp::BLHeliSettingPwmDither::Off:
                return "Off";
            case pcp::BLHeliSettingPwmDither::Level3:
                return "3";
            case pcp::BLHeliSettingPwmDither::Level7:
                return "7";
            case pcp::BLHeliSettingPwmDither::Level15:
                return "15";
            case pcp::BLHeliSettingPwmDither::Level31:
                return "31";
            default:
                return "<UNDEFINED>";
        }
    }
}  // namespace std

namespace pcp {
    std::string setting_to_string(pcp::BLHeliESCSetting setting, uint8_t value) {
        switch (setting) {
            case pcp::BLHeliESCSetting::FirmwareMajorVersion:
                return std::to_string(value);
            case pcp::BLHeliESCSetting::FirmwareMinorVersion:
                return std::to_string(value);
            case pcp::BLHeliESCSetting::LayoutRevision:
                return std::to_string(value);
            case pcp::BLHeliESCSetting::GovernorPGain:
                return std::to_string(static_cast<pcp::BLHeliGovernorGain>(value));
            case pcp::BLHeliESCSetting::GovernorIGain:
                return std::to_string(static_cast<pcp::BLHeliGovernorGain>(value));
            case pcp::BLHeliESCSetting::GovernorMode:
                return std::to_string(static_cast<pcp::BLHeliSettingGovernorMode>(value));
            case pcp::BLHeliESCSetting::LowVoltageLimit:
                return std::to_string(static_cast<pcp::BLHeliSettingLowVoltageLimit>(value));
            case pcp::BLHeliESCSetting::MotorGain:
                return std::to_string(static_cast<pcp::BLHeliSettingMotorGain>(value));
            case pcp::BLHeliESCSetting::MotorIdle:
                return std::to_string(static_cast<pcp::BLHeliSettingMotorIdle>(value));
            case pcp::BLHeliESCSetting::StartupPower:
                return std::to_string(static_cast<pcp::BLHeliSettingStartupPower>(value));
            case pcp::BLHeliESCSetting::PwmFrequency:
                return std::to_string(static_cast<pcp::BLHeliSettingPwmFrequency>(value));
            case pcp::BLHeliESCSetting::Direction:
                return std::to_string(static_cast<pcp::BLHeliSettingDirection>(value));
            case pcp::BLHeliESCSetting::InputPolarity:
                return std::to_string(static_cast<pcp::BLHeliSettingInputPolarity>(value));
            case pcp::BLHeliESCSetting::SignatureLow:
                return "Signature";
            case pcp::BLHeliESCSetting::SignatureHigh:
                return "Signature";
            case pcp::BLHeliESCSetting::EnableProgramByTx:
                return std::to_string(static_cast<pcp::BLHeliBool>(value));
            case pcp::BLHeliESCSetting::MainRearmStart:
                return std::to_string(static_cast<pcp::BLHeliBool>(value));
            case pcp::BLHeliESCSetting::GovernerSetupTarget:
                return std::to_string(static_cast<pcp::PercentageTarget>(value));
            case pcp::BLHeliESCSetting::StartupRpm:
                return "UNUSED";
            case pcp::BLHeliESCSetting::StartupAcceleration:
                return "UNUSED";
            case pcp::BLHeliESCSetting::VoltageCompensation:
                return "UNUSED";
            case pcp::BLHeliESCSetting::CommutationTiming:
                return std::to_string(static_cast<pcp::BLHeliSettingCommutationTiming>(value));
            case pcp::BLHeliESCSetting::DampingForce:
                return "UNUSED";
            case pcp::BLHeliESCSetting::GovernorRange:
                return std::to_string(static_cast<pcp::BLHeliSettingGovernorRange>(value));
            case pcp::BLHeliESCSetting::StartupMethod:
                return "UNUSED";
            case pcp::BLHeliESCSetting::MinThrottlePpm:
                return std::to_string(static_cast<pcp::BLHeliThrottleValue>(value));
            case pcp::BLHeliESCSetting::MaxThrottlePpm:
                return std::to_string(static_cast<pcp::BLHeliThrottleValue>(value));
            case pcp::BLHeliESCSetting::BeepStrength:
                return std::to_string(value);
            case pcp::BLHeliESCSetting::BeaconStrength:
                return std::to_string(value);
            case pcp::BLHeliESCSetting::BeaconDelay:
                return std::to_string(static_cast<pcp::BLHeliSettingBeaconDelay>(value));
            case pcp::BLHeliESCSetting::ThrottleRate:
                return "UNUSED";
            case pcp::BLHeliESCSetting::DemagCompensation:
                return std::to_string(static_cast<pcp::BLHeliSettingDemagCompensation>(value));
            case pcp::BLHeliESCSetting::BECVoltageHigh:
                return "Do Not Change";
            case pcp::BLHeliESCSetting::CentreThrottlePpm:
                return std::to_string(static_cast<pcp::BLHeliThrottleValue>(value));
            case pcp::BLHeliESCSetting::MainSpoolupTime:
                return "UNUSED";
            case pcp::BLHeliESCSetting::TemperatureProtection:
                return std::to_string(static_cast<pcp::BLHeliBool>(value));
            case pcp::BLHeliESCSetting::EnablePowerProtection:
                return std::to_string(static_cast<pcp::BLHeliBool>(value));
            case pcp::BLHeliESCSetting::EnablePwmInput:
                return std::to_string(static_cast<pcp::BLHeliBool>(value));
            case pcp::BLHeliESCSetting::EnablePwmDither:
                return std::to_string(static_cast<pcp::BLHeliSettingPwmDither>(value));
            case pcp::BLHeliESCSetting::EnableBrakeOnStop:
                return std::to_string(static_cast<pcp::BLHeliBool>(value));
        }
        return "Unknown Setting";
    }

    uint8_t BLHeliESCConfig::defaultValueForSetting(BLHeliESCSetting setting) const {
        switch (setting) {
            case pcp::BLHeliESCSetting::FirmwareMajorVersion:
                return 14;
            case pcp::BLHeliESCSetting::FirmwareMinorVersion:
                return 9;
            case pcp::BLHeliESCSetting::LayoutRevision:
                return 21;
            case pcp::BLHeliESCSetting::GovernorPGain:
                return static_cast<uint8_t>(rotorType() == BLHeliRotorType::Multi ? kDefaultGovernorPGainMulti : kDefaultGovernorPGain);
            case pcp::BLHeliESCSetting::GovernorIGain:
                return static_cast<uint8_t>(rotorType() == BLHeliRotorType::Multi ? kDefaultGovernorIGainMulti : kDefaultGovernorIGain);
            case pcp::BLHeliESCSetting::GovernorMode:
                return static_cast<uint8_t>(rotorType() == BLHeliRotorType::Multi ? kDefaultGovernorModeMulti : kDefaultGovernorMode);
            case pcp::BLHeliESCSetting::LowVoltageLimit:
                return static_cast<uint8_t>(kDefaultLowVoltageLimit);
            case pcp::BLHeliESCSetting::MotorGain:
                return static_cast<uint8_t>(kDefaultMotorGain);
            case pcp::BLHeliESCSetting::MotorIdle:
                return static_cast<uint8_t>(kDefaultMotorIdle);
            case pcp::BLHeliESCSetting::StartupPower:
                return static_cast<uint8_t>(kDefaultStartupPower);
            case pcp::BLHeliESCSetting::PwmFrequency:
                return static_cast<uint8_t>(rotorType() == BLHeliRotorType::Main
                                                ? kDefaultPwmFrequency
                                                : (_layoutSupportsDampedMode() ? kDefaultPwmFrequencyDampedTail : kDefaultPwmFrequencyTail));
            case pcp::BLHeliESCSetting::Direction:
                return static_cast<uint8_t>(kDefaultDirection);
            case pcp::BLHeliESCSetting::InputPolarity:
                return static_cast<uint8_t>(kDefaultInputPolarity);
            case pcp::BLHeliESCSetting::SignatureLow:
                switch (rotorType()) {
                    case BLHeliRotorType::Main:
                        return 0xa5;
                    case BLHeliRotorType::Tail:
                        return 0x5a;
                    case BLHeliRotorType::Multi:
                        return 0x55;
                    default:
                        return 0x00;
                }
            case pcp::BLHeliESCSetting::SignatureHigh:
                switch (rotorType()) {
                    case BLHeliRotorType::Main:
                        return 0x5a;
                    case BLHeliRotorType::Tail:
                        return 0xa5;
                    case BLHeliRotorType::Multi:
                        return 0xaa;
                    default:
                        return 0x00;
                }
            case pcp::BLHeliESCSetting::EnableProgramByTx:
                return static_cast<uint8_t>(kDefaultProgramByTx);
            case pcp::BLHeliESCSetting::MainRearmStart:
                return static_cast<uint8_t>(kDefaultMainRearmStart);
            case pcp::BLHeliESCSetting::GovernerSetupTarget:
                return 0;
            case pcp::BLHeliESCSetting::StartupRpm:
                return 0;
            case pcp::BLHeliESCSetting::StartupAcceleration:
                return 0;
            case pcp::BLHeliESCSetting::VoltageCompensation:
                return 0;
            case pcp::BLHeliESCSetting::CommutationTiming:
                return static_cast<uint8_t>(kDefaultMainCommutationTiming);
            case pcp::BLHeliESCSetting::DampingForce:
                return 0;
            case pcp::BLHeliESCSetting::GovernorRange:
                return static_cast<uint8_t>(kDefaultGovernorRange);
            case pcp::BLHeliESCSetting::StartupMethod:
                return 0;
            case pcp::BLHeliESCSetting::MinThrottlePpm:
                return static_cast<uint8_t>(kDefaultMinThrottlePpm);
            case pcp::BLHeliESCSetting::MaxThrottlePpm:
                return static_cast<uint8_t>(kDefaultMaxThrottlePpm);
            case pcp::BLHeliESCSetting::BeepStrength:
                switch (rotorType()) {
                    case BLHeliRotorType::Main:
                        return kDefaultBeepStrengthMain;
                    case BLHeliRotorType::Tail:
                        return kDefaultBeepStrengthTail;
                    case BLHeliRotorType::Multi:
                        return kDefaultBeepStrengthMulti;
                    default:
                        return kDefaultBeepStrengthMain;
                }
            case pcp::BLHeliESCSetting::BeaconStrength:
                switch (rotorType()) {
                    case BLHeliRotorType::Main:
                        return kDefaultBeaconStrengthMain;
                    case BLHeliRotorType::Tail:
                        return kDefaultBeaconStrengthTail;
                    case BLHeliRotorType::Multi:
                        return kDefaultBeaconStrengthMulti;
                    default:
                        return kDefaultBeaconStrengthMain;
                }
            case pcp::BLHeliESCSetting::BeaconDelay:
                return static_cast<uint8_t>(kDefaultBeaconDelay);
            case pcp::BLHeliESCSetting::ThrottleRate:
                return 0;
            case pcp::BLHeliESCSetting::DemagCompensation:
                return static_cast<uint8_t>(rotorType() == BLHeliRotorType::Multi ? kDefaultDemagCompensationMulti : kDefaultDemagCompensation);
            case pcp::BLHeliESCSetting::BECVoltageHigh:
                return 0;
            case pcp::BLHeliESCSetting::CentreThrottlePpm:
                return static_cast<uint8_t>(kDefaultCentreThrottlePpm);
            case pcp::BLHeliESCSetting::MainSpoolupTime:
                return 0;
            case pcp::BLHeliESCSetting::TemperatureProtection:
                return static_cast<uint8_t>(kDefaultTemperatureProtection);
            case pcp::BLHeliESCSetting::EnablePowerProtection:
                return static_cast<uint8_t>(kDefaultPowerProtection);
            case pcp::BLHeliESCSetting::EnablePwmInput:
                return static_cast<uint8_t>(kDefaultEnablePwmInput);
            case pcp::BLHeliESCSetting::EnablePwmDither:
                return static_cast<uint8_t>(kDefaultPwmDither);
            case pcp::BLHeliESCSetting::EnableBrakeOnStop:
                return static_cast<uint8_t>(kDefaultEnableBrakeOnStop);
            default:
                assert(false);
                return 0;
        }
    }

    static constexpr size_t kBLHeliDeviceLayoutStorageOffset = 0x40;
    static constexpr size_t kBLHeliDeviceNameOffset = 0x60;
    static constexpr size_t kBLHeliDeviceLayoutOffset = kBLHeliDeviceLayoutStorageOffset + 1;
    static constexpr size_t kBLHeliDeviceLayoutStorageMaxLength = 0x0e;
    static constexpr size_t kBLHeliDeviceLayoutMaxLength = kBLHeliDeviceLayoutStorageMaxLength - 2;
    static constexpr size_t kBLHeliDeviceNameMaxLength = 0x10;

    BLHeliESCConfig::BLHeliESCConfig() {}

    BLHeliESCConfig::BLHeliESCConfig(const uint8_t* escGreeting, const std::vector<uint8_t>& eepromBytes)
        : _bootloaderVersion(), _signature(), _bootVersion(escGreeting[6]), _bootPages(escGreeting[7]) {
        memcpy(_bootloaderVersion.data(), escGreeting, 4 * sizeof(char));
        memcpy(_signature.data(), escGreeting + 4 * sizeof(char), 2 * sizeof(char));
        assert(eepromBytes.size() >= kBLHeliEEPROMSize);
        memcpy(_eepromBytes.data(), eepromBytes.data(), kBLHeliEEPROMSize);

        _layout.clear();
        _layout.reserve(kBLHeliDeviceLayoutMaxLength);
        for (uint8_t i = 0; i < kBLHeliDeviceLayoutMaxLength; ++i) {
            if (_eepromBytes[kBLHeliDeviceLayoutOffset + i] == '#') {
                break;
            }
            _layout.push_back(_eepromBytes[kBLHeliDeviceLayoutOffset + i]);
        }

        _name.clear();
        _name.reserve(kBLHeliDeviceNameMaxLength);
        for (uint8_t i = 0; i < kBLHeliDeviceNameMaxLength; ++i) {
            _name.push_back(_eepromBytes[kBLHeliDeviceNameOffset + i]);
        }

        _parseDeviceSettings();
    }

    std::string BLHeliESCConfig::prettyLayout() const {
        static const std::unordered_map<std::string, std::string> kPrettyLayouts = {
            {"AIK_BL_30S", "AIKON Boltlite 30A"},
            {"MR25_15A", "Align MR25 15A"},
            {"AlignBL15P", "Align BL15P"},
            {"AlignBL15X", "Align BL15X"},
            {"AlignBL35P", "Align BL35P"},
            {"AlignBL35X", "Align BL35X"},
            {"DP3A", "DP 3A"},
            {"DYS_XM20A", "DYS XM 20A"},
            {"EAZY3Av2", "EAZY 3A v2"},
            {"EMAX20A", "EMAX 20A"},
            {"EMAX40A", "EMAX 40A"},
            {"EMAX_Ltng_20A", "EMAX Lightning 20A"},
            {"F85_3A", "F85 3A"},
            {"FVTLibee12A", "FVT Littlebee 12A"},
            {"FVTLibee20A", "FVT Littlebee 20A"},
            {"FVTLibee20APro", "FVT Littlebee 20A (Pro)"},
            {"FVTLibee30A", "FVT Littlebee 30A"},
            {"FC_Fairy_6A", "Flycolor Fairy 6A"},
            {"FC_Fairy_30A", "Flycolor Fairy 30A"},
            {"FC_FairyV2_30A", "Flycolor Fairy 30A v2"},
            {"FC_Raptor_20A", "Flycolor Raptor 20A"},
            {"FC_Rapt390_20A", "Flycolor Raptor 390 20A"},
            {"GauiGE18318A", "Gaui GE-183 18A"},
            {"G_Ultra20A", "Graupner Ultra 20A"},
            {"HTHumbird12A", "HTIRC Hummingbird 12A"},
            {"HTHumbird20A", "HTIRC Hummingbird 20A"},
            {"HTHumbird30APr", "HTIRC Hummingbird 30A (Pro)"},
            {"HKing10A", "H King 10A"},
            {"HKing20A", "H King 20A"},
            {"HKing35A", "H King 35A"},
            {"HKing50A", "H King 50A"},
            {"HiModelCool22A", "HiModel Cool 22A"},
            {"HiModelCool33A", "HiModel Cool 33A"},
            {"HiModelCool41A", "HiModel Cool 41A"},
            {"MDRX62H", "MDRX62H"},
            {"OvskyMR20A", "Oversky MR-20A"},
            {"OvskyMR20APro", "Oversky MR-20A (Pro)"},
            {"Platinum50Av3", "Hobbywing Platinum 50A v3"},
            {"Platinum150A", "Hobbywing Platinum Pro 150A"},
            {"PlatinumPro30A", "Hobbywing Platinum Pro 30A"},
            {"PolarisTdr12A", "Polaris Thunder 12A"},
            {"PolarisTdr20A", "Polaris Thunder 20A"},
            {"PolarisTdr30A", "Polaris Thunder 30A"},
            {"PolarisTdr40A", "Polaris Thunder 40A"},
            {"PolarisTdr60A", "Polaris Thunder 60A"},
            {"PolarisTdr80A", "Polaris Thunder 80A"},
            {"PolarisTdr100A", "Polaris Thunder 100A"},
            {"RCTimer30A", "RC Timer 30A"},
            {"RotorGeeks20A", "Rotor Geeks 20A"},
            {"RotorGeeks20AP", "Rotor Geeks 20A Plus"},
            {"SKMonster30A", "ServoKing Monster 30A"},
            {"SKMonster80A", "ServoKing Monster 80A"},
            {"SKMonster30APr", "ServoKing Monster 30A (Pro)"},
            {"SKMonster70APr", "ServoKing Monster 70A (Pro)"},
            {"SkyIII30A", "Hobbywing SkyIII 30A"},
            {"Skywalker20A", "Hobbywing Skywalker 20A"},
            {"Skywalker40A", "Hobbywing Skywalker 40A"},
            {"Supermicro3p5A", "SuperMicro 3.5A"},
            {"TBSCube12A", "TBS Cube 12A"},
            {"Tarot30A", "Tarot 30A"},
            {"TurnigyAE20A", "Turnigy AE 20A"},
            {"TurnigyAE25A", "Turnigy AE 25A"},
            {"TurnigyAE30A", "Turnigy AE 30A"},
            {"TurnigyAE40A", "Turnigy AE 40A"},
            {"TgyKF120AHV", "Turnigy KForce 120A HV"},
            {"TgyKF120AHVv2", "Turnigy KForce 120A HV v2"},
            {"TgyKF40A", "Turnigy KForce 40A"},
            {"TgyKF70AHV", "Turnigy KForce 70A HV"},
            {"Turnigy6A", "Turnigy Plush 6A"},
            {"Turnigy10A", "Turnigy Plush 10A"},
            {"Turnigy12A", "Turnigy Plush 12A"},
            {"Turnigy18A", "Turnigy Plush 18A"},
            {"Turnigy25A", "Turnigy Plush 25A"},
            {"Turnigy30A", "Turnigy Plush 30A"},
            {"Turnigy40A", "Turnigy Plush 40A"},
            {"Turnigy60A", "Turnigy Plush 60A"},
            {"Turnigy80A", "Turnigy Plush 80A"},
            {"TurnigyNfet18A", "Turnigy Plush Nfet 18A"},
            {"TurnigyNfet25A", "Turnigy Plush Nfet 25A"},
            {"TurnigyNfet30A", "Turnigy Plush Nfet 30A"},
            {"XP3A", "XP 3A"},
            {"XP7A", "XP 7A"},
            {"XP7AFast", "XP 7A (modified for fast switching)"},
            {"XP12A", "XP 12A"},
            {"XP18A", "XP 18A"},
            {"XP25A", "XP 25A"},
            {"XP35ASW", "XP 35A SW"},
            {"XRotor10A", "Hobbywing XRotor 10A"},
            {"XRotor20A", "Hobbywing XRotor 20A"},
            {"XRotor40A", "Hobbywing XRotor 40A"},
            {"ZTWSpPro20A", "ZTW Spider Pro 20A"},
            {"ZTWSpPro20AHV", "ZTW Spider Pro 20A HV"},
            {"ZTWSpPro20APrm", "ZTW Spider Pro 20A Premium"},
            {"ZTWSpPro30AHV", "ZTW Spider Pro 30A HV"},
        };
        if (kPrettyLayouts.count(layout()) > 0) {
            return kPrettyLayouts.at(layout());
        }

        std::string prettyLayout = layout();
        std::replace(prettyLayout.begin(), prettyLayout.end(), '_', ' ');
        std::regex amperageRegex("(\\d+)A");
        return std::regex_replace(prettyLayout, amperageRegex, " $& ");
    }

    void BLHeliESCConfig::_parseDeviceSettings(void) {
        static const std::vector<BLHeliESCSetting> kSettingsForMainRotor = {
            BLHeliESCSetting::GovernorPGain,     BLHeliESCSetting::GovernorIGain,         BLHeliESCSetting::GovernorMode,
            BLHeliESCSetting::LowVoltageLimit,   BLHeliESCSetting::StartupPower,          BLHeliESCSetting::PwmFrequency,
            BLHeliESCSetting::Direction,         BLHeliESCSetting::InputPolarity,         BLHeliESCSetting::EnableProgramByTx,
            BLHeliESCSetting::MainRearmStart,    BLHeliESCSetting::GovernerSetupTarget,   BLHeliESCSetting::CommutationTiming,
            BLHeliESCSetting::GovernorRange,     BLHeliESCSetting::MinThrottlePpm,        BLHeliESCSetting::MaxThrottlePpm,
            BLHeliESCSetting::BeepStrength,      BLHeliESCSetting::BeaconStrength,        BLHeliESCSetting::BeaconDelay,
            BLHeliESCSetting::DemagCompensation, BLHeliESCSetting::TemperatureProtection, BLHeliESCSetting::EnablePowerProtection,
            BLHeliESCSetting::EnablePwmInput,    BLHeliESCSetting::EnableBrakeOnStop,
        };
        static const std::vector<BLHeliESCSetting> kSettingsForTailRotor = {
            BLHeliESCSetting::MotorGain,
            BLHeliESCSetting::MotorIdle,
            BLHeliESCSetting::StartupPower,
            BLHeliESCSetting::PwmFrequency,
            BLHeliESCSetting::Direction,
            BLHeliESCSetting::InputPolarity,
            BLHeliESCSetting::EnableProgramByTx,
            BLHeliESCSetting::CommutationTiming,
            BLHeliESCSetting::MinThrottlePpm,
            BLHeliESCSetting::MaxThrottlePpm,
            BLHeliESCSetting::BeepStrength,
            BLHeliESCSetting::BeaconStrength,
            BLHeliESCSetting::BeaconDelay,
            BLHeliESCSetting::DemagCompensation,
            BLHeliESCSetting::CentreThrottlePpm,
            BLHeliESCSetting::TemperatureProtection,
            BLHeliESCSetting::EnablePowerProtection,
            BLHeliESCSetting::EnablePwmInput,
            BLHeliESCSetting::EnablePwmDither,
            BLHeliESCSetting::EnableBrakeOnStop,
        };
        static const std::vector<BLHeliESCSetting> kSettingsForMultiRotor = {
            BLHeliESCSetting::GovernorPGain,
            BLHeliESCSetting::GovernorIGain,
            BLHeliESCSetting::GovernorMode,
            BLHeliESCSetting::MotorGain,
            BLHeliESCSetting::StartupPower,
            BLHeliESCSetting::PwmFrequency,
            BLHeliESCSetting::Direction,
            BLHeliESCSetting::InputPolarity,
            BLHeliESCSetting::EnableProgramByTx,
            BLHeliESCSetting::CommutationTiming,
            BLHeliESCSetting::MinThrottlePpm,
            BLHeliESCSetting::MaxThrottlePpm,
            BLHeliESCSetting::BeepStrength,
            BLHeliESCSetting::BeaconStrength,
            BLHeliESCSetting::BeaconDelay,
            BLHeliESCSetting::DemagCompensation,
            BLHeliESCSetting::CentreThrottlePpm,
            BLHeliESCSetting::TemperatureProtection,
            BLHeliESCSetting::EnablePowerProtection,
            BLHeliESCSetting::EnablePwmInput,
            BLHeliESCSetting::EnablePwmDither,
            BLHeliESCSetting::EnableBrakeOnStop,
        };
        static const std::array<std::vector<BLHeliESCSetting>, kRotorTypeCount> kSettingsByRotorType = {
            kSettingsForMainRotor,
            kSettingsForTailRotor,
            kSettingsForMultiRotor,
        };
        const std::vector<BLHeliESCSetting>& availableSettings = kSettingsByRotorType[static_cast<uint8_t>(rotorType())];

        _majorVersion = _eepromBytes[static_cast<size_t>(BLHeliESCSetting::FirmwareMajorVersion)];
        _minorVersion = _eepromBytes[static_cast<size_t>(BLHeliESCSetting::FirmwareMinorVersion)];

        _settings.clear();
        _settings.reserve(availableSettings.size());
        for (BLHeliESCSetting setting : availableSettings) {
            _settings[setting] = _eepromBytes[static_cast<size_t>(setting)];
        }
    }

    bool BLHeliESCConfig::_layoutSupportsDampedMode(void) const {
        std::unordered_set<std::string> _dampedModeLayouts = {
            "AIK_BL_30S",     "MR25_15A",       "AlignBL35P",     "AlignBL35X",     "DALRC_XR20A",    "DP3A",           "DYS_XM20A",      "EAZY3Av2",
            "EMAX20A",        "EMAX40A",        "EMAX_Ltng_20A",  "EMAXNano20A",    "F85_3A",         "FVTLibee12A",    "FVTLibee20A",    "FVTLibee20APro",
            "FVTLibee30A",    "FC_Fairy_30A",   "FC_FairyV2_30A", "FC_Raptor_20A",  "FC_Rapt390_20A", "G_Ultra20A",     "HTHumbird12A",   "HTHumbird20A",
            "HTHumbird30APr", "HKing10A",       "HKing20A",       "HKing35A",       "HKing50A",       "OvskyMR20A",     "OvskyMR20APro",  "Platinum50Av3",
            "Platinum150A",   "PlatinumPro30A", "RotorGeeks20A",  "RotorGeeks20AP", "SKMonster30A",   "SKMonster30APr", "SKMonster70APr", "SKMonster80A",
            "Skywalker20A",   "Skywalker40A",   "Supermicro3p5A", "TBSCube12A",     "TurnigyAE45A",   "TgyKF40A",       "Turnigy40A",     "TurnigyNfet18A",
            "TurnigyNfet25A", "TurnigyNfet30A", "XP35ASW",        "XP3A",           "XP7AFast",       "XRotor10A",      "XRotor20A",      "XRotor40A",
            "ZTWSpPro20A",    "ZTWSpPro20AHV",  "ZTWSpPro20APrm", "ZTWSpPro30AHV",
        };
        return _dampedModeLayouts.contains(_layout);
    }

    std::string BLHeliESCConfig::versionString(void) const {
        return std::format("{}.{}", _majorVersion, _minorVersion);
    }

    BLHeliRotorType BLHeliESCConfig::rotorType(void) const {
        const uint16_t signature = static_cast<uint16_t>(_eepromBytes[static_cast<size_t>(BLHeliESCSetting::SignatureLow)]) +
                                   (static_cast<uint16_t>(_eepromBytes[static_cast<size_t>(BLHeliESCSetting::SignatureHigh)]) << 8);

        switch (signature) {
            case 0x5aa5:
                return BLHeliRotorType::Main;
            case 0xa55a:
                return BLHeliRotorType::Tail;
            case 0xaa55:
                return BLHeliRotorType::Multi;
            default:
                assert(false && "Invalid rotor type");
                return BLHeliRotorType::Main;
        }
    }
}  // namespace pcp
