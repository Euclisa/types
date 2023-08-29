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
    class _uint_view;

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
        requires(N >= 16 and __globals::is_power_2(N))
    class _uint_view
    {
    protected:
        _uint_view() = default;
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

        _uint_view(uint16_t *ptr);
        _uint_view(const _uint_view<N>&);
        _uint_view(_uint_view<N>&&);

        operator std::string() const;
        void operator=(const _uint_view<N>& x);
        _uint_view<N>& operator+=(const _uint_view<N>& x);
        _uint_view<N>& operator-=(const _uint_view<N>& x);
        _uint_view<N>& operator*=(const _uint_view<N>& b) requires(N < __globals::karatsuba_bound);
        _uint_view<N>& operator*=(const _uint_view<N>& b) requires(N >= __globals::karatsuba_bound);
        template<uint32_t M>
        bool operator==(const _uint_view<M>& x) const;
        _uint<N> operator+(const _uint_view<N>& other) const;
        _uint<N> operator-(const _uint_view<N>& other) const;
        _uint<N> operator*(const _uint_view<N>& other) const requires (N < __globals::karatsuba_bound);
        _uint<N> operator*(const _uint_view<N>& other) const requires (N >= __globals::karatsuba_bound);
    };


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    class _uint : public _uint_view<N>
    {
    public:
        _uint();
        _uint(uint64_t value);
        _uint(std::string_view hex_str);
        template<__globals::Iterator _Iterator>
        _uint(_Iterator begin, _Iterator end);
        _uint(const _uint_view<N>&);
        _uint(_uint<N>&&);

        template<uint32_t M>
        operator _uint<M>() const;
        void operator=(const _uint<N>& x);
        void operator=(const _uint_view<N>& x);

        ~_uint();
    };


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint()
    {
        this->value = new _uint_view<N>::word_type[_uint_view<N>::words_num];
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(uint64_t value) : _uint<N>()
    {
        for(uint32_t shift(0); shift < N; shift += _uint_view<N>::word_bits)
        {
            uint16_t l = shift < 64 ? (value >> shift) & uint64_t(0xffff) : 0;
            this->value[shift / _uint_view<N>::word_bits] = l;
        }
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(std::string_view hex_str) : _uint<N>()
    {
        std::string prefixed_hex_str(N/4-hex_str.length(),'0');
        prefixed_hex_str += hex_str;
        for(int i(_uint_view<N>::words_num-1); i >= 0; --i)
        {
            std::stringstream stream;
            stream << prefixed_hex_str.substr((_uint_view<N>::words_num-i-1)*_uint_view<N>::word_bits/4,_uint_view<N>::word_bits/4);
            stream >> std::hex >> this->value[i];
        }
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<__globals::Iterator _Iterator>
    _uint<N>::_uint(_Iterator begin, _Iterator end) : _uint_view<N>()
    {
        _Iterator curr = begin;
        std::size_t bit_i = 0;
        while(curr != end and bit_i < _uint_view<N>::words_num)
        {
            this->value[bit_i/_uint_view<N>::word_bits] += *(curr++) ? this->pow2(bit_i%_uint_view<N>::word_bits) : 0;
            ++bit_i;
        }
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::_uint(const _uint_view<N>& x) : _uint<N>()
    {
        std::copy(x.value,x.value+_uint_view<N>::words_num,this->value);
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
    _uint_view<N>::_uint_view(const _uint_view<N>& x)
    {
        this->value = x.value;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_view<N>::_uint_view(_uint_view<N>&& x)
    {
        this->value = x.value;
        x.value = nullptr;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_view<N>::_uint_view(uint16_t *ptr) : value(ptr) {}


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N>::~_uint()
    {
        if(this->value)
            delete[] this->value;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N/2> _uint_view<N>::get_lower_half()
    {
        _uint<N/2> res;
        for(uint32_t i(0); i < _uint_view<N/2>::words_num; ++i)
            res.value[i] = this->value[i];
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N/2> _uint_view<N>::get_upper_half()
    {
        _uint<N/2> res;
        for(uint32_t i(0); i < _uint_view<N/2>::words_num; ++i)
            res.value[i] = this->value[i+_uint<N/2>::words_num];
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
    uint16_t *_uint_view<N>::get_part_view()
    {
        constexpr uint32_t part_size = _uint_view<N>::words_num / DENOMINATOR;
        static_assert(part_size <= _uint_view<N>::words_num and NUMENATOR <= (_uint_view<N>::words_num-part_size) and __globals::is_power_2(DENOMINATOR));
        constexpr uint32_t offset = (_uint_view<N>::words_num / DENOMINATOR) * NUMENATOR;
        return this->value + NUMENATOR;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
    const uint16_t *_uint_view<N>::get_part_view() const
    {
        constexpr uint32_t part_size = _uint_view<N>::words_num / DENOMINATOR;
        static_assert(part_size <= _uint_view<N>::words_num and NUMENATOR <= (_uint_view<N>::words_num-part_size) and __globals::is_power_2(DENOMINATOR));
        constexpr uint32_t offset = (_uint_view<N>::words_num / DENOMINATOR) * NUMENATOR;
        return this->value + NUMENATOR;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<uint32_t M>
    _uint<N>::operator _uint<M>() const
    {
        _uint<M> res;
        for(uint32_t i(0); i < _uint_view<M>::words_num; ++i)
        {
            if(i < _uint_view<N>::words_num)
                res.value[i] = this->value[i];
            else
                res.value[i] = 0;
        }
        return res;
    }


    std::string ltrim(const std::string &s)
    {
        size_t start = s.find_first_not_of("0");
        return (start == std::string::npos) ? "0" : s.substr(start);
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_view<N>::operator std::string() const
    {
        std::string res;
        for(int i(_uint_view<N>::words_num-1); i >= 0; --i)
        {
            std::stringstream stream;
            stream.fill('0');
            stream.width(_uint_view<N>::word_bits/4);
            stream <<  std::hex << this->value[i];
            res += stream.str();
        }
        res = ltrim(res);

        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    template<uint32_t M>
    bool _uint_view<N>::operator==(const _uint_view<M>& x) const
    {
        bool res = false;
        uint32_t min_size = std::min(_uint_view<N>::words_num,_uint_view<M>::words_num);
        bool common_part_is_equal = std::equal(this->value,this->value+min_size,x.value);
        bool none_of_after_common_1 = std::none_of(this->value+min_size,this->value+_uint_view<N>::words_num,[](uint16_t arg) { return arg; });
        bool none_of_after_common_2 = std::none_of(x.value+min_size,x.value+_uint_view<M>::words_num,[](uint16_t arg) { return arg; });
        return common_part_is_equal and none_of_after_common_1 and none_of_after_common_2;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    void _uint_view<N>::operator=(const _uint_view<N>& x)
    {
        this->value = x.value;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    void _uint<N>::operator=(const _uint<N>& x)
    {
        std::copy(x.value,x.value+_uint_view<N>::words_num,this->value);
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    void _uint<N>::operator=(const _uint_view<N>& x)
    {
        std::copy(x.value,x.value+_uint_view<N>::words_num,this->value);
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_view<N>& _uint_view<N>::operator+=(const _uint_view<N>& b)
    {
        uint16_t r = 0;
        for(uint32_t i = 0; i < _uint_view<N>::words_num; ++i)
        {
            uint32_t word_sum = (uint32_t)this->value[i] + (uint32_t)b.value[i] + (uint32_t)r;
            this->value[i] = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint_view<N>::word_bits;
        }
        return *this;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_view<N>& _uint_view<N>::operator-=(const _uint_view<N>& b)
    {
        uint16_t r = 0;
        for(uint32_t i(0); i < _uint_view<N>::words_num; ++i)
        {
            uint32_t sub_total = (uint32_t)b.value[i] + (uint32_t)r;
            r = sub_total > this->value[i] ? 1 : 0;
            this->value[i] = ((uint32_t)(r ? _uint_view<N>::base : 0) - (uint32_t)sub_total) + (uint32_t)this->value[i];
        }
        return *this;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_view<N>& _uint_view<N>::operator*=(const _uint_view<N>& b) requires(N < __globals::karatsuba_bound)
    {
        uint64_t pseudo_res[_uint_view<N*2>::words_num];
        pseudo_res[_uint_view<2*N>::words_num-1] = 0;
        for(uint32_t i(0); i < _uint_view<2*N>::words_num-1; ++i)
        {
            pseudo_res[i] = 0;
            uint32_t lower_bound = i >= _uint_view<N>::words_num ? i-_uint_view<N>::words_num+1 : 0;
            uint32_t upper_bound = std::min(i,_uint_view<N>::words_num-1);
            for(uint32_t j(lower_bound); j <= upper_bound; ++j)
            {
                uint32_t a_v = this->value[j];
                uint32_t b_v = b.value[upper_bound-j+lower_bound];
                uint32_t val = a_v*b_v;
                pseudo_res[i] += val;
            }
        }
        for(uint32_t i(1); i < _uint_view<N*2>::words_num; ++i)
            pseudo_res[i] += (pseudo_res[i-1] >> 16);
        for(uint32_t i(0); i < _uint_view<N*2>::words_num; ++i)
            this->value[i] = pseudo_res[i] & uint16_t(0xffff);
        return *this;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint_view<N>& _uint_view<N>::operator*=(const _uint_view<N>& b) requires(N >= __globals::karatsuba_bound)
    {
        const _uint_view<N/2> a_lower_half(const_cast<_uint_view<N>::word_type*>(this->template get_part_view<2,0>()));
        const _uint_view<N/2> a_upper_half(const_cast<_uint_view<N>::word_type*>(this->template get_part_view<2,_uint_view<N>::words_num/2>()));
        const _uint_view<N/2> b_lower_half(const_cast<_uint_view<N>::word_type*>(b.template get_part_view<2,0>()));
        const _uint_view<N/2> b_upper_half(const_cast<_uint_view<N>::word_type*>(b.template get_part_view<2,_uint_view<N>::words_num/2>()));
        _uint<N> z_0 = a_lower_half * b_lower_half;
        _uint<N> z_2 = a_upper_half * b_upper_half;
        _uint<N> z_1 = (a_lower_half + a_upper_half)*(b_lower_half + b_upper_half) - z_0 - z_2;
        _uint_view<N> quoter_0(this->template get_part_view<2,0>());
        _uint_view<N> quoter_1(this->template get_part_view<2,_uint_view<N*2>::words_num/4>());
        _uint_view<N> quoter_2(this->template get_part_view<4,_uint_view<N*2>::words_num/2>());
        quoter_0 += z_0;
        quoter_1 += z_1;
        quoter_2 += z_2;
        return *this;
    }

    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N> _uint_view<N>::operator+(const _uint_view<N>& other) const
    {
        _uint<N> res;
        short r = 0;
        for(uint32_t i = 0; i < _uint_view<N>::words_num; ++i)
        {
            uint64_t word_sum = (uint32_t)this->value[i] + (uint32_t)other.value[i] + (uint32_t)r;
            res.value[i] = word_sum % _uint_view<N>::base;
            r = word_sum / _uint_view<N>::base;
        }
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N> _uint_view<N>::operator-(const _uint_view<N>& other) const
    {
        _uint<N> res;
        uint16_t r = 0;
        for(uint32_t i(0); i < _uint_view<N>::words_num; ++i)
        {
            uint64_t sub_total = (uint32_t)other.value[i] + (uint32_t)r;
            r = sub_total > this->value[i] ? 1 : 0;
            res.value[i] = ((uint32_t)(r ? _uint_view<N>::base : 0) - (uint32_t)sub_total) + (uint32_t)this->value[i];
        }
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N> _uint_view<N>::operator*(const _uint_view<N>& other) const requires (N < __globals::karatsuba_bound)
    {
        _uint<N*2> res;
        uint64_t pseudo_res[_uint_view<N*2>::words_num];
        pseudo_res[_uint_view<2*N>::words_num-1] = 0;
        for(uint32_t i(0); i < _uint_view<2*N>::words_num-1; ++i)
        {
            pseudo_res[i] = 0;
            uint32_t lower_bound = i >= _uint_view<N>::words_num ? i-_uint_view<N>::words_num+1 : 0;
            uint32_t upper_bound = std::min(i,_uint_view<N>::words_num-1);
            for(uint32_t j(lower_bound); j <= upper_bound; ++j)
            {
                uint32_t a_v = this->value[j];
                uint32_t b_v = other.value[upper_bound-j+lower_bound];
                uint32_t val = a_v*b_v;
                pseudo_res[i] += val;
            }
        }
        for(uint32_t i(1); i < _uint_view<N*2>::words_num; ++i)
            pseudo_res[i] += (pseudo_res[i-1] >> 16);
        for(uint32_t i(0); i < _uint_view<N*2>::words_num; ++i)
            res.value[i] = pseudo_res[i] & uint16_t(0xffff);
        return res;
    }


    template<uint32_t N>
        requires(N >= 16 and __globals::is_power_2(N))
    _uint<N> _uint_view<N>::operator*(const _uint_view<N>& other) const requires (N >= __globals::karatsuba_bound)
    {
        _uint<N*2> res(0);
        const _uint_view<N/2> a_lower_half(const_cast<_uint_view<N>::word_type*>(this->template get_part_view<2,0>()));
        const _uint_view<N/2> a_upper_half(const_cast<_uint_view<N>::word_type*>(this->template get_part_view<2,_uint_view<N/2>::words_num>()));
        const _uint_view<N/2> b_lower_half(const_cast<_uint_view<N>::word_type*>(other.template get_part_view<2,0>()));
        const _uint_view<N/2> b_upper_half(const_cast<_uint_view<N>::word_type*>(other.template get_part_view<2,_uint_view<N/2>::words_num>()));
        _uint<N> z_0 = a_lower_half * b_lower_half;
        _uint<N> z_2 = a_upper_half * b_upper_half;
        _uint<N> z_1 = (a_lower_half + a_upper_half)*(b_lower_half + b_upper_half) - z_0 - z_2;
        std::cout << "z0: " << z_0 << '\n';
        std::cout << "z1: " << z_1 << '\n';
        std::cout << "z2: " << z_2 << '\n';
        _uint_view<N> quoter_0(res.template get_part_view<2,0>());
        _uint_view<N> quoter_1(res.template get_part_view<2,_uint_view<N/2>::words_num>());
        _uint_view<N> quoter_2(res.template get_part_view<2,_uint_view<N>::words_num>());
        std::cout << "r0: " << res << '\n';
        quoter_0 += z_0;
        std::cout << "r1: " << res << '\n';
        quoter_1 += z_1;
        std::cout << "r2: " << res << '\n';
        quoter_2 += z_2;
        std::cout << "r3: " << res << '\n';
        return res;
    }


    template<uint32_t N>
    std::ostream& operator<<(std::ostream& out, const _uint_view<N>& x)
    {
        out << (std::string)x;
        return out;
    }


    template<uint32_t N>
    std::istream& operator>>(std::istream& in, _uint<N>& x)
    {
        std::string hex_uint;
        in >> hex_uint;
        x = _uint<N>(hex_uint);

        return in;
    }
}

#endif