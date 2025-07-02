
#include "case_converter.hpp"
#include "context.hpp"

using namespace idl;

struct Stream {
    std::ostream& stream;
    std::unique_ptr<std::ofstream> fstream;
    std::unique_ptr<std::ostringstream> sstream;
    std::string filename;
    idl_write_callback_t writer;
    idl_data_t writerData;
};

struct IsTrivial : Visitor {
    IsTrivial(bool isArray = false) noexcept : trivial(!isArray) {
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
                IsTrivial isTrivial(isArray);
                type->accept(isTrivial);
                trivial = isTrivial.trivial;
                if (!trivial) {
                    break;
                }
            }
        }
    }

    void visit(ASTCallback* node) override {
        trivial = false;
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
        str = isArray ? "ArrString" : "String";
    }

    void visit(ASTStr*) override {
        str = isArray ? "ArrString" : "String";
    }

    void visit(ASTBool* node) override {
        str = isArray ? "ArrBool" : "bool";
    }

    void visit(ASTInt8* node) override {
        str = calcName(node);
    }

    void visit(ASTUint8* node) override {
        str = calcName(node);
    }

    void visit(ASTInt16* node) override {
        str = calcName(node);
    }

    void visit(ASTUint16* node) override {
        str = calcName(node);
    }

    void visit(ASTInt32* node) override {
        str = calcName(node);
    }

    void visit(ASTUint32* node) override {
        str = calcName(node);
    }

    void visit(ASTInt64* node) override {
        str = calcName(node);
    }

    void visit(ASTUint64* node) override {
        str = calcName(node);
    }

    void visit(ASTFloat32* node) override {
        str = calcName(node);
    }

    void visit(ASTFloat64* node) override {
        str = calcName(node);
    }

    void visit(ASTData* node) override {
        str = calcName(node);
    }

    void visit(ASTConstData* node) override {
        str = calcName(node);
    }

    void visit(ASTStruct* node) override {
        str = calcName(node);
    }

    void visit(ASTField* node) override {
        str = camelCase(node);
    }

    void visit(ASTArg* node) override {
        str = camelCase(node);
    }

    void visit(ASTCallback* node) override {
        str = calcName(node);
    }

    void visit(ASTEnum* node) override {
        str = calcName(node);
    }

    void visit(ASTEnumConst* node) override {
        assert(!isArray);
        std::vector<int>* nums = nullptr;
        if (auto attr = node->findAttr<ASTAttrTokenizer>()) {
            nums = &attr->nums;
        }
        str = convert(node->name, Case::ScreamingSnakeCase, nums);
    }

    void visit(ASTInterface* node) override {
        str = calcName(node);
    }

    void visit(ASTMethod* node) override {
        assert(!isArray);
        str = camelCase(node);
    }

    void visit(ASTProperty* node) override {
        assert(!isArray);
        str = camelCase(node);
    }

    void visit(ASTEvent* node) override {
        assert(!isArray);
        str = camelCase(node);
    }

    void visit(ASTFunc* node) override {
        assert(!isArray);
        str = camelCase(node);
    }

    void visit(ASTVoid* node) override {
        assert(!isArray);
        str = "void";
    }

    void discarded(ASTNode*) override {
        assert(!"Js name is missing");
    }

    std::string calcName(ASTDecl* decl) {
        IsTrivial trivial(isArray);
        decl->accept(trivial);
        if (isArray) {
            return "Arr" + pascalCase(decl);
        } else if (trivial.trivial) {
            CName cname;
            decl->accept(cname);
            return cname.str;
        } else {
            return pascalCase(decl);
        }
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
        if (isArray) {
            value = defualtValue(node);
        }
    }

    void discarded(ASTNode*) override {
        assert(!"Default value is missing");
    }

    std::string defualtValue(ASTDecl* decl, const std::string& defValue = "0") {
        if (isArray) {
            JsName jsname(isArray);
            decl->accept(jsname);
            return jsname.str + "(val::undefined())";
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

struct Param {
    ASTDecl* type{};
    bool inParam{};
    bool outParam{};
    bool isUserdata{};
    bool isCallback{};
    bool isVector{};
    bool isSize{};
    bool isResult{};
    bool isError{};
    ASTArg* refArg{};
    std::string typeName;
    std::string paramName;
    std::string jsArgName;
};

static bool isArray(ASTDecl* decl) noexcept {
    return decl->findAttr<ASTAttrArray>() != nullptr;
}

static bool isOptional(ASTDecl* decl) noexcept {
    return decl->findAttr<ASTAttrOptional>() != nullptr && !isArray(decl) && !decl->findAttr<ASTAttrOut>() &&
           !decl->findAttr<ASTAttrResult>();
}

static bool isRef(ASTDecl* decl) noexcept {
    return decl->findAttr<ASTAttrRef>() != nullptr;
}

static ASTDecl* getType(ASTDecl* decl) noexcept {
    return decl->findAttr<ASTAttrType>()->type->decl;
}

static std::pair<ASTDecl*, int> getSizeDecl(ASTDecl* decl) noexcept {
    if (auto attr = decl->findAttr<ASTAttrArray>(); attr->ref) {
        return { attr->decl->decl, 0 };
    } else {
        return { nullptr, attr->size };
    }
}

static std::string getNameTS(ASTDecl* decl, bool isDeclArr = false) {
    if (decl->is<ASTStr>()) {
        return std::string("string") + (isDeclArr ? "[]" : "");
    } else if (decl->is<ASTBool>()) {
        return std::string("boolean") + (isDeclArr ? "[]" : "");
    } else if (decl->is<ASTVoid>()) {
        return "void";
    } else if (decl->is<ASTIntegerType>() || decl->is<ASTFloatType>()) {
        if (isDeclArr) {
            auto type = decl->name + "Array";
            if (decl->is<ASTInt64>() || decl->is<ASTUint64>()) {
                type = "Big" + type;
            }
            return type;
        }
        return "number";
    } else if (auto callback = decl->as<ASTCallback>()) {
        std::ostringstream ss;
        ss << '(';
        bool first = true;
        for (auto arg : callback->args) {
            if (arg->findAttr<ASTAttrUserData>() != nullptr || arg->findAttr<ASTAttrResult>() != nullptr) {
                continue;
            }
            if (!first) {
                ss << ", ";
            }
            first = false;
            JsName jsname;
            arg->accept(jsname);
            ss << jsname.str << ": " << getNameTS(getType(arg), isArray(arg));
        }
        ss << ") => " << getNameTS(getType(callback));
        auto type = ss.str();
        if (isDeclArr) {
            type = '(' + type + ")[]";
        }
        return type;
    }
    std::vector<int>* nums = nullptr;
    if (auto attr = decl->findAttr<ASTAttrTokenizer>()) {
        nums = &attr->nums;
    }
    return convert(decl->name, Case::PascalCase, nums) + (isDeclArr ? "[]" : "");
}

static Stream createStream(idl::Context& ctx,
                           const std::filesystem::path& out,
                           idl_write_callback_t writer,
                           idl_data_t writerData) {
    std::filesystem::create_directories(out);
    auto filename = (convert(ctx.api()->name, Case::LispCase) + ".js.cpp");
    auto mName    = out / filename;
    if (writer) {
        auto stream = std::make_unique<std::ostringstream>();
        auto ptr    = stream.get();
        return { *ptr, nullptr, std::move(stream), filename, writer, writerData };
    } else {
        auto stream = std::make_unique<std::ofstream>(std::ofstream(mName));
        if (stream->fail()) {
            idl::err<IDL_STATUS_E2067>(ctx.api()->location, mName.string());
        }
        auto ptr = stream.get();
        return { *ptr, std::move(stream) };
    }
}

static void generateComment(idl::Context& ctx, std::ostream& stream) {
    char datatime[100];
    auto now = std::time(nullptr);
    strftime(datatime, 100, "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&now));
    fmt::println(stream,
                 R"(/**
 * Auto-generated on {now}
 *
 * This file contains Embind bindings for JavaScript interoperability.
 * 
 * Building the JavaScript module
 * ==============================
 * You can use any build system that supports Emscripten (e.g., emcc, CMake, Makefile, etc.).
 * Below is an example using `emcc` directly:
 *
 *   emcc {module}.js.cpp \
 *        -I<path>/include \
 *        -Wl,--whole-archive <path>/lib/lib{module}.a -Wl,--no-whole-archive \
 *        -std=c++20 \
 *        -lembind \
 *        --emit-tsd {module}.d.ts \
 *        -o ./dist/{module}.js \
 *        -s WASM=1 \
 *        -s MODULARIZE=1 \
 *        -s ALLOW_MEMORY_GROWTH=1 \
 *        -s EXPORT_NAME={module} 
 * 
 * Note: Replace `<path>` with your actual library paths.
 * If using CMake or another build system, adjust flags accordingly.
 */
)",
                 fmt::arg("module", convert(ctx.api()->name, Case::LispCase)),
                 fmt::arg("now", datatime));
}

static void generateIncludes(idl::Context& ctx, std::ostream& stream) {
    const auto libHeader = convert(ctx.api()->name, Case::LispCase) + ".h";
    fmt::println(stream, "#include <emscripten/bind.h>");
    fmt::println(stream, "#include <emscripten/val.h>");
    fmt::println(stream, "");
    fmt::println(stream, "#include \"{}\"", libHeader);
    fmt::println(stream, "");
    fmt::println(stream, "#include <type_traits>");
    fmt::println(stream, "#include <vector>");
    fmt::println(stream, "#include <list>");
    fmt::println(stream, "#include <span>");
    fmt::println(stream, "");
    fmt::println(stream, "using namespace emscripten;");
    fmt::println(stream, "");
}

static void generateTypes(idl::Context& ctx, std::ostream& stream) {
    fmt::println(stream, "EMSCRIPTEN_DECLARE_VAL_TYPE(String);");
    ctx.filter<ASTCallback>([&stream](ASTCallback* callback) {
        JsName jsname;
        callback->accept(jsname);
        fmt::println(stream, "EMSCRIPTEN_DECLARE_VAL_TYPE({});", jsname.str);
    });
    ctx.filter<ASTTrivialType>([&stream](ASTTrivialType* trivialType) {
        if (!trivialType->is<ASTVoid>() && !trivialType->is<ASTChar>()) {
            JsName jsname(true);
            trivialType->accept(jsname);
            fmt::println(stream, "EMSCRIPTEN_DECLARE_VAL_TYPE({});", jsname.str);
        }
    });
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        JsName jsname(true);
        node->accept(jsname);
        fmt::println(stream, "EMSCRIPTEN_DECLARE_VAL_TYPE({});", jsname.str);
    });
    ctx.filter<ASTInterface>([&stream](ASTInterface* node) {
        JsName jsname(true);
        node->accept(jsname);
        fmt::println(stream, "EMSCRIPTEN_DECLARE_VAL_TYPE({});", jsname.str);
    });
    ctx.filter<ASTCallback>([&stream](ASTCallback* callback) {
        JsName jsname(true);
        callback->accept(jsname);
        fmt::println(stream, "EMSCRIPTEN_DECLARE_VAL_TYPE({});", jsname.str);
    });
    fmt::println(stream, "");
}

static void generateExceptions(idl::Context& ctx, std::ostream& stream) {
    const auto prefix = convert(ctx.api()->name, Case::PascalCase);

    ASTStr* type{};
    ctx.filter<ASTStr>([&type](ASTStr* str) {
        type = str;
        return false;
    });

    CName cname;
    type->accept(cname);
    fmt::println(stream, "struct {}Exception : std::runtime_error {{", prefix);
    fmt::println(stream, "    {}Exception({} message) : std::runtime_error(message) {{", prefix, cname.str);
    fmt::println(stream, "    }}");
    fmt::println(stream, "}};");
    fmt::println(stream, "");

    ctx.filter<ASTEnum>([&ctx, &stream, &prefix](ASTEnum* en) {
        if (en->findAttr<ASTAttrErrorCode>()) {
            ASTFunc* errcodeToString = nullptr;
            ctx.filter<ASTFunc>([en, &errcodeToString](ASTFunc* func) {
                if (func->findAttr<ASTAttrErrorCode>()) {
                    for (auto arg : func->args) {
                        if (auto argType = getType(arg); argType == en) {
                            errcodeToString = func;
                            return false;
                        }
                    }
                }
                return true;
            });

            CName cname;
            std::string noerrorcodeFirst{};
            auto noErrors = 0;
            for (auto ec : en->consts) {
                if (ec->findAttr<ASTAttrNoError>()) {
                    ++noErrors;
                    if (noerrorcodeFirst.empty()) {
                        ec->accept(cname);
                        noerrorcodeFirst = cname.str;
                    }
                }
            }

            en->accept(cname);
            fmt::println(stream, "void checkResult({} result) {{", cname.str);
            if (noErrors == 1 && errcodeToString) {
                errcodeToString->accept(cname);
                fmt::println(stream, "    if (result != {}) {{", noerrorcodeFirst);
                fmt::println(stream, "        throw {}Exception({}(result));", prefix, cname.str);
                fmt::println(stream, "    }}");
            } else {
                if (noErrors != 0) {
                    fmt::println(stream, "    switch (result) {{");
                    for (auto ec : en->consts) {
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
                    for (auto ec : en->consts) {
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
    });
}

static void generateNonTrivialTypes(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        IsTrivial trivial;
        node->accept(trivial);
        if (!trivial.trivial) {
            JsName jsname;
            node->accept(jsname);
            fmt::println(stream, "struct {} {{", jsname.str);
            for (auto field : node->fields) {
                const auto isArr = isArray(field);
                jsname.isArray   = isArr;
                getType(field)->accept(jsname);
                const auto typeStr = jsname.str;
                jsname.isArray     = false;
                field->accept(jsname);
                Value value(isArr);
                field->accept(value);
                fmt::println(stream, "    {} {}{{ {} }};", typeStr, jsname.str, value.value);
            }
            fmt::println(stream, "}};", jsname.str);
            fmt::println(stream, "");
        }
    });
}

static void generateClassDeclarations(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTInterface>([&stream](ASTInterface* node) {
        JsName jsname;
        node->accept(jsname);
        fmt::println(stream, "class {};", jsname.str);
    });
    fmt::println(stream, "");
}

static void generateArrItems(idl::Context& ctx, std::ostream& stream) {
    fmt::println(stream, "template <typename> struct ArrItem;");
    auto addArrItem = [&stream](ASTDecl* decl) {
        if (!decl->is<ASTVoid>() && !decl->is<ASTChar>()) {
            JsName jsname(true);
            decl->accept(jsname);
            const auto arrname = jsname.str;
            jsname.isArray     = false;
            decl->accept(jsname);
            std::string typed = "";
            if (decl->is<ASTIntegerType>() || decl->is<ASTFloatType>()) {
                typed = decl->name + "Array";
                if (decl->is<ASTInt64>() || decl->is<ASTUint64>()) {
                    typed = "Big" + typed;
                }
            }
            fmt::println(
                stream,
                "template <> struct ArrItem<{}> {{ using type = {}; static constexpr char typed[] = \"{}\"; }};",
                arrname,
                jsname.str,
                typed);
        }
    };
    ctx.filter<ASTTrivialType>(addArrItem);
    ctx.filter<ASTStruct>(addArrItem);
    ctx.filter<ASTInterface>(addArrItem);
    ctx.filter<ASTCallback>(addArrItem);
    fmt::println(stream, "");
}

static void generateJsConverters(idl::Context& ctx, std::ostream& stream) {
    CName cname;
    ASTDeclRef ref;
    ref.parent = ctx.api();
    ref.name   = "Char";
    ref.decl   = nullptr;
    ctx.resolveType(&ref)->accept(cname);
    auto charType = cname.str;
    ref.name      = "Bool";
    ref.decl      = nullptr;
    ctx.resolveType(&ref)->accept(cname);
    auto boolType = cname.str;
    ref.name      = "Str";
    ref.decl      = nullptr;
    ctx.resolveType(&ref)->accept(cname);
    auto strType = cname.str;

    fmt::println(stream,
                 R"(template <typename, typename>
struct JsConverter;

template <typename T, typename S>
inline T jsconvert(const S& obj) {{
    return JsConverter<T, S>::convert(obj);
}}

template <typename T, typename S>
struct JsConverter<T, std::span<S>> {{
    static T convert(std::span<S> obj) {{
        using ItemT = typename ArrItem<T>::type;
        if constexpr (std::size(ArrItem<T>::typed) > 1) {{
            val view{{typed_memory_view(obj.size(), obj.data())}};
            T arr(val::global(ArrItem<T>::typed).new_(obj.size()));
            arr.template call<void>("set", view);
            return arr;
        }} else {{
            T arr(val::array());
            for (auto& value : obj) {{
                arr.template call<void>("push", jsconvert<ItemT>(value));
            }}
            return arr;
        }}
    }}
}};

template <typename T, typename S>
struct JsConverter<std::optional<T>, S> {{
    static std::optional<T> convert(const S obj) {{
        return obj ? std::make_optional(jsconvert<T>(*obj)) : std::nullopt;
    }}
}};

template <typename T>
struct JsConverter<T, T> {{
    static T convert(const T& obj) {{
        return obj;
    }}
}};

template <>
struct JsConverter<String, {char}> {{
    static String convert({char} obj) {{
        {char} str[2] = {{ obj }};
        return String(val::u8string(str));
    }}
}};

template <>
struct JsConverter<bool, {bool}> {{
    static bool convert({bool} obj) {{
        return !!obj;
    }}
}};

template <>
struct JsConverter<String, {str}> {{
    static String convert({str} obj) {{
        return String(val::u8string(obj));
    }}
}};
)",
                 fmt::arg("char", charType),
                 fmt::arg("bool", boolType),
                 fmt::arg("str", strType));
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        IsTrivial trivial;
        node->accept(trivial);
        if (!trivial.trivial) {
            JsName jsname;
            node->accept(jsname);
            CName cname;
            node->accept(cname);
            fmt::println(stream, "template <>");
            fmt::println(stream, "struct JsConverter<{}, {}> {{", jsname.str, cname.str);
            fmt::println(stream, "    static {} convert(const {}& obj) {{", jsname.str, cname.str);
            fmt::println(stream, "        return {} {{", jsname.str);
            for (auto field : node->fields) {
                auto type      = getType(field);
                auto isArr     = isArray(field);
                auto isR       = !isArr && isRef(field);
                jsname.isArray = isArr;
                std::string spanBegin;
                std::string spanEnd;
                if (isArr) {
                    const auto [ref, size] = getSizeDecl(field);
                    std::string value;
                    if (ref) {
                        CName cname;
                        ref->accept(cname);
                        value = "size_t(obj." + cname.str + ")";
                    } else {
                        value = std::to_string(size);
                    }
                    spanBegin = "std::span{";
                    spanEnd   = ", " + value + "}";
                }
                type->accept(jsname);
                field->accept(cname);
                fmt::println(stream,
                             "            jsconvert<{}>({}{}obj.{}{}),",
                             jsname.str,
                             isR ? "*" : "",
                             spanBegin,
                             cname.str,
                             spanEnd);
            }
            fmt::println(stream, "        }};");
            fmt::println(stream, "    }}");
            fmt::println(stream, "}};");
            fmt::println(stream, "");
        }
    });
}

static void generateCConverters(idl::Context& ctx, std::ostream& stream) {
    CName cname;
    ASTDeclRef ref;
    ref.parent = ctx.api();
    ref.name   = "Char";
    ref.decl   = nullptr;
    ctx.resolveType(&ref)->accept(cname);
    auto charType = cname.str;
    ref.name      = "Bool";
    ref.decl      = nullptr;
    ctx.resolveType(&ref)->accept(cname);
    auto boolType = cname.str;
    ref.name      = "Str";
    ref.decl      = nullptr;
    ctx.resolveType(&ref)->accept(cname);
    auto strType = cname.str;

    fmt::println(stream,
                 R"(struct CContext {{
    template <typename T>
    T* allocate() {{
        const auto size = allocData<T>();
        auto& buffer = buffers.back();
        auto result = new(buffer.data.get() + buffer.offset) T{{}};
        buffer.offset += size;
        return result;
    }}

    template <typename T>
    T* allocateArray(size_t count) {{
        const auto size = allocData<T>(count);
        auto& buffer = buffers.back();
        auto result = new(buffer.data.get() + buffer.offset) T[count]{{}};
        buffer.offset += size;
        return result;
    }}

    template <typename T>
    size_t allocData(size_t count = 1) {{
        static_assert(std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T>, "T is not trivial type");
        constexpr auto mask = 7;
        auto size = (sizeof(T) * count + mask) & ~mask;
        if (buffers.empty() || buffers.back().offset + size > buffers.back().size) {{
            buffers.push_back({{}});
            auto allocSize = std::max(size, Buffer::defaultSize);
            buffers.back().size = allocSize;
            buffers.back().data = std::unique_ptr<char[]>(new char[allocSize]); 
        }}
        return size;
    }}

    struct Buffer {{
        static constexpr size_t defaultSize = 1024;
        size_t offset{{}};
        size_t size{{}};
        std::unique_ptr<char[]> data{{}};
    }};

    std::list<Buffer> buffers;
}};

template <typename>
struct arr_size {{
}};

template <typename T, typename S>
struct CConverter {{
    static T* convert(CContext& ctx, S& obj) {{
        if constexpr (std::is_integral_v<T>) {{
            auto vec = convertJSArrayToNumberVector<T>(obj);
            auto result = ctx.allocateArray<T>(vec.size());
            memcpy(result, vec.data(), sizeof(T) * vec.size());
            return result;
        }} else if (std::is_same_v<T, typename ArrItem<S>::type>) {{
            using ItemS = typename ArrItem<S>::type;
            auto vec = vecFromJSArray<ItemS>(obj);
            auto result = ctx.allocateArray<T>(vec.size());
            memcpy(result, vec.data(), sizeof(T) * vec.size());
            return result;
        }} else {{
            using ItemS = typename ArrItem<S>::type;
            auto vec = vecFromJSArray<ItemS>(obj);
            auto result = ctx.allocateArray<T>(vec.size());
            for (size_t i = 0; i < vec.size(); ++i) {{
                auto value = CConverter<T, ItemS>::convert(ctx, vec[i]);
                if constexpr (std::is_same_v<T, {str}>) {{
                    result[i] = value;
                }} else {{
                    result[i] = *value;
                }}
            }}
            return result;
        }}
    }}
}};

template <typename T, typename S>
struct CConverter<arr_size<T>, S> {{
    static T* convert(CContext& ctx, S& obj) {{
        auto result = ctx.allocate<T>();
        *result = obj["length"].template as<T>();
        return result;
    }}
}};

template <typename T, typename S>
inline auto cconvert(CContext& ctx, const S& obj) {{
    return CConverter<T, S>::convert(ctx, const_cast<S&>(obj));
}}

template <typename T>
struct CConverter<T, T> {{
    static T* convert(CContext& ctx, T& obj) {{
        return &obj;
    }}
}};

template <typename T, typename S>
struct CConverter<T, std::optional<S>> {{
    static auto convert(CContext& ctx, std::optional<S> obj) {{
        return obj ? cconvert<T>(ctx, obj.value()) : nullptr;
    }}
}};

template <>
struct CConverter<{bool}, bool> {{
    static {bool}* convert(CContext& ctx, bool obj) {{
        auto result = ctx.allocate<{bool}>();
        *result = obj ? 1 : 0;
        return result;
    }}
}};

template <>
struct CConverter<{str}, String> {{
    static {str} convert(CContext& ctx, String& obj) {{
        auto str = obj.as<std::string>();
        auto result = ctx.allocateArray<char>(str.length() + 1);
        memcpy(result, str.c_str(), str.length());
        result[str.length()] = '\0';
        return result;
    }}
}};

template <>
struct CConverter<{char}, String> {{
    static {char}* convert(CContext& ctx, String& obj) {{
        auto str = obj.as<std::string>();
        auto result = ctx.allocate<{char}>();
        *result = '\0';
        if (str.length() > 0) {{
            *result = str[0];
        }}
        return result;
    }}
}};
)",
                 fmt::arg("char", charType),
                 fmt::arg("bool", boolType),
                 fmt::arg("str", strType));
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        IsTrivial trivial;
        node->accept(trivial);
        if (!trivial.trivial) {
            JsName jsname;
            node->accept(jsname);
            CName cname;
            node->accept(cname);
            fmt::println(stream, "template <>");
            fmt::println(stream, "struct CConverter<{}, {}> {{", cname.str, jsname.str);
            fmt::println(stream, "    static {}* convert(CContext& ctx, {}& obj) {{", cname.str, jsname.str);
            fmt::println(stream, "        auto result = ctx.allocate<{}>();", cname.str);
            for (auto field : node->fields) {
                auto type  = getType(field);
                auto isArr = isArray(field);
                auto isR   = isArr || isRef(field) || type->is<ASTStr>();
                JsName jsname;
                field->accept(jsname);
                CName cname;
                field->accept(cname);
                const auto fieldCName = cname.str;
                type->accept(cname);
                const auto typeCName = cname.str;
                if (isArr) {
                    auto [ref, size] = getSizeDecl(field);
                    if (ref) {
                        ref->accept(cname);
                        const auto sizeName = cname.str;
                        getType(ref)->accept(cname);
                        fmt::println(stream,
                                     "        result->{} = *cconvert<arr_size<{}>>(ctx, obj.{});",
                                     sizeName,
                                     cname.str,
                                     jsname.str);
                    } else {
                        fmt::println(stream,
                                     "        auto {}Size = *cconvert<arr_size<size_t>>(ctx, obj.{});",
                                     jsname.str,
                                     jsname.str);
                        fmt::println(stream, "        auto {}MaxSize = std::size(result->{});", jsname.str, fieldCName);
                        fmt::println(
                            stream, "        auto {} = cconvert<{}>(ctx, obj.{});", jsname.str, typeCName, jsname.str);
                        fmt::println(stream,
                                     "        memcpy(result->{}, {}, std::min({}Size, {}MaxSize) * sizeof({}));",
                                     fieldCName,
                                     jsname.str,
                                     jsname.str,
                                     jsname.str,
                                     typeCName);
                        continue;
                    }
                }
                fmt::println(stream,
                             "        result->{} = {}cconvert<{}>(ctx, obj.{});",
                             fieldCName,
                             isR ? "" : "*",
                             typeCName,
                             jsname.str);
            }
            fmt::println(stream, "        return result;");
            fmt::println(stream, "    }}");
            fmt::println(stream, "}};");
            fmt::println(stream, "");
        }
    });
}

static void generateFunctionReturnType(idl::Context& ctx,
                                       std::ostream& stream,
                                       ASTDecl* func,
                                       const std::vector<ASTArg*>& args) {
    ASTDecl* returnType{};
    bool returnTypeIsArray{};
    bool returnTypeIsOptional{};
    for (auto arg : args) {
        if (arg->findAttr<ASTAttrResult>() != nullptr && arg->findAttr<ASTAttrErrorCode>() == nullptr) {
            returnType           = getType(arg);
            returnTypeIsArray    = isArray(arg);
            returnTypeIsOptional = isOptional(arg);
        }
    }
    if (!returnType) {
        returnType           = getType(func);
        returnTypeIsOptional = isOptional(func);
        if (returnType->findAttr<ASTAttrErrorCode>() != nullptr) {
            returnType = nullptr;
        }
    }
    std::string typeName = "void";
    if (returnType) {
        JsName jsname(returnTypeIsArray);
        returnType->accept(jsname);
        typeName = jsname.str;
        if (returnTypeIsOptional) {
            typeName = "std::optional<" + typeName + '>';
        }
    }
    fmt::print(stream, "{}", typeName);
}

static void generateFunctionArgs(idl::Context& ctx,
                                 std::ostream& stream,
                                 ASTDecl* func,
                                 const std::vector<ASTArg*>& args,
                                 const std::map<ASTArg*, ASTArg*>& sizeArgs) {
    bool first = true;
    for (auto arg : args) {
        if (sizeArgs.contains(arg)) {
            continue;
        }

        if (arg->findAttr<ASTAttrThis>() != nullptr || arg->findAttr<ASTAttrResult>() != nullptr ||
            arg->findAttr<ASTAttrUserData>() != nullptr || arg->findAttr<ASTAttrErrorCode>() != nullptr) {
            continue;
        }

        auto argType  = getType(arg);
        auto argIsArr = isArray(arg);

        JsName jsname(argIsArr);
        argType->accept(jsname);
        auto jsTypeName = jsname.str;

        jsname.isArray = false;
        arg->accept(jsname);
        const auto jsArgName = jsname.str;

        if (!first) {
            fmt::print(stream, ", ");
        }
        first = false;

        IsTrivial trivial;
        argType->accept(trivial);
        auto isR     = (!trivial.trivial && !argType->is<ASTBool>()) || argType->is<ASTStruct>();
        auto isConst = isR && (argType->is<ASTStr>() || argIsArr);

        if (isOptional(arg)) {
            isR        = false;
            isConst    = false;
            jsTypeName = "std::optional<" + jsTypeName + '>';
        }

        fmt::print(stream, "{}{}{} {}", isConst ? "const " : "", jsTypeName, isR ? "&" : "", jsArgName);
    }
}

static void generateFunctionCall(idl::Context& ctx,
                                 std::ostream& stream,
                                 ASTDecl* func,
                                 const std::vector<ASTArg*>& args,
                                 bool fetchOnly,
                                 const std::map<ASTArg*, Param>& params) {
    bool first   = true;
    int userData = 0;
    for (auto arg : args) {
        if (!first) {
            fmt::print(stream, ", ");
        }
        first = false;
        if (auto it = params.find(arg); it != params.end()) {
            auto& param = it->second;
            if (param.outParam) {
                if (param.isVector) {
                    if (fetchOnly) {
                        fmt::print(stream, "nullptr");
                    } else {
                        fmt::print(stream, "{}.data()", param.paramName);
                    }
                } else {
                    fmt::print(stream, "&{}", param.paramName);
                }
            } else {
                bool isStr   = !param.isVector && param.type->is<ASTStr>();
                bool isIface = param.type->is<ASTInterface>();
                bool isR     = isRef(arg) || isStr || isIface || param.isCallback || param.isUserdata;
                auto name    = param.paramName;
                if (param.isUserdata) {
                    name = "data" + std::to_string(userData++);
                }
                fmt::print(stream, "{}{}", isR ? "" : "*", name);
            }
        } else if (arg->findAttr<ASTAttrThis>()) {
            fmt::print(stream, "_handle");
        } else {
            assert(!"unreachable code");
        }
    }
}

static void generateFunctionReturn(idl::Context& ctx,
                                   std::ostream& stream,
                                   ASTDecl* func,
                                   const std::string& name,
                                   ASTDecl* decl,
                                   ASTDecl* type,
                                   bool isArr) {
    if (func->findAttr<ASTAttrCtor>()) {
        fmt::println(stream, "        _handle = {};", name);
    } else {
        auto isR = !isArr && isRef(func) && !isOptional(decl);
        JsName jsname;
        jsname.isArray = isArr;
        type->accept(jsname);

        if (isOptional(decl)) {
            jsname.str = "std::optional<" + jsname.str + '>';
        }

        std::string spanBegin;
        std::string spanEnd;
        if (isArr) {
            spanBegin = "std::span{";
            spanEnd   = ".data(), " + name + ".size()}";
        }
        fmt::println(
            stream, "        return jsconvert<{}>({}{}{}{});", jsname.str, isR ? "*" : "", spanBegin, name, spanEnd);
    }
}

static void generateFunction(idl::Context& ctx, std::ostream& stream, ASTDecl* func, const std::vector<ASTArg*>& args) {
    const auto isCtor = func->findAttr<ASTAttrCtor>() != nullptr;
    if (isCtor) {
        JsName jsname;
        func->parent->accept(jsname);
        fmt::print(stream, "    {}", jsname.str);
    } else {
        fmt::print(stream, "    ");
        if (func->findAttr<ASTAttrStatic>() && func->is<ASTMethod>()) {
            fmt::print(stream, "static ");
        }
        JsName jsname;
        func->accept(jsname);
        generateFunctionReturnType(ctx, stream, func, args);
        fmt::print(stream, " {}", jsname.str);
    }
    std::map<ASTArg*, ASTArg*> sizeArgs;
    for (auto arg : args) {
        if (auto attr = arg->findAttr<ASTAttrArray>()) {
            sizeArgs[attr->decl->decl->as<ASTArg>()] = arg;
        }
    }
    fmt::print(stream, "(");
    generateFunctionArgs(ctx, stream, func, args, sizeArgs);
    auto isConst = func->findAttr<ASTAttrConst>() && !func->findAttr<ASTAttrStatic>() && !func->is<ASTFunc>();
    fmt::println(stream, ") {}{{", isConst ? "const " : "");

    std::map<ASTArg*, Param> params;
    bool needFetchSizes{};
    bool needContext{};

    for (auto arg : args) {
        if (arg->findAttr<ASTAttrThis>()) {
            continue;
        }

        params[arg] = {};
        auto& param = params[arg];
        param.type  = getType(arg);

        JsName jsname;
        arg->accept(jsname);
        param.jsArgName = jsname.str;
        param.paramName = jsname.str + "LocalArg";

        if (auto it = sizeArgs.find(arg); it != sizeArgs.end()) {
            param.isSize = true;
            param.refArg = it->second;
        }

        CName cname;
        param.type->accept(cname);
        param.typeName = cname.str;

        param.isVector = arg->findAttr<ASTAttrArray>() != nullptr;

        if (arg->findAttr<ASTAttrOut>() || arg->findAttr<ASTAttrResult>()) {
            param.isResult = arg->findAttr<ASTAttrResult>() != nullptr;
            param.isError  = arg->findAttr<ASTAttrErrorCode>() != nullptr;
            param.outParam = true;

            if (param.isVector) {
                param.refArg   = arg->findAttr<ASTAttrArray>()->decl->decl->as<ASTArg>();
                param.typeName = "std::vector<" + param.typeName + '>';
                needFetchSizes = true;
            }
            if (arg->findAttr<ASTAttrIn>()) {
                param.inParam = true;
                needContext   = true;
            }
            if (arg->findAttr<ASTAttrUserData>()) {
                param.isUserdata = true;
            }
        } else {
            param.inParam = true;
            if (param.isSize) {
                param.refArg->accept(jsname);
                param.jsArgName = jsname.str;
            }
            if (param.type->is<ASTCallback>()) {
                param.isCallback = true;
            } else if (arg->findAttr<ASTAttrUserData>()) {
                param.isUserdata = true;
                param.paramName  = param.paramName + "Data";
            } else {
                needContext = true;
            }
        }
    }

    if (needContext) {
        fmt::println(stream, "        CContext ctx;");
    }

    int userDataCount = 0;
    for (auto& [_, param] : params) {
        if (!param.outParam) {
            if (param.isSize) {
                fmt::println(stream,
                             "        auto {} = cconvert<arr_size<{}>>(ctx, {});",
                             param.paramName,
                             param.typeName,
                             param.jsArgName);
            } else if (param.isCallback) {
                std::string paramData = "data" + std::to_string(userDataCount++);
                JsName jsname;
                func->accept(jsname);

                std::string storeCallback;
                if (func->is<ASTMethod>()) {
                    storeCallback = func->findAttr<ASTAttrStatic>() ? "storeStaticCallback" : "storeCallback";
                } else {
                    storeCallback = "storeFuncCallback";
                }

                fmt::println(stream,
                             "        auto {} = {}(\"{}\", {} ? &{}.value() : nullptr);",
                             paramData,
                             storeCallback,
                             jsname.str,
                             param.jsArgName,
                             param.jsArgName);

                fmt::print(stream, "        auto {} = {} ? [](", param.paramName, param.jsArgName);
                bool first = true;
                ASTArg* userdata{};
                for (auto arg : param.type->as<ASTCallback>()->args) {
                    if (!first) {
                        fmt::print(stream, ", ");
                    }
                    first = false;
                    if (arg->findAttr<ASTAttrConst>() && arg->findAttr<ASTAttrRef>()) {
                        fmt::print(stream, "const ");
                    }
                    bool isR = false;
                    if (arg->findAttr<ASTAttrRef>() || arg->findAttr<ASTAttrOut>()) {
                        isR = true;
                    }
                    CName cname;
                    getType(arg)->accept(cname);
                    fmt::print(stream, "{}{} ", cname.str, isR ? "*" : "");
                    arg->accept(jsname);
                    fmt::print(stream, "{}", jsname.str);
                    if (arg->findAttr<ASTAttrUserData>()) {
                        userdata = arg;
                    }
                }
                userdata->accept(jsname);
                fmt::println(stream, ") {{");
                fmt::println(stream,
                             "            auto& [callback, ctx] = *((std::pair<val, std::shared_ptr<CContext>>*) {});",
                             jsname.str);
                fmt::print(stream, "            ");
                if (!getType(param.type)->is<ASTVoid>()) {
                    fmt::print(stream, "auto functionReturn = ");
                }
                fmt::print(stream, "callback(");
                first = true;
                for (auto arg : param.type->as<ASTCallback>()->args) {
                    if (arg->findAttr<ASTAttrUserData>()) {
                        continue;
                    }
                    if (!first) {
                        fmt::print(stream, ", ");
                    }
                    first          = false;
                    auto type      = getType(arg);
                    auto isArr     = isArray(arg);
                    auto isR       = !isArr && isRef(arg);
                    jsname.isArray = isArr;
                    std::string spanBegin;
                    std::string spanEnd;
                    if (isArr) {
                        const auto [ref, size] = getSizeDecl(arg);
                        std::string value;
                        if (ref) {
                            CName cname;
                            ref->accept(cname);
                            value = "size_t(" + cname.str + ")";
                        } else {
                            value = std::to_string(size);
                        }
                        spanBegin = "std::span{";
                        spanEnd   = ", " + value + "}";
                    }
                    CName cname;
                    type->accept(jsname);
                    arg->accept(cname);
                    fmt::print(
                        stream, "jsconvert<{}>({}{}{}{})", jsname.str, isR ? "*" : "", spanBegin, cname.str, spanEnd);
                }
                fmt::println(stream, ");");
                if (!getType(param.type)->is<ASTVoid>()) {
                    JsName jsname;
                    getType(param.type)->accept(jsname);
                    if (isOptional(param.type)) {
                        jsname.str = "std::optional<" + jsname.str + '>';
                    }
                    CName cname;
                    getType(param.type)->accept(cname);
                    fmt::println(stream, "            ctx = std::make_shared<CContext>();");
                    fmt::println(stream,
                                 "            return cconvert<{}>(*ctx, functionReturn.as<{}>());",
                                 cname.str,
                                 jsname.str);
                }
                fmt::println(stream, "        }} : nullptr;");
            } else if (param.isUserdata) {
            } else {
                fmt::println(stream,
                             "        auto {} = cconvert<{}>(ctx, {});",
                             param.paramName,
                             param.typeName,
                             param.jsArgName);
            }
        }
    }

    for (auto& [_, param] : params) {
        if (param.outParam) {
            std::string value = "{}";
            if (param.inParam && !param.isSize) {
                value = fmt::format(" = cconvert<{}>(ctx, {})", param.typeName, param.jsArgName);
            }
            fmt::println(stream, "        {} {}{};", param.typeName, param.paramName, value);
        }
    }

    if (needFetchSizes) {
        fmt::print(stream, "        ");
        auto checkReturnError = getType(func)->findAttr<ASTAttrErrorCode>() != nullptr;
        if (checkReturnError) {
            fmt::print(stream, "const auto checkReturnError = ");
        }

        CName cname;
        func->accept(cname);
        fmt::print(stream, "{}(", cname.str);
        generateFunctionCall(ctx, stream, func, args, true, params);
        fmt::println(stream, ");");
        if (checkReturnError) {
            fmt::println(stream, "        checkResult(checkReturnError);");
        }
        for (const auto& [_, param] : params) {
            if (param.outParam && param.isError) {
                fmt::println(stream, "        checkResult({});", param.paramName);
            }
        }
        for (const auto& [_, param] : params) {
            if (param.outParam && param.isVector) {
                auto& sizeParam = params[param.refArg];
                fmt::println(stream, "        {}.resize({});", param.paramName, sizeParam.paramName);
            }
        }
    }
    {
        fmt::print(stream, "        ");
        if (!getType(func)->is<ASTVoid>()) {
            fmt::print(stream, "auto functionReturn = ");
        }
        CName cname;
        func->accept(cname);
        fmt::print(stream, "{}(", cname.str);
        generateFunctionCall(ctx, stream, func, args, false, params);
        fmt::println(stream, ");");
        if (getType(func)->findAttr<ASTAttrErrorCode>() != nullptr) {
            fmt::println(stream, "        checkResult(functionReturn);");
        }
        for (const auto& [_, param] : params) {
            if (param.outParam && param.isError) {
                fmt::println(stream, "        checkResult({});", param.paramName);
            }
        }
        for (const auto& [_, param] : params) {
            if (param.outParam && !param.isSize && !param.isResult) {
                // write to out js params
                // assert(!"not implemented");
            }
        }
    }
    bool returned{};
    for (auto& [arg, param] : params) {
        if (param.isResult && !param.isError) {
            generateFunctionReturn(ctx, stream, func, param.paramName, arg, param.type, param.isVector);
            returned = true;
            break;
        }
    }
    if (!returned && !getType(func)->is<ASTVoid>() && getType(func)->findAttr<ASTAttrErrorCode>() == nullptr) {
        if (getType(func)->is<ASTCallback>()) {
            for (auto& [_, param] : params) {
                if (param.isUserdata) {
                    JsName jsname;
                    getType(func)->accept(jsname);
                    fmt::println(stream,
                                 "        return {} ? std::make_optional({}((((std::pair<val, "
                                 "std::shared_ptr<CContext>>*) {})->first))) : std::nullopt;",
                                 param.paramName,
                                 jsname.str,
                                 param.paramName);
                    break;
                }
            }

        } else {
            generateFunctionReturn(ctx, stream, func, "functionReturn", func, getType(func), false);
        }
    }

    fmt::println(stream, "    }}");
    fmt::println(stream, "");
}

static void generateCppClasses(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTInterface>([&ctx, &stream](ASTInterface* node) {
        bool hasCallbacks{};
        bool hasStaticCallbacks{};
        for (auto method : node->methods) {
            for (auto arg : method->args) {
                if (arg->findAttr<ASTAttrIn>() && getType(arg)->is<ASTCallback>()) {
                    if (method->findAttr<ASTAttrStatic>()) {
                        hasStaticCallbacks = true;
                    } else {
                        hasCallbacks = true;
                    }
                    break;
                }
            }
            if (hasCallbacks && hasStaticCallbacks) {
                break;
            }
        }

        JsName jsname;
        node->accept(jsname);
        const auto jsTypeStr = jsname.str;
        CName cname;
        node->accept(cname);
        const auto handleTypeStr = cname.str;
        fmt::println(stream, "class {} {{", jsTypeStr);
        fmt::println(stream, "public:");
        for (auto method : node->methods) {
            if (method->findAttr<ASTAttrCtor>() != nullptr) {
                generateFunction(ctx, stream, method, method->args);
            }
        }
        fmt::println(stream, "    {}({} handle) : _handle(handle) {{", jsTypeStr, handleTypeStr);
        fmt::println(stream, "    }}");
        fmt::println(stream, "");

        ASTMethod* reference{};
        for (auto method : node->methods) {
            if (method->findAttr<ASTAttrRefInc>() != nullptr) {
                reference = method;
                break;
            }
        }
        if (reference || hasCallbacks) {
            fmt::print(stream, "    {}(const {}& other) : ", jsTypeStr, jsTypeStr);
            if (hasCallbacks) {
                fmt::print(stream, "_callbacks(other._callbacks), ");
            }
            fmt::println(stream, "_handle(other._handle) {{");
            if (reference) {
                reference->accept(cname);
                fmt::println(stream, "        if (_handle) {{");
                fmt::println(stream, "            {}(_handle);", cname.str);
                fmt::println(stream, "        }}");
            }
            fmt::println(stream, "    }}");
            fmt::println(stream, "");
        }
        for (auto method : node->methods) {
            if (method->findAttr<ASTAttrDestroy>() != nullptr) {
                method->accept(cname);
                fmt::println(stream, "    ~{}() {{", jsTypeStr);
                fmt::println(stream, "        {}(_handle);", cname.str);
                fmt::println(stream, "    }}");
                fmt::println(stream, "");
                break;
            }
        }
        for (auto method : node->methods) {
            if (method->findAttr<ASTAttrCtor>() == nullptr && method->findAttr<ASTAttrRefInc>() == nullptr &&
                method->findAttr<ASTAttrDestroy>() == nullptr) {
                generateFunction(ctx, stream, method, method->args);
            }
        }

        fmt::println(stream, "    {} handle() noexcept {{", handleTypeStr);
        fmt::println(stream, "        return _handle;");
        fmt::println(stream, "    }}");
        fmt::println(stream, "");
        fmt::println(stream, "private:");
        if (hasStaticCallbacks) {
            ASTDeclRef dataRef{};
            dataRef.parent = ctx.api();
            dataRef.name   = "Data";
            CName cname;
            ctx.resolveType(&dataRef)->accept(cname);
            fmt::println(
                stream, "    static {} storeStaticCallback(const std::string& func, val* callback) {{", cname.str);
            fmt::println(stream, "        if (callback) {{");
            fmt::println(
                stream,
                "            return ({}) &_staticCallbacks.insert_or_assign(func, std::make_pair(val(*callback), "
                "nullptr)).first->second;",
                cname.str);
            fmt::println(stream, "        }}");
            fmt::println(stream, "        _staticCallbacks.erase(func);");
            fmt::println(stream, "        return nullptr;");
            fmt::println(stream, "    }}");
            fmt::println(stream, "");
            fmt::println(
                stream,
                "    static std::map<std::string, std::pair<val, std::shared_ptr<CContext>>> _staticCallbacks;");
        }
        if (hasCallbacks) {
            ASTDeclRef dataRef{};
            dataRef.parent = ctx.api();
            dataRef.name   = "Data";
            CName cname;
            ctx.resolveType(&dataRef)->accept(cname);
            fmt::println(stream, "    {} storeCallback(const std::string& func, val* callback) {{", cname.str);
            fmt::println(stream, "        if (callback) {{");
            fmt::println(stream,
                         "            return ({}) &_callbacks.insert_or_assign(func, std::make_pair(val(*callback), "
                         "nullptr)).first->second;",
                         cname.str);
            fmt::println(stream, "        }}");
            fmt::println(stream, "        _callbacks.erase(func);");
            fmt::println(stream, "        return nullptr;");
            fmt::println(stream, "    }}");
            fmt::println(stream, "");
            fmt::println(stream,
                         "    std::map<std::string, std::pair<val, std::shared_ptr<CContext>>> _callbacks{{}};");
        }
        fmt::println(stream, "    {} _handle{{}};", handleTypeStr);
        fmt::println(stream, "}};");
        if (hasStaticCallbacks) {
            fmt::println(stream,
                         "std::map<std::string, std::pair<val, std::shared_ptr<CContext>>> {}::_staticCallbacks{{}};",
                         jsTypeStr);
        }
        fmt::println(stream, "template <>");
        fmt::println(stream, "struct JsConverter<{}, {}> {{", jsTypeStr, handleTypeStr);
        fmt::println(stream, "    static {} convert(const {}& obj) {{", jsTypeStr, handleTypeStr);
        fmt::println(stream, "        return {}(obj);", jsTypeStr);
        fmt::println(stream, "    }}");
        fmt::println(stream, "}};");
        fmt::println(stream, "template <>");
        fmt::println(stream, "struct CConverter<{}, {}> {{", handleTypeStr, jsTypeStr);
        fmt::println(stream, "    static {} convert(CContext& ctx, {}& obj) {{", handleTypeStr, jsTypeStr);
        fmt::println(stream, "        return obj.handle();");
        fmt::println(stream, "    }}");
        fmt::println(stream, "}};");
        fmt::println(stream, "");
    });
}

static void generateBeginBindings(idl::Context& ctx, std::ostream& stream) {
    const auto moduleName = convert(ctx.api()->name, Case::CamelCase);
    fmt::println(stream, "EMSCRIPTEN_BINDINGS({}) {{", moduleName);
}

static void generateFuncCallbackStore(idl::Context& ctx, std::ostream& stream) {
    bool hasCallbacks{};
    ctx.filter<ASTFunc>([&hasCallbacks](ASTFunc* node) {
        for (auto arg : node->args) {
            if (arg->findAttr<ASTAttrIn>() && getType(arg)->is<ASTCallback>()) {
                hasCallbacks = true;
                return false;
            }
        }
        return true;
    });
    if (hasCallbacks) {
        ASTDeclRef dataRef{};
        dataRef.parent = ctx.api();
        dataRef.name   = "Data";
        CName cname;
        ctx.resolveType(&dataRef)->accept(cname);
        fmt::println(stream, "{} storeFuncCallback(const std::string& func, val* callback) {{", cname.str);
        fmt::println(stream,
                     "    static std::map<std::string, std::pair<val, std::shared_ptr<CContext>>> callbacks{{}};");
        fmt::println(stream, "    if (callback) {{");
        fmt::println(stream,
                     "        return ({}) &callbacks.insert_or_assign(func, std::make_pair(val(*callback), "
                     "nullptr)).first->second;",
                     cname.str);
        fmt::println(stream, "    }}");
        fmt::println(stream, "    callbacks.erase(func);");
        fmt::println(stream, "    return nullptr;");
        fmt::println(stream, "}}");
        fmt::println(stream, "");
    }
}

static void generateCppFunctions(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTFunc>([&ctx, &stream](ASTFunc* node) {
        if (!node->findAttr<ASTAttrErrorCode>()) {
            generateFunction(ctx, stream, node, node->args);
        }
    });
}

static void generateRegisterTypes(idl::Context& ctx, std::ostream& stream) {
    auto isArr   = false;
    auto addType = [&stream, &isArr](ASTDecl* decl) {
        if (!decl->is<ASTVoid>() && !decl->is<ASTChar>()) {
            JsName jsname(isArr);
            decl->accept(jsname);
            fmt::println(stream, "    register_type<{}>(\"{}\");", jsname.str, getNameTS(decl, isArr));
        }
    };
    fmt::println(stream, "    register_type<String>(\"string\");");
    ctx.filter<ASTCallback>(addType);
    isArr = true;
    ctx.filter<ASTTrivialType>(addType);
    ctx.filter<ASTStruct>(addType);
    ctx.filter<ASTInterface>(addType);
    ctx.filter<ASTCallback>(addType);
    fmt::println(stream, "");
}

static void generateRegisterOptionals(idl::Context& ctx, std::ostream& stream) {
    auto addOptional = [&stream](ASTDecl* decl) {
        if (!decl->is<ASTVoid>() && !decl->is<ASTChar>() && !decl->findAttr<ASTAttrErrorCode>()) {
            JsName jsname;
            decl->accept(jsname);
            fmt::println(stream, "    register_optional<{}>();", jsname.str);
        }
    };
    ctx.filter<ASTTrivialType>(addOptional);
    ctx.filter<ASTEnum>(addOptional);
    ctx.filter<ASTStruct>(addOptional);
    ctx.filter<ASTInterface>(addOptional);
    ctx.filter<ASTCallback>(addOptional);
    fmt::println(stream, "");
}

static void generateEnums(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTEnum>([&stream](ASTEnum* node) {
        if (node->findAttr<ASTAttrErrorCode>() == nullptr) {
            CName cname;
            node->accept(cname);
            fmt::println(stream, "    enum_<{}>(\"{}\")", cname.str, getNameTS(node));
            for (auto ec : node->consts) {
                std::vector<int>* nums = nullptr;
                if (auto attr = ec->findAttr<ASTAttrTokenizer>()) {
                    nums = &attr->nums;
                }
                auto name = convert(ec->name, Case::ScreamingSnakeCase, nums);
                ec->accept(cname);
                fmt::println(stream, "        .value(\"{}\", {})", name, cname.str);
            }
            fmt::println(stream, "        ;");
            fmt::println(stream, "");
        }
    });
}

static void generateValueObjects(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        CName cname;
        JsName jsname;
        node->accept(jsname);
        IsTrivial trivial;
        node->accept(trivial);
        const auto typeName = jsname.str;
        fmt::println(stream, "    value_object<{}>(\"{}\")", typeName, getNameTS(node));
        for (auto field : node->fields) {
            field->accept(jsname);
            auto fieldNameJs = jsname.str;
            std::string fieldNameCpp;
            if (trivial.trivial) {
                field->accept(cname);
                fieldNameCpp = cname.str;
            } else {
                field->accept(jsname);
                fieldNameCpp = jsname.str;
            }
            fmt::println(stream, "        .field(\"{}\", &{}::{})", fieldNameJs, typeName, fieldNameCpp);
        }
        fmt::println(stream, "        ;");
        fmt::println(stream, "");
    });
}

static void generateClasses(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTInterface>([&ctx, &stream](ASTInterface* node) {
        std::set<ASTDecl*> excluded;
        JsName jsname;
        node->accept(jsname);
        const auto typeName = jsname.str;
        fmt::println(stream, "    class_<{}>(\"{}\")", typeName, typeName);
        for (auto method : node->methods) {
            if (method->findAttr<ASTAttrCtor>()) {
                if (method->args.size() == 0 ||
                    (method->args.size() == 1 && method->args[0]->findAttr<ASTAttrResult>())) {
                    fmt::println(stream, "        .constructor()");
                } else {
                    std::ostringstream ss;
                    generateFunctionArgs(ctx, ss, method, method->args, {});
                    fmt::println(stream, "        .constructor<{}>()", ss.str());
                }
            }
        }
        for (auto prop : node->props) {
            if (prop->findAttr<ASTAttrStatic>()) {
                // Embind unsupported static properties
                continue;
            }
            prop->accept(jsname);
            const auto propName = jsname.str;
            auto getter         = prop->findAttr<ASTAttrGet>();
            auto setter         = prop->findAttr<ASTAttrSet>();
            if (getter && setter) {
                getter->decl->decl->accept(jsname);
                const auto getterName = jsname.str;
                setter->decl->decl->accept(jsname);
                const auto setterName = jsname.str;
                fmt::println(stream,
                             "        .property(\"{}\", &{}::{}, &{}::{})",
                             propName,
                             typeName,
                             getterName,
                             typeName,
                             setterName);
                excluded.insert(getter->decl->decl);
                excluded.insert(setter->decl->decl);
            } else if (getter) {
                getter->decl->decl->accept(jsname);
                const auto getterName = jsname.str;
                fmt::println(stream, "        .property(\"{}\", &{}::{})", propName, typeName, getterName);
                excluded.insert(getter->decl->decl);
            } else if (setter) {
                setter->decl->decl->accept(jsname);
                const auto setterName = jsname.str;
                fmt::println(stream, "        .property(\"{}\", &{}::{})", propName, typeName, setterName);
                excluded.insert(setter->decl->decl);
            }
        }
        for (auto ev : node->events) {
            if (ev->findAttr<ASTAttrStatic>()) {
                // Embind unsupported static properties
                continue;
            }
            ev->accept(jsname);
            const auto evName = jsname.str;
            auto getter       = ev->findAttr<ASTAttrGet>();
            auto setter       = ev->findAttr<ASTAttrSet>();
            if (getter && setter) {
                getter->decl->decl->accept(jsname);
                const auto getterName = jsname.str;
                setter->decl->decl->accept(jsname);
                const auto setterName = jsname.str;
                fmt::println(stream,
                             "        .property(\"{}\", &{}::{}, &{}::{})",
                             evName,
                             typeName,
                             getterName,
                             typeName,
                             setterName);
                excluded.insert(getter->decl->decl);
                excluded.insert(setter->decl->decl);
            } else if (getter) {
                getter->decl->decl->accept(jsname);
                const auto getterName = jsname.str;
                fmt::println(stream, "        .property(\"{}\", &{}::{})", evName, typeName, getterName);
                excluded.insert(getter->decl->decl);
            } else if (setter) {
                setter->decl->decl->accept(jsname);
                const auto setterName = jsname.str;
                fmt::println(stream, "        .property(\"{}\", &{}::{})", evName, typeName, setterName);
                excluded.insert(setter->decl->decl);
            }
        }
        for (auto method : node->methods) {
            if (excluded.contains(method)) {
                continue;
            }
            if (method->findAttr<ASTAttrCtor>() || method->findAttr<ASTAttrRefInc>() ||
                method->findAttr<ASTAttrDestroy>()) {
                continue;
            }
            method->accept(jsname);
            const auto methodName = jsname.str;
            auto isClassFunc      = method->findAttr<ASTAttrStatic>() != nullptr;
            fmt::println(stream,
                         "        .{}function(\"{}\", &{}::{})",
                         isClassFunc ? "class_" : "",
                         methodName,
                         typeName,
                         methodName);
        }
        fmt::println(stream, "        ;");
        fmt::println(stream, "");
    });
}

static void generateFunctions(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTFunc>([&stream](ASTFunc* node) {
        if (!node->findAttr<ASTAttrErrorCode>()) {
            JsName jsname;
            node->accept(jsname);
            fmt::println(stream, "    function(\"{}\", &{});", jsname.str, jsname.str);
        }
    });
}

static void generateEndBindings(idl::Context& ctx, std::ostream& stream) {
    fmt::println(stream, "}}");
}

void generateJs(idl::Context& ctx,
                const std::filesystem::path& out,
                idl_write_callback_t writer,
                idl_data_t writerData,
                std::span<idl_utf8_t> includes) {
    auto stream = createStream(ctx, out, writer, writerData);
    generateComment(ctx, stream.stream);
    generateIncludes(ctx, stream.stream);
    generateTypes(ctx, stream.stream);
    generateExceptions(ctx, stream.stream);
    generateNonTrivialTypes(ctx, stream.stream);
    generateClassDeclarations(ctx, stream.stream);
    generateArrItems(ctx, stream.stream);
    generateJsConverters(ctx, stream.stream);
    generateCConverters(ctx, stream.stream);
    generateCppClasses(ctx, stream.stream);
    generateFuncCallbackStore(ctx, stream.stream);
    generateCppFunctions(ctx, stream.stream);
    generateBeginBindings(ctx, stream.stream);
    generateRegisterTypes(ctx, stream.stream);
    generateRegisterOptionals(ctx, stream.stream);
    generateEnums(ctx, stream.stream);
    generateValueObjects(ctx, stream.stream);
    generateClasses(ctx, stream.stream);
    generateFunctions(ctx, stream.stream);
    generateEndBindings(ctx, stream.stream);
    if (stream.writer) {
        const std::string data = stream.sstream->str();
        idl_source_t source{ stream.filename.c_str(), data.c_str(), (idl_uint32_t) data.length() + 1 };
        stream.writer(&source, stream.writerData);
    }
}
