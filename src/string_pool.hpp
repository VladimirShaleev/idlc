#ifndef IDL_STRING_POOL_HPP
#define IDL_STRING_POOL_HPP

#include "idl.hpp"

namespace idl {

struct String {
    uint32_t handle;
};

class StringPool final {
public:
    explicit StringPool(size_t slotCapacity = 1024, size_t bufferCapacity = 65536, size_t hashTableCapacity = 4096);
    ~StringPool();

    std::string_view operator[](String str) const noexcept;

    std::string_view get(String str) const noexcept;

    String insert(std::string_view str);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} // namespace idl

#endif
