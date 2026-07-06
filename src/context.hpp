#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "ast.hpp"
#include "case_converter.hpp"
#include "compilation_result.hpp"
#include "errors.hpp"
#include "options.hpp"

namespace idl {

enum class BoolType {
    Int32,
    Int8,
    StdBool
};

class Context final {
public:
    Context(Options* options, CompilationResult* result) noexcept : _options(options), _result(result) {
        _nodes.reserve(1024);
    }

    ASTNodeHandle api() noexcept {
        return _api;
    }

    bool useStdTypes() const noexcept {
        return _useStdTypes;
    }

    BoolType boolType() const noexcept {
        return _boolType;
    }

    ASTNode* getNode(ASTNodeHandle handle) noexcept {
        if (handle.handle < _nodes.size() && handle != NodeHandleNone) {
            return &_nodes[handle.handle];
        }
        return nullptr;
    }

    auto getNodeChilds(ASTNode* node) noexcept {
        struct SiblingIterator {
            ASTNodeHandle current;
            Context* context;

            auto operator*() const noexcept {
                return current;
            }

            SiblingIterator& operator++() noexcept {
                if (current != NodeHandleNone) {
                    auto node = context->getNode(current);
                    current   = node ? node->sibling : NodeHandleNone;
                }
                return *this;
            }

            bool operator!=(const SiblingIterator& other) const noexcept {
                return current != other.current;
            }
        };

        struct SiblingRange {
            SiblingIterator beginIt;
            SiblingIterator endIt;

            SiblingIterator begin() noexcept {
                return beginIt;
            }

            SiblingIterator end() noexcept {
                return endIt;
            }

            bool empty() const noexcept {
                return beginIt.current == endIt.current;
            }
        };

        return SiblingRange{
            { node ? node->child : NodeHandleNone, this },
            { NodeHandleNone,                      this }
        };
    }

    auto getNodeChilds(ASTNodeHandle node) noexcept {
        return getNodeChilds(getNode(node));
    }

    template <ASTNodeType Type>
    ASTNode* findChild(ASTNode* node) noexcept {
        auto curr = node->child;
        while (curr != NodeHandleNone) {
            auto node = getNode(curr);
            if (node->type == Type) {
                return node;
            }
            curr = node->sibling;
        }
        return nullptr;
    }

    template <ASTNodeType Type>
    ASTNode* findChild(ASTNodeHandle node) noexcept {
        return getNodeChilds(getNode(node));
    }

    void addChild(ASTNodeHandle parent, ASTNodeHandle child) noexcept {
        auto parentNode = getNode(parent);
        if (parentNode->child == NodeHandleNone) {
            parentNode->child = child;
        } else {
            auto childs    = getNodeChilds(parent);
            auto lastChild = *std::end(childs);

            getNode(lastChild)->sibling = child;
        }
    }

    ASTNodeHandle allocNode(const idl::location& loc, ASTNodeType type) {
        auto index = _nodes.size();
        auto& node = _nodes.emplace_back();

        node.location.filename = _stringPool.insert({ loc.begin.filename->c_str(), loc.begin.filename->length() });
        node.location.line     = loc.begin.line;
        node.location.column   = loc.begin.column;
        node.type              = type;
        node.parent            = NodeHandleNone;
        node.sibling           = NodeHandleNone;
        node.child             = NodeHandleNone;

        return { uint16_t(index) };
    }

    void addSymbol(ASTNodeHandle decl) {
        // if (getNode(decl)->as<ASTImport>()) {
        //     return;
        // }
        // const auto fullname = decl->fullnameLowecase();
        // if (_symbols.contains(fullname)) {
        //     log<IDL_STATUS_E3012>(decl->location, decl->fullname());
        // }
        // _symbols[fullname] = decl;
    }

    // ASTDecl* findSymbol(ASTDecl* decl, const idl::location& loc, const std::string& name, bool onlyType = false) {
    //     auto nameLower = name;
    //     std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](auto c) {
    //         return std::tolower(c);
    //     });
    //     while (decl) {
    //         const auto fullname = decl->fullnameLowecase() + '.' + nameLower;
    //         if (auto it = _symbols.find(fullname); it != _symbols.end()) {
    //             const auto actualName   = decl->fullname() + '.' + name;
    //             const auto expectedName = it->second->fullname();
    //             if (actualName != expectedName) {
    //                 log<IDL_STATUS_E3036>(loc, actualName, expectedName);
    //                 return nullptr;
    //             }
    //             if (onlyType) {
    //                 if (it->second->as<ASTType>()) {
    //                     return it->second;
    //                 }
    //             } else {
    //                 return it->second;
    //             }
    //         }
    //         decl = decl->parent ? decl->parent->as<ASTDecl>() : nullptr;
    //     }
    //     if (auto it = _symbols.find(nameLower); it != _symbols.end()) {
    //         const auto expectedName = it->second->fullname();
    //         if (name != expectedName) {
    //             log<IDL_STATUS_E3036>(loc, name, expectedName);
    //             return nullptr;
    //         }
    //         if (onlyType) {
    //             if (it->second->as<ASTType>()) {
    //                 return it->second;
    //             }
    //         } else {
    //             return it->second;
    //         }
    //     }
    //     err<IDL_STATUS_E3037>(loc, name);
    //     return nullptr;
    // }
    //
    // ASTDecl* resolveRef(ASTDecl* decl, const idl::location& loc, ASTDeclRef* declRef, bool onlyType = false) {
    //     if (!declRef->decl) {
    //         if (auto symbol = findSymbol(decl, loc, declRef->name, onlyType)) {
    //             declRef->decl = symbol;
    //             return symbol;
    //         }
    //     }
    //     return declRef->decl;
    // }

    template <typename Visitor, typename... Args>
    Visitor visit(ASTNodeHandle handle, Args&&... args) {
        Visitor visitor(*this, std::forward<Args>(args)...);
        auto node = getNode(handle);
        if (node) {
            switch (node->type) {
                case ASTNodeType::Api:
                    visitor.visit(node, Tag<ASTNodeType::Api>{});
                    break;
                case ASTNodeType::Import:
                    visitor.visit(node, Tag<ASTNodeType::Import>{});
                    break;
                case ASTNodeType::Enum:
                    visitor.visit(node, Tag<ASTNodeType::Enum>{});
                    break;
                case ASTNodeType::Const:
                    visitor.visit(node, Tag<ASTNodeType::Const>{});
                    break;
                case ASTNodeType::DeclRef:
                    visitor.visit(node, Tag<ASTNodeType::DeclRef>{});
                    break;
                case ASTNodeType::LiteralStr:
                    visitor.visit(node, Tag<ASTNodeType::LiteralStr>{});
                    break;
                case ASTNodeType::LiteralInt:
                    visitor.visit(node, Tag<ASTNodeType::LiteralInt>{});
                    break;
                case ASTNodeType::LiteralBool:
                    visitor.visit(node, Tag<ASTNodeType::LiteralBool>{});
                    break;
                case ASTNodeType::LiteralFloat:
                    visitor.visit(node, Tag<ASTNodeType::LiteralFloat>{});
                    break;
                case ASTNodeType::AttrFlags:
                    visitor.visit(node, Tag<ASTNodeType::AttrFlags>{});
                    break;
                case ASTNodeType::AttrHex:
                    visitor.visit(node, Tag<ASTNodeType::AttrHex>{});
                    break;
                case ASTNodeType::AttrValue:
                    visitor.visit(node, Tag<ASTNodeType::AttrValue>{});
                    break;
                case ASTNodeType::AttrType:
                    visitor.visit(node, Tag<ASTNodeType::AttrType>{});
                    break;
                case ASTNodeType::AttrCName:
                    visitor.visit(node, Tag<ASTNodeType::AttrCName>{});
                    break;
                case ASTNodeType::AttrTokenizer:
                    visitor.visit(node, Tag<ASTNodeType::AttrTokenizer>{});
                    break;
                case ASTNodeType::AttrOrder:
                    visitor.visit(node, Tag<ASTNodeType::AttrOrder>{});
                    break;
                case ASTNodeType::AttrSingle:
                    visitor.visit(node, Tag<ASTNodeType::AttrSingle>{});
                    break;
                case ASTNodeType::AttrVersion:
                    visitor.visit(node, Tag<ASTNodeType::AttrVersion>{});
                    break;
                case ASTNodeType::AttrDocBrief:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocBrief>{});
                    break;
                case ASTNodeType::AttrDocDetail:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocDetail>{});
                    break;
                case ASTNodeType::AttrDocAuthor:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocAuthor>{});
                    break;
                case ASTNodeType::AttrDocCopyright:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocCopyright>{});
                    break;
                case ASTNodeType::AttrDocLicense:
                    visitor.visit(node, Tag<ASTNodeType::AttrDocLicense>{});
                    break;
                case ASTNodeType::Void:
                    visitor.visit(node, Tag<ASTNodeType::Void>{});
                    break;
                case ASTNodeType::Data:
                    visitor.visit(node, Tag<ASTNodeType::Data>{});
                    break;
                case ASTNodeType::Char:
                    visitor.visit(node, Tag<ASTNodeType::Char>{});
                    break;
                case ASTNodeType::Str:
                    visitor.visit(node, Tag<ASTNodeType::Str>{});
                    break;
                case ASTNodeType::Bool:
                    visitor.visit(node, Tag<ASTNodeType::Bool>{});
                    break;
                case ASTNodeType::Int8:
                    visitor.visit(node, Tag<ASTNodeType::Int8>{});
                    break;
                case ASTNodeType::Uint8:
                    visitor.visit(node, Tag<ASTNodeType::Uint8>{});
                    break;
                case ASTNodeType::Int16:
                    visitor.visit(node, Tag<ASTNodeType::Int16>{});
                    break;
                case ASTNodeType::Uint16:
                    visitor.visit(node, Tag<ASTNodeType::Uint16>{});
                    break;
                case ASTNodeType::Int32:
                    visitor.visit(node, Tag<ASTNodeType::Int32>{});
                    break;
                case ASTNodeType::Uint32:
                    visitor.visit(node, Tag<ASTNodeType::Uint32>{});
                    break;
                case ASTNodeType::Int64:
                    visitor.visit(node, Tag<ASTNodeType::Int64>{});
                    break;
                case ASTNodeType::Uint64:
                    visitor.visit(node, Tag<ASTNodeType::Uint64>{});
                    break;
                case ASTNodeType::Float32:
                    visitor.visit(node, Tag<ASTNodeType::Float32>{});
                    break;
                case ASTNodeType::Float64:
                    visitor.visit(node, Tag<ASTNodeType::Float64>{});
                    break;
            }
        }
        return visitor;
    }

    // void initBuiltins(ASTApi* api) {
    //     _api = api;
    //
    //     static const std::string filename = "<builtin>";
    //
    //     const auto loc = idl::location(idl::position(&filename, 1, 1));
    //
    //     auto addBuiltin = [this, &loc]<typename Node>(std::string&& name, const std::string& detail, Node) {
    //         auto node    = allocNode<Node>(loc);
    //         node->name   = std::move(name);
    //         node->parent = _api;
    //         addSymbol(node);
    //         _api->childs.push_back(node);
    //     };
    //
    //     addBuiltin("Void", "void type.", ASTVoid{});
    //     addBuiltin("Char", "symbol type.", ASTChar{});
    //     addBuiltin("Bool", "boolean type.", ASTBool{});
    //     addBuiltin("Int8", "8 bit signed integer.", ASTInt8{});
    //     addBuiltin("Uint8", "8 bit unsigned integer.", ASTUint8{});
    //     addBuiltin("Int16", "16 bit signed integer.", ASTInt16{});
    //     addBuiltin("Uint16", "16 bit unsigned integer.", ASTUint16{});
    //     addBuiltin("Int32", "32 bit signed integer.", ASTInt32{});
    //     addBuiltin("Uint32", "32 bit unsigned integer.", ASTUint32{});
    //     addBuiltin("Int64", "64 bit signed integer.", ASTInt64{});
    //     addBuiltin("Uint64", "64 bit unsigned integer.", ASTUint64{});
    //     addBuiltin("Float32", "32 bit float point.", ASTFloat32{});
    //     addBuiltin("Float64", "64 bit float point.", ASTFloat64{});
    //     addBuiltin("Str", "utf8 string.", ASTStr{});
    //     addBuiltin("Data", "pointer to data.", ASTData{});
    // }

    template <idl_status_t Status, typename... Args>
    void log(const ASTLocation& loc, Args&&... args) {
        if (!_result) {
            return;
        }
        const auto message     = err<Status>(std::forward<Args>(args)...);
        const auto warnAsError = _options ? _options->getWarningsAsErrors() : false;
        _result->addMessage(Status, _stringPool[loc.filename].data(), loc.line, loc.column, message, warnAsError);
    }

    bool hasErrors() const noexcept {
        return _api != NodeHandleNone ? _result->hasErrors() : true;
    }

    String intern(std::string_view str) {
        return _stringPool.insert(str);
    }

    std::string_view getStr(String str) const noexcept {
        return _stringPool[str];
    }

private:
    Options* _options;
    CompilationResult* _result;
    StringPool _stringPool;
    std::vector<idl_message_t> _messages{};
    std::optional<idl_api_version_t> _version{};
    bool _useStdTypes{};
    BoolType _boolType{};
    ASTNodeHandle _api{ NodeHandleNone };
    std::vector<ASTNode> _nodes{};
    std::unordered_map<std::string, ASTNodeHandle> _symbols{};
    std::unordered_map<std::string, ASTNodeHandle> _docSymbols{};
    std::unordered_map<std::string, ASTNodeHandle> _literals{};
    uint32_t _lastOrder{};
};

} // namespace idl

#endif
