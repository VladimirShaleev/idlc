
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

    void visit(ASTArg* node) override {
        str = camelCase(node);
    }

    void visit(ASTCallback* node) override {
        str = std::string(isArray ? "Arr" : "") + pascalCase(node);
    }

    void visit(ASTEnum* node) override {
        str = std::string(isArray ? "Arr" : "") + pascalCase(node);
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
        str = std::string(isArray ? "Arr" : "") + pascalCase(node);
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

static void generateTypes(idl::Context& ctx, std::ostream& stream) {
    fmt::println(stream, "EMSCRIPTEN_DECLARE_VAL_TYPE(String);");
    ctx.filter<ASTTrivialType>([&stream](ASTTrivialType* trivialType) {
        if (!trivialType->is<ASTVoid>()) {
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
        JsName jsname;
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

static void generateUtils(idl::Context& ctx, std::ostream& stream) {
    fmt::println(stream, "template <typename Arr, typename T, typename S>");
    fmt::println(stream, "Arr createArray(const S* ptr, size_t size) {{");
    fmt::println(stream, "    Arr arr(val::array());");
    fmt::println(stream, "    for (size_t i = 0; i < size; ++i) {{");
    fmt::println(stream, "        if constexpr (std::is_same_v<S, idl_utf8_t>) {{");
    fmt::println(stream, "            arr.template call<void>(\"push\", String(val::u8string(ptr[i])));");
    fmt::println(stream, "        }} else {{");
    fmt::println(stream, "            arr.template call<void>(\"push\", val(T(ptr[i])));");
    fmt::println(stream, "        }}");
    fmt::println(stream, "    }}");
    fmt::println(stream, "    return arr;");
    fmt::println(stream, "}}");
    fmt::println(stream, "");
}

static void generateNonTrivialTypes(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        IsTrivial trivial{};
        node->accept(trivial);
        if (!trivial.trivial) {
            JsName jsname;
            node->accept(jsname);
            auto structName = jsname.str;

            fmt::println(stream, "class {} {{", structName);
            fmt::println(stream, "public:");

            std::vector<std::tuple<ASTField*, std::string, ASTDecl*, bool, bool, bool>> fields;

            for (auto field : node->fields) {
                field->accept(jsname);
                auto type    = field->findAttr<ASTAttrType>()->type->decl;
                auto isArray = field->findAttr<ASTAttrArray>() != nullptr;
                auto isRef   = field->findAttr<ASTAttrRef>();

                Value value(isArray);
                field->accept(value);

                IsTrivial trivial(isArray);
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
            fmt::println(stream, "    {}() = default;", structName);
            fmt::println(stream, "    {}(const {}& obj)", structName, cStruct);
            fmt::print(stream, "        : ");
            auto first = true;
            for (const auto& [field, name, type, trivial, isArray, isRef] : fields) {
                field->accept(cname);
                if (!first) {
                    fmt::println(stream, "");
                    fmt::print(stream, "        , ");
                }
                first = false;
                if (trivial) {
                    fmt::print(stream, "{}({}obj.{})", name, isRef ? "*" : "", cname.str);
                } else if (isArray) {
                    JsName arrName(true);
                    type->accept(arrName);
                    IsTrivial typeIsTrivial;
                    type->accept(typeIsTrivial);
                    std::string arrItemName;
                    if (typeIsTrivial.trivial) {
                        CName cnameType;
                        type->accept(cnameType);
                        arrItemName = cnameType.str;
                    } else {
                        JsName jsnameType;
                        type->accept(jsnameType);
                        arrItemName = jsnameType.str;
                    }
                    if (auto arr = field->findAttr<ASTAttrArray>(); arr->ref) {
                        CName count;
                        arr->decl->decl->accept(count);
                        fmt::print(stream,
                                   "{}(createArray<{}, {}>(obj.{}, (size_t) obj.{}))",
                                   name,
                                   arrName.str,
                                   arrItemName,
                                   cname.str,
                                   count.str);
                    } else {
                        fmt::print(stream,
                                   "{}(createArray<{}, {}>(obj.{}, {}))",
                                   name,
                                   arrName.str,
                                   arrItemName,
                                   cname.str,
                                   arr->size);
                    }
                } else if (type->is<ASTStruct>()) {
                    fmt::print(stream, "{}({}obj.{})", name, isRef ? "*" : "", cname.str);
                } else if (type->is<ASTStr>()) {
                    fmt::print(stream, "{}(String(val::u8string(obj.{})))", name, cname.str);
                } else if (type->is<ASTBool>()) {
                    fmt::print(stream, "{}({}obj.{})", name, isRef ? "*" : "", cname.str);
                }
            }
            fmt::println(stream, " {{");
            fmt::println(stream, "    }}");

            fmt::println(stream, "");
            fmt::println(stream, "    {}* get() {{", cStruct);
            for (const auto& [field, name, type, trivial, isArray, isRef] : fields) {
                field->accept(cname);
                if (trivial) {
                    fmt::println(stream, "        _raw.{} = {}{};", cname.str, isRef ? "&" : "", name);
                } else if (isArray) {
                    if (type->is<ASTStruct>()) {
                        IsTrivial typeTrivial{};
                        type->accept(typeTrivial);
                        if (typeTrivial.trivial) {
                            CName ctype;
                            type->accept(ctype);
                            fmt::println(stream, "        _raw_{} = vecFromJSArray<{}>({});", name, ctype.str, name);
                        } else {
                            type->accept(jsname);
                            fmt::println(stream,
                                         "        auto raw_{}Vec = vecFromJSArray<{}*>({}, allow_raw_pointers());",
                                         name,
                                         jsname.str,
                                         name);
                            fmt::println(stream, "        _raw_{}.resize(raw_{}Vec.size());", name, name);
                            fmt::println(stream, "        for (size_t i = 0; i < raw_{}Vec.size(); ++i) {{", name);
                            fmt::println(stream, "            _raw_{}[i] = *raw_{}Vec[i]->get();", name, name);
                            fmt::println(stream, "        }}");
                        }
                    } else if (type->is<ASTStr>()) {
                        fmt::println(stream, "        _raw_{}Data = vecFromJSArray<std::string>({});", name, name);
                        fmt::println(stream, "        _raw_{}.resize(_raw_{}Data.size());", name, name);
                        fmt::println(stream, "        for (size_t i = 0; i < _raw_{}Data.size(); ++i) {{", name);
                        fmt::println(stream, "            _raw_{}[i] = _raw_{}Data[i].c_str();", name, name);
                        fmt::println(stream, "        }}");
                    } else {
                        CName ctype;
                        type->accept(ctype);
                        fmt::println(
                            stream, "        _raw_{} = convertJSArrayToNumberVector<{}>({});", name, ctype.str, name);
                    }
                    if (field->findAttr<ASTAttrArray>()->ref) {
                        fmt::println(stream, "        _raw.{} = _raw_{}.data();", cname.str, name);
                    } else {
                        fmt::println(stream,
                                     "        for (size_t i = 0; i < std::size(_raw.{}) && i < _raw_{}.size(); ++i) {{",
                                     cname.str,
                                     name);
                        fmt::println(stream, "            _raw.{}[i] = _raw_{}[i];", cname.str, name);
                        fmt::println(stream, "        }}");
                    }
                } else if (type->is<ASTStr>()) {
                    fmt::println(stream, "        _raw_{} = {}.as<std::string>();", name, name);
                    fmt::println(stream, "        _raw.{} = _raw_{}.c_str();", cname.str, name);
                } else if (type->is<ASTStruct>()) {
                    if (isRef) {
                        fmt::println(stream, "        _raw_{} = *{}.get();", name, name);
                        fmt::println(stream, "        _raw.{} = &_raw_{};", cname.str, name);
                    } else {
                        fmt::println(stream, "        _raw.{} = *{}.get();", cname.str, name);
                    }
                } else if (type->is<ASTBool>()) {
                    if (isRef) {
                        fmt::println(stream, "        _raw_{} = {} ? 1 : 0;", name, name);
                        fmt::println(stream, "        _raw.{} = {}_raw_{};", cname.str, isRef ? "&" : "", name);
                    } else {
                        fmt::println(stream, "        _raw.{} = {} ? 1 : 0;", cname.str, name);
                    }
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
                    if (!(type->is<ASTBool>() && !isRef)) {
                        fmt::println(stream, "    {} _raw_{}{{}};", cname.str, name);
                    }
                } else if (type->is<ASTStruct>() && isRef) {
                    fmt::println(stream, "    {} _raw_{}{{}};", cname.str, name);
                }
            }
            fmt::println(stream, "    {} _raw{{}};", cStruct);

            fmt::println(stream, "}};");
            fmt::println(stream, "");
        }
    });
}

static void generateClasses(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTInterface>([&stream](ASTInterface* node) {
        JsName jsname;
        node->accept(jsname);
        const auto className = jsname.str;

        auto addFunc = [&stream, &className](ASTMethod* method) {
            JsName jsname;
            method->accept(jsname);
            const auto isConst  = method->findAttr<ASTAttrConst>() != nullptr;
            const auto isStatic = method->findAttr<ASTAttrStatic>() != nullptr;
            const auto isCtor   = method->findAttr<ASTAttrCtor>() != nullptr;
            const auto isArray  = method->findAttr<ASTAttrArray>() != nullptr;
            const auto isRetRef = method->findAttr<ASTAttrRef>() != nullptr;
            std::string returnTypeStr;
            ASTType* returnType{};
            ASTEnum* errorCode{};
            ASTArg* resultArg{};
            bool errorCodeInArg{};
            std::string resultValue{};
            std::map<ASTArg*, ASTArg*> sizeArgs{};
            JsName jsretname(isArray);
            returnType = method->findAttr<ASTAttrType>()->type->decl->as<ASTType>();
            returnType->accept(jsretname);
            returnTypeStr = jsretname.str;
            if (returnType->is<ASTVoid>()) {
                returnType = nullptr;
            } else if (!isArray) {
                IsTrivial trivial;
                returnType->accept(trivial);
                if (trivial.trivial) {
                    CName cname;
                    returnType->accept(cname);
                    returnTypeStr = cname.str;
                }
            }
            if (returnType->is<ASTEnum>() && returnType->findAttr<ASTAttrErrorCode>() != nullptr) {
                errorCode     = returnType->as<ASTEnum>();
                returnType    = nullptr;
                returnTypeStr = "void";
            }
            if (returnType == nullptr) {
                for (auto arg : method->args) {
                    if (arg->findAttr<ASTAttrResult>() != nullptr) {
                        resultArg             = arg;
                        returnType            = arg->findAttr<ASTAttrType>()->type->decl->as<ASTType>();
                        const auto isRetArray = arg->findAttr<ASTAttrArray>() != nullptr;
                        JsName jsretname(isRetArray);
                        returnType->accept(jsretname);
                        returnTypeStr = jsretname.str;
                    }
                    if (auto en = arg->as<ASTEnum>()) {
                        if (en->findAttr<ASTAttrErrorCode>() != nullptr &&
                            (en->findAttr<ASTAttrOut>() != nullptr || en->findAttr<ASTAttrResult>() != nullptr)) {
                            errorCode      = en;
                            errorCodeInArg = true;
                        }
                    }
                }
            }
            for (auto arg : method->args) {
                auto attrArr = arg->findAttr<ASTAttrArray>();
                if (attrArr && attrArr->ref) {
                    sizeArgs[attrArr->decl->decl->as<ASTArg>()] = arg;
                }
            }
            fmt::print(stream,
                       "    {}{}{}(",
                       isStatic && !isCtor ? "static " : "",
                       isCtor ? "" : returnTypeStr + ' ',
                       isCtor ? className : jsname.str);
            bool first = true;
            for (auto arg : method->args) {
                if (arg == resultArg || sizeArgs.contains(arg) || arg->findAttr<ASTAttrThis>() != nullptr ||
                    arg->findAttr<ASTAttrUserData>() != nullptr) {
                    continue;
                }
                auto type    = arg->findAttr<ASTAttrType>()->type->decl;
                auto typeArr = arg->findAttr<ASTAttrArray>() != nullptr;

                IsTrivial trivial(typeArr);
                type->accept(trivial);

                auto hasRef = false;
                std::string argTypeStr;
                if (trivial.trivial) {
                    CName cname;
                    type->accept(cname);
                    argTypeStr = cname.str;
                    if (!type->is<ASTTrivialType>()) {
                        argTypeStr += '&';
                        hasRef = true;
                    }
                } else {
                    JsName argType(typeArr);
                    type->accept(argType);
                    argTypeStr = argType.str;
                    if (!type->is<ASTBool>()) {
                        argTypeStr += '&';
                        hasRef = true;
                    }
                }
                if (!hasRef && arg->findAttr<ASTAttrOut>()) {
                    argTypeStr += '&';
                    hasRef = true;
                }

                JsName argName;
                arg->accept(argName);
                if (!first) {
                    fmt::print(stream, ", ");
                }
                first = false;
                fmt::print(stream, "{} {}", argTypeStr, argName.str);
            }
            fmt::println(stream, ") {}{{", isConst ? "const " : "");
            for (auto arg : method->args) {
                JsName argName;
                arg->accept(argName);
                auto isArgArr = arg->findAttr<ASTAttrArray>() != nullptr;
                auto argType  = arg->findAttr<ASTAttrType>()->type->decl;
                IsTrivial trival(isArgArr);
                argType->accept(trival);
                CName ctype;
                argType->accept(ctype);
                if (arg->findAttr<ASTAttrOut>() != nullptr || arg->findAttr<ASTAttrResult>()) {
                    if (isCtor && argType == method->parent) {
                        continue;
                    }
                    if (argType->is<ASTInterface>()) {
                        CName ctype;
                        argType->accept(ctype);
                        resultValue = argName.str + "Ret";
                        fmt::println(stream, "        {} {}{{}};", ctype.str, resultValue);
                    }
                } else if (!trival.trivial) {
                    if (isArgArr) {
                        if (argType->is<ASTStruct>()) {
                            IsTrivial typeTrivial{};
                            argType->accept(typeTrivial);
                            if (typeTrivial.trivial) {
                                CName ctype;
                                argType->accept(ctype);
                                fmt::print(stream,
                                           "        std::vector<{}> {}Arg = vecFromJSArray<{}>({});",
                                           ctype.str,
                                           argName.str,
                                           ctype.str,
                                           argName.str);
                            } else {
                                CName ctype;
                                argType->accept(ctype);
                                argType->accept(jsname);
                                fmt::println(
                                    stream,
                                    "        std::vector<{}*> {}ArgJs = vecFromJSArray<{}*>({}, allow_raw_pointers());",
                                    jsname.str,
                                    argName.str,
                                    jsname.str,
                                    argName.str);
                                fmt::println(stream, "        std::vector<{}> {}Arg;", ctype.str, argName.str);
                                fmt::println(stream, "        {}Arg.resize({}ArgJs.size());", argName.str, argName.str);
                                fmt::println(
                                    stream, "        for (size_t i = 0; i < {}ArgJs.size(); ++i) {{", argName.str);
                                fmt::println(
                                    stream, "            {}Arg[i] = *{}ArgJs[i]->get();", argName.str, argName.str);
                                fmt::println(stream, "        }}");
                            }
                        } else if (argType->is<ASTStr>()) {
                            CName ctype;
                            argType->accept(ctype);
                            fmt::println(stream,
                                         "        std::vector<std::string> {}ArgJs = vecFromJSArray<std::string>({});",
                                         argName.str,
                                         argName.str);
                            fmt::println(stream, "        std::vector<{}> {}Arg;", ctype.str, argName.str);
                            fmt::println(stream, "        {}Arg.resize({}ArgJs.size());", argName.str, argName.str);
                            fmt::println(stream, "        for (size_t i = 0; i < {}ArgJs.size(); ++i) {{", argName.str);
                            fmt::println(
                                stream, "            {}Arg[i] = {}ArgJs[i].c_str();", argName.str, argName.str);
                            fmt::println(stream, "        }}");
                        } else {
                            CName ctype;
                            argType->accept(ctype);
                            fmt::println(stream,
                                         "        {}Arg = convertJSArrayToNumberVector<{}>({});",
                                         argName.str,
                                         ctype.str,
                                         argName.str);
                        }
                    } else if (arg->is<ASTStruct>()) {
                        fmt::println(stream, "        {} {}Arg = {}.get();", ctype.str, argName.str, argName.str);
                    } else if (argType->is<ASTStr>()) {
                        fmt::println(
                            stream, "        std::string {}Arg = {}.as<std::string>();", argName.str, argName.str);
                    }
                }
            }
            fmt::print(stream, "        ");
            if (errorCode && !errorCodeInArg) {
                fmt::print(stream, "checkResult(");
            } else if (returnType && !isCtor && resultArg == nullptr) {
                fmt::print(stream, "return ");
            }
            CName cname;
            method->accept(cname);
            fmt::print(stream, "{}(", cname.str);
            first = true;
            for (auto arg : method->args) {
                if (!first) {
                    fmt::print(stream, ", ");
                }
                first = false;

                auto argType = arg->findAttr<ASTAttrType>()->type->decl;
                if (arg->findAttr<ASTAttrOut>() != nullptr || arg->findAttr<ASTAttrResult>()) {
                    if (isCtor && argType == method->parent) {
                        fmt::print(stream, "&_handle");
                    } else {
                        JsName jsname;
                        arg->accept(jsname);
                        if (argType->is<ASTInterface>()) {
                            fmt::print(stream, "&{}Ret", jsname.str);
                        }
                    }
                } else if (arg->findAttr<ASTAttrThis>() != nullptr) {
                    fmt::print(stream, "_handle");
                } else {
                    JsName jsname;
                    arg->accept(jsname);
                    auto isArgArr = arg->findAttr<ASTAttrArray>() != nullptr;
                    IsTrivial trival(isArgArr);
                    argType->accept(trival);
                    if (trival.trivial) {
                        if (auto sizeArg = sizeArgs.find(arg); sizeArg != sizeArgs.end()) {
                            const auto& [size, arr] = *sizeArg;
                            size->findAttr<ASTAttrType>()->type->decl->accept(cname);
                            arr->accept(jsname);
                            fmt::print(stream, "({}) {}Arg.size()", cname.str, jsname.str);
                        } else {
                            fmt::print(stream, "{}", jsname.str);
                        }
                    } else if (argType->is<ASTBool>()) {
                        fmt::print(stream, "{}", jsname.str);
                    } else if (arg->findAttr<ASTAttrArray>() != nullptr) {
                        fmt::print(stream, "{}Arg.data()", jsname.str);
                    } else if (argType->is<ASTStr>()) {
                        fmt::print(stream, "{}Arg.c_str()", jsname.str);
                    } else if (argType->is<ASTInterface>()) {
                        fmt::print(stream, "{}.handle()", jsname.str);
                    }
                }
            }
            fmt::print(stream, ")");
            if (errorCode && !errorCodeInArg) {
                fmt::print(stream, ")");
            }
            fmt::println(stream, ";");
            if (!isCtor) {
                if (returnType && resultValue.length() > 0) {
                    fmt::println(stream, "        return {}({});", returnTypeStr, resultValue);
                }
            }
            fmt::println(stream, "    }}");
            fmt::println(stream, "");
        };

        ASTMethod* refInc{};
        ASTMethod* destroy{};
        fmt::println(stream, "class {} {{", className);
        fmt::println(stream, "public:");
        for (auto method : node->methods) {
            if (method->findAttr<ASTAttrCtor>() != nullptr) {
                addFunc(method);
            }
            if (method->findAttr<ASTAttrRefInc>() != nullptr) {
                refInc = method;
            }
            if (method->findAttr<ASTAttrDestroy>() != nullptr) {
                destroy = method;
            }
        }

        CName cname;
        node->accept(cname);
        const auto handleType = cname.str;

        if (refInc) {
            refInc->accept(cname);
            fmt::println(stream, "    {}({} handle) noexcept : _handle(handle) {{", className, handleType);
            fmt::println(stream, "        if (_handle) {{");
            fmt::println(stream, "            {}(_handle);", cname.str);
            fmt::println(stream, "        }}");
            fmt::println(stream, "    }}");
            fmt::println(stream, "");

            fmt::println(stream, "    {}(const {}& obj) noexcept : _handle(obj._handle) {{", className, className);
            fmt::println(stream, "        if (_handle) {{");
            fmt::println(stream, "            {}(_handle);", cname.str);
            fmt::println(stream, "        }}");
            fmt::println(stream, "    }}");
            fmt::println(stream, "");
        }

        if (destroy) {
            destroy->accept(cname);
            fmt::println(stream, "    ~{}() {{", className);
            fmt::println(stream, "        {}(_handle);", cname.str);
            fmt::println(stream, "    }}");
            fmt::println(stream, "");
        }

        for (auto method : node->methods) {
            if (method->findAttr<ASTAttrCtor>() == nullptr && method->findAttr<ASTAttrRefInc>() == nullptr &&
                method->findAttr<ASTAttrDestroy>() == nullptr) {
                addFunc(method);
            }
        }

        fmt::println(stream, "    {} handle() noexcept {{", handleType);
        fmt::println(stream, "        return _handle;");
        fmt::println(stream, "    }}");
        fmt::println(stream, "");

        fmt::println(stream, "private:");
        fmt::println(stream, "    {} _handle{{}};", handleType);

        fmt::println(stream, "}};");
        fmt::println(stream, "");
    });
}

static void generateBeginBindings(idl::Context& ctx, std::ostream& stream) {
    const auto moduleName = convert(ctx.api()->name, Case::CamelCase);
    fmt::println(stream, "EMSCRIPTEN_BINDINGS({}) {{", moduleName);
}

static void generateRegisterTypes(idl::Context& ctx, std::ostream& stream) {
    fmt::println(stream, "    register_type<String>(\"string\");");
    fmt::println(stream, "    register_type<ArrString>(\"string[]\");");
    ctx.filter<ASTTrivialType>([&stream](ASTTrivialType* trivialType) {
        if (!trivialType->is<ASTVoid>()) {
            JsName jsname(true);
            trivialType->accept(jsname);
            if (trivialType->is<ASTIntegerType>()) {
                fmt::println(stream, "    register_type<{}>(\"number[]\");", jsname.str);
            } else if (trivialType->is<ASTBool>()) {
                fmt::println(stream, "    register_type<{}>(\"boolean[]\");", jsname.str);
            }
        }
    });
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        JsName jsnameArr(true);
        node->accept(jsnameArr);
        JsName jsname(true);
        node->accept(jsname);
        fmt::println(stream, "    register_type<{}>(\"{}[]\");", jsnameArr.str, jsname.str);
    });
    ctx.filter<ASTInterface>([&stream](ASTInterface* node) {
        JsName jsnameArr(true);
        node->accept(jsnameArr);
        JsName jsname(true);
        node->accept(jsname);
        fmt::println(stream, "    register_type<{}>(\"{}[]\");", jsnameArr.str, jsname.str);
    });
    fmt::println(stream, "");
}

static void generateEnums(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTEnum>([&stream](ASTEnum* en) {
        if (en->findAttr<ASTAttrErrorCode>() == nullptr) {
            CName cname;
            en->accept(cname);
            JsName jsname;
            en->accept(jsname);
            fmt::println(stream, "    enum_<{}>(\"{}\")", cname.str, jsname.str);
            for (auto ec : en->consts) {
                ec->accept(cname);
                ec->accept(jsname);
                fmt::println(stream, "        .value(\"{}\", {})", jsname.str, cname.str);
            }
            fmt::println(stream, "        ;");
            fmt::println(stream, "");
        }
    });
}

static void generateValueObjects(idl::Context& ctx, std::ostream& stream) {
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        IsTrivial trivial;
        node->accept(trivial);
        if (trivial.trivial) {
            CName cname;
            node->accept(cname);
            JsName jsname;
            node->accept(jsname);
            fmt::println(stream, "    value_object<{}>(\"{}\")", cname.str, jsname.str);
            for (auto field : node->fields) {
                field->accept(cname);
                field->accept(jsname);
                fmt::println(stream, "        .field(\"{}\", &{}::{})", jsname.str, cname.str, cname.str);
            }
            fmt::println(stream, "        ;");
            fmt::println(stream, "");
        }
    });
    ctx.filter<ASTStruct>([&stream](ASTStruct* node) {
        IsTrivial trivial;
        node->accept(trivial);
        if (!trivial.trivial) {
            JsName jsname;
            node->accept(jsname);
            const auto className = jsname.str;
            fmt::println(stream, "    class_<{}>(\"{}\")", className, className);
            fmt::println(stream, "        .smart_ptr<std::shared_ptr<{}>>(\"{}\")", className, className);
            fmt::println(stream, "        .constructor()");
            for (auto field : node->fields) {
                field->accept(jsname);
                fmt::println(stream, "        .property(\"{}\", &{}::{})", jsname.str, className, jsname.str);
            }
            fmt::println(stream, "        ;");
            fmt::println(stream, "");
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
    auto stream      = createStream(ctx, out, writer, writerData);
    ASTEnum* errcode = nullptr;
    generateIncludes(ctx, stream.stream);
    generateTypes(ctx, stream.stream);
    generateExceptions(ctx, stream.stream, errcode);
    generateUtils(ctx, stream.stream);
    generateNonTrivialTypes(ctx, stream.stream);
    generateClasses(ctx, stream.stream);
    generateBeginBindings(ctx, stream.stream);
    generateRegisterTypes(ctx, stream.stream);
    generateEnums(ctx, stream.stream);
    generateValueObjects(ctx, stream.stream);
    generateEndBindings(ctx, stream.stream);
}
