#include "generator_c.hpp"
#include "case_converter.hpp"

static void generateEnum(ASTEnum* node) {
    const auto isHexOut = node->findAttr<ASTAttrHex>() != nullptr;
    CName name{};
    std::vector<std::pair<std::string, std::string>> consts;
    consts.reserve(node->consts.size());
    size_t maxLength = 0;
    for (auto ec : node->consts) {
        auto literal = ec->findAttr<ASTAttrValue>()->value;
        std::string value;
        if (auto integer = literal->as<ASTLiteralInt>()) {
            if (!isHexOut) {
                value = fmt::format("{}", (int32_t) integer->value);
            } else {
                auto num          = (int32_t) integer->value;
                size_t hex_length = (num == 0) ? 1 : static_cast<size_t>(std::log2(num) / 4 + 1);
                size_t width      = (hex_length % 2 == 0) ? hex_length : hex_length + 1;
                value             = fmt::format("0x{:0{}x}", num, width);
            }
        } else if (auto refs = literal->as<ASTLiteralConsts>()) {
            for (auto ref : refs->decls) {
                ref->decl->accept(name);
                value += value.length() > 0 ? " | " + name.str : name.str;
            }
        } else {
            assert(!"unreachable code");
        }
        value += ',';

        ec->accept(name);
        consts.emplace_back(name.str, value);
        if (name.str.length() > maxLength) {
            maxLength = name.str.length();
        }
    }
    ASTEnumConst ec{};
    ec.parent = node;
    ec.name   = "MaxEnum";
    ec.accept(name);
    if (node->findAttr<ASTAttrFlags>()) {
        name.str = name.str.substr(0, name.str.length() - 4);
    }
    consts.emplace_back(name.str, "0x7FFFFFFF");
    if (name.str.length() > maxLength) {
        maxLength = name.str.length();
    }

    std::cout << "typedef enum\n{\n";
    for (const auto& [name, value] : consts) {
        std::cout << fmt::format("    {:<{}} = {}\n", name, maxLength, value);
    }
    node->accept(name);
    std::cout << "} " << name.str << ";\n";
}

void GeneratorC::generate(idl::Context& ctx, const std::filesystem::path& out) {
    ctx.filter<ASTEnum>(generateEnum);
}
