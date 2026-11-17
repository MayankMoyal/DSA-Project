#pragma once
#include "checks.h"
#include <charconv>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <system_error>

namespace oracle {
template<class Integer>
Integer number(const std::string& token) {
    Integer value{};
    const char* end = token.data() + token.size();
    auto result = std::from_chars(token.data(), end, value);
    if (result.ec != std::errc{} || result.ptr != end)
        throw std::invalid_argument("invalid decimal integer: " + token);
    return value;
}

// An emitter returns false when the consumer already has the first failure.
template<class Emit>
void read_trace(const std::filesystem::path& path, Emit emit) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot open input " + path.string());
    std::string line, tag, key, extra;
    std::size_t line_no = 0;
    while (std::getline(input, line)) {
        ++line_no;
        std::istringstream fields(line);
        if (!(fields >> tag) || tag.front() == '#') continue;
        if ((tag != "I" && tag != "D" && tag != "C") || !(fields >> key) || fields >> extra)
            throw std::invalid_argument("invalid operation on line " + std::to_string(line_no));
        Key value;
        try { value = number<Key>(key); }
        catch (const std::invalid_argument&) {
            throw std::invalid_argument("invalid key on line " + std::to_string(line_no));
        }
        if (!emit(Op{tag.front(), value})) return;
    }
    if (input.bad()) throw std::runtime_error("trace read failed");
}

struct Outcome {
    std::size_t operations = 0;
    std::optional<Failure> failure;
};

template<class Source>
Outcome replay(Source source) {
    Checks checks;
    Outcome result;
    try {
        source([&](Op op) {
            ++result.operations;
            checks.apply(op);
            if (result.operations <= 64 || result.operations % audit_interval == 0) checks.audit();
            return true;
        });
        checks.audit();
    } catch (const Failure& failure) {
        result.failure = failure;
    }
    return result;
}

inline void write_op(std::ostream& out, Op op) { out << op.kind << ' ' << op.key << '\n'; }
}
