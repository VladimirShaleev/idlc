#ifndef GENERATOR_C_HPP
#define GENERATOR_C_HPP

#include "context.hpp"

struct GeneratorC {
    void generate(idl::Context& ctx, const std::filesystem::path& out);
};

#endif
