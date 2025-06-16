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

    const ASTApi* api(const idl::location& loc) const noexcept {
        return _api;
    }

    template <typename Node, typename Exception>
    Node* allocNode(const idl::location& loc, int token) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        auto node = new (std::nothrow) Node{};
        if (!node) {
            throw Exception(loc, "out of memory");
        }
        if constexpr (std::is_same<Node, ASTApi>::value) {
            _api = node;
        }
        node->location = loc;
        node->token    = token;
        _nodes.push_back(node);
        return node;
    }

    template <typename Exception>
    ASTLiteral* intern(const idl::location& loc, const std::string& str, int token) {
        return internLiteral<Exception, ASTLiteralStr>(loc, "str|" + str, str, token);
    }

    template <typename Exception>
    ASTLiteral* intern(const idl::location& loc, bool b, int token) {
        return internLiteral<Exception, ASTLiteralBool>(loc, "bool|" + std::to_string(b), b, token);
    }

    template <typename Exception>
    ASTLiteral* intern(const idl::location& loc, int64_t num, int token) {
        return internLiteral<Exception, ASTLiteralInt>(loc, "int|" + std::to_string(num), num, token);
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

    void currentDeclLine(int currentDeclLine) noexcept {
        _currentDeclLine = currentDeclLine;
    }

    int currentDeclLine() const noexcept {
        return _currentDeclLine;
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
                doc.push_back(intern<Exception>(loc, str, -1));
            }
            doc.push_back(intern<Exception>(loc, detail.substr(prevPos), -1));

            auto node         = allocNode<Node, Exception>(loc, -1);
            node->name        = std::move(name);
            node->parent      = _api;
            node->doc         = allocNode<ASTDoc, Exception>(loc, -1);
            node->doc->detail = std::move(doc);
            node->doc->parent = node;
            addSymbol<Exception>(node);
        };

        addBuiltin("Char", "symbol type", ASTChar{});
        addBuiltin("Str", "utf8 string", ASTStr{});
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

    void calcEnumConsts() {
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
            auto attr          = allocNode<ASTAttrType, Exception>(ec->location, -1);
            attr->parent       = ec;
            attr->type         = allocNode<ASTDeclRef, Exception>(ec->location, -1);
            attr->type->name   = "Int32";
            attr->type->parent = attr;
            ec->attrs.push_back(attr);
        }

        std::vector<ASTEnumConst*> needAddValueAttrs{};
        filter<ASTEnum>([this, &needAddValueAttrs](auto en) {
            for (auto ec : en->consts) {
                calcEnumConst(ec);
                if (!ec->template findAttr<ASTAttrValue>()) {
                    needAddValueAttrs.push_back(ec);
                }
            }
            return true;
        });
        for (auto ec : needAddValueAttrs) {
            auto attr    = allocNode<ASTAttrValue, Exception>(ec->location, -1);
            attr->parent = ec;
            attr->value  = intern<Exception>(ec->location, (int64_t) ec->value, -1);
            ec->attrs.push_back(attr);
        }
    }

private:
    template <typename Node, typename Pred>
    bool filter(Pred&& pred) {
        static_assert(std::is_base_of<ASTNode, Node>::value, "Node must be inherited from ASTNode");
        for (auto node : _nodes) {
            if (auto ptr = dynamic_cast<Node*>(node)) {
                if (!pred(ptr)) {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename Exception, typename Node, typename Value>
    ASTLiteral* internLiteral(const idl::location& loc, const std::string& keyStr, const Value& value, int token) {
        const auto key = XXH64(keyStr.c_str(), keyStr.length(), 0);
        if (auto it = _literals.find(key); it != _literals.end()) {
            return it->second;
        }
        auto literal   = allocNode<Node, Exception>(loc, token);
        literal->value = value;
        _literals[key] = literal;
        return literal;
    }

    void calcEnumConst(ASTEnumConst* ec) {
        if (ec->evaluated) {
            return;
        }
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
                for (auto declRef : literalConsts->decls) {
                    auto decl = findSymbol(en, ec->location, declRef);
                    if (decl == ec) {
                        err<E2033>(decl->location, decl->fullname());
                    }
                    if (auto refEc = decl->as<ASTEnumConst>()) {
                        calcEnumConst(refEc);
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
                calcEnumConst(c);
                prevValue = c->value;
            }
            ec->value = prevValue + 1;
        }
        ec->evaluated = true;
    }

    struct Exception : std::runtime_error {
        Exception(const idl::location& loc, const std::string& m) : std::runtime_error(m), location(loc) {
        }

        const idl::location& location;
    };

    ASTApi* _api{};
    std::vector<ASTNode*> _nodes{};
    std::unordered_map<std::string, struct ASTDecl*> _symbols{};
    std::unordered_map<uint64_t, ASTLiteral*> _literals{};
    int _currentDeclLine{ -1 };
};

} // namespace idl

#endif
