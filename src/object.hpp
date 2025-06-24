#ifndef IDLC_OBJECT_HPP
#define IDLC_OBJECT_HPP

#include "idl.hpp"

namespace idl {

class Object {
public:
    virtual ~Object() = default;

    void reference() noexcept {
        ++_refCount;
    }

    void destroy() noexcept {
        if (--_refCount == 0) {
            delete this;
        }
    }

    template <typename T>
    T* as() noexcept {
        static_assert(std::is_base_of_v<Object, T>, "T must inheritance from Object");
        T* result;
        Object* ptr = this;
        memcpy(&result, &ptr, sizeof(T*));
        return result;
    }

    template <typename D, typename T, typename... Args>
    static idl_result_t create(T*& obj, Args&&... args) noexcept {
        static_assert(std::is_base_of_v<T, D>, "D must inheritance from T");
        try {
            obj = new D(args...);
            return IDL_RESULT_SUCCESS;
        } /* catch (const Exception& exc) {
            return exc.result();
        } */
        catch (const std::bad_alloc&) {
            return IDL_RESULT_ERROR_OUT_OF_MEMORY;
        } catch (...) {
            return IDL_RESULT_ERROR_UNKNOWN;
        }
    }

private:
    idl_sint32_t _refCount{ 1 };
};

} // namespace idl

#endif
