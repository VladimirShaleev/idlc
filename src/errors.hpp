#ifndef IDL_ERRORS_HPP
#define IDL_ERRORS_HPP

#include "idl.hpp"
#include "parser.hpp"

namespace idl {

template <idl_status_t Status, typename... Args>
[[noreturn]] inline std::string err(Args&&... args) {
    std::string str;
    if constexpr (Status == IDL_STATUS_N1001) {
        str = fmt::format("Unnecessary parentheses for a parameterless attribute '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_W2001) {
        str = fmt::format("The declaration '{}' is missing an attribute [{}]", args...);
    } else if constexpr (Status == IDL_STATUS_W2002) {
        str = fmt::format("Repeated import '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_W2003) {
        str = fmt::format("The constant '{}' refers to a constant declared below '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3001) {
        if constexpr (sizeof...(args) > 0) {
            str = fmt::format("Syntax error '{}'", args...);
        } else {
            str = fmt::format("Syntax error");
        }
    } else if constexpr (Status == IDL_STATUS_E3002) {
        str = fmt::format("Argument parsing error '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3003) {
        str = fmt::format("The [version] attribute must have three required integer parameters, such as version(1, "
                          "2, 3) or version(\"string\")");
    } else if constexpr (Status == IDL_STATUS_E3004) {
        str = fmt::format("Version values must be between 0 and 255, while the argument is {}", args...);
    } else if constexpr (Status == IDL_STATUS_E3005) {
        str = fmt::format("Invalid attribute '{}' for {} '{}' declaration, allowed attributes are {}", args...);
    } else if constexpr (Status == IDL_STATUS_E3006) {
        str = fmt::format("Attributes are not allowed for the {} '{}' declaration", args...);
    } else if constexpr (Status == IDL_STATUS_E3007) {
        str = fmt::format("Attribute duplication for attribute '{}' in {} '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3008) {
        str = fmt::format("The attribute '{}' must not have arguments", args...);
    } else if constexpr (Status == IDL_STATUS_E3009) {
        str = fmt::format("String closing character not found in string \"{}\"", args...);
    } else if constexpr (Status == IDL_STATUS_E3010) {
        str = fmt::format("API Redeclaration '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3011) {
        str = fmt::format("The first declaration in the description should always begin with the 'api' declaration");
    } else if constexpr (Status == IDL_STATUS_E3012) {
        str = fmt::format("Symbol redefinition '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3013) {
        str = fmt::format("Unknown attribute '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3014) {
        str = fmt::format("The [brief] attribute must contain one or more arguments");
    } else if constexpr (Status == IDL_STATUS_E3015) {
        str = fmt::format("Unknown attribute in the documentation '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3016) {
        str = fmt::format("The documentation string cannot be empty");
    } else if constexpr (Status == IDL_STATUS_E3017) {
        str = fmt::format("The [detail] attribute must contain one or more arguments");
    } else if constexpr (Status == IDL_STATUS_E3018) {
        str = fmt::format("Inline documentation only [detail] description is allowed");
    } else if constexpr (Status == IDL_STATUS_E3019) {
        str = fmt::format("The [order] attribute can contain one optional Boolean parameter");
    } else if constexpr (Status == IDL_STATUS_E3020) {
        str = fmt::format("Tabs are not allowed");
    } else if constexpr (Status == IDL_STATUS_E3021) {
        str = fmt::format("could not find file '{}' for import", args...);
    } else if constexpr (Status == IDL_STATUS_E3022) {
        str = fmt::format("Failed to open file '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3023) {
        str = fmt::format("A 'const' of '{}' can be defined only for an 'enum'", args...);
    } else if constexpr (Status == IDL_STATUS_E3024) {
        str = fmt::format("The [value] attribute must contain one or more arguments");
    } else if constexpr (Status == IDL_STATUS_E3025) {
        str = fmt::format("Arguments for the [value] attribute must be literals");
    } else if constexpr (Status == IDL_STATUS_E3026) {
        str = fmt::format("All literals in the [value] attribute must be of the same type");
    } else if constexpr (Status == IDL_STATUS_E3027) {
        str = fmt::format("The [type] attribute argument can only refer to symbols");
    } else if constexpr (Status == IDL_STATUS_E3028) {
        str = fmt::format("The [cname] attribute must contain a single string literal argument");
    } else if constexpr (Status == IDL_STATUS_E3029) {
        str =
            fmt::format("The [cname] attribute must specify a name (\"{}\") without spaces and punctuations", args...);
    } else if constexpr (Status == IDL_STATUS_E3030) {
        str = fmt::format("The [single] attribute can contain one optional Boolean parameter");
    } else if constexpr (Status == IDL_STATUS_E3031) {
        str = fmt::format("Invalid tokenizer format string \"{}\", a valid string looks like (2-^3-4)", args...);
    } else if constexpr (Status == IDL_STATUS_E3032) {
        str = fmt::format(
            "Integer tokenization parameters or a tokenizer string must be passed to the attribute [tokenizer]");
    } else if constexpr (Status == IDL_STATUS_E3033) {
        str = fmt::format(
            "The [tokenizer] attribute must contain one or more arguments (integers: 2, -2, 4 or string \"2-^3-4\")");
    } else if constexpr (Status == IDL_STATUS_E3034) {
        str = fmt::format("The [detail] attribute must contain one or more arguments");
    } else if constexpr (Status == IDL_STATUS_E3035) {
        str = fmt::format("The [copyright] attribute must contain one or more arguments");
    } else if constexpr (Status == IDL_STATUS_E3036) {
        str = fmt::format("Identifiers are case sensitive, error in '{}', but expected '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3037) {
        str = fmt::format("Symbol definition '{}' not found", args...);
    } else if constexpr (Status == IDL_STATUS_E3038) {
        str = fmt::format("Constants can only refer to other constants when evaluated");
    } else if constexpr (Status == IDL_STATUS_E3039) {
        str = fmt::format("A constant '{}' cannot refer to itself when evaluated", args...);
    } else if constexpr (Status == IDL_STATUS_E3040) {
        str = fmt::format("Enumeration constants can only be specified as integers or enum consts");
    } else if constexpr (Status == IDL_STATUS_E3041) {
        str = fmt::format("Failed to calculate the constant '{}'", args...);
    } else if constexpr (Status == IDL_STATUS_E3042) {
        str = fmt::format("Cyclic dependence of constant '{}'", args...);
    } else {
        assert(!"Unknown status code");
    }
    return str;
}

} // namespace idl

#endif
