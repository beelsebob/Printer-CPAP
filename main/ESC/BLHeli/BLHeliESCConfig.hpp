#pragma once

#include "Utilities/Maths.hpp"

#include <cstdint>
#include <format>
#include <unordered_map>
#include <vector>

namespace pcp {
    static constexpr size_t kBLHeliEEPROMSize = 0x70;

    enum class BLHeliRotorType : uint8_t { Main = 0, Tail = 1, Multi = 2 };

    enum class BLHeliESCSetting : uint8_t {
        FirmwareMajorVersion = 0,
        FirmwareMinorVersion = 1,
        LayoutRevision = 2,
        GovernorPGain = 3,
        GovernorIGain = 4,
        GovernorMode = 5,
        LowVoltageLimit = 6,
        MotorGain = 7,
        MotorIdle = 8,
        StartupPower = 9,
        PwmFrequency = 10,
        Direction = 11,
        InputPolarity = 12,
        SignatureLow = 13,
        SignatureHigh = 14,
        EnableProgramByTx = 15,
        MainRearmStart = 16,
        GovernerSetupTarget = 17,
        StartupRpm = 18,
        StartupAcceleration = 19,
        VoltageCompensation = 20,
        CommutationTiming = 21,
        DampingForce = 22,
        GovernorRange = 23,
        StartupMethod = 24,
        MinThrottlePpm = 25,
        MaxThrottlePpm = 26,
        BeepStrength = 27,
        BeaconStrength = 28,
        BeaconDelay = 29,
        ThrottleRate = 30,
        DemagCompensation = 31,
        BECVoltageHigh = 32,
        CentreThrottlePpm = 33,
        MainSpoolupTime = 34,
        TemperatureProtection = 35,
        EnablePowerProtection = 36,
        EnablePwmInput = 37,
        EnablePwmDither = 38,
        EnableBrakeOnStop = 39,
    };

    enum class BLHeliGovernorGain : uint8_t {
        Gain0p13 = 1,
        Gain0p17 = 2,
        Gain0p25 = 3,
        Gain0p38 = 4,
        Gain0p50 = 5,
        Gain0p75 = 6,
        Gain1p00 = 7,
        Gain1p50 = 8,
        Gain2p00 = 9,
        Gain3p00 = 10,
        Gain4p00 = 11,
        Gain6p00 = 12,
        Gain8p00 = 13,
    };

    static constexpr BLHeliGovernorGain kDefaultGovernorPGain = BLHeliGovernorGain::Gain1p00;
    static constexpr BLHeliGovernorGain kDefaultGovernorPGainMulti = BLHeliGovernorGain::Gain2p00;
    static constexpr BLHeliGovernorGain kDefaultGovernorIGain = BLHeliGovernorGain::Gain1p00;
    static constexpr BLHeliGovernorGain kDefaultGovernorIGainMulti = BLHeliGovernorGain::Gain2p00;

    enum class BLHeliSettingGovernorMode : uint8_t {
        Tx = 1,
        Arm = 2,
        Setup = 3,
        Off = 4,
    };

    static constexpr BLHeliSettingGovernorMode kDefaultGovernorMode = BLHeliSettingGovernorMode::Tx;
    static constexpr BLHeliSettingGovernorMode kDefaultGovernorModeMulti = BLHeliSettingGovernorMode::Off;

    enum class BLHeliSettingLowVoltageLimit : uint8_t {
        Off = 1,
        VoltPerCell3p0 = 2,
        VoltPerCell3p1 = 3,
        VoltPerCell3p2 = 4,
        VoltPerCell3p3 = 5,
        VoltPerCell3p4 = 6,
    };

    static constexpr BLHeliSettingLowVoltageLimit kDefaultLowVoltageLimit = BLHeliSettingLowVoltageLimit::VoltPerCell3p2;

    enum class BLHeliSettingMotorGain : uint8_t {
        MotorGain0p75 = 1,
        MotorGain0p88 = 2,
        MotorGain1p00 = 3,
        MotorGain1p12 = 4,
        MotorGain1p25 = 5,
    };

    static constexpr BLHeliSettingMotorGain kDefaultMotorGain = BLHeliSettingMotorGain::MotorGain1p00;

    enum class BLHeliSettingMotorIdle : uint8_t {
        Low = 1,
        MediumLow = 2,
        Medium = 3,
        MediumHigh = 4,
        High = 5,
    };

    static constexpr BLHeliSettingMotorIdle kDefaultMotorIdle = BLHeliSettingMotorIdle::MediumHigh;

    enum class BLHeliSettingStartupPower : uint8_t {
        StartupPower0p031 = 1,
        StartupPower0p047 = 2,
        StartupPower0p063 = 3,
        StartupPower0p094 = 4,
        StartupPower0p125 = 5,
        StartupPower0p188 = 6,
        StartupPower0p250 = 7,
        StartupPower0p380 = 8,
        StartupPower0p500 = 9,
        StartupPower0p750 = 10,
        StartupPower1p000 = 11,
        StartupPower1p250 = 12,
        StartupPower1p500 = 13,
    };

    static constexpr BLHeliSettingStartupPower kDefaultStartupPower = BLHeliSettingStartupPower::StartupPower0p750;

    enum class BLHeliSettingPwmFrequency : uint8_t {
        High = 1,
        Low = 2,
        DampedLight = 3,
    };

    static constexpr BLHeliSettingPwmFrequency kDefaultPwmFrequency = BLHeliSettingPwmFrequency::Low;
    static constexpr BLHeliSettingPwmFrequency kDefaultPwmFrequencyTail = BLHeliSettingPwmFrequency::High;
    static constexpr BLHeliSettingPwmFrequency kDefaultPwmFrequencyDampedTail = BLHeliSettingPwmFrequency::DampedLight;

    enum class BLHeliSettingDirection : uint8_t {
        Forward = 1,
        Reverse = 2,
        Bidirectional = 3,
    };

    static constexpr BLHeliSettingDirection kDefaultDirection = BLHeliSettingDirection::Forward;

    enum class BLHeliSettingInputPolarity : uint8_t {
        Positive = 1,
        Negative = 2,
    };

    static constexpr BLHeliSettingInputPolarity kDefaultInputPolarity = BLHeliSettingInputPolarity::Positive;

    enum class BLHeliBool : uint8_t {
        Off = 0,
        On = 1,
    };

    static constexpr BLHeliBool kDefaultProgramByTx = BLHeliBool::On;
    static constexpr BLHeliBool kDefaultMainRearmStart = BLHeliBool::Off;

    struct PercentageTarget {
        constexpr PercentageTarget(uint8_t t) : targetValue(t) {}

        constexpr PercentageTarget& operator=(uint8_t t) {
            targetValue = t;
            return *this;
        }

        constexpr operator uint8_t() const { return targetValue; }

        constexpr uint8_t approximatePercentageTarget(void) const { return invLerpPercentage(targetValue, (uint8_t)0x00, (uint8_t)0xff); }

        uint8_t targetValue;
    };

    static constexpr PercentageTarget kDefaultGovernorTarget = (uint8_t)180u;

    enum class BLHeliSettingCommutationTiming : uint8_t {
        Low = 1,
        MediumLow = 2,
        Medium = 3,
        MediumHigh = 4,
        High = 5,
    };

    static constexpr BLHeliSettingCommutationTiming kDefaultMainCommutationTiming = BLHeliSettingCommutationTiming::Medium;

    enum class BLHeliSettingGovernorRange : uint8_t { High = 1, Middle = 2, Low = 3, Off = 4 };

    static constexpr BLHeliSettingGovernorRange kDefaultGovernorRange = BLHeliSettingGovernorRange::Off;

    static constexpr uint8_t kDefaultMinThrottlePpm = 37;
    static constexpr uint8_t kDefaultMaxThrottlePpm = 208;
    static constexpr uint8_t kDefaultBeepStrengthMain = 120;
    static constexpr uint8_t kDefaultBeepStrengthTail = 250;
    static constexpr uint8_t kDefaultBeepStrengthMulti = 40;
    static constexpr uint8_t kDefaultBeaconStrengthMain = 200;
    static constexpr uint8_t kDefaultBeaconStrengthTail = 250;
    static constexpr uint8_t kDefaultBeaconStrengthMulti = 80;

    struct BLHeliThrottleValue {
        constexpr BLHeliThrottleValue(uint8_t v) : value(v) {}

        constexpr BLHeliThrottleValue& operator=(uint8_t v) {
            value = v;
            return *this;
        }

        constexpr operator uint8_t() const { return value; }

        constexpr uint16_t ppm(void) const { return 1000u + 4u * (uint16_t)value; }

        uint8_t value;
    };

    enum class BLHeliSettingBeaconDelay : uint8_t {
        Minutes1 = 1,
        Minutes2 = 2,
        Minutes5 = 3,
        Minutes10 = 4,
        NoBeacon = 5,
    };

    static constexpr BLHeliSettingBeaconDelay kDefaultBeaconDelay = BLHeliSettingBeaconDelay::Minutes10;

    enum class BLHeliSettingDemagCompensation : uint8_t {
        Off = 1,
        Low = 2,
        High = 3,
    };

    static constexpr BLHeliSettingDemagCompensation kDefaultDemagCompensation = BLHeliSettingDemagCompensation::Off;
    static constexpr BLHeliSettingDemagCompensation kDefaultDemagCompensationMulti = BLHeliSettingDemagCompensation::Low;

    static constexpr uint8_t kDefaultCentreThrottlePpm = 122;

    static constexpr BLHeliBool kDefaultTemperatureProtection = BLHeliBool::On;
    static constexpr BLHeliBool kDefaultPowerProtection = BLHeliBool::On;
    static constexpr BLHeliBool kDefaultEnablePwmInput = BLHeliBool::Off;

    enum class BLHeliSettingPwmDither : uint8_t {
        Off = 1,
        Level3 = 2,
        Level7 = 3,
        Level15 = 4,
        Level31 = 5,
    };

    static constexpr BLHeliSettingPwmDither kDefaultPwmDither = BLHeliSettingPwmDither::Level7;

    static constexpr BLHeliBool kDefaultEnableBrakeOnStop = BLHeliBool::Off;
}  // namespace pcp

namespace std {
    std::string to_string(const pcp::BLHeliRotorType& rotorType);
    std::string to_string(const pcp::BLHeliESCSetting& setting);
    std::string to_string(const pcp::BLHeliGovernorGain& setting);
    std::string to_string(const pcp::BLHeliSettingGovernorMode& setting);
    std::string to_string(const pcp::BLHeliSettingLowVoltageLimit& setting);
    std::string to_string(const pcp::BLHeliSettingMotorGain& setting);
    std::string to_string(const pcp::BLHeliSettingMotorIdle& setting);
    std::string to_string(const pcp::BLHeliSettingStartupPower& setting);
    std::string to_string(const pcp::BLHeliSettingPwmFrequency& setting);
    std::string to_string(const pcp::BLHeliSettingDirection& setting);
    std::string to_string(const pcp::BLHeliBool& setting);
    std::string to_string(const pcp::PercentageTarget& setting);
    std::string to_string(const pcp::BLHeliSettingCommutationTiming& setting);
    std::string to_string(const pcp::BLHeliSettingGovernorRange& setting);
    std::string to_string(const pcp::BLHeliThrottleValue& setting);
    std::string to_string(const pcp::BLHeliSettingBeaconDelay& setting);
    std::string to_string(const pcp::BLHeliSettingDemagCompensation& setting);
}  // namespace std

namespace pcp {
    std::string setting_to_string(pcp::BLHeliESCSetting setting, uint8_t value);

    static constexpr uint8_t kRotorTypeCount = 3;

    struct BLHeliESCConfig {
        BLHeliESCConfig();

        BLHeliESCConfig(const uint8_t* escGreeting, const std::vector<uint8_t>& eepromBytes);

        const std::array<uint8_t, 4>& bootloaderVersion() const { return _bootloaderVersion; }

        const std::array<uint8_t, 2>& deviceSignature() const { return _signature; }

        const std::string& layout() const { return _layout; }

        std::string prettyLayout() const;

        std::string& name(void) { return _name; }

        const std::string& name(void) const { return _name; }

        BLHeliRotorType rotorType(void) const;

        std::string versionString(void) const;

        const std::unordered_map<BLHeliESCSetting, uint8_t>& settings(void) const { return _settings; }

        std::unordered_map<BLHeliESCSetting, uint8_t>& settings(void) { return _settings; }

        uint8_t defaultValueForSetting(BLHeliESCSetting setting) const;

    private:
        void _parseDeviceSettings(void);

        bool _layoutSupportsDampedMode(void) const;

        std::array<uint8_t, 4> _bootloaderVersion{'\0'};
        std::array<uint8_t, 2> _signature{'\0'};
        uint8_t _bootVersion = '\0';
        uint8_t _bootPages = '\0';
        uint8_t _majorVersion = 0;
        uint8_t _minorVersion = 0;

        std::array<uint8_t, kBLHeliEEPROMSize> _eepromBytes{'\0'};

        std::string _layout{};
        std::string _name{};

        std::unordered_map<BLHeliESCSetting, uint8_t> _settings{};
    };
}  // namespace pcp

namespace std {
    inline std::string to_string(const pcp::BLHeliESCConfig& device) {
        return std::format("<BLHeliESCConfig: {} - {}>", device.layout(), device.name());
    }
}  // namespace std
