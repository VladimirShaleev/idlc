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

    void addMessage(const Exception& exc, bool isError = true) {
        _messages.push_back({});
        auto& message    = _messages.back();
        message.status   = exc.status();
        message.is_error = isError ? 1 : 0;
        message.message  = getStr(exc.what());
        message.filename = getStr(exc.filename());
        message.line     = exc.line();
        message.column   = exc.column();
        if (isError) {
            _hasErrors = true;
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
