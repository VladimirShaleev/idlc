
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

    void visit(ASTBool*) override {
        trivial = false;
    }

    void visit(ASTStruct* node) override {
        if (trivial) {
            for (auto field : node->fields) {
                auto type    = field->findAttr<ASTAttrType>()->type->decl;
                auto isArray = field->findAttr<ASTAttrArray>() != nullptr;
                auto isRef   = field->findAttr<ASTAttrRef>() != nullptr && !isArray && !type->is<ASTTrivialType>();
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
    explicit JsName(bool isArr = false) noexcept : isArray(isArr) {
    }

    void visit(ASTChar* node) override {
        str = arrName(node);
    }

    void visit(ASTStr*) override {
        str = isArray ? "ArrString" : "String";
    }

    void visit(ASTBool* node) override {
        str = isArray ? "ArrBool" : "bool";
    }

    void visit(ASTInt8* node) override {
        str = arrName(node);
    }

    void visit(ASTUint8* node) override {
        str = arrName(node);
    }

    void visit(ASTInt16* node) override {
        str = arrName(node);
    }

    void visit(ASTUint16* node) override {
        str = arrName(node);
    }

    void visit(ASTInt32* node) override {
        str = arrName(node);
    }

    void visit(ASTUint32* node) override {
        str = arrName(node);
    }

    void visit(ASTInt64* node) override {
        str = arrName(node);
    }

    void visit(ASTUint64* node) override {
        str = arrName(node);
    }

    void visit(ASTFloat32* node) override {
        str = arrName(node);
    }

    void visit(ASTFloat64* node) override {
        str = arrName(node);
    }

    void visit(ASTData* node) override {
        str = arrName(node);
    }

    void visit(ASTConstData* node) override {
        str = arrName(node);
    }

    void visit(ASTStruct* node) override {
        str = std::string(isArray ? "Arr" : "") + pascalCase(node);
    }

    void visit(ASTField* node) override {
        str = camelCase(node);
    }

    void discarded(ASTNode*) override {
        assert(!"Js name is missing");
    }

    std::string arrName(ASTDecl* decl) {
        assert(isArray);
        return "Arr" + pascalCase(decl);
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

    bool isArray{};
    std::string str;
};

struct DefaultValue : Visitor {
    explicit DefaultValue(bool isArr = false) noexcept : isArray(isArr) {
    }

    void visit(ASTChar* node) override {
        value = defualtValue(node, "\\0");
    }

    void visit(ASTStr* node) override {
        value = defualtValue(node, "String(val(\"\"))");
    }

    void visit(ASTBool* node) override {
        value = defualtValue(node, "false");
    }

    void visit(ASTInt8* node) override {
        value = defualtValue(node);
    }

    void visit(ASTUint8* node) override {
        value = defualtValue(node);
    }

    void visit(ASTInt16* node) override {
        value = defualtValue(node);
    }

    void visit(ASTUint16* node) override {
        value = defualtValue(node);
    }

    void visit(ASTInt32* node) override {
        value = defualtValue(node);
    }

    void visit(ASTUint32* node) override {
        value = defualtValue(node);
    }

    void visit(ASTInt64* node) override {
        value = defualtValue(node);
    }

    void visit(ASTUint64* node) override {
        value = defualtValue(node);
    }

    void visit(ASTFloat32* node) override {
        value = defualtValue(node, "0.0f");
    }

    void visit(ASTFloat64* node) override {
        value = defualtValue(node, "0.0");
    }

    void visit(ASTData* node) override {
        value = defualtValue(node, "nullptr");
    }

    void visit(ASTConstData* node) override {
        value = defualtValue(node, "nullptr");
    }

    void visit(ASTEnum* node) override {
        CName cname;
        node->consts.front()->accept(cname);
        value = cname.str;
    }

    void visit(ASTStruct* node) override {
        IsTrivial trivial(isArray);
        node->accept(trivial);
        if (trivial.trivial) {
            CName cname;
            node->accept(cname);
            value = cname.str;
        } else {
            JsName jsname(isArray);
            node->accept(jsname);
            value = jsname.str;
        }
        if (isArray) {
            value += "(val::array())";
        } else {
            value += "{}";
        }
    }

    void discarded(ASTNode*) override {
        assert(!"Default value is missing");
    }

    std::string defualtValue(ASTDecl* decl, const std::string& defValue = "0") {
        if (isArray) {
            JsName jsname(isArray);
            decl->accept(jsname);
            return jsname.str + "(val::array())";
        }
        return defValue;
    }

    bool isArray{};
    std::string value;
};

struct Value : Visitor {
    explicit Value(bool isArr = false) noexcept : isArray(isArr) {
    }

    void visit(ASTField* node) override {
        if (auto attr = node->findAttr<ASTAttrValue>()) {
            if (auto intStr = attr->value->as<ASTLiteralStr>()) {
                auto str = intStr->value;
                auto pos = str.find('\"');
                while (pos != std::string::npos) {
                    str.replace(pos, 1, "\\\"");
                    pos = str.find('\"', pos + 2);
                }
                value = "String(val(\"" + str + "\"))";
            } else if (auto litInt = attr->value->as<ASTLiteralInt>()) {
                value = std::to_string(litInt->value);
            } else if (auto litBool = attr->value->as<ASTLiteralBool>()) {
                value = litBool->value ? "true" : "false";
            } else if (auto litConsts = attr->value->as<ASTLiteralConsts>()) {
                CName cname;
                for (auto decl : litConsts->decls) {
                    if (value.length() > 0) {
                        value += " | ";
                    }
                    auto ec = decl->decl->as<ASTEnumConst>();
                    ec->accept(cname);
                    value += cname.str;
                }
            }
        } else {
            auto type = node->findAttr<ASTAttrType>()->type->decl;
            DefaultValue defValue(isArray);
            type->accept(defValue);
            value = defValue.value;
        }
    }

    void discarded(ASTNode*) override {
        assert(!"Decl default value is missing");
    }

    bool isArray{};
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

            std::vector<std::tuple<ASTField*, std::string, ASTDecl*, bool, bool, bool>> fields;

            for (auto field : node->fields) {
                field->accept(jsname);
                auto type    = field->findAttr<ASTAttrType>()->type->decl;
                auto isArray = field->findAttr<ASTAttrArray>() != nullptr;
                auto isRef   = field->findAttr<ASTAttrRef>() != nullptr && !isArray && !type->is<ASTTrivialType>();

                Value value(isArray);
                field->accept(value);

                IsTrivial trivial(isArray, isRef);
                type->accept(trivial);
                std::string typeStr;
                if (trivial.trivial) {
                    CName cname;
                    type->accept(cname);
                    typeStr = cname.str;
                } else {
                    JsName jstype(isArray);
                    type->accept(jstype);
                    typeStr = jstype.str;
                }
                fmt::println(stream, "    {} {}{{{}}};", typeStr, jsname.str, value.value);
                fields.emplace_back(field, jsname.str, type, trivial.trivial, isArray, isRef);
            }

            CName cname;
            node->accept(cname);
            const auto cStruct = cname.str;

            fmt::println(stream, "");
            fmt::println(stream, "    const {}* get() {{", cStruct);
            for (const auto& [field, name, type, trivial, isArray, isRef] : fields) {
                field->accept(cname);
                if (trivial) {
                    fmt::println(stream, "        _raw.{} = {};", cname.str, name);
                }
            }
            fmt::println(stream, "        return &_raw;");
            fmt::println(stream, "    }}");

            fmt::println(stream, "");
            fmt::println(stream, "private:");

            for (const auto& [field, name, type, trivial, isArray, isRef] : fields) {
                if (trivial) {
                    continue;
                }
                type->accept(cname);
                if (isArray) {
                    if (type->is<ASTStr>()) {
                        fmt::println(stream, "    std::vector<std::string> _raw_{}Data{{}};", name);
                    }
                    fmt::println(stream, "    std::vector<{}> _raw_{}{{}};", cname.str, name);
                } else if (!type->is<ASTStruct>()) {
                    if (type->is<ASTStr>()) {
                        cname.str = "std::string";
                    }
                    fmt::println(stream, "    {} _raw_{}{{}};", cname.str, name);
                }
            }
            fmt::println(stream, "    {} _raw{{}};", cStruct);

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
