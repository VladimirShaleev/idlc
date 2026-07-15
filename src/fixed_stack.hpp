#ifndef IDLC_FIXED_STACK_HPP
#define IDLC_FIXED_STACK_HPP

#include "idl.hpp"

namespace idl {

template <typename T>
class FixedStackIterator final {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;
    using const_pointer     = const value_type*;
    using const_reference   = const value_type*;

    constexpr FixedStackIterator() noexcept = default;

    explicit constexpr FixedStackIterator(pointer at) noexcept : _current(at) {
    }

    template <typename U, typename = typename std::enable_if_t<std::is_convertible_v<U*, T*>>>
    constexpr FixedStackIterator(const FixedStackIterator<U>& it) noexcept : _current(it._current) {
    }

    [[nodiscard]] constexpr reference operator*() noexcept {
        return *_current;
    }

    [[nodiscard]] constexpr const_reference operator*() const noexcept {
        return *_current;
    }

    [[nodiscard]] constexpr pointer operator->() noexcept {
        return _current;
    }

    [[nodiscard]] constexpr const_pointer operator->() const noexcept {
        return _current;
    }

    [[nodiscard]] constexpr reference operator[](size_t n) noexcept {
        return _current[n];
    }

    [[nodiscard]] constexpr const_reference operator[](size_t n) const noexcept {
        return _current[n];
    }

    constexpr FixedStackIterator& operator++() noexcept {
        ++_current;
        return *this;
    }

    constexpr FixedStackIterator operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr FixedStackIterator& operator--() noexcept {
        --_current;
        return *this;
    }

    constexpr FixedStackIterator operator--(int) noexcept {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    constexpr FixedStackIterator& operator+=(difference_type n) noexcept {
        _current += n;
        return *this;
    }

    constexpr FixedStackIterator& operator-=(difference_type n) noexcept {
        _current -= n;
        return *this;
    }

    [[nodiscard]] constexpr FixedStackIterator operator+(difference_type n) const noexcept {
        auto tmp = *this;
        return tmp += n;
    }

    [[nodiscard]] friend constexpr FixedStackIterator operator+(difference_type n,
                                                                const FixedStackIterator& it) noexcept {
        return it + n;
    }

    [[nodiscard]] constexpr FixedStackIterator operator-(difference_type n) const noexcept {
        auto tmp = *this;
        return tmp -= n;
    }

    [[nodiscard]] constexpr difference_type operator-(const FixedStackIterator& it) const noexcept {
        return _current - it._current;
    }

    [[nodiscard]] constexpr bool operator==(const FixedStackIterator& other) const noexcept {
        return _current == other._current;
    }

    [[nodiscard]] constexpr bool operator!=(const FixedStackIterator& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] constexpr std::strong_ordering operator<=>(const FixedStackIterator& it) const noexcept {
        return _current <=> it._current;
    }

private:
    pointer _current;
};

template <typename T, size_t N>
class FixedStack final {
public:
    using value_type             = T;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using iterator               = FixedStackIterator<value_type>;
    using const_iterator         = FixedStackIterator<const value_type>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    [[nodiscard]] constexpr reference top() noexcept {
        assert(_tail > 0);
        return _data[_tail - 1];
    }

    [[nodiscard]] constexpr const_reference top() const noexcept {
        assert(_tail > 0);
        return _data[_tail - 1];
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return _tail;
    }

    [[nodiscard]] constexpr size_type capacity() const noexcept {
        return N;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return N;
    }

    template <typename... Args>
    constexpr reference emplace(Args&&... args) {
        if (size() == capacity()) {
            throw std::bad_alloc();
        }
        return _data[_tail++] = value_type{ std::forward<Args>(args)... };
    }

    constexpr reference push(const_reference value) {
        if (size() == capacity()) {
            throw std::bad_alloc();
        }
        return _data[_tail++] = value;
    }

    constexpr reference push(value_type&& value) {
        if (size() == capacity()) {
            throw std::bad_alloc();
        }
        return _data[_tail++] = std::move(value);
    }

    constexpr void pop() noexcept {
        assert(size() > 0);
        --_tail;
    }

    constexpr void clear() noexcept {
        _tail = 0;
    }

    [[nodiscard]] constexpr iterator begin() noexcept {
        return FixedStackIterator(empty() ? nullptr : &_data[0]);
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept {
        return const_iterator(empty() ? nullptr : &_data[0]);
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return begin();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    [[nodiscard]] constexpr iterator end() noexcept {
        return iterator(empty() ? nullptr : &_data[_tail]);
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return const_iterator(empty() ? nullptr : &_data[_tail]);
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return end();
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

private:
    value_type _data[N + 1];
    size_t _tail{};
};

} // namespace idl

#endif
