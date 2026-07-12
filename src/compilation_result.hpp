#ifndef IDL_COMPILATION_RESULT_HPP
#define IDL_COMPILATION_RESULT_HPP

#include "ast.hpp"
#include "errors.hpp"
#include "object.hpp"

struct _idl_compilation_result : public idl::Object {};

namespace idl {

class CompilationResultBase : public _idl_compilation_result {
public:
    bool hasNotes() const noexcept {
        return _hasNotes;
    }

    bool hasWarnings() const noexcept {
        return _hasWarnings;
    }

    bool hasErrors() const noexcept {
        return _hasErrors;
    }

    virtual void addMessage(idl_status_t status,
                            bool warnAsError,
                            std::string_view filename,
                            idl_uint32_t line,
                            idl_uint32_t column,
                            std::function<std::string(void)> message) = 0;

    virtual void getMessages(idl_uint32_t& messageCount, idl_message_t* messages) const noexcept {
        messageCount = 0;
    }

    idl_ast_node_h getApi() const noexcept {
        return {};
    }

    idl_ast_node_type_t getNodeType(idl_ast_node_h node) const noexcept {
        return {};
    }

    idl_ast_node_state_flags_t getNodeState(idl_ast_node_h node) const noexcept {
        return {};
    }

    void getNodeLocation(idl_ast_node_h node, idl_ast_location_t& location) const noexcept {
    }

    idl_ast_node_h getParentNode(idl_ast_node_h node) const noexcept {
        return {};
    }

    idl_ast_node_h getNextNode(idl_ast_node_h node) const noexcept {
        return {};
    }

    idl_ast_node_h getChildNode(idl_ast_node_h node) const noexcept {
        return {};
    }

    bool isNodeType(idl_ast_node_h node, idl_ast_node_type_t type) const noexcept {
        return {};
    }

    idl_utf8_t getNodeValueStr(idl_ast_node_h node) const noexcept {
        return {};
    }

    idl_uint64_t getNodeValueInt(idl_ast_node_h node) const noexcept {
        return {};
    }

    idl_float64_t getNodeValueFloat(idl_ast_node_h node) const noexcept {
        return {};
    }

    idl_bool_t getNodeValueBool(idl_ast_node_h node) const noexcept {
        return {};
    }

    idl_ast_node_h getNodeValueDeclRef(idl_ast_node_h node) const noexcept {
        return {};
    }

protected:
    bool applyStatusAndCheckIsError(idl_status_t status, bool warnAsError) noexcept {
        if (status >= IDL_STATUS_E3001 || (warnAsError && status >= IDL_STATUS_W2001 && status < IDL_STATUS_E3001)) {
            _hasErrors = true;
            return true;
        } else if (status >= IDL_STATUS_W2001) {
            _hasWarnings = true;
        } else {
            _hasNotes = true;
        }
        return false;
    }

private:
    bool _hasNotes{};
    bool _hasWarnings{};
    bool _hasErrors{};
    std::vector<ASTNode> _nodes{};
};

class CompilationResult final : public CompilationResultBase {
public:
    void addMessage(idl_status_t status,
                    bool warnAsError,
                    std::string_view filename,
                    idl_uint32_t line,
                    idl_uint32_t column,
                    std::function<std::string(void)> message) override {
        _messages.push_back({});
        auto& msg    = _messages.back();
        msg.status   = status;
        msg.is_error = applyStatusAndCheckIsError(status, warnAsError);
        msg.message  = getStr(message());
        msg.filename = getStr(std::string(filename.data(), filename.length()));
        msg.line     = line;
        msg.column   = column;
    }

    void getMessages(idl_uint32_t& messageCount, idl_message_t* messages) const noexcept override {
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

    std::vector<std::unique_ptr<std::string>> _strPool{};
    std::vector<idl_message_t> _messages{};
};

class CompilationResultStub final : public CompilationResultBase {
public:
    void addMessage(idl_status_t status,
                    bool warnAsError,
                    std::string_view /* filename */,
                    idl_uint32_t /* line */,
                    idl_uint32_t /* column */,
                    std::function<std::string(void)> /* message */) override {
        applyStatusAndCheckIsError(status, warnAsError);
    }
};

} // namespace idl

#endif
