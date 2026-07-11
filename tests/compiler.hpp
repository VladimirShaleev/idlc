#ifndef IDL_COMPILER_HPP
#define IDL_COMPILER_HPP

#include <gtest/gtest.h>

#include <idlc/idl.h>

struct Source {
    std::string name;
    std::string code;
};

std::pair<idl_result_t, std::vector<std::string>> compile(std::string_view testCase,
                                                          bool warnAsErrors   = false,
                                                          bool returnMessages = true);

#endif
