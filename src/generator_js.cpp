
#include "case_converter.hpp"
#include "context.hpp"

using namespace idl;

struct Stream {
    std::ostream& stream;
    std::unique_ptr<std::ofstream> fstream;
    std::unique_ptr<std::ostringstream> sstream;
    idl_write_callback_t writer;
    idl_data_t writerData;
};

struct IsTrivial : Visitor {
    IsTrivial(bool isArray = false, bool isRef = false) noexcept : trivial(!isArray && !isRef) {
    }

    void visit(ASTStr*) override {
        trivial = false;
    }

    void visit(ASTStruct* node) override {
        if (trivial) {
            for (auto field : node->fields) {
                auto isArray = field->findAttr<ASTAttrArray>() != nullptr;
                auto isRef   = field->findAttr<ASTAttrRef>() != nullptr;
                auto type    = field->findAttr<ASTAttrType>()->type->decl;
                IsTrivial isTrivial(isArray, isRef);
                type->accept(isTrivial);
                trivial = isTrivial.trivial;
                if (!trivial) {
                    break;
                }
            }
        }
    }

    void visit(ASTInterface*) override {
        trivial = false;
    }

    bool trivial{};
};

struct JsName : Visitor {
    void visit(ASTStr*) override {
        str = "String";
    }

    void visit(ASTStruct* node) override {
        str = pascalCase(node);
    }

    void visit(ASTField* node) override {
        str = camelCase(node);
    }

    void discarded(ASTNode*) override {
        assert(!"Js name is missing");
    }

    static std::string camelCase(ASTDecl* decl) {
        std::vector<int>* nums = nullptr;
        if (auto attr = decl->findAttr<ASTAttrTokenizer>()) {
            nums = &attr->nums;
        }
        return convert(decl->name, Case::CamelCase, nums);
    }

    static std::string pascalCase(ASTDecl* decl) {
        std::vector<int>* nums = nullptr;
        if (auto attr = decl->findAttr<ASTAttrTokenizer>()) {
            nums = &attr->nums;
        }
        return convert(decl->name, Case::PascalCase, nums);
    }

    std::string str;
};

struct DefaultValue : Visitor {
    void visit(ASTStr* node) override {
        value = "String(val(\"\"))";
    }

    void visit(ASTUint32* node) override {
        value = "0";
    }

    void visit(ASTEnum* node) override {
        CName cname;
        node->consts.front()->accept(cname);
        value = cname.str;
    }

    void discarded(ASTNode*) override {
        assert(!"Default value is missing");
    }

    std::string value;
};

struct DeclDefaultValue : Visitor {
    void visit(ASTField* node) override {
        if (auto attr = node->findAttr<ASTAttrValue>()) {
        } else {
            auto type = node->findAttr<ASTAttrType>()->type->decl;
            DefaultValue defValue;
            type->accept(defValue);
            value = defValue.value;
        }
    }

    void discarded(ASTNode*) override {
        assert(!"Decl default value is missing");
    }

    std::string value;
};

static std::string moduleName(idl::Context& ctx) {
    return convert(ctx.api()->name, Case::LispCase);
}

static std::string ModuleName(idl::Context& ctx) {
    return convert(ctx.api()->name, Case::PascalCase);
}

static Stream createStream(idl::Context& ctx,
                           const std::filesystem::path& out,
                           idl_write_callback_t writer,
                           idl_data_t writerData) {
    std::filesystem::create_directories(out);
    auto mName = out / (moduleName(ctx) + ".js.cpp");
    if (writer) {
        auto stream = std::make_unique<std::ostringstream>();
        auto ptr    = stream.get();
        return { *ptr, nullptr, std::move(stream), writer, writerData };
    } else {
        auto stream = std::make_unique<std::ofstream>(std::ofstream(mName));
        if (stream->fail()) {
            idl::err<IDL_STATUS_E2067>(ctx.api()->location, mName.string());
        }
        auto ptr = stream.get();
        return { *ptr, std::move(stream) };
    }
}

static void generateIncludes(idl::Context& ctx, std::ostream& stream) {
    const auto libHeader = convert(ctx.api()->name, Case::LispCase) + ".h";
    fmt::println(stream, "#include <emscripten/bind.h>");
    fmt::println(stream, "#include <emscripten/val.h>");
    fmt::println(stream, "");
    fmt::println(stream, "#include \"{}\"", libHeader);
    fmt::println(stream, "");
    fmt::println(stream, "using namespace emscripten;");
    fmt::println(stream, "");
}

static void generateExceptions(idl::Context& ctx, std::ostream& stream, ASTEnum*& errcode) {
    errcode = nullptr;
    ctx.filter<ASTEnum>([&errcode](ASTEnum* en) {
        if (en->findAttr<ASTAttrErrorCode>()) {
            errcode = en;
            return false;
        }
        return true;
    });
    if (errcode) {
        const auto prefix = ModuleName(ctx);

        CName cname;
        ASTStr* type{};
        ctx.filter<ASTStr>([&type](ASTStr* str) {
            type = str;
            return false;
        });

        ASTFunc* errcodeToString = nullptr;
        ctx.filter<ASTFunc>([&errcodeToString](ASTFunc* func) {
            if (func->findAttr<ASTAttrErrorCode>()) {
                errcodeToString = func;
                return false;
            }
            return true;
        });

        std::string noerrorcodeFirst{};
        auto noErrors = 0;
        for (auto ec : errcode->consts) {
            if (ec->findAttr<ASTAttrNoError>()) {
                ++noErrors;
                if (noerrorcodeFirst.empty()) {
                    ec->accept(cname);
                    noerrorcodeFirst = cname.str;
                }
            }
        }

        type->accept(cname);
        fmt::println(stream, "struct {}Exception : std::runtime_error {{", prefix);
        fmt::println(stream, "    {}Exception({} message) : std::runtime_error(message) {{", prefix, cname.str);
        fmt::println(stream, "    }}");
        fmt::println(stream, "}};");
        fmt::println(stream, "");

        errcode->accept(cname);
        fmt::println(stream, "void checkResult({} result) {{", cname.str);
        if (noErrors == 1 && errcodeToString) {
            errcodeToString->accept(cname);
            fmt::println(stream, "    if (result != {}) {{", noerrorcodeFirst);
            fmt::println(stream, "        throw {}Exception({}(result));", prefix, cname.str);
            fmt::println(stream, "    }}");
        } else {
            if (noErrors != 0) {
                fmt::println(stream, "    switch (result) {{");
                for (auto ec : errcode->consts) {
                    if (ec->findAttr<ASTAttrNoError>()) {
                        ec->accept(cname);
                        fmt::println(stream, "        case {}:", cname.str);
                    }
                }
                fmt::println(stream, "            return;");
                fmt::println(stream, "        default:");
                fmt::println(stream, "            break;");
                fmt::println(stream, "    }}");
            }
            if (errcodeToString) {
                errcodeToString->accept(cname);
                fmt::println(stream, "    throw {}Exception({}(result));", prefix, cname.str);
            } else {
                fmt::println(stream, "    switch (result) {{");
                for (auto ec : errcode->consts) {
                    if (!ec->findAttr<ASTAttrNoError>()) {
                        ec->accept(cname);
                        fmt::println(stream, "        case {}:", cname.str);
                        fmt::println(stream, "            throw {}Exception(\"{}\");", prefix, cname.str);
                    }
                }
                fmt::println(stream, "        default:");
                fmt::println(stream, "            assert(!\"unreachable code\");");
                fmt::println(stream, "            break;");
                fmt::println(stream, "    }}");
            }
        }
        fmt::println(stream, "}}");
        fmt::println(stream, "");
    }
}

static void generateNonTrivialTypes(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        IsTrivial trivial{};
        node->accept(trivial);
        if (!trivial.trivial) {
            JsName jsname;
            node->accept(jsname);

            fmt::println(stream, "class {} {{", jsname.str);
            fmt::println(stream, "public:");

            for (auto field : node->fields) {
                field->accept(jsname);

                DeclDefaultValue value;
                field->accept(value);

                auto isArray = field->findAttr<ASTAttrArray>() != nullptr;
                auto isRef   = field->findAttr<ASTAttrRef>() != nullptr;
                auto type    = field->findAttr<ASTAttrType>()->type->decl;
                IsTrivial trivial(isArray, isRef);
                type->accept(trivial);
                if (trivial.trivial) {
                    CName cname;
                    type->accept(cname);
                    fmt::println(stream, "    {} {}{{{}}};", cname.str, jsname.str, value.value);
                } else {
                }
            }

            fmt::println(stream, "}};");
            fmt::println(stream, "");
        }
    });
}

void generateJs(idl::Context& ctx,
                const std::filesystem::path& out,
                idl_write_callback_t writer,
                idl_data_t writerData,
                std::span<idl_utf8_t> includes) {
    auto stream      = createStream(ctx, out, writer, writerData);
    ASTEnum* errcode = nullptr;
    generateIncludes(ctx, stream.stream);
    generateExceptions(ctx, stream.stream, errcode);
    generateNonTrivialTypes(ctx, stream.stream);
}
