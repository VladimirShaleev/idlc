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
    E2021,
    E2022,
    E2023,
    E2024,
    E2025,
    E2026,
    E2027,
    E2028,
    E2029,
    E2030,
    E2031,
    E2032,
    E2033,
    E2034,
    E2035,
    E2036,
    E2037,
    E2038,
    E2039,
    E2040,
    E2041,
    E2042,
    E2043,
    E2044,
    E2045,
    E2046,
    E2047,
    E2048,
    E2049,
    E2050,
    E2051,
    E2052,
    E2053,
    E2054,
    E2055,
    E2056,
    E2057,
    E2058,
    E2059,
    E2060,
    E2061,
    E2062,
    E2063,
    E2064,
    E2065,
    E2066,
    E2067,
    E2068,
    E2069,
    E2070,
    E2071
};

template <Warning Code, typename... Args>
inline std::string warn_str(Args&&... args) {
    std::ostringstream ss;
    ss << "[" << Code << "] ";
    if constexpr (Code == W1001) {
        std::cerr << "TODO";
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
        ss << fmt::format("the name or type must start with a capital letter '{}'", args...);
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
        ss << "it is acceptable to use either documentation or inline documentation, but not both";
    }
    if constexpr (Code == E2022) {
        ss << "constants can only be added to an enumeration type";
    }
    if constexpr (Code == E2023) {
        ss << "the 'value' attribute must specify the value in the argument";
    }
    if constexpr (Code == E2024) {
        ss << "the 'value' attribute must contain only one value";
    }
    if constexpr (Code == E2025) {
        ss << "the 'value' attribute must specify an integer.";
    }
    if constexpr (Code == E2026) {
        ss << fmt::format("an enumeration '{}' must contain at least one constant", args...);
    }
    if constexpr (Code == E2027) {
        ss << "fields can only be added to a structured type";
    }
    if constexpr (Code == E2028) {
        ss << "the 'type' attribute must specify the type in the argument";
    }
    if constexpr (Code == E2029) {
        ss << "the 'type' attribute must contain only one type";
    }
    if constexpr (Code == E2030) {
        ss << fmt::format("symbol redefinition '{}'", args...);
    }
    if constexpr (Code == E2031) {
        ss << "enumeration constants can only be specified as integers or enum consts";
    }
    if constexpr (Code == E2032) {
        ss << fmt::format("symbol definition '{}' not found", args...);
    }
    if constexpr (Code == E2033) {
        ss << fmt::format("a constant '{}' cannot refer to itself when evaluated", args...);
    }
    if constexpr (Code == E2034) {
        ss << "constants can only refer to other constants when evaluated";
    }
    if constexpr (Code == E2035) {
        ss << fmt::format("declaration '{}' is not a type", args...);
    }
    if constexpr (Code == E2036) {
        ss << "enumeration constant can only be of type 'Int32'";
    }
    if constexpr (Code == E2037) {
        ss << fmt::format("identifiers are case sensitive, error in '{}', but expected '{}'", args...);
    }
    if constexpr (Code == E2038) {
        ss << "constant cannot go beyond the range of Int32 [-2147483648, 2147483647]";
    }
    if constexpr (Code == E2039) {
        ss << fmt::format("constant '{}' was duplicated", args...);
    }
    if constexpr (Code == E2040) {
        ss << fmt::format("cyclic dependence of constant '{}'", args...);
    }
    if constexpr (Code == E2041) {
        ss << fmt::format("could not find file '{}' for import", args...);
    }
    if constexpr (Code == E2042) {
        ss << fmt::format("failed to open file '{}'", args...);
    }
    if constexpr (Code == E2043) {
        ss << "methods can only be added to a interface type";
    }
    if constexpr (Code == E2044) {
        ss << "arguments can only be added to a method";
    }
    if constexpr (Code == E2045) {
        ss << "out of memory";
    }
    if constexpr (Code == E2046) {
        ss << fmt::format("static method '{}' cannot include argument '{}' with attribute 'this'", args...);
    }
    if constexpr (Code == E2047) {
        ss << fmt::format("constructor '{}' cannot include argument '{}' with attribute 'this'", args...);
    }
    if constexpr (Code == E2048) {
        ss << fmt::format("method '{}' must include one argument with the 'this' attribute.", args...);
    }
    if constexpr (Code == E2049) {
        ss << "the 'get' attribute must specify a reference to the method in the argument";
    }
    if constexpr (Code == E2050) {
        ss << "the 'set' attribute must specify a reference to the method in the argument";
    }
    if constexpr (Code == E2051) {
        ss << fmt::format("argument '{}' of method '{}' cannot be of type 'Void'", args...);
    }
    if constexpr (Code == E2052) {
        ss << fmt::format("the property '{}' must contain at least the 'get' attribute or the 'set' attribute or both",
                          args...);
    }
    if constexpr (Code == E2053) {
        ss << fmt::format("getter '{}' must be a method", args...);
    }
    if constexpr (Code == E2054) {
        ss << fmt::format("property getter '{}' from '{}' refers to a method '{}' from another interface '{}'",
                          args...);
    }
    if constexpr (Code == E2055) {
        ss << fmt::format(
            "if the getter method '{}' is static, then the property '{}' must also be static, and vice versa", args...);
    }
    if constexpr (Code == E2056) {
        ss << fmt::format("a static getter method '{}' must not have arguments", args...);
    }
    if constexpr (Code == E2057) {
        ss << fmt::format("a getter method '{}' must have one argument", args...);
    }
    if constexpr (Code == E2058) {
        ss << fmt::format("getter method {} cannot return 'Void'", args...);
    }
    if constexpr (Code == E2059) {
        ss << fmt::format("setter '{}' must be a method", args...);
    }
    if constexpr (Code == E2060) {
        ss << fmt::format(
            "if the setter method '{}' is static, then the property '{}' must also be static, and vice versa", args...);
    }
    if constexpr (Code == E2061) {
        ss << fmt::format("property setter '{}' from '{}' refers to a method '{}' from another interface '{}'",
                          args...);
    }
    if constexpr (Code == E2062) {
        ss << fmt::format("a static setter method '{}' must have one argument", args...);
    }
    if constexpr (Code == E2063) {
        ss << fmt::format("a setter method '{}' must have two arguments", args...);
    }
    if constexpr (Code == E2064) {
        ss << fmt::format("the return type '{}' of the getter method '{}' is different from the argument type '{}' of "
                          "the setter method "
                          "'{}'",
                          args...);
    }
    if constexpr (Code == E2065) {
        ss << fmt::format("the property type '{}' does not match the return type '{}' of the getter method '{}'",
                          args...);
    }
    if constexpr (Code == E2066) {
        ss << fmt::format("the property type '{}' does not match the setter method '{}' argument type '{}'", args...);
    }
    if constexpr (Code == E2067) {
        ss << fmt::format("failed to create file '{}'", args...);
    }
    if constexpr (Code == E2068) {
        ss << fmt::format("field '{}' of struct '{}' cannot be of type 'Void'", args...);
    }
    if constexpr (Code == E2069) {
        ss << fmt::format("the handle type must be specified for '{}'", args...);
    }
    if constexpr (Code == E2070) {
        ss << fmt::format("the handle type must be struct for '{}'", args...);
    }
    if constexpr (Code == E2071) {
        ss << fmt::format("the structure '{}' specified in the handle type '{}' must be marked with the 'handle' attribute", args...);
    }
    return ss.str();
}

template <Warning Code, typename... Args>
inline void warn(const Location& loc, Args&&... args) {
    std::cout << "warning: " << warn_str<Code>(args...) << " at " << loc << std::endl;
}

template <Error Code, typename... Args>
[[noreturn]] inline void err(const Location& loc, Args&&... args) {
    std::cerr << "error: " << err_str<Code>(args...) << " at " << loc << std::endl;
    exit(1);
}

#endif
