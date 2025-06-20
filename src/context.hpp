#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <chrono>
#include <sstream>
#include <unordered_map>

#include <xxhash.h>

#include "ast.hpp"
#include "errors.hpp"
#include "location.hpp"
#include "visitors.hpp"

namespace idl {

struct ApiVersion {
    int major{};
    int minor{};
    int micro{};
};

class Context final {
public:
    ~Context() {
        for (auto node : _nodes) {
            delete node;
        }
    }

    ASTApi* api() noexcept {
        return _api;
    }

    template <typename Node, typename Exception = void>
    Node* allocNode(const idl::location& loc) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        auto node = new (std::nothrow) Node{};
        if (!node) {
            if constexpr (std::is_same_v<Exception, void>) {
                err<E2045>(loc);
            } else {
                throw Exception(loc, err_str<E2045>());
            }
        }
        if constexpr (std::is_same<Node, ASTApi>::value) {
            _api = node;
        }
        node->location = loc;
        _nodes.push_back(node);
        return node;
    }

    template <typename Exception = void>
    ASTLiteral* intern(const idl::location& loc, const std::string& str) {
        return internLiteral<ASTLiteralStr, Exception>(loc, "str|" + str, str);
    }

    template <typename Exception = void>
    ASTLiteral* intern(const idl::location& loc, bool b) {
        return internLiteral<ASTLiteralBool, Exception>(loc, "bool|" + std::to_string(b), b);
    }

    template <typename Exception = void>
    ASTLiteral* intern(const idl::location& loc, int64_t num) {
        return internLiteral<ASTLiteralInt, Exception>(loc, "int|" + std::to_string(num), num);
    }

    template <typename Exception>
    void addSymbol(ASTDecl* decl) {
        const auto fullname = decl->fullnameLowecase();
        if (_symbols.contains(fullname)) {
            throw Exception(decl->location, err_str<E2030>(decl->fullname()));
        }
        _symbols[fullname] = decl;
        if (!_files.empty()) {
            decl->file = _files.back();
            _files.back()->decls.push_back(decl);
        }
    }

    template <typename Exception>
    void addDocSymbol(ASTDocDecl* decl) {
        const auto fullname = decl->fullnameLowecase();
        if (_docSymbols.contains(fullname)) {
            throw Exception(decl->location, err_str<E2030>(decl->fullname()));
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
                    err<E2037>(loc, actualName, expectedName);
                }
                if (onlyType) {
                    if (it->second->is<ASTType>()) {
                        return it->second;
                    }
                } else {
                    return it->second;
                }
            }
            decl = decl->parent->as<ASTDecl>();
        }
        err<E2032>(loc, name);
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
            err<E2035>(declRef->location, decl->fullname());
        }
    }

    void setDeclaring(bool active = true) noexcept {
        _declaring = active;
    }

    bool isDeclaring() const noexcept {
        return _declaring;
    }

    template <typename Exception>
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
                    doc.push_back(intern<Exception>(loc, std::string(" ")));
                }
                str     = detail.substr(prevPos, pos - prevPos);
                prevPos = pos + 1;
                doc.push_back(intern<Exception>(loc, str));
            }
            if (prevPos < detail.length()) {
                if (!doc.empty()) {
                    doc.push_back(intern<Exception>(loc, std::string(" ")));
                }
                doc.push_back(intern<Exception>(loc, detail.substr(prevPos)));
            }

            auto node         = allocNode<Node, Exception>(loc);
            node->name        = std::move(name);
            node->parent      = _api;
            node->doc         = allocNode<ASTDoc, Exception>(loc);
            node->doc->detail = std::move(doc);
            node->doc->parent = node;

            auto attr    = allocNode<ASTAttrCName>(loc);
            attr->name   = cname;
            attr->parent = node;
            node->attrs.push_back(attr);

            addSymbol<Exception>(node);
        };

        addBuiltin("Void", "void", "void type", ASTVoid{});
        addBuiltin("Char", "char", "symbol type", ASTChar{});
        addBuiltin("Bool", "bool", "boolean type", ASTBool{});
        addBuiltin("Int8", "sint8", "8 bit signed integer", ASTInt8{});
        addBuiltin("Uint8", "uint8", "8 bit unsigned integer", ASTUint8{});
        addBuiltin("Int16", "sint16", "16 bit signed integer", ASTInt16{});
        addBuiltin("Uint16", "uint16", "16 bit unsigned integer", ASTUint16{});
        addBuiltin("Int32", "sint32", "32 bit signed integer", ASTInt32{});
        addBuiltin("Uint32", "uint32", "32 bit unsigned integer", ASTUint32{});
        addBuiltin("Int64", "sint64", "64 bit signed integer", ASTInt64{});
        addBuiltin("Uint64", "uint64", "64 bit unsigned integer", ASTUint64{});
        addBuiltin("Float32", "float32", "32 bit float point", ASTFloat32{});
        addBuiltin("Float64", "float64", "64 bit float point", ASTFloat64{});
        addBuiltin("Str", "utf8", "utf8 string", ASTStr{});
        addBuiltin("Data", "data", "pointer to data", ASTData{});
        addBuiltin("ConstData", "cdata", "pointer to immutable data", ASTConstData{});

        const std::chrono::time_point now{ std::chrono::system_clock::now() };
        const std::chrono::year_month_day ymd{ std::chrono::floor<std::chrono::days>(now) };
        auto year   = allocNode<ASTYear, Exception>(loc);
        year->name  = "Year";
        year->value = static_cast<int>(ymd.year());
        addDocSymbol<Exception>(year);
    }

    template <typename Exception>
    void addAttrs(ASTDecl* node, const std::vector<ASTAttr*>& attrs) {
        node->attrs.insert(node->attrs.end(), attrs.begin(), attrs.end());
        std::set<std::type_index> uniqueAttrs;
        for (auto attr : node->attrs) {
            if (uniqueAttrs.contains(typeid(*attr))) {
                AttrName name;
                attr->accept(name);
                throw Exception(attr->location, err_str<E2013>(name.str));
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
                throw Exception(attr->location, err_str<E2014>(ss.str()));
            }
            attr->parent = node;
        }
    }

    void prepareEnumConsts() {
        std::vector<ASTEnumConst*> needAddTypeAttrs{};
        filter<ASTEnum>([this, &needAddTypeAttrs](ASTEnum* en) {
            if (en->consts.empty()) {
                err<E2026>(en->location, en->name);
                return false;
            }
            for (auto ec : en->consts) {
                if (!ec->findAttr<ASTAttrType>()) {
                    needAddTypeAttrs.push_back(ec);
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
            attr->value  = intern(ec->location, (int64_t) ec->value);
            ec->attrs.push_back(attr);
        }
    }

    void prepareStructs() {
        std::vector<ASTField*> needAddType{};
        std::vector<ASTField*> needAddRef{};
        filter<ASTStruct>([this, &needAddType, &needAddRef](ASTStruct* node) {
            if (node->fields.empty()) {
                err<E2081>(node->location, node->fullname());
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
                        err<E2077>(field->location, field->name, node->fullname());
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
                    err<E2068>(field->location, field->name, node->fullname());
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
                            err<E2079>(field->location);
                        }
                        auto type = resolveType(sizeField->findAttr<ASTAttrType>()->type);
                        if (!type->is<ASTIntegerType>()) {
                            err<E2080>(attr->location, field->fullname());
                        }
                    } else {
                        err<E2078>(attr->location, field->fullname());
                    }
                }
            }
        });
    }

    void prepareCallbacks() {
        std::vector<ASTCallback*> needAddRetType{};
        std::vector<ASTArg*> needAddArgType{};
        std::vector<ASTArg*> needAddRef{};
        filter<ASTCallback>([this, &needAddRetType, &needAddArgType, &needAddRef](ASTCallback* node) {
            if (!node->findAttr<ASTAttrType>()) {
                needAddRetType.push_back(node);
            }
            int countUserData = 0;
            int countResult   = 0;
            for (auto arg : node->args) {
                if (!arg->findAttr<ASTAttrType>()) {
                    needAddArgType.push_back(arg);
                }
                if (arg->findAttr<ASTAttrThis>()) {
                    err<E2083>(arg->location, node->fullname(), arg->name);
                }
                countUserData += arg->findAttr<ASTAttrUserData>() ? 1 : 0;
                countResult += arg->findAttr<ASTAttrResult>() ? 1 : 0;
                if (countUserData > 1) {
                    err<E2082>(arg->location);
                }
                if (countResult > 1) {
                    err<E2084>(arg->location);
                }
                if (auto attr = arg->findAttr<ASTAttrArray>()) {
                    if (attr->ref) {
                        if (arg->findAttr<ASTAttrRef>() == nullptr) {
                            needAddRef.push_back(arg);
                        }
                    } else {
                        err<E2102>(arg->location, arg->name, node->fullname());
                    }
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
        for (auto node : needAddArgType) {
            auto attr          = allocNode<ASTAttrType>(node->location);
            attr->parent       = node;
            attr->type         = allocNode<ASTDeclRef>(node->location);
            attr->type->name   = "Int32";
            attr->type->parent = attr;
            node->attrs.push_back(attr);
        }
        for (auto node : needAddRef) {
            auto attr    = allocNode<ASTAttrRef>(node->location);
            attr->parent = node;
            node->attrs.push_back(attr);
        }
        filter<ASTCallback>([this](ASTCallback* node) {
            auto attr = node->findAttr<ASTAttrType>();
            resolveType(attr->type);
            for (auto arg : node->args) {
                auto argAttr = arg->findAttr<ASTAttrType>();
                if (resolveType(argAttr->type)->is<ASTVoid>()) {
                    err<E2074>(arg->location, arg->name, node->fullname());
                }
                if (auto attr = arg->findAttr<ASTAttrArray>(); attr) {
                    assert(attr->ref);
                    auto symbol = findSymbol(node, attr->location, attr->decl);
                    if (auto sizeField = symbol->as<ASTArg>()) {
                        if (arg->parent != sizeField->parent) {
                            err<E2107>(arg->location);
                        }
                        auto type = resolveType(sizeField->findAttr<ASTAttrType>()->type);
                        if (!type->is<ASTIntegerType>()) {
                            err<E2080>(attr->location, arg->fullname());
                        }
                    } else {
                        err<E2108>(attr->location, arg->fullname());
                    }
                }
            }
        });
    }

    void prepareFunctions() {
        std::vector<ASTFunc*> needAddRetType{};
        std::vector<ASTArg*> needAddArgType{};
        std::vector<ASTArg*> needAddRef{};
        filter<ASTFunc>([this, &needAddRetType, &needAddArgType, &needAddRef](ASTFunc* node) {
            if (!node->findAttr<ASTAttrType>()) {
                needAddRetType.push_back(node);
            }
            int countUserData = 0;
            int countResult   = 0;
            for (auto arg : node->args) {
                if (!arg->findAttr<ASTAttrType>()) {
                    needAddArgType.push_back(arg);
                }
                if (arg->findAttr<ASTAttrThis>()) {
                    err<E2073>(arg->location, node->fullname(), arg->name);
                }
                countUserData += arg->findAttr<ASTAttrUserData>() ? 1 : 0;
                countResult += arg->findAttr<ASTAttrResult>() ? 1 : 0;
                if (countUserData > 1) {
                    err<E2082>(arg->location);
                }
                if (countResult > 1) {
                    err<E2084>(arg->location);
                }
                if (auto attr = arg->findAttr<ASTAttrArray>()) {
                    if (attr->ref) {
                        if (arg->findAttr<ASTAttrRef>() == nullptr) {
                            needAddRef.push_back(arg);
                        }
                    } else {
                        err<E2102>(arg->location, arg->name, node->fullname());
                    }
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
        for (auto node : needAddArgType) {
            auto attr          = allocNode<ASTAttrType>(node->location);
            attr->parent       = node;
            attr->type         = allocNode<ASTDeclRef>(node->location);
            attr->type->name   = "Int32";
            attr->type->parent = attr;
            node->attrs.push_back(attr);
        }
        for (auto node : needAddRef) {
            auto attr    = allocNode<ASTAttrRef>(node->location);
            attr->parent = node;
            node->attrs.push_back(attr);
        }
        filter<ASTFunc>([this](ASTFunc* node) {
            auto attr = node->findAttr<ASTAttrType>();
            auto type = resolveType(attr->type);
            for (auto arg : node->args) {
                auto argAttr = arg->findAttr<ASTAttrType>();
                if (resolveType(argAttr->type)->is<ASTVoid>()) {
                    err<E2074>(arg->location, arg->name, node->fullname());
                }
                if (auto attr = arg->findAttr<ASTAttrArray>(); attr) {
                    assert(attr->ref);
                    auto symbol = findSymbol(node, attr->location, attr->decl);
                    if (auto sizeField = symbol->as<ASTArg>()) {
                        if (arg->parent != sizeField->parent) {
                            err<E2105>(arg->location);
                        }
                        auto type = resolveType(sizeField->findAttr<ASTAttrType>()->type);
                        if (!type->is<ASTIntegerType>()) {
                            err<E2080>(attr->location, arg->fullname());
                        }
                    } else {
                        err<E2106>(attr->location, arg->fullname());
                    }
                }
            }
            if (node->findAttr<ASTAttrErrorCode>()) {
                auto argType        = node->args[0]->findAttr<ASTAttrType>()->type->decl;
                auto argIsErrorCode = argType->findAttr<ASTAttrErrorCode>() != nullptr;
                if (!type->is<ASTStr>() || node->args.size() != 1 || !argIsErrorCode) {
                    err<E2085>(node->location);
                }
            }
        });
    }

    void prepareMethods() {
        std::vector<ASTMethod*> needAddRetType{};
        std::vector<ASTMethod*> needAddStatic{};
        std::vector<ASTArg*> needAddArgType{};
        std::vector<ASTArg*> needAddArgIn{};
        std::vector<ASTArg*> needAddArgOut{};
        std::vector<ASTArg*> needAddRef{};
        filter<ASTMethod>(
            [this, &needAddRetType, &needAddStatic, &needAddArgType, &needAddArgIn, &needAddArgOut, &needAddRef](
                ASTMethod* node) {
            const auto isStatic = node->findAttr<ASTAttrStatic>();
            const auto isCtor   = node->findAttr<ASTAttrCtor>();
            if (!node->findAttr<ASTAttrType>()) {
                needAddRetType.push_back(node);
            }
            if (isCtor && !isStatic) {
                needAddStatic.push_back(node);
            }
            if (isCtor || isStatic) {
                for (auto arg : node->args) {
                    if (arg->findAttr<ASTAttrThis>()) {
                        if (isCtor) {
                            err<E2047>(arg->location, node->fullname(), arg->name);
                        } else {
                            err<E2046>(arg->location, node->fullname(), arg->name);
                        }
                    }
                }
            }
            if (!isCtor && !isStatic) {
                int countThis = 0;
                for (auto arg : node->args) {
                    countThis += arg->findAttr<ASTAttrThis>() ? 1 : 0;
                }
                if (countThis != 1) {
                    err<E2048>(node->location, node->fullname());
                }
            }
            int countUserData = 0;
            int countResult   = 0;
            for (auto arg : node->args) {
                if (!arg->findAttr<ASTAttrType>()) {
                    needAddArgType.push_back(arg);
                }
                auto hasOut = arg->findAttr<ASTAttrOut>() != nullptr;
                if (arg->findAttr<ASTAttrResult>() && !hasOut) {
                    needAddArgOut.push_back(arg);
                    hasOut = true;
                }
                if (!hasOut && !arg->findAttr<ASTAttrIn>()) {
                    needAddArgIn.push_back(arg);
                }
                countUserData += arg->findAttr<ASTAttrUserData>() ? 1 : 0;
                countResult += arg->findAttr<ASTAttrResult>() ? 1 : 0;
                if (countUserData > 1) {
                    err<E2082>(arg->location);
                }
                if (countResult > 1) {
                    err<E2084>(arg->location);
                }
                if (auto attr = arg->findAttr<ASTAttrArray>()) {
                    if (attr->ref) {
                        if (arg->findAttr<ASTAttrRef>() == nullptr) {
                            needAddRef.push_back(arg);
                        }
                    } else {
                        err<E2102>(arg->location, arg->name, node->fullname());
                    }
                }
            }
            return true;
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
        filter<ASTMethod>([this](ASTMethod* node) {
            auto attr = node->findAttr<ASTAttrType>();
            resolveType(attr->type);
            for (auto arg : node->args) {
                auto argAttr = arg->findAttr<ASTAttrType>();
                if (resolveType(argAttr->type)->is<ASTVoid>()) {
                    err<E2051>(arg->location, arg->name, node->fullname());
                }
                if (auto attr = arg->findAttr<ASTAttrArray>(); attr) {
                    assert(attr->ref);
                    auto symbol = findSymbol(node, attr->location, attr->decl);
                    if (auto sizeField = symbol->as<ASTArg>()) {
                        if (arg->parent != sizeField->parent) {
                            err<E2103>(arg->location);
                        }
                        auto type = resolveType(sizeField->findAttr<ASTAttrType>()->type);
                        if (!type->is<ASTIntegerType>()) {
                            err<E2080>(attr->location, arg->fullname());
                        }
                    } else {
                        err<E2104>(attr->location, arg->fullname());
                    }
                }
            }
            if (node->findAttr<ASTAttrRef>()) {
                if (node->findAttr<ASTAttrStatic>() || node->args.size() != 1) {
                    err<E2086>(node->location);
                }
            }
            if (node->findAttr<ASTAttrDestroy>()) {
                if (node->findAttr<ASTAttrStatic>() || node->args.size() != 1) {
                    err<E2087>(node->location);
                }
            }
            return true;
        });
    }

    void prepareProperties() {
        std::vector<std::pair<ASTProperty*, std::string>> needAddType{};
        filter<ASTProperty>([this, &needAddType](ASTProperty* node) {
            auto getter = node->findAttr<ASTAttrGet>();
            auto setter = node->findAttr<ASTAttrSet>();
            if (!getter && !setter) {
                err<E2052>(node->location, node->fullname());
            }
            auto isStaticProp       = node->findAttr<ASTAttrStatic>() != nullptr;
            ASTType* getterType     = nullptr;
            ASTType* setterType     = nullptr;
            ASTMethod* getterMethod = nullptr;
            ASTMethod* setterMethod = nullptr;
            if (getter) {
                auto decl = findSymbol(node, getter->location, getter->decl);
                if (auto method = decl->as<ASTMethod>()) {
                    getterMethod = method;
                    if (method->parent != node->parent) {
                        auto iface      = node->parent->as<ASTInterface>()->fullname();
                        auto otherIface = method->parent->as<ASTInterface>()->fullname();
                        err<E2054>(getter->location, node->name, iface, method->name, otherIface);
                    }
                    auto isStaticGetter = method->findAttr<ASTAttrStatic>() != nullptr;
                    if (isStaticProp != isStaticGetter) {
                        err<E2055>(getter->location, method->fullname(), node->fullname());
                    }
                    getterType          = resolveType(method->findAttr<ASTAttrType>()->type);
                    const auto argCount = method->args.size();
                    if (getterType->is<ASTVoid>()) {
                        bool isValidProp = false;
                        auto count       = isStaticProp ? 2 : 3;
                        if (argCount == count) {
                            auto res = std::find_if(method->args.begin(), method->args.end(), [](ASTArg* arg) {
                                return arg->findAttr<ASTAttrResult>() != nullptr;
                            });
                            if (res != method->args.end() && (*res)->findAttr<ASTAttrArray>()) {
                                if ((*res)->findAttr<ASTAttrArray>()->decl->decl->findAttr<ASTAttrOut>()) {
                                    isValidProp = true;
                                }
                            }
                        }
                        if (!isValidProp) {
                            err<E2058>(getter->location, method->fullname());
                        }
                    } else {
                        if (isStaticProp && argCount != 0) {
                            err<E2056>(getter->location, method->fullname());
                        } else if (!isStaticProp && argCount != 1) {
                            err<E2057>(getter->location, method->fullname());
                        }
                    }
                } else {
                    err<E2053>(getter->location, decl->fullname());
                }
            }
            if (setter) {
                auto decl = findSymbol(node, setter->location, setter->decl);
                if (auto method = decl->as<ASTMethod>()) {
                    setterMethod = method;
                    if (method->parent != node->parent) {
                        auto iface      = node->parent->as<ASTInterface>()->fullname();
                        auto otherIface = method->parent->as<ASTInterface>()->fullname();
                        err<E2061>(setter->location, node->name, iface, method->name, otherIface);
                    }
                    auto isStaticSetter = method->findAttr<ASTAttrStatic>() != nullptr;
                    if (isStaticProp != isStaticSetter) {
                        err<E2060>(setter->location, method->fullname(), node->fullname());
                    }
                    const auto argCount = method->args.size();
                    if (isStaticProp && argCount != 1) {
                        err<E2062>(setter->location, method->fullname());
                    } else if (!isStaticProp && argCount != 2) {
                        err<E2063>(setter->location, method->fullname());
                    }
                    for (auto arg : method->args) {
                        if (arg->findAttr<ASTAttrThis>() == nullptr) {
                            setterType = resolveType(arg->findAttr<ASTAttrType>()->type);
                            break;
                        }
                    }
                    assert(setterType);
                } else {
                    err<E2059>(setter->location, decl->fullname());
                }
            }
            if (getterType && setterType && getterType != setterType) {
                err<E2064>(node->location,
                           getterType->fullname(),
                           getterMethod->fullname(),
                           setterType->fullname(),
                           setterMethod->fullname());
            }
            if (auto attr = node->findAttr<ASTAttrType>()) {
                auto type = resolveType(attr->type);
                if (getterType && getterType != type) {
                    err<E2065>(attr->location, type->fullname(), getterType->fullname(), getterMethod->fullname());
                }
                if (setterType && setterType != type) {
                    err<E2066>(attr->location, type->fullname(), setterMethod->fullname(), setterType->fullname());
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

    void prepareEvents() {
        std::vector<std::pair<ASTEvent*, std::string>> needAddType{};
        filter<ASTEvent>([this, &needAddType](ASTEvent* node) {
            auto getter = node->findAttr<ASTAttrGet>();
            auto setter = node->findAttr<ASTAttrSet>();
            if (!getter && !setter) {
                err<E2091>(node->location, node->fullname());
            }
            auto isStaticProp       = node->findAttr<ASTAttrStatic>() != nullptr;
            ASTType* getterType     = nullptr;
            ASTType* setterType     = nullptr;
            ASTMethod* getterMethod = nullptr;
            ASTMethod* setterMethod = nullptr;
            if (getter) {
                auto decl = findSymbol(node, getter->location, getter->decl);
                if (auto method = decl->as<ASTMethod>()) {
                    getterMethod = method;
                    if (method->parent != node->parent) {
                        auto iface      = node->parent->as<ASTInterface>()->fullname();
                        auto otherIface = method->parent->as<ASTInterface>()->fullname();
                        err<E2092>(getter->location, node->name, iface, method->name, otherIface);
                    }
                    auto isStaticGetter = method->findAttr<ASTAttrStatic>() != nullptr;
                    if (isStaticProp != isStaticGetter) {
                        err<E2093>(getter->location, method->fullname(), node->fullname());
                    }
                    const auto argCount = method->args.size();
                    if (isStaticProp) {
                        if ((argCount == 1 && !method->args[0]->findAttr<ASTAttrUserData>()) || argCount > 1) {
                            err<E2094>(getter->location, method->fullname());
                        }
                    } else if (!isStaticProp) {
                        if (argCount == 2 && (!method->args[0]->findAttr<ASTAttrUserData>() &&
                                              !method->args[1]->findAttr<ASTAttrUserData>())) {
                            err<E2095>(getter->location, method->fullname());
                        } else if (argCount > 2) {
                            err<E2095>(getter->location, method->fullname());
                        }
                    }
                    getterType = resolveType(method->findAttr<ASTAttrType>()->type);
                    if (getterType->is<ASTVoid>()) {
                        err<E2058>(getter->location, method->fullname());
                    }
                } else {
                    err<E2053>(getter->location, decl->fullname());
                }
            }
            if (setter) {
                auto decl = findSymbol(node, setter->location, setter->decl);
                if (auto method = decl->as<ASTMethod>()) {
                    setterMethod = method;
                    if (method->parent != node->parent) {
                        auto iface      = node->parent->as<ASTInterface>()->fullname();
                        auto otherIface = method->parent->as<ASTInterface>()->fullname();
                        err<E2096>(setter->location, node->name, iface, method->name, otherIface);
                    }
                    auto isStaticSetter = method->findAttr<ASTAttrStatic>() != nullptr;
                    if (isStaticProp != isStaticSetter) {
                        err<E2097>(setter->location, method->fullname(), node->fullname());
                    }
                    const auto argCount = method->args.size();
                    if (isStaticProp && argCount != 1) {
                        if ((argCount == 2 && !method->args[0]->findAttr<ASTAttrUserData>() &&
                             !method->args[1]->findAttr<ASTAttrUserData>()) ||
                            argCount > 2) {
                            err<E2098>(getter->location, method->fullname());
                        }
                    } else if (!isStaticProp) {
                        if (argCount == 3 && (!method->args[0]->findAttr<ASTAttrUserData>() &&
                                              !method->args[1]->findAttr<ASTAttrUserData>() &&
                                              !method->args[2]->findAttr<ASTAttrUserData>())) {
                            err<E2099>(getter->location, method->fullname());
                        } else if (argCount > 3) {
                            err<E2099>(getter->location, method->fullname());
                        }
                    }
                    for (auto arg : method->args) {
                        if (arg->findAttr<ASTAttrThis>() == nullptr && arg->findAttr<ASTAttrUserData>() == nullptr) {
                            setterType = resolveType(arg->findAttr<ASTAttrType>()->type);
                            break;
                        }
                    }
                    assert(setterType);
                } else {
                    err<E2059>(setter->location, decl->fullname());
                }
            }
            if (getterType && setterType && getterType != setterType) {
                err<E2064>(node->location,
                           getterType->fullname(),
                           getterMethod->fullname(),
                           setterType->fullname(),
                           setterMethod->fullname());
            }
            if (auto attr = node->findAttr<ASTAttrType>()) {
                auto type = resolveType(attr->type);
                if (getterType && getterType != type) {
                    err<E2100>(attr->location, type->fullname(), getterType->fullname(), getterMethod->fullname());
                }
                if (setterType && setterType != type) {
                    err<E2101>(attr->location, type->fullname(), setterMethod->fullname(), setterType->fullname());
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

    void prepareInterfaces() {
        filter<ASTInterface>([this](ASTInterface* node) {
            int refMethodCount     = 0;
            int destroyMethodCount = 0;
            for (auto method : node->methods) {
                refMethodCount += method->findAttr<ASTAttrRef>() ? 1 : 0;
                destroyMethodCount += method->findAttr<ASTAttrDestroy>() ? 1 : 0;
                if (refMethodCount > 1) {
                    err<E2088>(method->location);
                }
                if (destroyMethodCount > 1) {
                    err<E2089>(method->location);
                }
            }
        });
    }

    void prepareHandles() {
        filter<ASTHandle>([this](ASTHandle* node) {
            if (auto attr = node->findAttr<ASTAttrType>()) {
                if (auto type = resolveType(attr->type); type->is<ASTStruct>()) {
                    if (!type->findAttr<ASTAttrHandle>()) {
                        err<E2071>(node->location, type->fullname(), node->fullname());
                    }
                } else {
                    err<E2070>(node->location, node->fullname());
                }
            } else {
                err<E2069>(node->location, node->fullname());
            }
        });
    }

    void prepareDocumentation() {
        filter<ASTDecl>([this](ASTDecl* node) {
            if (node->doc) {
                ValidateDoc validator;
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

    const std::optional<ApiVersion>& apiVersion() const noexcept {
        return _version;
    }

    void apiVersion(std::optional<ApiVersion> version) noexcept {
        _version = version;
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
    template <typename Node, typename Exception, typename Value>
    ASTLiteral* internLiteral(const idl::location& loc, const std::string& keyStr, const Value& value) {
        const auto key = XXH64(keyStr.c_str(), keyStr.length(), 0);
        if (auto it = _literals.find(key); it != _literals.end()) {
            return it->second;
        }
        auto literal   = allocNode<Node, Exception>(loc);
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
            err<E2040>(ec->location, ss.str());
        }
        deps.push_back(ec);

        if (auto typeAttr = ec->findAttr<ASTAttrType>()) {
            auto type = resolveType(typeAttr->type);
            if (!type->is<ASTInt32>()) {
                err<E2036>(typeAttr->location);
            }
        }

        auto en        = ec->parent->as<ASTEnum>();
        auto attrValue = ec->findAttr<ASTAttrValue>();

        if (attrValue) {
            if (auto literalInt = attrValue->value->as<ASTLiteralInt>()) {
                if (literalInt->value < INT32_MIN || literalInt->value > INT32_MAX) {
                    err<E2038>(attrValue->location);
                }
                ec->value = (int32_t) literalInt->value;
            } else if (auto literalConsts = attrValue->value->as<ASTLiteralConsts>()) {
                std::set<ASTDecl*> uniqueDecls;
                for (auto declRef : literalConsts->decls) {
                    auto decl = findSymbol(en, ec->location, declRef);
                    if (uniqueDecls.contains(decl)) {
                        err<E2039>(decl->location, decl->fullname());
                    }
                    uniqueDecls.insert(decl);
                    if (decl == ec) {
                        err<E2033>(decl->location, decl->fullname());
                    }
                    if (auto refEc = decl->as<ASTEnumConst>()) {
                        calcEnumConst(refEc, deps);
                        ec->value |= refEc->value;
                    } else {
                        err<E2034>(ec->location);
                    }
                }
            } else {
                err<E2031>(attrValue->location);
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

    std::optional<ApiVersion> _version{};
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
