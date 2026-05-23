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
    if constexpr (Status == IDL_STATUS_N1001) {
        str = fmt::format("Note");
    } else if constexpr (Status == IDL_STATUS_W2001) {
        str = fmt::format("Warn");
    } else if constexpr (Status == IDL_STATUS_E3001) {
        str = fmt::format("Err");
    } else if constexpr (Status == IDL_STATUS_E3002) {
        str =
            fmt::format("The version attribute must have three required integer parameters, such as version(1, 2, 3).");
    } else {
        assert(!"unknown status code");
    }
    throw Exception(Status, *loc.begin.filename, loc.begin.line, loc.begin.column, str);
}

} // namespace idl

#endif
