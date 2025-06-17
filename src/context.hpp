#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <sstream>
#include <unordered_map>

#include <xxhash.h>

#include "ast.hpp"
#include "errors.hpp"
#include "location.hpp"
#include "visitors.hpp"

namespace idl {

class Context final {
public:
    ~Context() {
        for (auto node : _nodes) {
            delete node;
        }
    }

    const ASTApi* api() const noexcept {
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
        return internLiteral<Exception, ASTLiteralStr>(loc, "str|" + str, str);
    }

    template <typename Exception = void>
    ASTLiteral* intern(const idl::location& loc, bool b) {
        return internLiteral<Exception, ASTLiteralBool>(loc, "bool|" + std::to_string(b), b);
    }

    template <typename Exception = void>
    ASTLiteral* intern(const idl::location& loc, int64_t num) {
        return internLiteral<Exception, ASTLiteralInt>(loc, "int|" + std::to_string(num), num);
    }

    template <typename Exception>
    void addSymbol(ASTDecl* decl) {
        const auto fullname = decl->fullnameLowecase();
        if (_symbols.contains(fullname)) {
            throw Exception(decl->location, err_str<E2030>(decl->fullname()));
        }
        _symbols[fullname] = decl;
    }

    ASTDecl* findSymbol(ASTDecl* decl, const idl::location& loc, const std::string& name) {
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
                return it->second;
            }
            decl = decl->parent->as<ASTDecl>();
        }
        err<E2032>(loc, name);
    }

    ASTDecl* findSymbol(ASTDecl* decl, const idl::location& loc, ASTDeclRef* declRef) {
        if (!declRef->decl) {
            auto symbol   = findSymbol(decl, loc, declRef->name);
            declRef->decl = symbol;
            return symbol;
        } else {
            return declRef->decl;
        }
    }

    ASTType* resolveType(ASTDeclRef* declRef) {
        auto parent = declRef->parent->is<ASTAttr>() ? declRef->parent->parent : declRef->parent;
        auto decl   = findSymbol(parent->as<ASTDecl>(), declRef->location, declRef);

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

        auto addBuiltin = [this, &loc]<typename Node>(std::string&& name, const std::string& detail, Node) {
            std::vector<ASTNode*> doc{};
            size_t prevPos = 0;
            size_t pos     = 0;
            std::string str;
            while ((pos = detail.find(' ', prevPos)) != std::string::npos) {
                str     = detail.substr(prevPos, pos - prevPos);
                prevPos = pos + 1;
                doc.push_back(intern<Exception>(loc, str));
            }
            doc.push_back(intern<Exception>(loc, detail.substr(prevPos)));

            auto node         = allocNode<Node, Exception>(loc);
            node->name        = std::move(name);
            node->parent      = _api;
            node->doc         = allocNode<ASTDoc, Exception>(loc);
            node->doc->detail = std::move(doc);
            node->doc->parent = node;
            addSymbol<Exception>(node);
        };

        addBuiltin("Void", "void type", ASTVoid{});
        addBuiltin("Char", "symbol type", ASTChar{});
        addBuiltin("Bool", "boolean type", ASTBool{});
        addBuiltin("Int8", "8 bit signed integer", ASTInt8{});
        addBuiltin("Uint8", "8 bit unsigned integer", ASTUint8{});
        addBuiltin("Int16", "16 bit signed integer", ASTInt16{});
        addBuiltin("Uint16", "16 bit unsigned integer", ASTUint16{});
        addBuiltin("Int32", "32 bit signed integer", ASTInt32{});
        addBuiltin("Uint32", "32 bit unsigned integer", ASTUint32{});
        addBuiltin("Int64", "64 bit signed integer", ASTInt64{});
        addBuiltin("Uint64", "64 bit unsigned integer", ASTUint64{});
        addBuiltin("Float32", "32 bit float point", ASTFloat32{});
        addBuiltin("Float64", "64 bit float point", ASTFloat64{});
        addBuiltin("Str", "utf8 string", ASTStr{});
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
        filter<ASTEnum>([this, &needAddTypeAttrs](auto en) {
            if (en->consts.empty()) {
                err<E2026>(en->location, en->name);
                return false;
            }
            for (auto ec : en->consts) {
                if (!ec->template findAttr<ASTAttrType>()) {
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
        filter<ASTEnum>([this, &needAddValueAttrs](auto en) {
            std::vector<ASTEnumConst*> deps;
            for (auto ec : en->consts) {
                calcEnumConst(ec, deps);
                if (!ec->template findAttr<ASTAttrValue>()) {
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
        filter<ASTStruct>([this, &needAddType](auto node) {
            for (auto field : node->fields) {
                if (!field->template findAttr<ASTAttrType>()) {
                    needAddType.push_back(field);
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
        filter<ASTStruct>([this, &needAddType](auto node) {
            for (auto field : node->fields) {
                auto attr = field->template findAttr<ASTAttrType>();
                if (resolveType(attr->type)->template is<ASTVoid>()) {
                    err<E2068>(field->location, field->name, node->fullname());
                }
            }
        });
    }

    void prepareMethods() {
        std::vector<ASTMethod*> needAddRetType{};
        std::vector<ASTMethod*> needAddStatic{};
        std::vector<ASTArg*> needAddArgType{};
        filter<ASTMethod>([this, &needAddRetType, &needAddStatic, &needAddArgType](auto node) {
            const auto isStatic = node->template findAttr<ASTAttrStatic>();
            const auto isCtor   = node->template findAttr<ASTAttrCtor>();
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
                    countThis += arg->template findAttr<ASTAttrThis>() ? 1 : 0;
                }
                if (countThis != 1) {
                    err<E2048>(node->location, node->fullname());
                }
            }
            for (auto arg : node->args) {
                if (!arg->template findAttr<ASTAttrType>()) {
                    needAddArgType.push_back(arg);
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
        filter<ASTMethod>([this](auto node) {
            auto attr = node->template findAttr<ASTAttrType>();
            resolveType(attr->type);
            for (auto arg : node->args) {
                auto argAttr = arg->template findAttr<ASTAttrType>();
                if (dynamic_cast<ASTVoid*>(resolveType(argAttr->type)) != nullptr) {
                    err<E2051>(arg->location, arg->name, node->fullname());
                }
            }
            return true;
        });
    }

    void prepareProperties() {
        std::vector<std::pair<ASTProperty*, std::string>> needAddType{};
        filter<ASTProperty>([this, &needAddType](auto node) {
            auto getter = node->template findAttr<ASTAttrGet>();
            auto setter = node->template findAttr<ASTAttrSet>();
            if (!getter && !setter) {
                err<E2052>(node->location, node->fullname());
            }
            auto isStaticProp       = node->template findAttr<ASTAttrStatic>() != nullptr;
            ASTType* getterType     = nullptr;
            ASTType* setterType     = nullptr;
            ASTMethod* getterMethod = nullptr;
            ASTMethod* setterMethod = nullptr;
            if (getter) {
                auto decl = findSymbol(node, getter->location, getter->decl);
                if (auto method = dynamic_cast<ASTMethod*>(decl)) {
                    getterMethod = method;
                    if (method->parent != node->parent) {
                        auto iface      = dynamic_cast<ASTInterface*>(node->parent)->fullname();
                        auto otherIface = dynamic_cast<ASTInterface*>(method->parent)->fullname();
                        err<E2054>(getter->location, node->name, iface, method->name, otherIface);
                    }
                    auto isStaticGetter = method->template findAttr<ASTAttrStatic>() != nullptr;
                    if (isStaticProp != isStaticGetter) {
                        err<E2055>(getter->location, method->fullname(), node->fullname());
                    }
                    const auto argCount = method->args.size();
                    if (isStaticProp && argCount != 0) {
                        err<E2056>(getter->location, method->fullname());
                    } else if (!isStaticProp && argCount != 1) {
                        err<E2057>(getter->location, method->fullname());
                    }
                    getterType = resolveType(method->template findAttr<ASTAttrType>()->type);
                    if (dynamic_cast<ASTVoid*>(getterType) != nullptr) {
                        err<E2058>(getter->location, method->fullname());
                    }
                } else {
                    err<E2053>(getter->location, decl->fullname());
                }
            }
            if (setter) {
                auto decl = findSymbol(node, setter->location, setter->decl);
                if (auto method = dynamic_cast<ASTMethod*>(decl)) {
                    setterMethod = method;
                    if (method->parent != node->parent) {
                        auto iface      = dynamic_cast<ASTInterface*>(node->parent)->fullname();
                        auto otherIface = dynamic_cast<ASTInterface*>(method->parent)->fullname();
                        err<E2061>(setter->location, node->name, iface, method->name, otherIface);
                    }
                    auto isStaticSetter = method->template findAttr<ASTAttrStatic>() != nullptr;
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
                        if (arg->template findAttr<ASTAttrThis>() == nullptr) {
                            setterType = resolveType(arg->template findAttr<ASTAttrType>()->type);
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
            if (auto attr = node->template findAttr<ASTAttrType>()) {
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

    void prepareHandles() {
        
    }

    template <typename Node, typename Pred>
    bool filter(Pred&& pred) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        constexpr auto isVoid = std::is_same_v<decltype(pred((Node*) nullptr)), void>;
        for (auto node : _nodes) {
            if (auto ptr = dynamic_cast<Node*>(node)) {
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
    template <typename Exception, typename Node, typename Value>
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
            if (auto literalInt = attrValue->value->template as<ASTLiteralInt>()) {
                if (literalInt->value < INT32_MIN || literalInt->value > INT32_MAX) {
                    err<E2038>(attrValue->location);
                }
                ec->value = (int32_t) literalInt->value;
            } else if (auto literalConsts = attrValue->value->template as<ASTLiteralConsts>()) {
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

    ASTApi* _api{};
    std::vector<ASTNode*> _nodes{};
    std::unordered_map<std::string, struct ASTDecl*> _symbols{};
    std::unordered_map<uint64_t, ASTLiteral*> _literals{};
    bool _declaring{};
};

} // namespace idl

#endif
