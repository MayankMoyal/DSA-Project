#pragma once

#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace adversary {
inline std::uint64_t unsignedArgument(std::string_view text, const char* name) {
    const std::string error = std::string(name) + " must contain decimal digits and fit in uint64";
    if (text.empty() || text.find_first_not_of("0123456789") != std::string_view::npos) {
        throw std::invalid_argument(error);
    }
    std::uint64_t value = 0;
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size()) {
        throw std::invalid_argument(error);
    }
    return value;
}

inline int populationArgument(std::string_view text) {
    const auto n = unsignedArgument(text, "n");
    if (n < 1 || n > 1000000) {
        throw std::invalid_argument("n must be in [1,1000000]");
    }
    return static_cast<int>(n);
}
}
