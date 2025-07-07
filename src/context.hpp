#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "ast.hpp"
#include "errors.hpp"
#include "visitors.hpp"

namespace idl {

class Context final {
public:
    Context(Options* options, CompilationResult* result) noexcept : _options(options), _result(result) {
    }

    ~Context() {
        for (auto node : _nodes) {
            delete node;
        }
    }

    ASTApi* api() noexcept {
        return _api;
    }

    template <typename Node>
    Node* allocNode(const idl::location& loc) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        auto node = new (std::nothrow) Node{};
        if (!node) {
            err<IDL_STATUS_E2045>(loc);
        }
        if constexpr (std::is_same<Node, ASTApi>::value) {
            _api = node;
        }
        node->location = loc;
        _nodes.push_back(node);
        return node;
    }

    ASTLiteral* internStr(const idl::location& loc, const std::string& str) {
        return internLiteral<ASTLiteralStr>(loc, "str|" + str, str);
    }

    ASTLiteral* internBool(const idl::location& loc, bool b) {
        return internLiteral<ASTLiteralBool>(loc, "bool|" + std::to_string(b), b);
    }

    ASTLiteral* internInt(const idl::location& loc, int64_t num) {
        return internLiteral<ASTLiteralInt>(loc, "int|" + std::to_string(num), num);
    }

    void addSymbol(ASTDecl* decl) {
        const auto fullname = decl->fullnameLowecase();
        if (_symbols.contains(fullname)) {
            err<IDL_STATUS_E2030>(decl->location, decl->fullname());
        }
        _symbols[fullname] = decl;
        if (!_files.empty()) {
            decl->file = _files.back();
            _files.back()->decls.push_back(decl);
        }
    }

    void addDocSymbol(ASTDocDecl* decl) {
        const auto fullname = decl->fullnameLowecase();
        if (_docSymbols.contains(fullname)) {
            err<IDL_STATUS_E2030>(decl->location, decl->fullname());
        }
        _docSymbols[fullname] = decl;
    }

    ASTDecl* findSymbol(ASTDecl* decl, const idl::location& loc, const std::string& name, bool onlyType = false) {
        auto nameLower = name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](auto c) {
            return std::tolower(c);
        });
        while (decl) {
            const auto fullname = decl->fullnameLowecase() + '.' + nameLower;
            if (auto it = _symbols.find(fullname); it != _symbols.end()) {
                const auto actualName   = decl->fullname() + '.' + name;
                const auto expectedName = it->second->fullname();
                if (actualName != expectedName) {
                    err<IDL_STATUS_E2037>(loc, actualName, expectedName);
                }
                if (onlyType) {
                    if (it->second->is<ASTType>()) {
                        return it->second;
                    }
                } else {
                    return it->second;
                }
            }
            decl = decl->parent ? decl->parent->as<ASTDecl>() : nullptr;
        }
        err<IDL_STATUS_E2032>(loc, name);
    }

    ASTDecl* findSymbol(ASTDecl* decl, const idl::location& loc, ASTDeclRef* declRef, bool onlyType = false) {
        if (!declRef->decl) {
            auto symbol   = findSymbol(decl, loc, declRef->name, onlyType);
            declRef->decl = symbol;
            return symbol;
        } else {
            return declRef->decl;
        }
    }

    ASTDecl* findDocSymbol(ASTDeclRef* declRef) {
        if (!declRef->decl) {
            auto nameLower = declRef->name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](auto c) {
                return std::tolower(c);
            });
            auto it       = _docSymbols.find(nameLower);
            declRef->decl = it != _docSymbols.end() ? it->second : nullptr;
            return declRef->decl;
        } else {
            return declRef->decl;
        }
    }

    ASTType* resolveType(ASTDeclRef* declRef) {
        auto parent = declRef->parent->is<ASTAttr>() ? declRef->parent->parent : declRef->parent;
        auto decl   = findSymbol(parent->as<ASTDecl>(), declRef->location, declRef, true);

        if (auto type = decl->as<ASTType>()) {
            return type;
        } else {
            err<IDL_STATUS_E2035>(declRef->location, decl->fullname());
        }
    }

    void setDeclaring(bool active = true) noexcept {
        _declaring = active;
    }

    bool isDeclaring() const noexcept {
        return _declaring;
    }

    void initBuiltins() {
        static const std::string filename = "<builtin>";

        const auto loc = idl::location(idl::position(&filename, 1, 1));

        auto addBuiltin =
            [this, &loc]<typename Node>(std::string&& name, std::string&& cname, const std::string& detail, Node) {
            std::vector<ASTNode*> doc{};
            size_t prevPos = 0;
            size_t pos     = 0;
            std::string str;
            while ((pos = detail.find(' ', prevPos)) != std::string::npos) {
                if (!doc.empty()) {
                    doc.push_back(internStr(loc, std::string(" ")));
                }
                str     = detail.substr(prevPos, pos - prevPos);
                prevPos = pos + 1;
                doc.push_back(internStr(loc, str));
            }
            if (prevPos < detail.length()) {
                if (!doc.empty()) {
                    doc.push_back(internStr(loc, std::string(" ")));
                }
                doc.push_back(internStr(loc, detail.substr(prevPos)));
            }

            auto node         = allocNode<Node>(loc);
            node->name        = std::move(name);
            node->parent      = _api;
            node->doc         = allocNode<ASTDoc>(loc);
            node->doc->detail = std::move(doc);
            node->doc->parent = node;

            auto attr    = allocNode<ASTAttrCName>(loc);
            attr->name   = cname;
            attr->parent = node;
            node->attrs.push_back(attr);

            addSymbol(node);
        };

        addBuiltin("Void", "void", "void type.", ASTVoid{});
        addBuiltin("Char", "char", "symbol type.", ASTChar{});
        addBuiltin("Bool", "bool", "boolean type.", ASTBool{});
        addBuiltin("Int8", "sint8", "8 bit signed integer.", ASTInt8{});
        addBuiltin("Uint8", "uint8", "8 bit unsigned integer.", ASTUint8{});
        addBuiltin("Int16", "sint16", "16 bit signed integer.", ASTInt16{});
        addBuiltin("Uint16", "uint16", "16 bit unsigned integer.", ASTUint16{});
        addBuiltin("Int32", "sint32", "32 bit signed integer.", ASTInt32{});
        addBuiltin("Uint32", "uint32", "32 bit unsigned integer.", ASTUint32{});
        addBuiltin("Int64", "sint64", "64 bit signed integer.", ASTInt64{});
        addBuiltin("Uint64", "uint64", "64 bit unsigned integer.", ASTUint64{});
        addBuiltin("Float32", "float32", "32 bit float point.", ASTFloat32{});
        addBuiltin("Float64", "float64", "64 bit float point.", ASTFloat64{});
        addBuiltin("Str", "utf8", "utf8 string.", ASTStr{});
        addBuiltin("Data", "data", "pointer to data.", ASTData{});
        addBuiltin("ConstData", "cdata", "pointer to immutable data.", ASTConstData{});

        const std::chrono::time_point now{ std::chrono::system_clock::now() };
        const std::chrono::year_month_day ymd{ std::chrono::floor<std::chrono::days>(now) };
        auto year   = allocNode<ASTYear>(loc);
        year->name  = "Year";
        year->value = static_cast<int>(ymd.year());
        addDocSymbol(year);

        auto major   = allocNode<ASTMajor>(loc);
        auto minor   = allocNode<ASTMinor>(loc);
        auto micro   = allocNode<ASTMicro>(loc);
        major->name  = "Major";
        minor->name  = "Minor";
        micro->name  = "Micro";
        major->value = 0;
        minor->value = 0;
        micro->value = 0;
        if (auto version = api()->findAttr<ASTAttrVersion>()) {
            major->value = version->major;
            minor->value = version->minor;
            micro->value = version->micro;
        }
        addDocSymbol(major);
        addDocSymbol(minor);
        addDocSymbol(micro);

        auto True    = allocNode<ASTDocBool>(loc);
        auto False   = allocNode<ASTDocBool>(loc);
        True->name   = "True";
        False->name  = "False";
        True->value  = true;
        False->value = false;
        addDocSymbol(True);
        addDocSymbol(False);
    }

    void addAttrs(ASTDecl* node, const std::vector<ASTAttr*>& attrs) {
        node->attrs.insert(node->attrs.end(), attrs.begin(), attrs.end());
        std::set<std::type_index> uniqueAttrs;
        for (auto attr : node->attrs) {
            if (uniqueAttrs.contains(typeid(*attr))) {
                AttrName name;
                attr->accept(name);
                err<IDL_STATUS_E2013>(attr->location, name.str);
            }
            uniqueAttrs.insert(typeid(*attr));
        }
        AllowedAttrs allowAttrs{};
        node->accept(allowAttrs);
        for (auto attr : node->attrs) {
            if (!allowAttrs.allowed.contains(typeid(*attr))) {
                auto first = true;
                std::ostringstream ss;
                for (const auto& [_, name] : allowAttrs.allowed) {
                    if (!first) {
                        ss << ", ";
                    }
                    ss << '\'' << name << '\'';
                    first = false;
                }
                err<IDL_STATUS_E2014>(attr->location, ss.str());
            }
            attr->parent = node;
        }
    }

    void prepareEnumConsts() {
        std::vector<ASTEnumConst*> needAddTypeAttrs{};
        filter<ASTEnum>([this, &needAddTypeAttrs](ASTEnum* en) {
            if (en->consts.empty()) {
                err<IDL_STATUS_E2026>(en->location, en->name);
                return false;
            }
            for (auto ec : en->consts) {
                if (!ec->findAttr<ASTAttrType>()) {
                    needAddTypeAttrs.push_back(ec);
                }
                if (ec->findAttr<ASTAttrNoError>() && !en->findAttr<ASTAttrErrorCode>()) {
                    err<IDL_STATUS_E2072>(ec->location, ec->name, en->fullname());
                }
            }
            return true;
        });
        for (auto ec : needAddTypeAttrs) {
            auto attr          = allocNode<ASTAttrType>(ec->location);
            attr->parent       = ec;
            attr->type         = allocNode<ASTDeclRef>(ec->location);
            attr->type->name   = "Int32";
            attr->type->parent = attr;
            ec->attrs.push_back(attr);
        }

        std::vector<ASTEnumConst*> needAddValueAttrs{};
        filter<ASTEnum>([this, &needAddValueAttrs](ASTEnum* en) {
            std::vector<ASTEnumConst*> deps;
            for (auto ec : en->consts) {
                calcEnumConst(ec, deps);
                if (!ec->findAttr<ASTAttrValue>()) {
                    needAddValueAttrs.push_back(ec);
                }
            }
            return true;
        });
        for (auto ec : needAddValueAttrs) {
            auto attr    = allocNode<ASTAttrValue>(ec->location);
            attr->parent = ec;
            attr->value  = internInt(ec->location, (int64_t) ec->value);
            ec->attrs.push_back(attr);
        }
    }

    void prepareStructs() {
        std::vector<ASTField*> needAddType{};
        std::vector<ASTField*> needAddRef{};
        filter<ASTStruct>([this, &needAddType, &needAddRef](ASTStruct* node) {
            if (node->fields.empty()) {
                err<IDL_STATUS_E2081>(node->location, node->fullname());
            }
            for (auto field : node->fields) {
                if (!field->findAttr<ASTAttrType>()) {
                    needAddType.push_back(field);
                }
                if (auto attr = field->findAttr<ASTAttrArray>()) {
                    if (attr->ref) {
                        if (field->findAttr<ASTAttrRef>() == nullptr) {
                            needAddRef.push_back(field);
                        }
                    } else if (attr->size < 1) {
                        err<IDL_STATUS_E2077>(field->location, field->name, node->fullname());
                    }
                }
                if (field->findAttr<ASTAttrArray>() && field->findAttr<ASTAttrDataSize>()) {
                    err<IDL_STATUS_E2124>(field->location, field->fullname());
                }
                if (auto value = field->findAttr<ASTAttrValue>()) {
                    if (auto literalConsts = value->value->as<ASTLiteralConsts>()) {
                        std::set<ASTDecl*> uniqueDecls;
                        for (auto declRef : literalConsts->decls) {
                            auto decl = findSymbol(node, declRef->location, declRef);
                            if (uniqueDecls.contains(decl)) {
                                err<IDL_STATUS_E2039>(decl->location, decl->fullname());
                            }
                            uniqueDecls.insert(decl);
                        }
                    }
                }
            }
        });
        for (auto field : needAddType) {
            auto attr          = allocNode<ASTAttrType>(field->location);
            attr->parent       = field;
            attr->type         = allocNode<ASTDeclRef>(field->location);
            attr->type->name   = "Int32";
            attr->type->parent = attr;
            field->attrs.push_back(attr);
        }
        for (auto field : needAddRef) {
            auto attr    = allocNode<ASTAttrRef>(field->location);
            attr->parent = field;
            field->attrs.push_back(attr);
        }
        filter<ASTStruct>([this](ASTStruct* node) {
            for (auto field : node->fields) {
                auto attr = field->findAttr<ASTAttrType>();
                if (resolveType(attr->type)->is<ASTVoid>()) {
                    err<IDL_STATUS_E2068>(field->location, field->name, node->fullname());
                }
            }
        });
        filter<ASTStruct>([this](ASTStruct* node) {
            for (auto field : node->fields) {
                if (auto attr = field->findAttr<ASTAttrArray>(); attr && attr->ref) {
                    auto symbol = findSymbol(node, attr->location, attr->decl);
                    if (auto sizeField = symbol->as<ASTField>()) {
                        auto parent1 = node;
                        auto parent2 = symbol;
                        while (true) {
                            auto st = parent1->parent->as<ASTStruct>();
                            if (st == nullptr) {
                                break;
                            }
                            parent1 = st;
                        }
                        while (true) {
                            auto st = parent2->parent->as<ASTStruct>();
                            if (st == nullptr) {
                                break;
                            }
                            parent2 = st;
                        }
                        if (parent1 != parent2) {
                            err<IDL_STATUS_E2079>(field->location);
                        }
                        auto type = resolveType(sizeField->findAttr<ASTAttrType>()->type);
                        if (!type->is<ASTIntegerType>()) {
                            err<IDL_STATUS_E2080>(attr->location, field->fullname());
                        }
                    } else {
                        err<IDL_STATUS_E2078>(attr->location, field->fullname());
                    }
                }
                if (auto attr = field->findAttr<ASTAttrDataSize>()) {
                    auto dataType = resolveType(field->findAttr<ASTAttrType>()->type);
                    if (!dataType->is<ASTData>() && !dataType->is<ASTConstData>()) {
                        err<IDL_STATUS_E2119>(attr->location, field->name, node->fullname());
                    }
                    auto symbol = findSymbol(node, attr->location, attr->decl);
                    if (auto sizeField = symbol->as<ASTField>()) {
                        auto parent1 = node;
                        auto parent2 = symbol;
                        while (true) {
                            auto st = parent1->parent->as<ASTStruct>();
                            if (st == nullptr) {
                                break;
                            }
                            parent1 = st;
                        }
                        while (true) {
                            auto st = parent2->parent->as<ASTStruct>();
                            if (st == nullptr) {
                                break;
                            }
                            parent2 = st;
                        }
                        if (parent1 != parent2) {
                            err<IDL_STATUS_E2118>(field->location);
                        }
                        auto type = resolveType(sizeField->findAttr<ASTAttrType>()->type);
                        if (!type->is<ASTIntegerType>()) {
                            err<IDL_STATUS_E2114>(attr->location, field->fullname());
                        }
                    } else {
                        err<IDL_STATUS_E2113>(attr->location, field->fullname());
                    }
                }
            }
        });
    }

    template <typename T>
    void prepareInvokable() {
        std::vector<T*> needAddRetType{};
        std::vector<T*> needAddStatic{};
        std::vector<ASTArg*> needAddArgType{};
        std::vector<ASTArg*> needAddArgIn{};
        std::vector<ASTArg*> needAddArgOut{};
        std::vector<ASTArg*> needAddRef{};
        std::vector<ASTDecl*> needAddOptional{};
        filter<T>([this,
                   &needAddRetType,
                   &needAddStatic,
                   &needAddArgType,
                   &needAddArgIn,
                   &needAddArgOut,
                   &needAddRef,
                   &needAddOptional](T* node) {
            const auto isMethod = std::is_same_v<T, ASTMethod>;
            const auto isStatic = isMethod && node->template findAttr<ASTAttrStatic>();
            const auto isCtor   = isMethod && node->template findAttr<ASTAttrCtor>();
            if (!node->template findAttr<ASTAttrType>()) {
                needAddRetType.push_back(node);
            }
            if (isCtor && !isStatic) {
                needAddStatic.push_back(node);
            }
            if (isCtor || isStatic) {
                for (auto arg : node->args) {
                    if (arg->template findAttr<ASTAttrThis>()) {
                        if (isCtor) {
                            err<IDL_STATUS_E2047>(arg->location, node->fullname(), arg->name);
                        } else {
                            err<IDL_STATUS_E2046>(arg->location, node->fullname(), arg->name);
                        }
                    }
                }
            }
            if (isMethod && !isCtor && !isStatic) {
                int countThis = 0;
                for (auto arg : node->args) {
                    countThis += arg->template findAttr<ASTAttrThis>() ? 1 : 0;
                }
                if (countThis != 1) {
                    err<IDL_STATUS_E2048>(node->location, node->fullname());
                }
            }
            int countUserData = 0;
            int countResult   = 0;
            for (auto arg : node->args) {
                if (!arg->template findAttr<ASTAttrType>()) {
                    needAddArgType.push_back(arg);
                }
                auto hasOut = arg->template findAttr<ASTAttrOut>() != nullptr;
                if (arg->template findAttr<ASTAttrResult>() && !hasOut) {
                    needAddArgOut.push_back(arg);
                    hasOut = true;
                }
                if (!hasOut && !arg->template findAttr<ASTAttrIn>()) {
                    needAddArgIn.push_back(arg);
                }
                if (!isMethod && arg->template findAttr<ASTAttrThis>()) {
                    if constexpr (std::is_same_v<T, ASTCallback>) {
                        err<IDL_STATUS_E2083>(arg->location, node->fullname(), arg->name);
                    } else if constexpr (std::is_same_v<T, ASTFunc>) {
                        err<IDL_STATUS_E2073>(arg->location, node->fullname(), arg->name);
                    }
                }
                countUserData += arg->template findAttr<ASTAttrUserData>() ? 1 : 0;
                countResult += arg->template findAttr<ASTAttrResult>() ? 1 : 0;
                if (countUserData > 1) {
                    err<IDL_STATUS_E2082>(arg->location);
                }
                if (countResult > 1) {
                    err<IDL_STATUS_E2084>(arg->location);
                }
                if (auto attr = arg->template findAttr<ASTAttrArray>()) {
                    if (attr->ref) {
                        if (arg->template findAttr<ASTAttrRef>() == nullptr) {
                            needAddRef.push_back(arg);
                        }
                    } else {
                        err<IDL_STATUS_E2102>(arg->location, arg->name, node->fullname());
                    }
                }
                if (arg->template findAttr<ASTAttrArray>() && arg->template findAttr<ASTAttrDataSize>()) {
                    err<IDL_STATUS_E2124>(arg->location, arg->fullname());
                }
            }
        });
        for (auto node : needAddRetType) {
            auto attr          = allocNode<ASTAttrType>(node->location);
            attr->parent       = node;
            attr->type         = allocNode<ASTDeclRef>(node->location);
            attr->type->name   = "Void";
            attr->type->parent = attr;
            node->attrs.push_back(attr);
        }
        for (auto node : needAddStatic) {
            auto attr    = allocNode<ASTAttrStatic>(node->location);
            attr->parent = node;
            node->attrs.push_back(attr);
        }
        for (auto node : needAddArgType) {
            auto attr          = allocNode<ASTAttrType>(node->location);
            attr->parent       = node;
            attr->type         = allocNode<ASTDeclRef>(node->location);
            attr->type->name   = "Int32";
            attr->type->parent = attr;
            node->attrs.push_back(attr);
        }
        for (auto node : needAddArgIn) {
            auto attr    = allocNode<ASTAttrIn>(node->location);
            attr->parent = node;
            node->attrs.push_back(attr);
        }
        for (auto node : needAddArgOut) {
            auto attr    = allocNode<ASTAttrOut>(node->location);
            attr->parent = node;
            node->attrs.push_back(attr);
        }
        for (auto node : needAddRef) {
            auto attr    = allocNode<ASTAttrRef>(node->location);
            attr->parent = node;
            node->attrs.push_back(attr);
        }
        filter<T>([this, &needAddOptional](T* node) {
            auto attr    = node->template findAttr<ASTAttrType>();
            auto retType = resolveType(attr->type);
            if (retType->template is<ASTCallback>() && !node->template findAttr<ASTAttrOptional>()) {
                needAddOptional.push_back(node);
            }
            for (auto arg : node->args) {
                auto argAttr = arg->template findAttr<ASTAttrType>();
                if (resolveType(argAttr->type)->template is<ASTVoid>()) {
                    if constexpr (std::is_same_v<T, ASTMethod>) {
                        err<IDL_STATUS_E2051>(arg->location, arg->name, node->fullname());
                    } else {
                        err<IDL_STATUS_E2074>(arg->location, arg->name, node->fullname());
                    }
                }
                if (auto attr = arg->template findAttr<ASTAttrArray>(); attr) {
                    assert(attr->ref);
                    auto symbol = findSymbol(node, attr->location, attr->decl);
                    if (auto sizeField = symbol->template as<ASTArg>()) {
                        if (arg->parent != sizeField->parent) {
                            if constexpr (std::is_same_v<T, ASTCallback>) {
                                err<IDL_STATUS_E2107>(arg->location);
                            } else if constexpr (std::is_same_v<T, ASTFunc>) {
                                err<IDL_STATUS_E2105>(arg->location);
                            } else if constexpr (std::is_same_v<T, ASTMethod>) {
                                err<IDL_STATUS_E2103>(arg->location);
                            } else {
                                static_assert(false, "unknown invokable");
                            }
                        }
                        auto type = resolveType(sizeField->template findAttr<ASTAttrType>()->type);
                        if (!type->template is<ASTIntegerType>()) {
                            err<IDL_STATUS_E2080>(attr->location, arg->fullname());
                        }
                    } else {
                        if constexpr (std::is_same_v<T, ASTCallback>) {
                            err<IDL_STATUS_E2108>(attr->location, arg->fullname());
                        } else if constexpr (std::is_same_v<T, ASTFunc>) {
                            err<IDL_STATUS_E2106>(attr->location, arg->fullname());
                        } else if constexpr (std::is_same_v<T, ASTMethod>) {
                            err<IDL_STATUS_E2104>(attr->location, arg->fullname());
                        } else {
                            static_assert(false, "unknown invokable");
                        }
                    }
                }
                if (auto attr = arg->template findAttr<ASTAttrDataSize>(); attr) {
                    auto symbol   = findSymbol(node, attr->location, attr->decl);
                    auto dataType = resolveType(arg->template findAttr<ASTAttrType>()->type);
                    if (!dataType->template is<ASTData>() && !dataType->template is<ASTConstData>()) {
                        err<IDL_STATUS_E2121>(attr->location, arg->name, node->fullname());
                    }
                    if (auto sizeField = symbol->template as<ASTArg>()) {
                        if (arg->parent != sizeField->parent) {
                            if constexpr (std::is_same_v<T, ASTCallback>) {
                                err<IDL_STATUS_E2120>(arg->location);
                            } else if constexpr (std::is_same_v<T, ASTFunc>) {
                                err<IDL_STATUS_E2122>(arg->location);
                            } else if constexpr (std::is_same_v<T, ASTMethod>) {
                                err<IDL_STATUS_E2123>(attr->location, arg->fullname());
                            } else {
                                static_assert(false, "unknown invokable");
                            }
                        }
                        auto type = resolveType(sizeField->template findAttr<ASTAttrType>()->type);
                        if (!type->template is<ASTIntegerType>()) {
                            err<IDL_STATUS_E2114>(attr->location, arg->fullname());
                        }
                    } else {
                        if constexpr (std::is_same_v<T, ASTCallback>) {
                            err<IDL_STATUS_E2117>(attr->location, arg->fullname());
                        } else if constexpr (std::is_same_v<T, ASTFunc>) {
                            err<IDL_STATUS_E2116>(attr->location, arg->fullname());
                        } else if constexpr (std::is_same_v<T, ASTMethod>) {
                            err<IDL_STATUS_E2115>(attr->location, arg->fullname());
                        } else {
                            static_assert(false, "unknown invokable");
                        }
                    }
                }
                if (resolveType(argAttr->type)->template is<ASTCallback>() &&
                    !arg->template findAttr<ASTAttrOptional>()) {
                    needAddOptional.push_back(arg);
                }
            }
            if (node->template findAttr<ASTAttrErrorCode>()) {
                if (!std::is_same_v<T, ASTFunc>) {
                    err<IDL_STATUS_E2125>(node->location, node->fullname());
                }
                auto argType        = node->args[0]->template findAttr<ASTAttrType>()->type->decl;
                auto argIsErrorCode = argType->template findAttr<ASTAttrErrorCode>() != nullptr;
                if (!retType->template is<ASTStr>() || node->args.size() != 1 || !argIsErrorCode) {
                    err<IDL_STATUS_E2085>(node->location);
                }
            }
            if (node->template findAttr<ASTAttrRefInc>()) {
                if (!std::is_same_v<T, ASTMethod>) {
                    err<IDL_STATUS_E2126>(node->location, node->fullname());
                }
                if (node->template findAttr<ASTAttrStatic>() || node->args.size() != 1) {
                    err<IDL_STATUS_E2086>(node->location);
                }
            }
            if (node->template findAttr<ASTAttrDestroy>()) {
                if (!std::is_same_v<T, ASTMethod>) {
                    err<IDL_STATUS_E2127>(node->location, node->fullname());
                }
                if (node->template findAttr<ASTAttrStatic>() || node->args.size() != 1) {
                    err<IDL_STATUS_E2087>(node->location);
                }
            }
        });
        for (auto node : needAddOptional) {
            auto attr    = allocNode<ASTAttrOptional>(node->location);
            attr->parent = node;
            node->attrs.push_back(attr);
        }
    }

    template <typename T>
    void prepareGetterSetter() {
        std::vector<std::pair<T*, std::string>> needAddType{};
        filter<T>([this, &needAddType](T* node) {
            auto getter = node->template findAttr<ASTAttrGet>();
            auto setter = node->template findAttr<ASTAttrSet>();
            if (!getter && !setter) {
                if constexpr (std::is_same_v<T, ASTProperty>) {
                    err<IDL_STATUS_E2052>(node->location, node->fullname());
                } else {
                    err<IDL_STATUS_E2091>(node->location, node->fullname());
                }
            }
            auto isStaticProp       = node->template findAttr<ASTAttrStatic>() != nullptr;
            ASTType* getterType     = nullptr;
            ASTType* setterType     = nullptr;
            ASTMethod* getterMethod = nullptr;
            ASTMethod* setterMethod = nullptr;
            if (getter) {
                auto decl = findSymbol(node, getter->location, getter->decl);
                if (auto method = decl->template as<ASTMethod>()) {
                    getterMethod = method;
                    if (method->parent != node->parent) {
                        auto iface             = node->parent->template as<ASTInterface>()->fullname();
                        auto otherIface        = method->parent->template as<ASTInterface>()->fullname();
                        constexpr auto errCode = std::is_same_v<T, ASTProperty> ? IDL_STATUS_E2054 : IDL_STATUS_E2092;
                        err<errCode>(getter->location, node->name, iface, method->name, otherIface);
                    }
                    auto isStaticGetter = method->template findAttr<ASTAttrStatic>() != nullptr;
                    if (isStaticProp != isStaticGetter) {
                        constexpr auto errCode = std::is_same_v<T, ASTProperty> ? IDL_STATUS_E2055 : IDL_STATUS_E2093;
                        err<errCode>(getter->location, method->fullname(), node->fullname());
                    }
                    getterType          = resolveType(method->template findAttr<ASTAttrType>()->type);
                    const auto argCount = method->args.size();

                    if constexpr (std::is_same_v<T, ASTProperty>) {
                        if (getterType->is<ASTVoid>()) {
                            bool isValidProp = false;
                            auto count       = isStaticProp ? 2 : 3;
                            if (argCount == count) {
                                auto res = std::find_if(method->args.begin(), method->args.end(), [](ASTArg* arg) {
                                    return arg->findAttr<ASTAttrResult>() != nullptr;
                                });
                                auto arrAttr =
                                    res != method->args.end() ? (*res)->template findAttr<ASTAttrArray>() : nullptr;
                                if (arrAttr) {
                                    auto arrDecl = findSymbol(node, arrAttr->location, arrAttr->decl);
                                    if (arrDecl->template findAttr<ASTAttrOut>()) {
                                        isValidProp = true;
                                        if ((*res)->template findAttr<ASTAttrType>()) {
                                            getterType = resolveType((*res)->template findAttr<ASTAttrType>()->type);
                                        }
                                    }
                                }
                                auto datasizeAttr =
                                    res != method->args.end() ? (*res)->template findAttr<ASTAttrDataSize>() : nullptr;
                                if (datasizeAttr) {
                                    auto datasizeDecl = findSymbol(node, datasizeAttr->location, datasizeAttr->decl);
                                    if (datasizeDecl->template findAttr<ASTAttrOut>()) {
                                        isValidProp = true;
                                        if ((*res)->template findAttr<ASTAttrType>()) {
                                            getterType = resolveType((*res)->template findAttr<ASTAttrType>()->type);
                                        }
                                    }
                                }
                            }
                            if (!isValidProp) {
                                err<IDL_STATUS_E2058>(getter->location, method->fullname());
                            }
                        } else {
                            if (isStaticProp && argCount != 0) {
                                err<IDL_STATUS_E2056>(getter->location, method->fullname());
                            } else if (!isStaticProp && argCount != 1) {
                                err<IDL_STATUS_E2057>(getter->location, method->fullname());
                            }
                        }
                    } else if (std::is_same_v<T, ASTEvent>) {
                        if (isStaticProp) {
                            if ((argCount == 1 && !method->args[0]->template findAttr<ASTAttrUserData>()) ||
                                argCount > 1) {
                                err<IDL_STATUS_E2094>(getter->location, method->fullname());
                            }
                        } else {
                            if (argCount == 2 && (!method->args[0]->template findAttr<ASTAttrUserData>() &&
                                                  !method->args[1]->template findAttr<ASTAttrUserData>())) {
                                err<IDL_STATUS_E2095>(getter->location, method->fullname());
                            } else if (argCount > 2) {
                                err<IDL_STATUS_E2095>(getter->location, method->fullname());
                            }
                        }
                        getterType = resolveType(method->template findAttr<ASTAttrType>()->type);
                        if (getterType->is<ASTVoid>()) {
                            err<IDL_STATUS_E2058>(getter->location, method->fullname());
                        }
                    }
                } else {
                    err<IDL_STATUS_E2053>(getter->location, decl->fullname());
                }
            }
            if (setter) {
                auto decl = findSymbol(node, setter->location, setter->decl);
                if (auto method = decl->template as<ASTMethod>()) {
                    setterMethod = method;
                    if (method->parent != node->parent) {
                        auto iface             = node->parent->template as<ASTInterface>()->fullname();
                        auto otherIface        = method->parent->template as<ASTInterface>()->fullname();
                        constexpr auto errCode = std::is_same_v<T, ASTProperty> ? IDL_STATUS_E2061 : IDL_STATUS_E2096;
                        err<errCode>(setter->location, node->name, iface, method->name, otherIface);
                    }
                    auto isStaticSetter = method->template findAttr<ASTAttrStatic>() != nullptr;
                    if (isStaticProp != isStaticSetter) {
                        constexpr auto errCode = std::is_same_v<T, ASTProperty> ? IDL_STATUS_E2060 : IDL_STATUS_E2097;
                        err<errCode>(setter->location, method->fullname(), node->fullname());
                    }
                    auto isValid        = false;
                    const auto argCount = method->args.size();

                    if constexpr (std::is_same_v<T, ASTProperty>) {
                        if (argCount == (isStaticProp ? 2 : 3)) {
                            auto res = std::find_if(method->args.begin(), method->args.end(), [](ASTArg* arg) {
                                return arg->findAttr<ASTAttrArray>() != nullptr;
                            });
                            auto arrAttr =
                                res != method->args.end() ? (*res)->template findAttr<ASTAttrArray>() : nullptr;
                            if (arrAttr) {
                                auto arrDecl  = findSymbol(node, arrAttr->location, arrAttr->decl);
                                auto sizeType = resolveType(arrDecl->template findAttr<ASTAttrType>()->type);
                                if (sizeType->template is<ASTIntegerType>()) {
                                    isValid = true;
                                    if ((*res)->template findAttr<ASTAttrType>()) {
                                        setterType = resolveType((*res)->template findAttr<ASTAttrType>()->type);
                                    }
                                }
                            }
                            res = std::find_if(method->args.begin(), method->args.end(), [](ASTArg* arg) {
                                return arg->findAttr<ASTAttrDataSize>() != nullptr;
                            });
                            auto datasizeAttr =
                                res != method->args.end() ? (*res)->template findAttr<ASTAttrDataSize>() : nullptr;
                            if (datasizeAttr) {
                                auto datasizeDecl = findSymbol(node, datasizeAttr->location, datasizeAttr->decl);
                                auto sizeType     = resolveType(datasizeDecl->template findAttr<ASTAttrType>()->type);
                                if (sizeType->template is<ASTIntegerType>()) {
                                    isValid = true;
                                    if ((*res)->template findAttr<ASTAttrType>()) {
                                        setterType = resolveType((*res)->template findAttr<ASTAttrType>()->type);
                                    }
                                }
                            }
                        }
                        if (!isValid) {
                            if (isStaticProp && argCount != 1) {
                                err<IDL_STATUS_E2062>(setter->location, method->fullname());
                            } else if (!isStaticProp && argCount != 2) {
                                err<IDL_STATUS_E2063>(setter->location, method->fullname());
                            }
                        }
                        if (!setterType) {
                            for (auto arg : method->args) {
                                if (arg->template findAttr<ASTAttrThis>() == nullptr) {
                                    setterType = resolveType(arg->template findAttr<ASTAttrType>()->type);
                                    break;
                                }
                            }
                        }
                    } else if constexpr (std::is_same_v<T, ASTEvent>) {
                        if (isStaticProp && argCount != 1) {
                            if ((argCount == 2 && !method->args[0]->template findAttr<ASTAttrUserData>() &&
                                 !method->args[1]->template findAttr<ASTAttrUserData>()) ||
                                argCount > 2) {
                                err<IDL_STATUS_E2098>(getter->location, method->fullname());
                            }
                        } else if (!isStaticProp) {
                            if (argCount == 3 && (!method->args[0]->template findAttr<ASTAttrUserData>() &&
                                                  !method->args[1]->template findAttr<ASTAttrUserData>() &&
                                                  !method->args[2]->template findAttr<ASTAttrUserData>())) {
                                err<IDL_STATUS_E2099>(getter->location, method->fullname());
                            } else if (argCount > 3) {
                                err<IDL_STATUS_E2099>(getter->location, method->fullname());
                            }
                        }
                        for (auto arg : method->args) {
                            if (arg->template findAttr<ASTAttrThis>() == nullptr &&
                                arg->template findAttr<ASTAttrUserData>() == nullptr) {
                                setterType = resolveType(arg->template findAttr<ASTAttrType>()->type);
                                break;
                            }
                        }
                    }
                    assert(setterType);
                } else {
                    err<IDL_STATUS_E2059>(setter->location, decl->fullname());
                }
            }
            if (getterType && setterType && getterType != setterType) {
                err<IDL_STATUS_E2064>(node->location,
                                      getterType->fullname(),
                                      getterMethod->fullname(),
                                      setterType->fullname(),
                                      setterMethod->fullname());
            }
            if (auto attr = node->template findAttr<ASTAttrType>()) {
                auto type = resolveType(attr->type);
                if (getterType && getterType != type) {
                    constexpr auto errCode = std::is_same_v<T, ASTProperty> ? IDL_STATUS_E2065 : IDL_STATUS_E2100;
                    err<errCode>(attr->location, type->fullname(), getterType->fullname(), getterMethod->fullname());
                }
                if (setterType && setterType != type) {
                    constexpr auto errCode = std::is_same_v<T, ASTProperty> ? IDL_STATUS_E2066 : IDL_STATUS_E2101;
                    err<errCode>(attr->location, type->fullname(), setterMethod->fullname(), setterType->fullname());
                }
            } else {
                needAddType.emplace_back(node, getterType ? getterType->name : setterType->name);
            }
            return true;
        });
        for (auto& [node, type] : needAddType) {
            auto attr          = allocNode<ASTAttrType>(node->location);
            attr->parent       = node;
            attr->type         = allocNode<ASTDeclRef>(node->location);
            attr->type->name   = type;
            attr->type->parent = attr;
            node->attrs.push_back(attr);
        }
    }

    void prepareCallbacks() {
        prepareInvokable<ASTCallback>();
    }

    void prepareFunctions() {
        prepareInvokable<ASTFunc>();
    }

    void prepareMethods() {
        prepareInvokable<ASTMethod>();
    }

    void prepareProperties() {
        prepareGetterSetter<ASTProperty>();
    }

    void prepareEvents() {
        prepareGetterSetter<ASTEvent>();
    }

    void prepareInterfaces() {
        filter<ASTInterface>([this](ASTInterface* node) {
            int refMethodCount     = 0;
            int destroyMethodCount = 0;
            for (auto method : node->methods) {
                refMethodCount += method->findAttr<ASTAttrRef>() ? 1 : 0;
                destroyMethodCount += method->findAttr<ASTAttrDestroy>() ? 1 : 0;
                if (refMethodCount > 1) {
                    err<IDL_STATUS_E2088>(method->location);
                }
                if (destroyMethodCount > 1) {
                    err<IDL_STATUS_E2089>(method->location);
                }
            }
        });
    }

    void prepareHandles() {
        filter<ASTHandle>([this](ASTHandle* node) {
            if (auto attr = node->findAttr<ASTAttrType>()) {
                if (auto type = resolveType(attr->type); type->is<ASTStruct>()) {
                    if (!type->findAttr<ASTAttrHandle>()) {
                        err<IDL_STATUS_E2071>(node->location, type->fullname(), node->fullname());
                    }
                } else {
                    err<IDL_STATUS_E2070>(node->location, node->fullname());
                }
            } else {
                err<IDL_STATUS_E2069>(node->location, node->fullname());
            }
        });
    }

    void prepareDocumentation() {
        filter<ASTDecl>([this](ASTDecl* node) {
            if (node->doc) {
                DocValidator validator(_options, _result);
                node->accept(validator);
                auto prepare = [this, node](const std::vector<ASTNode*>& nodes) {
                    for (auto doc : nodes) {
                        if (auto declRef = doc->as<ASTDeclRef>()) {
                            findDocSymbol(declRef);
                            findSymbol(node, declRef->location, declRef);
                        }
                    }
                };
                auto prepares = [&prepare](const std::vector<std::vector<ASTNode*>>& nodes) {
                    for (auto node : nodes) {
                        prepare(node);
                    }
                };
                prepare(node->doc->brief);
                prepare(node->doc->detail);
                prepare(node->doc->ret);
                prepare(node->doc->copyright);
                prepare(node->doc->license);
                prepares(node->doc->authors);
                prepares(node->doc->note);
                prepares(node->doc->warn);
                prepares(node->doc->see);
            }
        });
    }

    const std::optional<idl_api_version_t>& apiVersion() const noexcept {
        return _version;
    }

    void apiVersion(std::optional<idl_api_version_t> version) noexcept {
        _version = version;
        filter<ASTMajor>([this](ASTMajor* node) {
            node->value = _version.value_or(idl_api_version_t{}).major;
            return false;
        });
        filter<ASTMinor>([this](ASTMinor* node) {
            node->value = _version.value_or(idl_api_version_t{}).minor;
            return false;
        });
        filter<ASTMicro>([this](ASTMicro* node) {
            node->value = _version.value_or(idl_api_version_t{}).micro;
            return false;
        });
    }

    void pushFile(ASTFile* file) {
        _files.push_back(file);
    }

    void popFile() {
        _files.pop_back();
    }

    template <typename Node, typename Pred>
    bool filter(Pred&& pred) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        constexpr auto isVoid = std::is_same_v<decltype(pred((Node*) nullptr)), void>;
        for (auto node : _nodes) {
            if (auto ptr = node->as<Node>()) {
                if constexpr (isVoid) {
                    pred(ptr);
                } else {
                    if (!pred(ptr)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

private:
    template <typename Node, typename Value>
    ASTLiteral* internLiteral(const idl::location& loc, const std::string& keyStr, const Value& value) {
        const auto key = XXH64(keyStr.c_str(), keyStr.length(), 0);
        if (auto it = _literals.find(key); it != _literals.end()) {
            return it->second;
        }
        auto literal   = allocNode<Node>(loc);
        literal->value = value;
        _literals[key] = literal;
        return literal;
    }

    void calcEnumConst(ASTEnumConst* ec, std::vector<ASTEnumConst*>& deps) {
        if (ec->evaluated) {
            return;
        }

        if (auto it = std::find(deps.begin(), deps.end(), ec); it != deps.end()) {
            std::ostringstream ss;
            auto first = true;
            for (auto dep : deps) {
                if (!first) {
                    ss << " -> ";
                }
                first = false;
                ss << dep->fullname();
            }
            ss << " -> " << ec->fullname();
            err<IDL_STATUS_E2040>(ec->location, ss.str());
        }
        deps.push_back(ec);

        if (auto typeAttr = ec->findAttr<ASTAttrType>()) {
            auto type = resolveType(typeAttr->type);
            if (!type->is<ASTInt32>()) {
                err<IDL_STATUS_E2036>(typeAttr->location);
            }
        }

        auto en        = ec->parent->as<ASTEnum>();
        auto attrValue = ec->findAttr<ASTAttrValue>();

        if (attrValue) {
            if (auto literalInt = attrValue->value->as<ASTLiteralInt>()) {
                if (literalInt->value < INT32_MIN || literalInt->value > INT32_MAX) {
                    err<IDL_STATUS_E2038>(attrValue->location);
                }
                ec->value = (int32_t) literalInt->value;
            } else if (auto literalConsts = attrValue->value->as<ASTLiteralConsts>()) {
                std::set<ASTDecl*> uniqueDecls;
                for (auto declRef : literalConsts->decls) {
                    auto decl = findSymbol(en, ec->location, declRef);
                    if (uniqueDecls.contains(decl)) {
                        err<IDL_STATUS_E2039>(decl->location, decl->fullname());
                    }
                    uniqueDecls.insert(decl);
                    if (decl == ec) {
                        err<IDL_STATUS_E2033>(decl->location, decl->fullname());
                    }
                    if (auto refEc = decl->as<ASTEnumConst>()) {
                        calcEnumConst(refEc, deps);
                        ec->value |= refEc->value;
                    } else {
                        err<IDL_STATUS_E2034>(ec->location);
                    }
                }
            } else {
                err<IDL_STATUS_E2031>(attrValue->location);
            }
        } else {
            int32_t prevValue = -1;
            for (auto c : en->consts) {
                if (c == ec) {
                    break;
                }
                calcEnumConst(c, deps);
                prevValue = c->value;
            }
            ec->value = prevValue + 1;
        }
        ec->evaluated = true;
        deps.pop_back();
    }

    Options* _options;
    CompilationResult* _result;
    std::optional<idl_api_version_t> _version{};
    ASTApi* _api{};
    std::vector<ASTNode*> _nodes{};
    std::unordered_map<std::string, struct ASTDecl*> _symbols{};
    std::unordered_map<std::string, struct ASTDocDecl*> _docSymbols{};
    std::unordered_map<uint64_t, ASTLiteral*> _literals{};
    std::vector<ASTFile*> _files{};
    bool _declaring{};
};

} // namespace idl

#endif
