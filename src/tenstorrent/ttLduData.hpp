#pragma once

#include <Field.H>
#include <scalarField.H>
#include <FieldField.H>
#include <lduInterfaceFieldPtrsList.H>
#include <tt-metalium/buffer.hpp>
#include <memory>

struct tt_ldu_meta {
    bool contents_on_device_ = false;
    bool addrs_on_device_ = false;

    // matrix contents. They may change at some point. But also will not be changed on-device
    std::shared_ptr<tt::tt_metal::Buffer> d_data_ = nullptr;

    uint32_t cell_count = 0;
    uint32_t lower_contents_start_ = 0;
    uint32_t sparse_count = 0;
    uint32_t upper_contents_start_ = 0;

    // addrs for the sparse data and interfaces. If those change after setup, I am giving up
    std::shared_ptr<tt::tt_metal::Buffer> d_addrs_ = nullptr;

    uint32_t upper_addrs_start_ = 0;
    uint32_t iface_map_start_ = 0;
};

extern std::unordered_map<const void*, tt_ldu_meta> ldu_tt_meta_map;

template<typename result> result& get_tt_meta(const void* key, std::unordered_map<const void*, result>& map) {
    auto it = map.find(key);
    if (it == map.end()) {
        map[key] = result();
        return map[key];
    } else {
        return it->second;
    }
}

template<typename result> void clear_tt_meta(const void* key, std::unordered_map<const void*, result>& map) {
    auto it = map.find(key);
    if (it != map.end()) {
        map.erase(it);
    }
}

/**
 * @brief Computes the ceiling of a / b.
 *
 * Returns the smallest integer greater than or equal to a / b.
 *
 * @param a The numerator.
 * @param b The denominator. Must be non-zero.
 * @return The result of ceiling division (a + b - 1) / b.
 *
 * @note If b is zero, this results in undefined behavior.
 */
template <typename A, typename B>
auto div_up(A a, B b) noexcept -> std::common_type_t<A, B> {
    using T = std::common_type_t<A, B>;
    assert(b != 0 && "Divide by zero error in div_up");
    return static_cast<T>((static_cast<T>(a) + static_cast<T>(b) - 1) / static_cast<T>(b));
}

/**
 * @brief Rounds up a to the nearest multiple of b.
 *
 * Computes the smallest multiple of b that is greater than or equal to a.
 *
 * @param a The number to round.
 * @param b The multiple to round up to. Must be non-zero.
 * @return The rounded-up value.
 *
 * @note Internally uses div_up. If b is zero, this results in undefined behavior.
 */
template <typename A, typename B>
auto round_up(A a, B b) {
    using T = std::common_type_t<A, B>;
    return static_cast<T>(b) * div_up(static_cast<T>(a), static_cast<T>(b));
}