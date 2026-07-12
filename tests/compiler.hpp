#ifndef IDL_COMPILER_HPP
#define IDL_COMPILER_HPP

#include <gtest/gtest.h>

#include <idlc/idl.h>

#include "finally.hpp"

static constexpr idl_ast_node_h HandleNone{ 0xFFFF };

[[nodiscard]] inline bool operator==(const idl_ast_node_h& lhs, const idl_ast_node_h& rhs) noexcept {
    return lhs.handle == rhs.handle;
}

[[nodiscard]] inline bool operator!=(const idl_ast_node_h& lhs, const idl_ast_node_h& rhs) noexcept {
    return lhs.handle != rhs.handle;
}

[[nodiscard]] std::tuple<idl_result_t, idl_compilation_result_t, std::vector<std::string>> compile(
    std::string_view testCase, bool warnAsErrors = false, bool returnMessages = true);

[[nodiscard]] idl_ast_node_h findChild(idl_compilation_result_t result, idl_ast_node_h node, idl_ast_node_type_t type);

[[nodiscard]] std::string getStr(idl_compilation_result_t result, idl_ast_node_h node);

#endif
