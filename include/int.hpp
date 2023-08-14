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

    // ===BEGIN TYPEDEFS===

    typedef _uint<128> uint128_t;
    typedef _uint<256> uint256_t;
    typedef _uint<512> uint512_t;
    typedef _uint<1024> uint1024_t;
    typedef _uint<2048> uint2048_t;
    typedef _uint<4096> uint4096_t;

    // ===END TYPEDEFS===

    template<uint32_t N>
    _uint<N> operator+(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
    _uint<N> operator+(const _uint<N>& a, uint32_t b);

    template<uint32_t N>
    _uint<N> operator+(uint32_t b, const _uint<N>& a);

    template<uint32_t N>
    _uint<N> operator-(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
    _uint<N> operator-(const _uint<N>& a, uint32_t b);

    template<uint32_t N>
    _uint<N> operator-(uint32_t b, const _uint<N>& a);

    template<uint32_t N>
        requires(N < __globals::powers_2[7])
    _uint<N*2> operator*(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
        requires(N >= __globals::powers_2[7])
    _uint<N*2> operator*(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
    _uint<N*2> operator*(const _uint<N>& a, uint32_t b);

    template<uint32_t N>
    _uint<N*2> operator*(uint32_t b, const _uint<N>& a);

    template<uint32_t N>
    _uint<N> operator+=(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
    _uint<N> operator+=(const _uint<N>& a, uint32_t b);

    template<uint32_t N>
    _uint<N> operator-=(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
    _uint<N> operator-=(const _uint<N>& a, uint32_t b);

    template<uint32_t N>
    _uint<N> operator*=(const _uint<N>& a, const _uint<N>& b);

    template<uint32_t N>
    _uint<N> operator*=(const _uint<N>& a, uint32_t b);

    template<uint32_t N>
    _uint<N> operator<<(const _uint<N>& x, short shift);

    template<uint32_t N>
    _uint<N> operator>>(const _uint<N>& x, short shift);

    template<uint32_t N>
    std::ostream& operator<<(std::ostream& out, const _uint<N>& x);

    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    class _uint
    {
    public:
        typedef uint16_t word_type;
        static constexpr uint8_t word_bits = sizeof(word_type) * 8;
        static constexpr uint32_t words_num = N / word_bits;
        static constexpr uint32_t base = 1 << word_bits;

        uint16_t *value;

        _uint<N/2> get_lower_half();
        _uint<N/2> get_upper_half();

        template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
        uint16_t *get_part_view();

        template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
        const uint16_t *get_part_view() const;
        _uint();
        _uint(uint64_t value);
        _uint(uint16_t *ptr);
        _uint(const uint16_t *ptr);
        _uint(std::string_view hex_str);

        _uint(const _uint<N>&);
        _uint(_uint<N>&&);

        template<__globals::Iterator _Iterator>
        _uint(_Iterator begin, _Iterator end);

        template<uint32_t M>
        operator _uint<M>() const;
        void operator=(const _uint<N>& x);
    };


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    class _uint_c : public _uint<N>
    {
    public:
        ~_uint_c();
    };


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint() : value(new _uint<N>::word_type[words_num]) {}


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(uint64_t value) : _uint<N>()
    {
        for(uint16_t shift(0); shift < std::min(N,(uint32_t)64); shift += 16)
            this->value[shift >> 4] = (value >> shift) & uint16_t(-1);
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(std::string_view hex_str) : _uint<N>()
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
    _uint<N>::_uint(_Iterator begin, _Iterator end) : _uint<N>()
    {
        _Iterator curr = begin;
        std::size_t bit_i = 0;
        while(curr != end and bit_i < N)
        {
            this->value[bit_i/_uint<N>::word_bits] += *(curr++) ? this->pow2(bit_i%_uint<N>::word_bits) : 0;
            ++bit_i;
        }
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(const _uint<N>& x) : _uint<N>()
    {
        std::copy(x.value,x.value+N,this->value);
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(_uint<N>&& x) : _uint<N>()
    {
        this->value = x.value;
        x.value = nullptr;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(uint16_t *ptr) : value(ptr) {}


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(const uint16_t *ptr) : value(ptr) {}


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_c<N>::~_uint_c()
    {
        delete[] this->value;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N/2> _uint<N>::get_lower_half()
    {
        _uint<N/2> res;
        for(uint32_t i(0); i < N/2; ++i)
            res.value[i] = this->value[i];
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N/2> _uint<N>::get_upper_half()
    {
        _uint<N/2> res;
        for(uint32_t i(0); i < N/2; ++i)
            res.value[i+N/2] = this->value[i];
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
    _uint<N>::operator _uint<M>() const
    {
        _uint<M> res;
        for(uint32_t i(0); i < _uint<M>::words_num; ++i)
        {
            if(i < N)
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
        std::copy(x.value,x.value+N,this->value);
    }


    template<uint32_t N>
    _uint<N> operator+(const _uint<N>& a, const _uint<N>& b)
    {
        _uint<N> res;
        short r = 0;
        for(std::size_t i = 0; i < _uint<N>::words_num; ++i)
        {
            uint64_t word_sum = a.value[i] + b.value[i] + r;
            res.value[i] = word_sum % _uint<N>::base;
            r = word_sum / _uint<N>::base;
        }
        return res;
    }


    template<uint32_t N>
    _uint<N> operator+(const _uint<N>& a, uint32_t b)
    {
        _uint<N> res;
        uint64_t word_sum = a.value[0] + b;
        res.value[0] = word_sum % _uint<N>::base;
        short r = word_sum / _uint<N>::base;
        for(std::size_t i = 1; i < _uint<N>::words_num; ++i)
        {
            word_sum = a.value[i] + r;
            res.value[i] = word_sum % _uint<N>::base;
            r = word_sum / _uint<N>::base;
        }
        return res;
    }


    template<uint32_t N>
    _uint<N> operator+(uint32_t a, const _uint<N>& b)
    {
        return b + a;
    }


    template<uint32_t N>
    _uint<N> operator-(const _uint<N>& a, const _uint<N>& b)
    {
        _uint<N> res;
        short r = 0;
        for(std::size_t i(0); i < _uint<N>::words_num; ++i)
        {
            uint64_t sub_total = b.value[i] + r;
            r = sub_total > a.value[i];
            res.value[i] = ((r ? _uint<N>::base : 0) - sub_total) + a.value[i];
        }
        return res;
    }


    template<uint32_t N>
    _uint<N> operator-(const _uint<N>& a, uint32_t b)
    {
        _uint<N> res;
        short r = b > a.value[0];
        res.value[0] = ((r ? _uint<N>::base : 0) - b) + a.value[0];
        for(std::size_t i(1); i < _uint<N>::words_num; ++i)
        {
            r = r > a.value[i];
            res.value[i] = ((r ? _uint<N>::base : 0) - r) + a.value[i];
        }
        return res;
    }


    template<uint32_t N>
    _uint<N> operator-(uint32_t a, const _uint<N>& b)
    {
        _uint<N> res;
        short r = b.value[0] > a;
        res.value[0] = ((r ? _uint<N>::base : 0) - b.value[0]) + a;
        for(std::size_t i(1); i < _uint<N>::words_num; ++i)
        {
            uint64_t sub_total = b.value[i] + r;
            r = sub_total > 0;
            res.value[i] = ((r ? _uint<N>::base : 0) - sub_total);
        }
        return res;
    }


    template<uint32_t N>
        requires(N < __globals::powers_2[7])
    _uint<N*2> operator*(const _uint<N>& a, const _uint<N>& b)
    {
        _uint<N*2> res;
        uint64_t pseudo_res[_uint<N*2>::words_num];
        for(uint32_t i(0); i < _uint<2*N>::words_num-1; ++i)
        {
            pseudo_res[i] = 0;
            uint32_t lower_bound = i > _uint<N>::words_num-1 ? i-_uint<N>::words_num+1 : 0;
            uint32_t upper_bound = std::min(i,_uint<N>::words_num-1);
            for(uint32_t j(lower_bound); j <= upper_bound; ++j)
                pseudo_res[i] += a.value[j]*b.value[upper_bound-j];
        }
        res.value[0] = pseudo_res[0] & uint16_t(0xffff);
        for(uint32_t i(1); i < _uint<N*2>::words_num; ++i)
            pseudo_res[i] += (pseudo_res[i-1] >> 16);
        for(uint32_t i(1); i < _uint<N*2>::words_num; ++i)
            res.value[i] = pseudo_res[i] & uint16_t(0xffff);
        return res;
    }


    template<uint32_t N>
        requires(N >= __globals::powers_2[7])
    _uint<N*2> operator*(const _uint<N>& a, const _uint<N>& b)
    {
        _uint<N*2> res;
        if(_uint<N>::words_num == 1)
        {
            uint32_t prod = a.value[0] * b.value[0];
            res.value[0] = prod & uint16_t(0xffff);
            res.value[1] = prod >> 16;
        }
        else
        {
            const _uint<N/2> a_lower_half(const_cast<_uint<N>::word_type*>(a.template get_part_view<2,0>()));
            const _uint<N/2> a_upper_half(const_cast<_uint<N>::word_type*>(a.template get_part_view<2,1>()));
            const _uint<N/2> b_lower_half(const_cast<_uint<N>::word_type*>(b.template get_part_view<2,0>()));
            const _uint<N/2> b_upper_half(const_cast<_uint<N>::word_type*>(b.template get_part_view<2,1>()));
            _uint<N> z_0 = a_lower_half * b_lower_half;
            _uint<N> z_2 = a_upper_half * b_upper_half;
            _uint<N> z_1 = (a_lower_half + b_lower_half) * (a_upper_half + b_upper_half) - z_0 - z_1;
            _uint<N/2> quoter_0(res.template get_part_view<4,0>());
            _uint<N/2> quoter_1(res.template get_part_view<4,1>());
            _uint<N/2> quoter_2(res.template get_part_view<4,2>());
            _uint<N/2> quoter_3(res.template get_part_view<4,3>());
            quoter_0 = z_0.get_lower_half();
            quoter_1 = _uint<N/2>(z_0.template get_part_view<2,1>()) + _uint<N/2>(z_1.template get_part_view<2,0>());
            quoter_2 = _uint<N/2>(z_1.template get_part_view<2,1>()) + _uint<N/2>(z_2.template get_part_view<2,0>());
            quoter_3 = z_2.get_upper_half();
        }
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