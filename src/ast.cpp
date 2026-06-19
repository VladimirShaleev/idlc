#include "ast.hpp"

namespace idl {

std::string ASTDecl::fullname() const {
    assert(name.length() > 0);
    std::ostringstream ss;
    if (parent) {
        if (auto parentDecl = parent->as<ASTDecl>()) {
            ss << parentDecl->fullname() << '.';
        }
    }
    ss << name;
    return ss.str();
}

std::string ASTDecl::fullnameLowecase() const {
    auto str = fullname();
    std::transform(str.begin(), str.end(), str.begin(), [](auto c) {
        return std::tolower(c);
    });
    return str;
}

Visitor::Visitor(class Context& ctx) noexcept : ctx(ctx) {
}

Visitor::~Visitor() = default;

void Visitor::visit(ASTLiteralStr* node) {
    discarded(node);
}

void Visitor::visit(ASTLiteralBool* node) {
    discarded(node);
}

void Visitor::visit(ASTLiteralInt* node) {
    discarded(node);
}

void Visitor::visit(ASTLiteralFloat* node) {
    discarded(node);
}

void Visitor::visit(ASTApi* node) {
    discarded(node);
}

void Visitor::visit(ASTEnum* node) {
    discarded(node);
}

void Visitor::visit(ASTConst* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrVersion* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrFlags* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrHex* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrBrief* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrDetail* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrValue* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrType* node) {
    discarded(node);
}

void Visitor::visit(ASTDeclRef* node) {
    discarded(node);
}

void Visitor::visit(ASTImport* node) {
    discarded(node);
}

void Visitor::discarded(ASTNode* node) {
    assert(!"visit method not implemented for this node type");
}

void ASTLiteralStr::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTLiteralBool::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTLiteralInt::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTLiteralFloat::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTApi::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTEnum::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTConst::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrVersion::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrFlags::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrHex::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrBrief::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrDetail::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrValue::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrType::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTDeclRef::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTImport::accept(Visitor& visitor) {
    visitor.visit(this);
}

} // namespace idl
