#pragma once

#include <limits>  // For std::numeric_limits
#include "myalloc.h"

namespace myAlloc {

template <typename T>
class Allocator {
 public:
    // Define the necessary types for an allocator
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Constructor
    Allocator() noexcept = default;

    // Convert an allocator of type U to an allocator of type T
    template <typename U>
    Allocator(const Allocator<U>&) noexcept {}

    // Allocate memory for n objects of type T
    T* allocate(std::size_t n) {
        if (n > std::numeric_limits<size_type>::max() / sizeof(T))
            throw std::bad_alloc();
        return static_cast<T*>(myalloc(n * sizeof(T)));
    }

    // Deallocate memory
    void deallocate(T* p, std::size_t) noexcept {
        myfree(p);
    }
};

// Comparing two Allocator objects for equality
template <typename T1, typename T2>
bool operator==(const Allocator<T1>&, const Allocator<T2>&) noexcept {
    return true;
}

template <typename T1, typename T2>
bool operator!=(const Allocator<T1>&, const Allocator<T2>&) noexcept {
    return false;
}

} // namespace