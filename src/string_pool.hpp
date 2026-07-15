#ifndef IDL_STRING_POOL_HPP
#define IDL_STRING_POOL_HPP

#include "idl.hpp"

namespace idl {

struct String {
    uint32_t handle;
};

[[nodiscard]] inline bool operator==(const String& lhs, const String& rhs) noexcept {
    return lhs.handle == rhs.handle;
}

[[nodiscard]] inline bool operator!=(const String& lhs, const String& rhs) noexcept {
    return lhs.handle != rhs.handle;
}

[[nodiscard]] inline std::strong_ordering operator<=>(const String& lhs, const String& rhs) noexcept {
    return lhs.handle <=> rhs.handle;
}

class StringPool final {
public:
    explicit StringPool(size_t slotCapacity = 1024, size_t bufferCapacity = 65536, size_t hashTableCapacity = 4096);
    ~StringPool();

    [[nodiscard]] std::string_view operator[](String str) const noexcept;

    [[nodiscard]] std::string_view get(String str) const noexcept;

    String insert(std::string_view str);

    [[nodiscard]] std::optional<String> find(std::string_view str) const noexcept;

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} // namespace idl

namespace std {

template <>
struct hash<idl::String> {
    [[nodiscard]] size_t operator()(const idl::String str) const noexcept {
        hash<uint32_t> handleHash;
        return handleHash(str.handle);
    }
};

} // namespace std

#endif
