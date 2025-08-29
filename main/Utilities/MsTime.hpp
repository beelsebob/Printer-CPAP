#pragma once

#include <algorithm>
#include <cstdint>

namespace pcp {
    class MsTime {
    public:
        MsTime() {}

        MsTime(int32_t t) : _time(t) {}

        int32_t get() { return _time; }

        MsTime& operator+=(MsTime b) {
            _time += b._time;
            return *this;
        }

        MsTime& operator-=(MsTime b) {
            _time -= b._time;
            return *this;
        }

        MsTime& operator*=(uint32_t b) {
            _time *= b;
            return *this;
        }

        MsTime& operator/=(uint32_t b) {
            _time /= b;
            return *this;
        }

    private:
        int32_t _time = 0;
    };

    inline MsTime operator+(MsTime a, MsTime b) {
        return MsTime(a.get() + b.get());
    }

    inline MsTime operator-(MsTime a, MsTime b) {
        return MsTime(a.get() - b.get());
    }

    inline MsTime operator*(MsTime a, int32_t b) {
        return MsTime(a.get() * b);
    }

    inline MsTime operator*(int32_t a, MsTime b) {
        return MsTime(a * b.get());
    }

    inline MsTime operator/(MsTime a, int32_t b) {
        return MsTime(a.get() / b);
    }

    inline int32_t operator/(MsTime a, MsTime b) {
        return a.get() / b.get();
    }

    inline bool operator>(MsTime a, MsTime b) {
        return a.get() > b.get();
    }

    inline bool operator<(MsTime a, MsTime b) {
        return a.get() < b.get();
    }

    inline bool operator>=(MsTime a, MsTime b) {
        return a.get() >= b.get();
    }

    inline bool operator<=(MsTime a, MsTime b) {
        return a.get() <= b.get();
    }

    inline bool operator==(MsTime a, MsTime b) {
        return a.get() == b.get();
    }

    inline bool operator!=(MsTime a, MsTime b) {
        return a.get() != b.get();
    }

    inline MsTime min(MsTime a, MsTime b) {
        return MsTime(std::min(a.get(), b.get()));
    }

    inline MsTime max(MsTime a, MsTime b) {
        return MsTime(std::max(a.get(), b.get()));
    }

    inline MsTime clamp(MsTime x, MsTime a, MsTime b) {
        return min(max(x, a), b);
    }

    inline MsTime operator""_ms(unsigned long long t) {
        return MsTime(static_cast<int32_t>(t));
    }

    inline MsTime operator""_s(unsigned long long t) {
        return MsTime(static_cast<int32_t>(t) * 1000);
    }
}  // namespace pcp
