#ifndef IDL_COMPILATION_RESULT_HPP
#define IDL_COMPILATION_RESULT_HPP

#include "ast.hpp"
#include "errors.hpp"
#include "object.hpp"

struct _idl_compilation_result : public idl::Object {};

namespace idl {

class CompilationResultBase : public _idl_compilation_result {
public:
    CompilationResultBase() {
        _nodes.reserve(1024);
    }

    [[nodiscard]] bool hasNotes() const noexcept {
        return _hasNotes;
    }

    [[nodiscard]] bool hasWarnings() const noexcept {
        return _hasWarnings;
    }

    [[nodiscard]] bool hasErrors() const noexcept {
        return _hasErrors;
    }

    virtual void addMessage(idl_status_t status,
                            bool warnAsError,
                            String filename,
                            idl_uint32_t line,
                            idl_uint32_t column,
                            std::function<std::string(void)> message) = 0;

    virtual void getMessages(idl_uint32_t& messageCount, idl_message_t* messages) const noexcept {
        messageCount = 0;
    }

    [[nodiscard]] ASTNodeHandle allocNode(const ASTLocation& loc, ASTNodeType type, bool addedByCompiler = true) {
        auto index = _nodes.size();
        auto& node = _nodes.emplace_back();

        node.location = loc;
        node.type     = type;
        node.flags    = addedByCompiler ? IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT : IDL_AST_NODE_STATE_NONE_BIT;
        node.parent   = HandleNone;
        node.sibling  = HandleNone;
        node.child    = HandleNone;

        return { uint16_t(index) };
    }

    [[nodiscard]] ASTNode* getNode(ASTNodeHandle node) noexcept {
        if (node.handle < _nodes.size() && node != HandleNone) {
            return &_nodes[node.handle];
        }
        return nullptr;
    }

    [[nodiscard]] const ASTNode* getNode(ASTNodeHandle node) const noexcept {
        if (node.handle < _nodes.size() && node != HandleNone) {
            return &_nodes[node.handle];
        }
        return nullptr;
    }

    [[nodiscard]] String intern(std::string_view str) {
        return _stringPool.insert(str);
    }

    [[nodiscard]] std::string_view getStr(String str) const noexcept {
        return _stringPool[str];
    }

    [[nodiscard]] std::optional<String> findStr(std::string_view str) const noexcept {
        return _stringPool.find(str);
    }

    [[nodiscard]] idl_ast_node_h getApi() const noexcept {
        return _api;
    }

    void setApi(ASTNodeHandle node) noexcept {
        _api = node;
    }

    [[nodiscard]] idl_ast_node_type_t getNodeType(idl_ast_node_h node) const noexcept {
        auto curr = getNode(node);
        return idl_ast_node_type_t(curr ? curr->type : 0);
    }

    [[nodiscard]] idl_ast_node_state_flags_t getNodeState(idl_ast_node_h node) const noexcept {
        auto curr = getNode(node);
        return idl_ast_node_state_flags_t(curr ? curr->flags : 0);
    }

    void getNodeLocation(idl_ast_node_h node, idl_ast_location_t& location) const noexcept {
        auto curr = getNode(node);
        if (curr) {
            location.filename = _stringPool[curr->location.filename].data();
            location.line     = curr->location.line;
            location.column   = curr->location.column;
        } else {
            location.filename = "";
            location.line     = 0;
            location.column   = 0;
        }
    }

    [[nodiscard]] idl_ast_node_h getParentNode(idl_ast_node_h node) const noexcept {
        auto curr = getNode(node);
        return curr ? curr->parent : HandleNone;
    }

    [[nodiscard]] idl_ast_node_h getNextNode(idl_ast_node_h node) const noexcept {
        auto curr = getNode(node);
        return curr ? curr->sibling : HandleNone;
    }

    [[nodiscard]] idl_ast_node_h getChildNode(idl_ast_node_h node) const noexcept {
        auto curr = getNode(node);
        return curr ? curr->child : HandleNone;
    }

    [[nodiscard]] bool isNodeType(idl_ast_node_h node, idl_ast_node_type_t type) const noexcept {
        return ::idl::isNodeType(getNode(node), type);
    }

    [[nodiscard]] idl_utf8_t getNodeValueStr(idl_ast_node_h node) const noexcept {
        if (isNodeType(node, IDL_AST_NODE_TYPE_DECL) || isNodeType(node, IDL_AST_NODE_TYPE_LITERAL_STR)) {
            return _stringPool[_nodes[node.handle].valueStr].data();
        }
        return "";
    }

    [[nodiscard]] idl_uint64_t getNodeValueInt(idl_ast_node_h node) const noexcept {
        if (isNodeType(node, IDL_AST_NODE_TYPE_LITERAL_INT) ||
            (isNodeType(node, IDL_AST_NODE_TYPE_ATTR_VALUE) &&
             isNodeType(getParentNode(node), IDL_AST_NODE_TYPE_CONST))) {
            return _nodes[node.handle].valueInt;
        }
        return 0;
    }

    [[nodiscard]] idl_float64_t getNodeValueFloat(idl_ast_node_h node) const noexcept {
        if (isNodeType(node, IDL_AST_NODE_TYPE_LITERAL_FLOAT)) {
            return _nodes[node.handle].valueFloat;
        }
        return 0.0;
    }

    [[nodiscard]] idl_bool_t getNodeValueBool(idl_ast_node_h node) const noexcept {
        if (isNodeType(node, IDL_AST_NODE_TYPE_LITERAL_BOOL)) {
            return _nodes[node.handle].valueBool;
        }
        return false;
    }

    [[nodiscard]] idl_ast_node_h getNodeValueDeclRef(idl_ast_node_h node) const noexcept {
        if (isNodeType(node, IDL_AST_NODE_TYPE_DECL_REF)) {
            return _nodes[node.handle].valueDeclRef.handle;
        }
        return HandleNone;
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
    StringPool _stringPool;
    ASTNodeHandle _api{ HandleNone };
    std::vector<ASTNode> _nodes{};
};

class CompilationResult final : public CompilationResultBase {
public:
    void addMessage(idl_status_t status,
                    bool warnAsError,
                    String filename,
                    idl_uint32_t line,
                    idl_uint32_t column,
                    std::function<std::string(void)> message) override {
        const auto msgStr    = message();
        const auto msgHandle = intern({ msgStr.c_str(), msgStr.length() });

        _messages.push_back({});
        auto& msg    = _messages.back();
        msg.status   = status;
        msg.is_error = applyStatusAndCheckIsError(status, warnAsError);
        msg.message  = getStr(msgHandle).data();
        msg.filename = getStr(filename).data();
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
    std::vector<idl_message_t> _messages{};
};

class CompilationResultStub final : public CompilationResultBase {
public:
    void addMessage(idl_status_t status,
                    bool warnAsError,
                    String /* filename */,
                    idl_uint32_t /* line */,
                    idl_uint32_t /* column */,
                    std::function<std::string(void)> /* message */) override {
        applyStatusAndCheckIsError(status, warnAsError);
    }
};

} // namespace idl

#endif
