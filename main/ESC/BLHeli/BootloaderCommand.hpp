#pragma once

#include "Utilities/Void.hpp"
#include "Utilities/to_stringExtras.hpp"

#include <cstdint>
#include <optional>
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
            case BootloaderResultCode::ErrorTimeout:
                return "ErrorTimeout";
            case BootloaderResultCode::Success:
                return "Success";
            case BootloaderResultCode::ErrorVerify:
                return "ErrorVerify";
            case BootloaderResultCode::ErrorCommand:
                return "ErrorCommand";
            case BootloaderResultCode::ErrorCRC:
                return "ErrorCRC";
            case BootloaderResultCode::ErrorProg:
                return "ErrorProg";
            case BootloaderResultCode::ErrorNone:
                return "ErrorNone";
        }
        assert(false && "Unhandled case in switch statement");
        return "UnknownError" + std::to_string(to_uint8(res));
    }

    template <typename T>
    struct BootloaderResult {
    private:
        template <typename U>
        struct _Storage {
            _Storage() {}

            _Storage(const U& other) {
                U* storage = reinterpret_cast<U*>(_storage);
                new (storage) U(other);
                _isStored = true;
            }

            _Storage(U&& other) {
                U* storage = reinterpret_cast<U*>(_storage);
                new (storage) U(std::move(other));
                _isStored = true;
            }

            _Storage(const _Storage<U>& other) {
                if (other._isStored) {
                    U* storage = reinterpret_cast<U*>(_storage);
                    const U* otherStorage = reinterpret_cast<U*>(other.storage);
                    new (storage) U(*otherStorage);
                    _isStored = true;
                }
            }

            _Storage(_Storage<U>&& other) {
                if (other._isStored) {
                    U* storage = reinterpret_cast<U*>(_storage);
                    U* otherStorage = reinterpret_cast<U*>(other.storage);
                    new (storage) U(std::move(*otherStorage));
                    _isStored = true;
                }
            }

            ~_Storage() {
                if (_isStored) {
                    U* storage = reinterpret_cast<U*>(_storage);
                    storage->~U();
                }
            }

            _Storage& operator=(const U& other) {
                U* storage = reinterpret_cast<U*>(_storage);
                if (_isStored) {
                    *storage = other;
                } else {
                    new (storage) U(other);
                }
                _isStored = true;
                return *this;
            }

            _Storage& operator=(U&& other) {
                U* storage = reinterpret_cast<U*>(_storage);
                if (_isStored) {
                    *storage = std::move(other);
                } else {
                    new (storage) U(std::move(other));
                }
                _isStored = true;
                return *this;
            }

            _Storage& operator=(const _Storage<U>& other) {
                U* storage = reinterpret_cast<U*>(_storage);
                const U* otherStorage = reinterpret_cast<U*>(other.storage);
                if (_isStored) {
                    *storage = otherStorage;
                } else {
                    new (storage) U(otherStorage);
                }
                _isStored = true;
                return *this;
            }

            _Storage& operator=(_Storage<U>&& other) {
                U* storage = reinterpret_cast<U*>(_storage);
                U* otherStorage = reinterpret_cast<U*>(other.storage);
                if (_isStored) {
                    *storage = std::move(otherStorage);
                } else {
                    new (storage) U(std::move(otherStorage));
                }
                _isStored = true;
                return *this;
            }

            U& value() {
                U* storage = reinterpret_cast<U*>(_storage);
                return *storage;
            }

            const U& value() const {
                const U* storage = reinterpret_cast<const U*>(_storage);
                return *storage;
            }

            bool _isStored = false;
            uint8_t _storage[sizeof(U)]{0xde};
        };

        BootloaderResultCode _resultCode;

        _Storage<T> _storage;

    public:
        BootloaderResult() : _resultCode(BootloaderResultCode::Success), _storage() {}

        BootloaderResult(BootloaderResultCode code) : _resultCode(code) { assert(code != BootloaderResultCode::Success); }

        BootloaderResult(const T& result) : _resultCode(BootloaderResultCode::Success), _storage(result) {}

        BootloaderResult(const BootloaderResult<T>& result) = default;
        BootloaderResult(BootloaderResult<T>&& result) = default;

        BootloaderResult<T>& operator=(const BootloaderResult<T>& result) = default;
        BootloaderResult<T>& operator=(BootloaderResult<T>&& result) = default;

        bool has_value() const { return _resultCode == BootloaderResultCode::Success; }

        BootloaderResultCode resultCode() const { return _resultCode; }

        T& value() { return _storage.value(); }

        const T& value() const { return _storage.value(); }

        operator bool() const { return _resultCode == BootloaderResultCode::Success; }
    };
}  // namespace pcp

namespace std {
    template <typename T>
    std::string to_string(const pcp::BootloaderResult<T>& r) {
        if (r.has_value()) {
            return std::to_string(r.value());
        } else {
            return "<" + to_string(r.resultCode()) + ">";
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
