
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

static bool isArray(ASTDecl* decl) noexcept {
    return decl->findAttr<ASTAttrArray>() != nullptr;
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

static Stream createStream(idl::Context& ctx,
                           const std::filesystem::path& out,
                           idl_write_callback_t writer,
                           idl_data_t writerData) {
    std::filesystem::create_directories(out);
    auto mName = out / (convert(ctx.api()->name, Case::LispCase) + ".js.cpp");
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
        const auto prefix = convert(ctx.api()->name, Case::PascalCase);

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

static void generateBeginBindings(idl::Context& ctx, std::ostream& stream) {
    const auto moduleName = convert(ctx.api()->name, Case::CamelCase);
    fmt::println(stream, "EMSCRIPTEN_BINDINGS({}) {{", moduleName);
}

static void generateEndBindings(idl::Context& ctx, std::ostream& stream) {
    fmt::println(stream, "}}");
}

void generateJs(idl::Context& ctx,
                const std::filesystem::path& out,
                idl_write_callback_t writer,
                idl_data_t writerData,
                std::span<idl_utf8_t> includes) {
    auto stream      = createStream(ctx, out, writer, writerData);
    ASTEnum* errcode = nullptr;
    generateIncludes(ctx, stream.stream);
    generateTypes(ctx, stream.stream);
    generateExceptions(ctx, stream.stream, errcode);
    generateNonTrivialTypes(ctx, stream.stream);
    generateClassDeclarations(ctx, stream.stream);
    generateArrItems(ctx, stream.stream);
    generateJsConverters(ctx, stream.stream);
    generateCConverters(ctx, stream.stream);
    generateBeginBindings(ctx, stream.stream);
    generateEndBindings(ctx, stream.stream);
}
