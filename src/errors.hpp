#ifndef ERRORS_HPP
#define ERRORS_HPP

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

#include <fmt/base.h>

#include "location.hpp"

enum Warning {
    W1001 = 1001,
    W1002
};

enum Error {
    E2001 = 2001,
    E2002,
    E2003,
    E2004,
    E2005,
    E2006,
    E2007,
    E2008,
    E2009,
    E2010,
    E2011,
    E2012,
    E2013,
    E2014,
    E2015,
    E2016,
    E2017,
    E2018,
    E2019,
    E2020,
    E2021
};

template <Warning Code, typename... Args>
inline std::string warn_str(Args&&... args) {
    std::ostringstream ss;
    ss << "[" << Code << "] ";
    if constexpr (Code == W1001) {
        std::cerr << "the beginning of the documentation should be marked with a space";
    }
    if constexpr (Code == W1002) {
        std::cerr << "the beginning of the documentation should be marked with a single space";
    }
    return ss.str();
}

template <Error Code, typename... Args>
inline std::string err_str(Args&&... args) {
    std::ostringstream ss;
    ss << "[E" << Code << "] ";
    if constexpr (Code == E2001) {
        ss << fmt::format("unexpected character '{}'", args...);
    }
    if constexpr (Code == E2002) {
        ss << "tabs are not allowed";
    }
    if constexpr (Code == E2003) {
        ss << fmt::format("the name must start with a capital letter '{}'", args...);
    }
    if constexpr (Code == E2004) {
        ss << "there can only be one api declaration";
    }
    if constexpr (Code == E2005) {
        ss << "there is no documentation in the declaration";
    }
    if constexpr (Code == E2006) {
        ss << "documentation cannot be an empty string";
    }
    if constexpr (Code == E2007) {
        ss << "the brief should only be listed once in the documentation";
    }
    if constexpr (Code == E2008) {
        ss << "the detail should only be listed once in the documentation";
    }
    if constexpr (Code == E2009) {
        ss << "the copyright should only be listed once in the documentation";
    }
    if constexpr (Code == E2010) {
        ss << "the license should only be listed once in the documentation";
    }
    if constexpr (Code == E2011) {
        ss << "multi-line documentation must start with 4 spaces";
    }
    if constexpr (Code == E2012) {
        ss << "the .idl file must start with the 'api' element";
    }
    if constexpr (Code == E2013) {
        ss << fmt::format("attribute '{}' cannot be duplicated", args...);
    }
    if constexpr (Code == E2014) {
        ss << fmt::format("the following attributes: {} - are allowed in this context", args...);
    }
    if constexpr (Code == E2015) {
        ss << fmt::format("unknown attribute '{}'", args...);
    }
    if constexpr (Code == E2016) {
        ss << "the 'platform' attribute must specify at least one argument";
    }
    if constexpr (Code == E2017) {
        ss << fmt::format("the following arguments: {} - are allowed in 'platform' attribute", args...);
    }
    if constexpr (Code == E2018) {
        ss << fmt::format("argument '{}' in the 'platform' attribute cannot be duplicated", args...);
    }
    if constexpr (Code == E2019) {
        ss << "inline documentation only [detail] description is allowed";
    }
    if constexpr (Code == E2020) {
        ss << fmt::format("invalid attribute {} in documentation", args...);
    }
    if constexpr (Code == E2021) {
        ss << "it is acceptable to use either documentation or inline documentation, but not both.";
    }
    return ss.str();
}

template <Warning Code, typename... Args>
inline void warn(const Location& loc, Args&&... args) {
    std::cout << "warning: " << warn_str<Code>(args...) << " at " << loc << std::endl;
}

template <Error Code, typename... Args>
inline void err(const Location& loc, Args&&... args) {
    std::cerr << "error: " << err_str<Code>(args...) << " at " << loc << std::endl;
    exit(1);
}

#endif
