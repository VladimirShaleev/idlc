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

void Visitor::visit(ASTAttrTokenizer* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrOrder* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrSingle* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrVersion* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrAuthor* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrCopyright* node) {
    discarded(node);
}

void Visitor::visit(ASTAttrLicense* node) {
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

void Visitor::visit(ASTAttrCName* node) {
    discarded(node);
}

void Visitor::visit(ASTDeclRef* node) {
    discarded(node);
}

void Visitor::visit(ASTImport* node) {
    discarded(node);
}

void Visitor::visit(ASTVoid* node) {
    discarded(node);
}

void Visitor::visit(ASTChar* node) {
    discarded(node);
}

void Visitor::visit(ASTStr* node) {
    discarded(node);
}

void Visitor::visit(ASTBool* node) {
    discarded(node);
}

void Visitor::visit(ASTInt8* node) {
    discarded(node);
}

void Visitor::visit(ASTUint8* node) {
    discarded(node);
}

void Visitor::visit(ASTInt16* node) {
    discarded(node);
}

void Visitor::visit(ASTUint16* node) {
    discarded(node);
}

void Visitor::visit(ASTInt32* node) {
    discarded(node);
}

void Visitor::visit(ASTUint32* node) {
    discarded(node);
}

void Visitor::visit(ASTInt64* node) {
    discarded(node);
}

void Visitor::visit(ASTUint64* node) {
    discarded(node);
}

void Visitor::visit(ASTFloat32* node) {
    discarded(node);
}

void Visitor::visit(ASTFloat64* node) {
    discarded(node);
}

void Visitor::visit(ASTData* node) {
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

void ASTAttrTokenizer::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrOrder::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrSingle::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrVersion::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrAuthor::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrCopyright::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTAttrLicense::accept(Visitor& visitor) {
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

void ASTAttrCName::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTDeclRef::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTImport::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTVoid::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTChar::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTStr::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTBool::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTInt8::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTUint8::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTInt16::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTUint16::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTInt32::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTUint32::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTInt64::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTUint64::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTFloat32::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTFloat64::accept(Visitor& visitor) {
    visitor.visit(this);
}

void ASTData::accept(Visitor& visitor) {
    visitor.visit(this);
}

} // namespace idl
