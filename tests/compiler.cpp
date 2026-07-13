#include "compiler.hpp"

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(idlc::cases);

using namespace std::string_literals;

static idl_source_t* import(idl_utf8_t name, idl_uint32_t depth, idl_data_t data) {
    auto fs = cmrc::idlc::cases::get_filesystem();
    try {
        auto file = fs.open("cases/"s + name + ".idl"s);
        return new idl_source_t{ name, (const char*) file.begin(), idl_uint32_t(file.size()) };
    } catch (const std::system_error& exc) {
        if (exc.code() == make_error_code(std::errc::no_such_file_or_directory)) {
            return nullptr;
        }
        throw;
    }
}

static void releaseImport(idl_source_t* source, idl_data_t data) {
    delete source;
}

std::tuple<idl_result_t, idl_compilation_result_t, std::vector<std::string>> compile(std::string_view testCase,
                                                                                     bool warnAsErrors,
                                                                                     bool returnMessages) {
    idl_options_t options{};
    auto code = idl_options_create(&options);
    if (code != IDL_RESULT_SUCCESS) {
        return { code, nullptr, {} };
    }
    deferred(idl_options_destroy(options));
    idl_options_set_debug_mode(options, 0);
    idl_options_set_warnings_as_errors(options, warnAsErrors);
    idl_options_set_importer(options, import, nullptr);
    idl_options_set_release_import(options, releaseImport, nullptr);

    idl_compiler_t compiler{};
    code = idl_compiler_create(&compiler);
    if (code != IDL_RESULT_SUCCESS) {
        return { code, nullptr, {} };
    }
    deferred(idl_compiler_destroy(compiler));

    idl_compilation_result_t result{};
    idl_compilation_result_t* resultPtr = returnMessages ? &result : nullptr;
    code = idl_compiler_compile(compiler, IDL_GENERATOR_C, testCase.data(), 0, nullptr, options, resultPtr);

    std::vector<std::string> results;
    if (returnMessages) {
        idl_uint32_t count{};
        idl_compilation_result_get_messages(result, &count, nullptr);
        std::vector<idl_message_t> messages;

        messages.resize(count);
        idl_compilation_result_get_messages(result, &count, messages.data());

        results.reserve(count);
        for (const auto& message : messages) {
            std::ostringstream ss;
            std::string status;
            char prefixStatus;
            if (message.status >= IDL_STATUS_E3001) {
                status       = "error";
                prefixStatus = 'E';
            } else if (message.status >= IDL_STATUS_W2001) {
                status       = "warning";
                prefixStatus = 'W';
            } else {
                status       = "note";
                prefixStatus = 'N';
            }
            if (message.is_error) {
                status = "error";
            }
            ss << status << " [" << prefixStatus << (int) message.status << "]: " << message.message;
            if (message.line > 0) {
                ss << " at " << message.filename << ':' << message.line << ':' << message.column;
            }
            results.push_back(ss.str());
        }
    }

    return { code, result, results };
}

idl_ast_node_h findChild(idl_compilation_result_t result, idl_ast_node_h node, idl_ast_node_type_t type) {
    auto curr = idl_compilation_result_get_child_node(result, node);
    while (curr != HandleNone) {
        if (idl_compilation_result_is_node_type(result, curr, type)) {
            return curr;
        }
        curr = idl_compilation_result_get_next_node(result, curr);
    }
    return HandleNone;
}

std::vector<idl_ast_node_h> getChilds(idl_compilation_result_t result, idl_ast_node_h node) {
    std::vector<idl_ast_node_h> childs;
    auto curr = idl_compilation_result_get_child_node(result, node);
    while (curr != HandleNone) {
        childs.push_back(curr);
        curr = idl_compilation_result_get_next_node(result, curr);
    }
    return childs;
}

std::vector<idl_ast_node_h> getChilds(idl_compilation_result_t result, idl_ast_node_h node, idl_ast_node_type_t type) {
    std::vector<idl_ast_node_h> childs;
    auto curr = idl_compilation_result_get_child_node(result, node);
    while (curr != HandleNone) {
        if (idl_compilation_result_is_node_type(result, curr, type)) {
            childs.push_back(curr);
        }
        curr = idl_compilation_result_get_next_node(result, curr);
    }
    return childs;
}

bool isType(idl_compilation_result_t result, idl_ast_node_h node, idl_ast_node_type_t type) {
    return idl_compilation_result_is_node_type(result, node, type);
}

std::vector<idl_ast_node_h> getAttrs(idl_compilation_result_t result, idl_ast_node_h node, AttrFilter filter) {
    std::vector<idl_ast_node_h> attrs;
    auto curr = idl_compilation_result_get_child_node(result, node);
    while (curr != HandleNone) {
        if (idl_compilation_result_is_node_type(result, curr, IDL_AST_NODE_TYPE_ATTR)) {
            if ((filter & AttrFilterAll) == AttrFilterAll) {
                attrs.push_back(curr);
            } else if ((filter & AttrFilterDoc) == AttrFilterDoc) {
                if (idl_compilation_result_is_node_type(result, curr, IDL_AST_NODE_TYPE_ATTR_DOC)) {
                    attrs.push_back(curr);
                }
            } else if ((filter & AttrFilterNonDoc) == AttrFilterNonDoc) {
                if (!idl_compilation_result_is_node_type(result, curr, IDL_AST_NODE_TYPE_ATTR_DOC)) {
                    attrs.push_back(curr);
                }
            }
        }
        curr = idl_compilation_result_get_next_node(result, curr);
    }
    return attrs;
}

bool hasAllState(idl_compilation_result_t result, idl_ast_node_h node, idl_ast_node_state_flags_t state) {
    const auto flags = idl_compilation_result_get_node_state(result, node);
    return (flags & state) == state;
}

bool hasAnyState(idl_compilation_result_t result, idl_ast_node_h node, idl_ast_node_state_flags_t state) {
    const auto flags = idl_compilation_result_get_node_state(result, node);
    return (flags & state) != 0;
}

bool checkConst(idl_compilation_result_t result,
                idl_ast_node_h node,
                uint64_t value,
                bool addedByCompiler,
                bool buildFailed,
                bool forwardDecl) {
    if (node == HandleNone) {
        false;
    }

    if (!hasAllState(result,
                     node,
                     IDL_AST_NODE_STATE_EVAULATED_BIT |
                         (buildFailed ? IDL_AST_NODE_STATE_BUILD_ERROR_BIT : IDL_AST_NODE_STATE_NONE_BIT) |
                         (forwardDecl ? IDL_AST_NODE_STATE_FORWARD_DECL_BIT : IDL_AST_NODE_STATE_NONE_BIT))) {
        return false;
    }

    if (hasAnyState(result,
                    node,
                    IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT | IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT |
                        (buildFailed ? IDL_AST_NODE_STATE_NONE_BIT : IDL_AST_NODE_STATE_BUILD_ERROR_BIT) |
                        (forwardDecl ? IDL_AST_NODE_STATE_NONE_BIT : IDL_AST_NODE_STATE_FORWARD_DECL_BIT))) {
        return false;
    }

    auto evaulated = findChild(result, node, IDL_AST_NODE_TYPE_ATTR_VALUE);
    if (evaulated == HandleNone) {
        return false;
    }

    if (getInt(result, evaulated) != value) {
        return false;
    }

    if (!hasAllState(result,
                     evaulated,
                     addedByCompiler ? IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT : IDL_AST_NODE_STATE_NONE_BIT)) {
        return false;
    }

    if (hasAnyState(result,
                    evaulated,
                    IDL_AST_NODE_STATE_BUILD_ERROR_BIT | IDL_AST_NODE_STATE_FORWARD_DECL_BIT |
                        IDL_AST_NODE_STATE_EVAULATED_BIT | IDL_AST_NODE_STATE_REPLACED_BY_COMPILER_BIT |
                        (addedByCompiler ? IDL_AST_NODE_STATE_NONE_BIT : IDL_AST_NODE_STATE_ADDED_BY_COMPILER_BIT))) {
        return false;
    }

    return true;
}

std::string getStr(idl_compilation_result_t result, idl_ast_node_h node) {
    auto str = idl_compilation_result_get_node_value_str(result, node);
    return str ? std::string(str) : ""s;
}

uint64_t getInt(idl_compilation_result_t result, idl_ast_node_h node) {
    return idl_compilation_result_get_node_value_int(result, node);
}

idl_ast_node_h getDeclRef(idl_compilation_result_t result, idl_ast_node_h node) {
    return idl_compilation_result_get_node_value_decl_ref(result, node);
}
