#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <vector>

namespace std {
    inline std::string to_string(const std::string& s) {
        return std::format("\"{}\"", s);
    }

    template <typename T>
    std::string to_string(const std::vector<T>& v) {
        std::string s = "{";
        for (const auto& i : v) {
            s += to_string(i);
            s += ", ";
        }
        s[s.size() - 2] = '}';
        s[s.size() - 1] = '\0';
        return s;
    }
}  // namespace std
