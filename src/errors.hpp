#ifndef IDL_ERRORS_HPP
#define IDL_ERRORS_HPP

#include "idl.hpp"
#include "parser.hpp"

namespace idl {

class Exception : public std::runtime_error {
public:
    Exception(
        idl_status_t status, std::string filename, idl_uint32_t line, idl_uint32_t column, const std::string& message) :
        std::runtime_error(message),
        _filename(filename),
        _line(line),
        _column(column),
        _status(status) {
    }

    const std::string& filename() const noexcept {
        return _filename;
    }

    idl_uint32_t line() const noexcept {
        return _line;
    }

    idl_uint32_t column() const noexcept {
        return _column;
    }

    idl_status_t status() const noexcept {
        return _status;
    }

private:
    std::string _filename;
    idl_uint32_t _line;
    idl_uint32_t _column;
    idl_status_t _status;
};

template <idl_status_t Status, typename... Args>
[[noreturn]] inline void err(const idl::location& loc, Args&&... args) {
    std::string str;
    if constexpr (Status == IDL_STATUS_W1001) {
        str = fmt::format("there is no information about the author ('author' attribute) in the '{}' declaration",
                          args...);
    } else if constexpr (Status == IDL_STATUS_W1002) {
        str = fmt::format("the declaration '{}' does not contain information about copyright (attribute 'copyright')",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2001) {
        str = fmt::format("unexpected character '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2002) {
        str = "tabs are not allowed";
    } else if constexpr (Status == IDL_STATUS_E2003) {
        str = fmt::format("the name or type must start with a capital letter '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2004) {
        str = "there can only be one api declaration";
    } else if constexpr (Status == IDL_STATUS_E2005) {
        str = fmt::format("there is no documentation in the declaration '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2006) {
        str = "documentation cannot be an empty string";
    } else if constexpr (Status == IDL_STATUS_E2007) {
        str = "the brief should only be listed once in the documentation";
    } else if constexpr (Status == IDL_STATUS_E2008) {
        str = "the detail should only be listed once in the documentation";
    } else if constexpr (Status == IDL_STATUS_E2009) {
        str = "the copyright should only be listed once in the documentation";
    } else if constexpr (Status == IDL_STATUS_E2010) {
        str = "the license should only be listed once in the documentation";
    } else if constexpr (Status == IDL_STATUS_E2011) {
        str = "unknown error";
    } else if constexpr (Status == IDL_STATUS_E2012) {
        str = "the .idl file must start with the 'api' element";
    } else if constexpr (Status == IDL_STATUS_E2013) {
        str = fmt::format("attribute '{}' cannot be duplicated", args...);
    } else if constexpr (Status == IDL_STATUS_E2014) {
        str = fmt::format("the following attributes: {} - are allowed in this context", args...);
    } else if constexpr (Status == IDL_STATUS_E2015) {
        str = fmt::format("unknown attribute '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2016) {
        str = "the 'platform' attribute must specify at least one argument";
    } else if constexpr (Status == IDL_STATUS_E2017) {
        str = fmt::format("the following arguments: {} - are allowed in 'platform' attribute", args...);
    } else if constexpr (Status == IDL_STATUS_E2018) {
        str = fmt::format("argument '{}' in the 'platform' attribute cannot be duplicated", args...);
    } else if constexpr (Status == IDL_STATUS_E2019) {
        str = "inline documentation only [detail] description is allowed";
    } else if constexpr (Status == IDL_STATUS_E2020) {
        str = fmt::format("invalid attribute {} in documentation", args...);
    } else if constexpr (Status == IDL_STATUS_E2021) {
        str = "it is acceptable to use either documentation or inline documentation, but not both";
    } else if constexpr (Status == IDL_STATUS_E2022) {
        str = "constants can only be added to an enumeration type";
    } else if constexpr (Status == IDL_STATUS_E2023) {
        str = "the 'value' attribute must specify the value in the argument";
    } else if constexpr (Status == IDL_STATUS_E2024) {
        str = "the 'value' attribute must contain only one value";
    } else if constexpr (Status == IDL_STATUS_E2025) {
        str = "the 'value' attribute must specify an integer.";
    } else if constexpr (Status == IDL_STATUS_E2026) {
        str = fmt::format("an enumeration '{}' must contain at least one constant", args...);
    } else if constexpr (Status == IDL_STATUS_E2027) {
        str = "fields can only be added to a structured type";
    } else if constexpr (Status == IDL_STATUS_E2028) {
        str = "the 'type' attribute must specify the type in the argument";
    } else if constexpr (Status == IDL_STATUS_E2029) {
        str = "the 'type' attribute must contain only one type";
    } else if constexpr (Status == IDL_STATUS_E2030) {
        str = fmt::format("symbol redefinition '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2031) {
        str = "enumeration constants can only be specified as integers or enum consts";
    } else if constexpr (Status == IDL_STATUS_E2032) {
        str = fmt::format("symbol definition '{}' not found", args...);
    } else if constexpr (Status == IDL_STATUS_E2033) {
        str = fmt::format("a constant '{}' cannot refer to itself when evaluated", args...);
    } else if constexpr (Status == IDL_STATUS_E2034) {
        str = "constants can only refer to other constants when evaluated";
    } else if constexpr (Status == IDL_STATUS_E2035) {
        str = fmt::format("declaration '{}' is not a type", args...);
    } else if constexpr (Status == IDL_STATUS_E2036) {
        str = "enumeration constant can only be of type 'Int32'";
    } else if constexpr (Status == IDL_STATUS_E2037) {
        str = fmt::format("identifiers are case sensitive, error in '{}', but expected '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2038) {
        str = "constant cannot go beyond the range of Int32 [-2147483648, 2147483647]";
    } else if constexpr (Status == IDL_STATUS_E2039) {
        str = fmt::format("constant '{}' was duplicated", args...);
    } else if constexpr (Status == IDL_STATUS_E2040) {
        str = fmt::format("cyclic dependence of constant '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2041) {
        str = fmt::format("could not find file '{}' for import", args...);
    } else if constexpr (Status == IDL_STATUS_E2042) {
        str = fmt::format("failed to open file '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2043) {
        str = "methods can only be added to a interface type";
    } else if constexpr (Status == IDL_STATUS_E2044) {
        str = "arguments can only be added to a method, function or callback";
    } else if constexpr (Status == IDL_STATUS_E2045) {
        str = "out of memory";
    } else if constexpr (Status == IDL_STATUS_E2046) {
        str = fmt::format("static method '{}' cannot include argument '{}' with attribute 'this'", args...);
    } else if constexpr (Status == IDL_STATUS_E2047) {
        str = fmt::format("constructor '{}' cannot include argument '{}' with attribute 'this'", args...);
    } else if constexpr (Status == IDL_STATUS_E2048) {
        str = fmt::format("method '{}' must include one argument with the 'this' attribute.", args...);
    } else if constexpr (Status == IDL_STATUS_E2049) {
        str = "the 'get' attribute must specify a reference to the method in the argument";
    } else if constexpr (Status == IDL_STATUS_E2050) {
        str = "the 'set' attribute must specify a reference to the method in the argument";
    } else if constexpr (Status == IDL_STATUS_E2051) {
        str = fmt::format("argument '{}' of method '{}' cannot be of type 'Void'", args...);
    } else if constexpr (Status == IDL_STATUS_E2052) {
        str = fmt::format("the property '{}' must contain at least the 'get' attribute or the 'set' attribute or both",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2053) {
        str = fmt::format("getter '{}' must be a method", args...);
    } else if constexpr (Status == IDL_STATUS_E2054) {
        str =
            fmt::format("property getter '{}' from '{}' refers to a method '{}' from another interface '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2055) {
        str = fmt::format(
            "if the getter method '{}' is static, then the property '{}' must also be static, and vice versa", args...);
    } else if constexpr (Status == IDL_STATUS_E2056) {
        str = fmt::format("a static getter method '{}' must not have arguments", args...);
    } else if constexpr (Status == IDL_STATUS_E2057) {
        str = fmt::format("a getter method '{}' must have one argument", args...);
    } else if constexpr (Status == IDL_STATUS_E2058) {
        str = fmt::format("getter method '{}' cannot return 'Void'", args...);
    } else if constexpr (Status == IDL_STATUS_E2059) {
        str = fmt::format("setter '{}' must be a method", args...);
    } else if constexpr (Status == IDL_STATUS_E2060) {
        str = fmt::format(
            "if the setter method '{}' is static, then the property '{}' must also be static, and vice versa", args...);
    } else if constexpr (Status == IDL_STATUS_E2061) {
        str =
            fmt::format("property setter '{}' from '{}' refers to a method '{}' from another interface '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2062) {
        str = fmt::format("a static setter method '{}' must have one argument", args...);
    } else if constexpr (Status == IDL_STATUS_E2063) {
        str = fmt::format("a setter method '{}' must have two arguments", args...);
    } else if constexpr (Status == IDL_STATUS_E2064) {
        str = fmt::format("the return type '{}' of the getter method '{}' is different from the argument type '{}' of "
                          "the setter method "
                          "'{}'",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2065) {
        str = fmt::format("the property type '{}' does not match the return type '{}' of the getter method '{}'",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2066) {
        str = fmt::format("the property type '{}' does not match the setter method '{}' argument type '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2067) {
        str = fmt::format("failed to create file '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2068) {
        str = fmt::format("field '{}' of struct '{}' cannot be of type 'Void'", args...);
    } else if constexpr (Status == IDL_STATUS_E2069) {
        str = fmt::format("the handle type must be specified for '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2070) {
        str = fmt::format("the handle type must be struct for '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2071) {
        str = fmt::format(
            "the structure '{}' specified in the handle type '{}' must be marked with the 'handle' attribute", args...);
    } else if constexpr (Status == IDL_STATUS_E2072) {
        str = fmt::format("it is not possible to add the 'noerror' attribute to the '{}' constant because the '{}' "
                          "enum does not have the 'errorcode' attribute.",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2073) {
        str = fmt::format("function '{}' argument '{}' cannot be marked with the 'this' attribute", args...);
    } else if constexpr (Status == IDL_STATUS_E2074) {
        str = fmt::format("argument '{}' of function '{}' cannot be of type 'Void'", args...);
    } else if constexpr (Status == IDL_STATUS_E2075) {
        str = "the 'cname' attribute must specify a string in the argument";
    } else if constexpr (Status == IDL_STATUS_E2076) {
        str = "the 'array' attribute must specify a size in the argument";
    } else if constexpr (Status == IDL_STATUS_E2077) {
        str = fmt::format("fixed size array '{}' of structure '{}' must be of size 1 or more", args...);
    } else if constexpr (Status == IDL_STATUS_E2078) {
        str = fmt::format(
            "the 'array' attribute of the '{}' must point to a field of the structure or set fixed size value",
            args...);
    } else if constexpr (Status == IDL_STATUS_E2079) {
        str = "the reference to the dynamic size array is located outside the visibility of the structure";
    } else if constexpr (Status == IDL_STATUS_E2080) {
        str = fmt::format("the 'array' attribute for array '{}' must point to an integer field for a dynamic array",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2081) {
        str = fmt::format("an struct '{}' must contain at least one field", args...);
    } else if constexpr (Status == IDL_STATUS_E2082) {
        str = "there can be only one argument with the 'userdata' attribute";
    } else if constexpr (Status == IDL_STATUS_E2083) {
        str = fmt::format("callback '{}' argument '{}' cannot be marked with the 'this' attribute", args...);
    } else if constexpr (Status == IDL_STATUS_E2084) {
        str = "there can be only one argument with the 'result' attribute";
    } else if constexpr (Status == IDL_STATUS_E2085) {
        str = "The function to convert an error code to a string must return a string and take one argument (the error "
              "code)";
    } else if constexpr (Status == IDL_STATUS_E2086) {
        str = "The method for incrementing the reference counter ('refinc' attribute) of an object must be non-static "
              "and take one argument 'this'";
    } else if constexpr (Status == IDL_STATUS_E2087) {
        str = "The method for destroy of an object must be non-static and take one argument 'this'";
    } else if constexpr (Status == IDL_STATUS_E2088) {
        str = "there can only be one method to increment reference counter";
    } else if constexpr (Status == IDL_STATUS_E2089) {
        str = "there can only be one method to destroy object";
    } else if constexpr (Status == IDL_STATUS_E2090) {
        str = "events can only be added to a interface type";
    } else if constexpr (Status == IDL_STATUS_E2091) {
        str = fmt::format("the event '{}' must contain at least the 'get' attribute or the 'set' attribute or both",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2092) {
        str = fmt::format("event getter '{}' from '{}' refers to a method '{}' from another interface '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2093) {
        str = fmt::format(
            "if the getter method '{}' is static, then the event '{}' must also be static, and vice versa", args...);
    } else if constexpr (Status == IDL_STATUS_E2094) {
        str = fmt::format("static getter '{}' for event must have no arguments or one argument 'userdata'", args...);
    } else if constexpr (Status == IDL_STATUS_E2095) {
        str = fmt::format("getter '{}' for event must have one arguments or two arguments 'this' and 'userdata'",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2096) {
        str = fmt::format("event setter '{}' from '{}' refers to a method '{}' from another interface '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2097) {
        str = fmt::format(
            "if the setter method '{}' is static, then the event '{}' must also be static, and vice versa", args...);
    } else if constexpr (Status == IDL_STATUS_E2098) {
        str = fmt::format("static setter '{}' for event must have one argument or setter and 'userdata' arguments",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2099) {
        str = fmt::format("setter '{}' for event must have two arguments 'this' and 'setter' or three arguments "
                          "'this', 'setter' and 'userdata'",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2100) {
        str = fmt::format("the event type '{}' does not match the return type '{}' of the getter method '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2101) {
        str = fmt::format("the event type '{}' does not match the setter method '{}' argument type '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E2102) {
        str = fmt::format("the argument '{}' of a method, function, or callback ('{}') cannot be a fixed-size array",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2103) {
        str = "the reference to the dynamic size array is located outside the visibility of the method";
    } else if constexpr (Status == IDL_STATUS_E2104) {
        str = fmt::format("the 'array' attribute of the '{}' must point to a argument of the method", args...);
    } else if constexpr (Status == IDL_STATUS_E2105) {
        str = "the reference to the dynamic size array is located outside the visibility of the function";
    } else if constexpr (Status == IDL_STATUS_E2106) {
        str = fmt::format("the 'array' attribute of the '{}' must point to a argument of the function", args...);
    } else if constexpr (Status == IDL_STATUS_E2107) {
        str = "the reference to the dynamic size array is located outside the visibility of the callback";
    } else if constexpr (Status == IDL_STATUS_E2108) {
        str = fmt::format("the 'array' attribute of the '{}' must point to a argument of the callback", args...);
    } else if constexpr (Status == IDL_STATUS_E2109) {
        str = "the 'tokenizer' attribute must specify a indices string in the argument";
    } else if constexpr (Status == IDL_STATUS_E2110) {
        str = "the 'version' attribute must specify a semver in the argument";
    } else if constexpr (Status == IDL_STATUS_E2111) {
        str = fmt::format("the '{}' declaration does not have a brief ('brief' attribute) or detailed description "
                          "('detail' attribute)",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2112) {
        str = "the 'datasize' attribute must specify a size in the argument";
    } else if constexpr (Status == IDL_STATUS_E2113) {
        str = fmt::format("the 'datasize' attribute of the '{}' must point to a field of the structure", args...);
    } else if constexpr (Status == IDL_STATUS_E2114) {
        str = fmt::format(
            "the 'datasize' attribute '{}' of the must point to an integer field to specify the buffer size", args...);
    } else if constexpr (Status == IDL_STATUS_E2115) {
        str = fmt::format("the 'datasize' attribute of the '{}' must point to a argument of the method", args...);
    } else if constexpr (Status == IDL_STATUS_E2116) {
        str = fmt::format("the 'datasize' attribute of the '{}' must point to a argument of the function", args...);
    } else if constexpr (Status == IDL_STATUS_E2117) {
        str = fmt::format("the 'datasize' attribute of the '{}' must point to a argument of the callback", args...);
    } else if constexpr (Status == IDL_STATUS_E2118) {
        str = "the reference to the size buffer is located outside the visibility of the structure";
    } else if constexpr (Status == IDL_STATUS_E2119) {
        str = fmt::format("attribute 'datasize' cannot be attached to the '{}' field of the '{}' structure, the attribute "
                          "is only applicable to 'Data' or 'ConstData' types",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2120) {
        str = "the reference to the size buffer is located outside the visibility of the callback";
    } else if constexpr (Status == IDL_STATUS_E2121) {
        str = fmt::format("attribute 'datasize' cannot be attached to the '{}' arg of the '{}', the attribute "
                          "is only applicable to 'Data' or 'ConstData' types",
                          args...);
    } else if constexpr (Status == IDL_STATUS_E2122) {
        str = "the reference to the size buffer is located outside the visibility of the function";
    } else if constexpr (Status == IDL_STATUS_E2123) {
        str = "the reference to the size buffer is located outside the visibility of the method";
    } else if constexpr (Status == IDL_STATUS_E2124) {
        str = fmt::format("the declaration '{}' can only specify the 'array' or 'datasize' attribute, but not both.",
                          args...);
    } else {
        static_assert(false, "unknown status code");
    }
    throw Exception(Status, *loc.begin.filename, loc.begin.line, loc.begin.column, str);
}

} // namespace idl

#endif
