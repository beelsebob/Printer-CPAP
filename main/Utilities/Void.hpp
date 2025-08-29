#pragma once

#include <string>

namespace pcp {
    struct Void {};
}  // namespace pcp

namespace std {
    inline std::string to_string(pcp::Void v) {
        return "<Void>";
    }
}  // namespace std
