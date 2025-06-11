#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "location.hh"

struct Location {
    Location(const idl::location& loc) noexcept : location(loc) {
    }

    const idl::location& location;
};

template <typename YYChar>
std::basic_ostream<YYChar>& operator<<(std::basic_ostream<YYChar>& ostr, const Location& loc) {
    const auto& pos = loc.location.begin;
    if (pos.filename) {
        ostr << *pos.filename << ':';
    }
    return ostr << pos.line << ':' << pos.column;
}

#endif
