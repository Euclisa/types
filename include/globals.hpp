#ifndef LRF_GLOBALS_HPP
#define LRF_GLOBALS_HPP

#include <array>
#include <algorithm>
#include <cstdint>

namespace lrf
{
    namespace __globals
    {
        constexpr uint32_t karatsuba_bound = 1 << 9;

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
    }
}

#endif