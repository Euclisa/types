#ifndef LRF_GLOBALS_HPP
#define LRF_GLOBALS_HPP

#include <array>
#include <algorithm>
#include <cstdint>
#include <type_traits>

namespace lrf
{
    namespace __globals
    {
        constexpr uint32_t karatsuba_bound = 1 << 20;

        template<typename T>
        concept Iterator = std::input_iterator<T> and requires(T x) { { *x } -> std::same_as<bool>; };
        
        constexpr std::array<uint32_t,32> powers_2 = {
            0x1,
            0x2,
            0x4,
            0x8,
            0x10,
            0x20,
            0x40,
            0x80,
            0x100,
            0x200,
            0x400,
            0x800,
            0x1000,
            0x2000,
            0x4000,
            0x8000,
            0x10000,
            0x20000,
            0x40000,
            0x80000,
            0x100000,
            0x200000,
            0x400000,
            0x800000,
            0x1000000,
            0x2000000,
            0x4000000,
            0x8000000,
            0x10000000,
            0x20000000,
            0x40000000,
            0x80000000,
        };

        constexpr bool is_power_2(std::size_t x) { return std::find(powers_2.begin(),powers_2.end(),(uint32_t)x) != powers_2.end(); }

        constexpr uint32_t pow2(uint8_t x) { return powers_2[x]; }

        template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
        constexpr uint32_t max_addition_output_significant_bits()
        {
            constexpr uint32_t upper_bound = std::max(N,M);
            constexpr uint32_t significancy_bound = ((std::max(N_significant,M_significant)+1)/16+1)*16;
            return std::min(significancy_bound,upper_bound);
        }

        template<uint32_t N, uint32_t M>
        constexpr uint32_t max_addition_output_bits() { return std::max(N,M); }

        template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
        constexpr uint32_t max_subtraction_output_significant_bits() { return std::max(N,M); }

        template<uint32_t N, uint32_t M>
        constexpr uint32_t max_subtraction_output_bits() { return std::max(N,M); }

        template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
        constexpr uint32_t max_multiplication_output_significant_bits() { return std::min(std::max(N,M),N_significant+M_significant); }

        template<uint32_t N, uint32_t M>
        constexpr uint32_t max_multiplication_output_bits() { return std::max(N,M); }

        template<typename T>
        struct unqualified
        {
            typedef std::remove_const_t<std::remove_reference_t<T>> type;
        };

        template<typename T>
        using unqualified_t = unqualified<T>::type;
    }
}

#endif