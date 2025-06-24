#ifndef IDL_ERRORS_HPP
#define IDL_ERRORS_HPP

#include "idl.hpp"
#include "parser.hpp"

namespace idl {

class Exception : public std::runtime_error {
public:
    Exception(idl_result_t code, const idl::location& loc, const std::string& message) :
        std::runtime_error(message),
        _result(code),
        _location(loc) {
    }

    idl_result_t code() const noexcept {
        return _result;
    }

    const idl::location& location() const noexcept {
        return _location;
    }

private:
    idl_result_t _result;
    idl::location _location;
};

template <idl_result_t Code, typename... Args>
inline std::string _warn_str(Args&&... args) {
    std::ostringstream ss;
    ss << "[W" << Code << "] ";
    if constexpr (Code == IDL_RESULT_W1001) {
        ss << fmt::format("there is no information about the author ('author' attribte) in the '{}' declaration",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_W1002) {
        ss << fmt::format("the declaration '{}' does not contain information about copyright (attribute 'copyright')",
                          args...);
    }
    return ss.str();
}

template <idl_result_t Code, typename... Args>
inline std::string _err_str(Args&&... args) {
    std::ostringstream ss;
    ss << "[E" << Code << "] ";
    if constexpr (Code == IDL_RESULT_E2001) {
        ss << fmt::format("unexpected character '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2002) {
        ss << "tabs are not allowed";
    }
    if constexpr (Code == IDL_RESULT_E2003) {
        ss << fmt::format("the name or type must start with a capital letter '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2004) {
        ss << "there can only be one api declaration";
    }
    if constexpr (Code == IDL_RESULT_E2005) {
        ss << fmt::format("there is no documentation in the declaration '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2006) {
        ss << "documentation cannot be an empty string";
    }
    if constexpr (Code == IDL_RESULT_E2007) {
        ss << "the brief should only be listed once in the documentation";
    }
    if constexpr (Code == IDL_RESULT_E2008) {
        ss << "the detail should only be listed once in the documentation";
    }
    if constexpr (Code == IDL_RESULT_E2009) {
        ss << "the copyright should only be listed once in the documentation";
    }
    if constexpr (Code == IDL_RESULT_E2010) {
        ss << "the license should only be listed once in the documentation";
    }
    if constexpr (Code == IDL_RESULT_E2011) {
        ss << "multi-line documentation must start with 4 spaces";
    }
    if constexpr (Code == IDL_RESULT_E2012) {
        ss << "the .idl file must start with the 'api' element";
    }
    if constexpr (Code == IDL_RESULT_E2013) {
        ss << fmt::format("attribute '{}' cannot be duplicated", args...);
    }
    if constexpr (Code == IDL_RESULT_E2014) {
        ss << fmt::format("the following attributes: {} - are allowed in this context", args...);
    }
    if constexpr (Code == IDL_RESULT_E2015) {
        ss << fmt::format("unknown attribute '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2016) {
        ss << "the 'platform' attribute must specify at least one argument";
    }
    if constexpr (Code == IDL_RESULT_E2017) {
        ss << fmt::format("the following arguments: {} - are allowed in 'platform' attribute", args...);
    }
    if constexpr (Code == IDL_RESULT_E2018) {
        ss << fmt::format("argument '{}' in the 'platform' attribute cannot be duplicated", args...);
    }
    if constexpr (Code == IDL_RESULT_E2019) {
        ss << "inline documentation only [detail] description is allowed";
    }
    if constexpr (Code == IDL_RESULT_E2020) {
        ss << fmt::format("invalid attribute {} in documentation", args...);
    }
    if constexpr (Code == IDL_RESULT_E2021) {
        ss << "it is acceptable to use either documentation or inline documentation, but not both";
    }
    if constexpr (Code == IDL_RESULT_E2022) {
        ss << "constants can only be added to an enumeration type";
    }
    if constexpr (Code == IDL_RESULT_E2023) {
        ss << "the 'value' attribute must specify the value in the argument";
    }
    if constexpr (Code == IDL_RESULT_E2024) {
        ss << "the 'value' attribute must contain only one value";
    }
    if constexpr (Code == IDL_RESULT_E2025) {
        ss << "the 'value' attribute must specify an integer.";
    }
    if constexpr (Code == IDL_RESULT_E2026) {
        ss << fmt::format("an enumeration '{}' must contain at least one constant", args...);
    }
    if constexpr (Code == IDL_RESULT_E2027) {
        ss << "fields can only be added to a structured type";
    }
    if constexpr (Code == IDL_RESULT_E2028) {
        ss << "the 'type' attribute must specify the type in the argument";
    }
    if constexpr (Code == IDL_RESULT_E2029) {
        ss << "the 'type' attribute must contain only one type";
    }
    if constexpr (Code == IDL_RESULT_E2030) {
        ss << fmt::format("symbol redefinition '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2031) {
        ss << "enumeration constants can only be specified as integers or enum consts";
    }
    if constexpr (Code == IDL_RESULT_E2032) {
        ss << fmt::format("symbol definition '{}' not found", args...);
    }
    if constexpr (Code == IDL_RESULT_E2033) {
        ss << fmt::format("a constant '{}' cannot refer to itself when evaluated", args...);
    }
    if constexpr (Code == IDL_RESULT_E2034) {
        ss << "constants can only refer to other constants when evaluated";
    }
    if constexpr (Code == IDL_RESULT_E2035) {
        ss << fmt::format("declaration '{}' is not a type", args...);
    }
    if constexpr (Code == IDL_RESULT_E2036) {
        ss << "enumeration constant can only be of type 'Int32'";
    }
    if constexpr (Code == IDL_RESULT_E2037) {
        ss << fmt::format("identifiers are case sensitive, error in '{}', but expected '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2038) {
        ss << "constant cannot go beyond the range of Int32 [-2147483648, 2147483647]";
    }
    if constexpr (Code == IDL_RESULT_E2039) {
        ss << fmt::format("constant '{}' was duplicated", args...);
    }
    if constexpr (Code == IDL_RESULT_E2040) {
        ss << fmt::format("cyclic dependence of constant '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2041) {
        ss << fmt::format("could not find file '{}' for import", args...);
    }
    if constexpr (Code == IDL_RESULT_E2042) {
        ss << fmt::format("failed to open file '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2043) {
        ss << "methods can only be added to a interface type";
    }
    if constexpr (Code == IDL_RESULT_E2044) {
        ss << "arguments can only be added to a method, function or callback";
    }
    if constexpr (Code == IDL_RESULT_E2045) {
        ss << "out of memory";
    }
    if constexpr (Code == IDL_RESULT_E2046) {
        ss << fmt::format("static method '{}' cannot include argument '{}' with attribute 'this'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2047) {
        ss << fmt::format("constructor '{}' cannot include argument '{}' with attribute 'this'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2048) {
        ss << fmt::format("method '{}' must include one argument with the 'this' attribute.", args...);
    }
    if constexpr (Code == IDL_RESULT_E2049) {
        ss << "the 'get' attribute must specify a reference to the method in the argument";
    }
    if constexpr (Code == IDL_RESULT_E2050) {
        ss << "the 'set' attribute must specify a reference to the method in the argument";
    }
    if constexpr (Code == IDL_RESULT_E2051) {
        ss << fmt::format("argument '{}' of method '{}' cannot be of type 'Void'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2052) {
        ss << fmt::format("the property '{}' must contain at least the 'get' attribute or the 'set' attribute or both",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2053) {
        ss << fmt::format("getter '{}' must be a method", args...);
    }
    if constexpr (Code == IDL_RESULT_E2054) {
        ss << fmt::format("property getter '{}' from '{}' refers to a method '{}' from another interface '{}'",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2055) {
        ss << fmt::format(
            "if the getter method '{}' is static, then the property '{}' must also be static, and vice versa", args...);
    }
    if constexpr (Code == IDL_RESULT_E2056) {
        ss << fmt::format("a static getter method '{}' must not have arguments", args...);
    }
    if constexpr (Code == IDL_RESULT_E2057) {
        ss << fmt::format("a getter method '{}' must have one argument", args...);
    }
    if constexpr (Code == IDL_RESULT_E2058) {
        ss << fmt::format("getter method {} cannot return 'Void'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2059) {
        ss << fmt::format("setter '{}' must be a method", args...);
    }
    if constexpr (Code == IDL_RESULT_E2060) {
        ss << fmt::format(
            "if the setter method '{}' is static, then the property '{}' must also be static, and vice versa", args...);
    }
    if constexpr (Code == IDL_RESULT_E2061) {
        ss << fmt::format("property setter '{}' from '{}' refers to a method '{}' from another interface '{}'",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2062) {
        ss << fmt::format("a static setter method '{}' must have one argument", args...);
    }
    if constexpr (Code == IDL_RESULT_E2063) {
        ss << fmt::format("a setter method '{}' must have two arguments", args...);
    }
    if constexpr (Code == IDL_RESULT_E2064) {
        ss << fmt::format("the return type '{}' of the getter method '{}' is different from the argument type '{}' of "
                          "the setter method "
                          "'{}'",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2065) {
        ss << fmt::format("the property type '{}' does not match the return type '{}' of the getter method '{}'",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2066) {
        ss << fmt::format("the property type '{}' does not match the setter method '{}' argument type '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2067) {
        ss << fmt::format("failed to create file '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2068) {
        ss << fmt::format("field '{}' of struct '{}' cannot be of type 'Void'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2069) {
        ss << fmt::format("the handle type must be specified for '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2070) {
        ss << fmt::format("the handle type must be struct for '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2071) {
        ss << fmt::format(
            "the structure '{}' specified in the handle type '{}' must be marked with the 'handle' attribute", args...);
    }
    if constexpr (Code == IDL_RESULT_E2072) {
        ss << "function can only be added to a interface type";
    }
    if constexpr (Code == IDL_RESULT_E2073) {
        ss << fmt::format("function '{}' argument '{}' cannot be marked with the 'this' attribute", args...);
    }
    if constexpr (Code == IDL_RESULT_E2074) {
        ss << fmt::format("argument '{}' of function '{}' cannot be of type 'Void'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2075) {
        ss << "the 'cname' attribute must specify a string in the argument";
    }
    if constexpr (Code == IDL_RESULT_E2076) {
        ss << "the 'array' attribute must specify a size in the argument";
    }
    if constexpr (Code == IDL_RESULT_E2077) {
        ss << fmt::format("fixed size array '{}' of structure '{}' must be of size 1 or more", args...);
    }
    if constexpr (Code == IDL_RESULT_E2078) {
        ss << fmt::format(
            "the 'array' attribute of the '{}' must point to a field of the structure or set fixed size value",
            args...);
    }
    if constexpr (Code == IDL_RESULT_E2079) {
        ss << "the reference to the dynamic size array is located outside the visibility of the structure";
    }
    if constexpr (Code == IDL_RESULT_E2080) {
        ss << fmt::format("the 'array' attribute for array '{}' must point to an integer field for a dynamic array",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2081) {
        ss << fmt::format("an struct '{}' must contain at least one field", args...);
    }
    if constexpr (Code == IDL_RESULT_E2082) {
        ss << "there can be only one argument with the 'userdata' attribute";
    }
    if constexpr (Code == IDL_RESULT_E2083) {
        ss << fmt::format("callback '{}' argument '{}' cannot be marked with the 'this' attribute", args...);
    }
    if constexpr (Code == IDL_RESULT_E2084) {
        ss << "there can be only one argument with the 'result' attribute";
    }
    if constexpr (Code == IDL_RESULT_E2085) {
        ss << "The function to convert an error code to a string must return a string and take one argument (the error "
              "code)";
    }
    if constexpr (Code == IDL_RESULT_E2086) {
        ss << "The method for incrementing the reference counter of an object must be non-static and take one "
              "argument 'this'";
    }
    if constexpr (Code == IDL_RESULT_E2087) {
        ss << "The method for destroy of an object must be non-static and take one argument 'this'";
    }
    if constexpr (Code == IDL_RESULT_E2088) {
        ss << "there can only be one method to increment reference counter";
    }
    if constexpr (Code == IDL_RESULT_E2089) {
        ss << "there can only be one method to destroy object";
    }
    if constexpr (Code == IDL_RESULT_E2090) {
        ss << "events can only be added to a interface type";
    }
    if constexpr (Code == IDL_RESULT_E2091) {
        ss << fmt::format("the event '{}' must contain at least the 'get' attribute or the 'set' attribute or both",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2092) {
        ss << fmt::format("event getter '{}' from '{}' refers to a method '{}' from another interface '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2093) {
        ss << fmt::format(
            "if the getter method '{}' is static, then the event '{}' must also be static, and vice versa", args...);
    }
    if constexpr (Code == IDL_RESULT_E2094) {
        ss << fmt::format("static getter '{}' for event must have no arguments or one argument 'userdata'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2095) {
        ss << fmt::format("getter '{}' for event must have one arguments or two arguments 'this' and 'userdata'",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2096) {
        ss << fmt::format("event setter '{}' from '{}' refers to a method '{}' from another interface '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2097) {
        ss << fmt::format(
            "if the setter method '{}' is static, then the event '{}' must also be static, and vice versa", args...);
    }
    if constexpr (Code == IDL_RESULT_E2098) {
        ss << fmt::format("static setter '{}' for event must have one argument or setter and 'userdata' arguments",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2099) {
        ss << fmt::format("setter '{}' for event must have two arguments 'this' and 'setter' or three arguments "
                          "'this', 'setter' and 'userdata'",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2100) {
        ss << fmt::format("the event type '{}' does not match the return type '{}' of the getter method '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2101) {
        ss << fmt::format("the event type '{}' does not match the setter method '{}' argument type '{}'", args...);
    }
    if constexpr (Code == IDL_RESULT_E2102) {
        ss << fmt::format("the argument '{}' of a method, function, or callback ('{}') cannot be a fixed-size array",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2103) {
        ss << "the reference to the dynamic size array is located outside the visibility of the method";
    }
    if constexpr (Code == IDL_RESULT_E2104) {
        ss << fmt::format("the 'array' attribute of the '{}' must point to a argument of the method", args...);
    }
    if constexpr (Code == IDL_RESULT_E2105) {
        ss << "the reference to the dynamic size array is located outside the visibility of the function";
    }
    if constexpr (Code == IDL_RESULT_E2106) {
        ss << fmt::format("the 'array' attribute of the '{}' must point to a argument of the function", args...);
    }
    if constexpr (Code == IDL_RESULT_E2107) {
        ss << "the reference to the dynamic size array is located outside the visibility of the callback";
    }
    if constexpr (Code == IDL_RESULT_E2108) {
        ss << fmt::format("the 'array' attribute of the '{}' must point to a argument of the callback", args...);
    }
    if constexpr (Code == IDL_RESULT_E2109) {
        ss << "the 'tokenizer' attribute must specify a indices string in the argument";
    }
    if constexpr (Code == IDL_RESULT_E2110) {
        ss << "the 'version' attribute must specify a semver in the argument";
    }
    if constexpr (Code == IDL_RESULT_E2111) {
        ss << fmt::format("the '{}' declaration does not have a brief ('brief' attribute) or detailed description "
                          "('detail' attribute)",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2112) {
        ss << fmt::format("it is not possible to add the 'noerror' attribute to the '{}' constant because the '{}' "
                          "enum does not have the 'errorcode' attribute.",
                          args...);
    }
    if constexpr (Code == IDL_RESULT_E2113) {
        ss << "unknown error";
    }
    return ss.str();
}

template <idl_result_t Code, typename... Args>
inline void warn(const idl::location& loc, Args&&... args) {
    throw Exception(Code, loc, _warn_str<Code>(args...));
}

template <idl_result_t Code, typename... Args>
[[noreturn]] inline void err(const idl::location& loc, Args&&... args) {
    throw Exception(Code, loc, _err_str<Code>(args...));
}

} // namespace idl

#endif
