#ifndef GENERATOR_C_HPP
#define GENERATOR_C_HPP

#include "visitors.hpp"

struct GeneratorC {
    void generate(const ASTApi* api, const std::filesystem::path& out);
};

#endif
