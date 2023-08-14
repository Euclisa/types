#ifndef INT_HPP
#define INT_HPP

#include <string>
#include <cstdint>
#include <concepts>
#include <array>
#include <vector>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include "globals.hpp"

namespace lrf
{
    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    class _uint;

    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    class _uint_c;

    // ===BEGIN TYPEDEFS===

    typedef _uint<128> uint128_t;
    typedef _uint<256> uint256_t;
    typedef _uint<512> uint512_t;
    typedef _uint<1024> uint1024_t;
    typedef _uint<2048> uint2048_t;
    typedef _uint<4096> uint4096_t;

    // ===END TYPEDEFS===

    template<uint32_t N>
    _uint_c<N> operator+(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
    _uint_c<N> operator-(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
        requires(N < __globals::powers_2[7])
    _uint_c<N*2> operator*(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
        requires(N >= __globals::powers_2[7])
    _uint_c<N*2> operator*(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
    _uint_c<N*2> operator*(const _uint<N>& a, uint32_t b);

    template<uint32_t N>
    _uint_c<N*2> operator*(uint32_t b, const _uint<N>& a);

    template<uint32_t N>
    std::ostream& operator<<(std::ostream& out, const _uint<N>& x);

    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    class _uint
    {
    protected:
        _uint() = default;
    public:
        typedef uint16_t word_type;
        static constexpr uint8_t word_bits = sizeof(word_type) * 8;
        static constexpr uint32_t words_num = N / word_bits;
        static constexpr uint32_t base = 1 << word_bits;

        uint16_t *value;

        _uint_c<N/2> get_lower_half();
        _uint_c<N/2> get_upper_half();
        template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
        uint16_t *get_part_view();
        template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
        const uint16_t *get_part_view() const;

        _uint(uint16_t *ptr);
        _uint(const _uint<N>&);
        _uint(_uint<N>&&);

        void operator=(const _uint<N>& x);
    };


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    class _uint_c : public _uint<N>
    {
    public:
        _uint_c();
        _uint_c(uint64_t value);
        _uint_c(std::string_view hex_str);
        template<__globals::Iterator _Iterator>
        _uint_c(_Iterator begin, _Iterator end);
        _uint_c(const _uint<N>&);
        _uint_c(_uint_c<N>&&);

        template<uint32_t M>
        operator _uint_c<M>() const;

        ~_uint_c();
    };


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N>::_uint_c()
    {
        this->value = new _uint<N>::word_type[_uint<N>::words_num];
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N>::_uint_c(uint64_t value) : _uint<N>()
    {
        for(uint16_t shift(0); shift < std::min(N,(uint32_t)64); shift += 16)
            this->value[shift >> 4] = (value >> shift) & uint16_t(-1);
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N>::_uint_c(std::string_view hex_str) : _uint_c<N>()
    {
        std::string prefixed_hex_str(N/4-hex_str.length(),'0');
        prefixed_hex_str += hex_str;
        for(int i(_uint<N>::words_num-1); i >= 0; --i)
        {
            std::stringstream stream;
            stream << prefixed_hex_str.substr((_uint<N>::words_num-i-1)*_uint<N>::word_bits/4,_uint<N>::word_bits/4);
            stream >> std::hex >> this->value[i];
        }
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<__globals::Iterator _Iterator>
    _uint_c<N>::_uint_c(_Iterator begin, _Iterator end) : _uint<N>()
    {
        _Iterator curr = begin;
        std::size_t bit_i = 0;
        while(curr != end and bit_i < _uint<N>::words_num)
        {
            this->value[bit_i/_uint<N>::word_bits] += *(curr++) ? this->pow2(bit_i%_uint<N>::word_bits) : 0;
            ++bit_i;
        }
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N>::_uint_c(const _uint<N>& x) : _uint_c<N>()
    {
        std::copy(x.value,x.value+_uint<N>::words_num,this->value);
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N>::_uint_c(_uint_c<N>&& x)
    {
        this->value = x.value;
        x.value = nullptr;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(const _uint<N>& x)
    {
        this->value = x.value;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(_uint<N>&& x)
    {
        this->value = x.value;
        x.value = nullptr;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(uint16_t *ptr) : value(ptr) {}


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N>::~_uint_c()
    {
        if(this->value)
            delete[] this->value;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N/2> _uint<N>::get_lower_half()
    {
        _uint_c<N/2> res;
        for(uint32_t i(0); i < _uint<N/2>::words_num; ++i)
            res.value[i] = this->value[i];
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N/2> _uint<N>::get_upper_half()
    {
        _uint_c<N/2> res;
        for(uint32_t i(0); i < _uint<N/2>::words_num; ++i)
            res.value[i] = this->value[i+_uint_c<N/2>::words_num];
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
    uint16_t *_uint<N>::get_part_view()
    {
        static_assert(DENOMINATOR <= _uint<N>::words_num and NUMENATOR < DENOMINATOR and __globals::is_power_2(DENOMINATOR));
        constexpr uint32_t offset = (_uint<N>::words_num / DENOMINATOR) * NUMENATOR;
        return this->value + offset;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
    const uint16_t *_uint<N>::get_part_view() const
    {
        static_assert(DENOMINATOR <= _uint<N>::words_num and NUMENATOR < DENOMINATOR and __globals::is_power_2(DENOMINATOR));
        constexpr uint32_t offset = (_uint<N>::words_num / DENOMINATOR) * NUMENATOR;
        return this->value + offset;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<uint32_t M>
    _uint_c<N>::operator _uint_c<M>() const
    {
        _uint_c<M> res;
        for(uint32_t i(0); i < _uint<M>::words_num; ++i)
        {
            if(i < _uint<N>::words_num)
                res.value[i] = this->value[i];
            else
                res.value[i] = 0;
        }
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    void _uint<N>::operator=(const _uint<N>& x)
    {
        std::copy(x.value,x.value+_uint<N>::words_num,this->value);
    }


    template<uint32_t N>
    _uint_c<N> operator+(const _uint<N>& a, const _uint<N>& b)
    {
        _uint_c<N> res;
        short r = 0;
        for(uint32_t i = 0; i < _uint<N>::words_num; ++i)
        {
            uint64_t word_sum = (uint32_t)a.value[i] + (uint32_t)b.value[i] + (uint32_t)r;
            res.value[i] = word_sum % _uint<N>::base;
            r = word_sum / _uint<N>::base;
        }
        return res;
    }


    template<uint32_t N>
    _uint_c<N> operator-(const _uint<N>& a, const _uint<N>& b)
    {
        _uint_c<N> res;
        short r = 0;
        for(uint32_t i(0); i < _uint<N>::words_num; ++i)
        {
            uint64_t sub_total = (uint32_t)b.value[i] + (uint32_t)r;
            r = sub_total > a.value[i];
            res.value[i] = ((uint32_t)(r ? _uint<N>::base : 0) - (uint32_t)sub_total) + (uint32_t)a.value[i];
        }
        return res;
    }


    template<uint32_t N>
        requires(N < __globals::powers_2[7])
    _uint_c<N*2> operator*(const _uint<N>& a, const _uint<N>& b)
    {
        _uint_c<N*2> res;
        std::cout << "a: " << a << "; b: " << b << '\n';
        uint64_t pseudo_res[_uint<N*2>::words_num];
        pseudo_res[_uint<2*N>::words_num-1] = 0;
        for(uint32_t i(0); i < _uint<2*N>::words_num-1; ++i)
        {
            pseudo_res[i] = 0;
            uint32_t lower_bound = i >= _uint<N>::words_num ? i-_uint<N>::words_num+1 : 0;
            uint32_t upper_bound = std::min(i,_uint<N>::words_num-1);
            for(uint32_t j(lower_bound); j <= upper_bound; ++j)
            {
                uint32_t a_v = a.value[j];
                uint32_t b_v = b.value[upper_bound-j+lower_bound];
                uint32_t val = a_v*b_v;
                pseudo_res[i] += val;
            }
            std::cout << std::hex << pseudo_res[i] << '\n';
        }
        for(uint32_t i(1); i < _uint<N*2>::words_num; ++i)
            pseudo_res[i] += (pseudo_res[i-1] >> 16);
        for(uint32_t i(0); i < _uint<N*2>::words_num; ++i)
            res.value[i] = pseudo_res[i] & uint16_t(0xffff);
        std::cout << "res: " << res << '\n';
        return res;
    }


    template<uint32_t N>
        requires(N >= __globals::powers_2[7])
    _uint_c<N*2> operator*(const _uint<N>& a, const _uint<N>& b)
    {
        _uint_c<N*2> res;
        std::cout << "a: " << a << "; b: " << b << '\n';
        if(_uint<N>::words_num == 1)
        {
            uint32_t prod = (uint32_t)a.value[0] * (uint32_t)b.value[0];
            res.value[0] = prod & uint16_t(0xffff);
            res.value[1] = prod >> 16;
        }
        else
        {
            const _uint<N/2> a_lower_half(const_cast<_uint<N>::word_type*>(a.template get_part_view<2,0>()));
            const _uint<N/2> a_upper_half(const_cast<_uint<N>::word_type*>(a.template get_part_view<2,1>()));
            const _uint<N/2> b_lower_half(const_cast<_uint<N>::word_type*>(b.template get_part_view<2,0>()));
            const _uint<N/2> b_upper_half(const_cast<_uint<N>::word_type*>(b.template get_part_view<2,1>()));
            _uint_c<N> z_0 = a_lower_half * b_lower_half;
            _uint_c<N> z_2 = a_upper_half * b_upper_half;
            _uint_c<N> z_1 = (a_lower_half + b_lower_half) * (a_upper_half + b_upper_half) - z_0 - z_1;
            _uint<N/2> quoter_0(res.template get_part_view<4,0>());
            _uint<N/2> quoter_1(res.template get_part_view<4,1>());
            _uint<N/2> quoter_2(res.template get_part_view<4,2>());
            _uint<N/2> quoter_3(res.template get_part_view<4,3>());
            quoter_0 = z_0.get_lower_half();
            quoter_1 = _uint<N/2>(z_0.template get_part_view<2,1>()) + _uint<N/2>(z_1.template get_part_view<2,0>());
            quoter_2 = _uint<N/2>(z_1.template get_part_view<2,1>()) + _uint<N/2>(z_2.template get_part_view<2,0>());
            quoter_3 = z_2.get_upper_half();
            std::cout << "q0: " << quoter_0 << "; q1: " << quoter_1 << "; q2: " << quoter_2 << "; q3: " << quoter_3 << '\n';
        }
        std::cout << "res: " << res << '\n';
        std::cout << "==\n";
        return res;
    }


    template<uint32_t N>
    std::ostream& operator<<(std::ostream& out, const _uint<N>& x)
    {
        for(int i(_uint<N>::words_num-1); i >= 0; --i)
        {
            std::stringstream stream;
            stream.fill('0');
            stream.width(_uint<N>::word_bits/4);
            stream <<  std::hex << x.value[i];
            out << stream.str();
        }
        return out;
    }
}

#endif