#pragma once

#include "Utilities/Void.hpp"
#include "Utilities/to_stringExtras.hpp"

#include <cstdint>
#include <string>

namespace pcp {
    enum class BootloaderResultCode : uint8_t {
        ErrorTimeout = 0x01,
        Success = 0x30,
        ErrorVerify = 0xc0,
        ErrorCommand = 0xc1,
        ErrorCRC = 0xc2,
        ErrorProg = 0xc5,
        ErrorNone = 0xff
    };

    inline uint8_t to_uint8(BootloaderResultCode res) {
        return static_cast<uint8_t>(res);
    }

    inline std::string to_string(BootloaderResultCode res) {
        switch (res) {
            case BootloaderResultCode::ErrorTimeout: return "ErrorTimeout";
            case BootloaderResultCode::Success: return "Success";
            case BootloaderResultCode::ErrorVerify: return "ErrorVerify";
            case BootloaderResultCode::ErrorCommand: return "ErrorCommand";
            case BootloaderResultCode::ErrorCRC: return "ErrorCRC";
            case BootloaderResultCode::ErrorProg: return "ErrorProg";
            case BootloaderResultCode::ErrorNone: return "ErrorNone";
        }
        assert(false && "Unhandled case in switch statement");
        return "UnknownError" + std::to_string(to_uint8(res));
    }

    template <typename T>
    class BootloaderResult {
    public:
        BootloaderResult() : resultCode(BootloaderResultCode::Success), data() {}

        BootloaderResult(BootloaderResultCode code) : resultCode(code) { assert(code != BootloaderResultCode::Success); }

        BootloaderResult(const T& result) : resultCode(BootloaderResultCode::Success), data(result) {}

        BootloaderResult(const BootloaderResult<T>& result) = default;
        BootloaderResult(BootloaderResult<T>&& result) = default;

        BootloaderResult<T>& operator=(const BootloaderResult<T>& result) = default;
        BootloaderResult<T>& operator=(BootloaderResult<T>&& result) = default;

        BootloaderResultCode resultCode;
        T data;

        operator bool() const { return resultCode == BootloaderResultCode::Success; }
    };
}  // namespace pcp

namespace std {
    template <typename T>
    std::string to_string(const pcp::BootloaderResult<T>& r) {
        if (r.resultCode == pcp::BootloaderResultCode::Success) {
            return std::to_string(r.data);
        } else {
            return "<" + to_string(r.resultCode) + ">";
        }
    }
}  // namespace std

namespace pcp {
    enum class BootloaderCommandType : uint8_t {
        Run = 0x00,
        ProgramFlash = 0x01,
        EraseFlash = 0x02,
        ReadFlash = 0x03,
        KeepAlive = 0xfd,
        SetBuffer = 0xfe,
        SetAddress = 0xff,
    };

    template <BootloaderCommandType cmd>
    struct BootloaderCommand {};

    template <>
    struct BootloaderCommand<BootloaderCommandType::Run> {
        static constexpr bool hasCommandData = false;
        static constexpr bool hasArgument = false;
        using ReturnType = Void;

        size_t expectedReturnBytes(void) { return 0; }
    };

    template <>
    struct BootloaderCommand<BootloaderCommandType::ReadFlash> {
        static constexpr bool hasCommandData = true;
        static constexpr bool hasArgument = false;
        uint8_t commandData = 0x00;
        using ReturnType = std::vector<uint8_t>;

        size_t expectedReturnBytes(void) { return static_cast<size_t>(commandData); }
    };

    template <>
    struct BootloaderCommand<BootloaderCommandType::KeepAlive> {
        static constexpr bool hasCommandData = false;
        static constexpr bool hasArgument = false;
        using ReturnType = Void;

        size_t expectedReturnBytes(void) { return 0; }
    };

    template <>
    struct BootloaderCommand<BootloaderCommandType::SetBuffer> {
        static constexpr bool hasCommandData = false;
        static constexpr bool hasArgument = true;
        uint16_t argument = 0x0000;
        using ReturnType = Void;

        size_t expectedReturnBytes(void) { return 0; }
    };

    template <>
    struct BootloaderCommand<BootloaderCommandType::SetAddress> {
        static constexpr bool hasCommandData = false;
        static constexpr bool hasArgument = true;
        uint16_t argument = 0x0000;
        using ReturnType = Void;

        size_t expectedReturnBytes(void) { return 0; }
    };
}  // namespace pcp
