#ifndef IDL_COMPILATION_RESULT_HPP
#define IDL_COMPILATION_RESULT_HPP

#include "errors.hpp"
#include "object.hpp"

struct _idl_compilation_result : public idl::Object {};

namespace idl {

class CompilationResult final : public _idl_compilation_result {
public:
    bool hasWarnings() const noexcept {
        return _hasWarnings;
    }

    bool hasErrors() const noexcept {
        return _hasErrors;
    }

    void addMessage(idl_status_t status,
                    std::string_view filename,
                    idl_uint32_t line,
                    idl_uint32_t column,
                    const std::string& message,
                    bool warnAsError) {
        _messages.push_back({});
        auto& msg    = _messages.back();
        msg.status   = status;
        msg.is_error = status >= IDL_STATUS_E3001 ? 1 : 0;
        msg.message  = getStr(message);
        msg.filename = getStr(std::string(filename.data(), filename.length()));
        msg.line     = line;
        msg.column   = column;
        if (msg.is_error || (warnAsError && msg.status >= IDL_STATUS_W2001 && msg.status < IDL_STATUS_E3001)) {
            _hasErrors = true;
            if (!msg.is_error) {
                _hasWarnings = true;
            }
        } else {
            _hasWarnings = true;
        }
    }

    void getMessages(idl_uint32_t& messageCount, idl_message_t* messages) const noexcept {
        if (messages) {
            messageCount = std::min(messageCount, (idl_uint32_t) _messages.size());
            for (idl_uint32_t i = 0; i < messageCount; ++i) {
                messages[i] = _messages[i];
            }
        } else {
            messageCount = (idl_uint32_t) _messages.size();
        }
    }

private:
    idl_utf8_t getStr(const std::string& str) {
        _strPool.push_back(std::make_unique<std::string>(str));
        return _strPool.back()->c_str();
    }

    bool _hasWarnings{};
    bool _hasErrors{};
    std::vector<std::unique_ptr<std::string>> _strPool{};
    std::vector<idl_message_t> _messages{};
};

} // namespace idl

#endif
