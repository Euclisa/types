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
    template<uint32_t N, uint32_t N_significant=N>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    class _uint_view;

    template<uint32_t N, uint32_t N_significant=N>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    class _uint;

    // ===BEGIN TYPEDEFS===

    typedef _uint<128> uint128_t;
    typedef _uint<256> uint256_t;
    typedef _uint<512> uint512_t;
    typedef _uint<1024> uint1024_t;
    typedef _uint<2048> uint2048_t;
    typedef _uint<4096> uint4096_t;


    template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
    using _uint_add_out_t = _uint<__globals::max_addition_output_bits<N,M>(),__globals::max_addition_output_significant_bits<N,N_significant,M,M_significant>()>;

    template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
    using _uint_sub_out_t = _uint<__globals::max_subtraction_output_bits<N,M>(),__globals::max_subtraction_output_significant_bits<N,N_significant,M,M_significant>()>;

    template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
    using _uint_mul_out_t = _uint<__globals::max_multiplication_output_bits<N,M>(),__globals::max_multiplication_output_significant_bits<N,N_significant,M,M_significant>()>;

    // ===END TYPEDEFS===

    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    class _uint_view
    {
    public:
        typedef uint16_t word_type;
        static constexpr uint8_t word_bits = sizeof(word_type) * 8;
        static constexpr uint32_t words_num = N / word_bits;
        static constexpr uint32_t significant_words_num = N_significant / word_bits;
        static constexpr uint32_t base = 1 << word_bits;

        uint16_t *value;

        template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
        uint16_t *get_part_view();
        template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
        const uint16_t *get_part_view() const;

        _uint_view(uint16_t *ptr);
        _uint_view(const _uint_view<N,N_significant>&);
        _uint_view(_uint_view<N,N_significant>&&);

        template<uint32_t M, uint32_t M_significant>
        operator _uint<M,M_significant>() const;

        operator std::string() const;

        _uint_view<N,N_significant>& operator=(const _uint_view<N,N_significant>& x);

        template<uint32_t M, uint32_t M_significant>
        _uint_view<N,N_significant>& operator=(const _uint_view<M,M_significant>& x);

        template<uint32_t M, uint32_t M_significant>
        _uint_view<N,N_significant>&
        operator+=(const _uint_view<M,M_significant>& x) requires(N_significant == N);

        template<uint32_t M, uint32_t M_significant>
        _uint_view<N,N_significant>&
        operator-=(const _uint_view<M,M_significant>& x) requires(N_significant == N);

        template<uint32_t M, uint32_t M_significant>
        _uint_view<N,N_significant>&
        operator*=(const _uint_view<M,M_significant>& b) requires(N < __globals::karatsuba_bound and N_significant == N);

        template<uint32_t M, uint32_t M_significant>
        _uint_view<N,N_significant>&
        operator*=(const _uint_view<M,M_significant>& b) requires(N >= __globals::karatsuba_bound and N_significant == N);

        template<uint32_t M, uint32_t M_significant>
        _uint_add_out_t<N,N_significant,M,M_significant>
        operator+(const _uint_view<M,M_significant>& other) const;

        template<uint32_t M, uint32_t M_significant>
        _uint_sub_out_t<N,N_significant,M,M_significant>
        operator-(const _uint_view<M,M_significant>& other) const;

        template<uint32_t M, uint32_t M_significant>
        _uint_mul_out_t<N,N_significant,M,M_significant>
        operator*(const _uint_view<M,M_significant>& other) const requires (N < __globals::karatsuba_bound);
        
        template<uint32_t M, uint32_t M_significant>
        _uint_mul_out_t<N,N_significant,M,M_significant>
        operator*(const _uint_view<M,M_significant>& other) const requires (N >= __globals::karatsuba_bound);

        template<uint32_t M, uint32_t M_significant>
        bool operator==(const _uint_view<M,M_significant>& x) const;
    };


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    class _uint : public _uint_view<N,N_significant>
    {
    public:
        _uint();
        _uint(uint64_t value);
        _uint(std::string_view hex_str);
        template<__globals::Iterator _Iterator>
        _uint(_Iterator begin, _Iterator end);
        _uint(const _uint_view<N,N_significant>&);
        _uint(const _uint<N,N_significant>&);
        _uint(_uint<N,N_significant>&&);

        _uint<N,N_significant>& operator=(const _uint<N,N_significant>&);

        template<uint32_t M, uint32_t M_significant>
        operator _uint<M,M_significant>() const;
        template<uint32_t M, uint32_t M_significant>
        _uint<N,N_significant>& operator=(const _uint_view<M,M_significant>& x);

        ~_uint();
    };


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint<N,N_significant>::_uint() : _uint_view<N,N_significant>(new typename _uint<N,N_significant>::word_type[_uint<N,N_significant>::words_num]) {}


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint<N,N_significant>::_uint(uint64_t value) : _uint<N,N_significant>()
    {
        for(uint32_t shift(0); shift < N; shift += _uint<N,N_significant>::word_bits)
        {
            uint16_t l = shift < 64 ? (value >> shift) & uint64_t(0xffff) : 0;
            this->value[shift / _uint<N,N_significant>::word_bits] = l;
        }
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint<N,N_significant>::_uint(std::string_view hex_str) : _uint<N,N_significant>()
    {
        std::string prefixed_hex_str(std::max((int64_t)N/4-(int64_t)hex_str.length(),(int64_t)0),'0');
        prefixed_hex_str += hex_str;
        for(uint32_t i(0); i < _uint<N,N_significant>::words_num; ++i)
        {
            std::stringstream stream;
            stream << prefixed_hex_str.substr(i*_uint<N,N_significant>::word_bits/4,_uint<N,N_significant>::word_bits/4);
            stream >> std::hex >> this->value[_uint<N,N_significant>::words_num-i-1];
        }
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<__globals::Iterator _Iterator>
    _uint<N,N_significant>::_uint(_Iterator begin, _Iterator end) : _uint<N,N_significant>()
    {
        _Iterator curr = begin;
        std::size_t bit_i = 0;
        while(curr != end and bit_i < _uint_view<N,N_significant>::significant_words_num)
        {
            this->value[bit_i/_uint<N,N_significant>::word_bits] += *(curr++) ? this->pow2(bit_i%_uint<N,N_significant>::word_bits) : 0;
            ++bit_i;
        }
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint<N,N_significant>::_uint(const _uint_view<N,N_significant>& x) : _uint<N,N_significant>()
    {
        std::copy(x.value,x.value+_uint<N,N_significant>::words_num,this->value);
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint<N,N_significant>::_uint(const _uint<N,N_significant>& x) : _uint<N,N_significant>()
    {
        std::copy(x.value,x.value+_uint<N,N_significant>::words_num,this->value);
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint<N,N_significant>::_uint(_uint<N,N_significant>&& x) : _uint_view<N,N_significant>(std::forward<_uint<N,N_significant>>(x)) {}


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint<N,N_significant>::~_uint()
    {
        if(this->value)
            delete[] this->value;
    }

    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint<N,N_significant>& _uint<N,N_significant>::operator=(const _uint<N,N_significant>& x)
    {
        std::copy(x.value,x.value+_uint<N,N_significant>::words_num,this->value);
        return *this;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint<N,N_significant>& _uint<N,N_significant>::operator=(const _uint_view<M,M_significant>& x)
    {
        constexpr uint32_t min_significant_size = std::min(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        std::copy(x.value,x.value+min_significant_size,this->value);
        std::fill(this->value+min_significant_size,this->value+_uint<N,N_significant>::words_num,0);
        return *this;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant>::_uint_view(const _uint_view<N,N_significant>& x)
    {
        this->value = x.value;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant>::_uint_view(_uint_view<N,N_significant>&& x)
    {
        this->value = x.value;
        x.value = nullptr;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant>::_uint_view(uint16_t *ptr) : value(ptr) {}


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
    uint16_t *_uint_view<N,N_significant>::get_part_view()
    {
        constexpr uint32_t part_size = _uint<N,N_significant>::words_num / DENOMINATOR;
        static_assert(part_size <= _uint_view<N,N_significant>::words_num and NUMENATOR <= (_uint<N,N_significant>::words_num-part_size) and __globals::is_power_2(DENOMINATOR));
        constexpr uint32_t offset = (_uint_view<N,N_significant>::words_num / DENOMINATOR) * NUMENATOR;
        return this->value + NUMENATOR;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
    const uint16_t *_uint_view<N,N_significant>::get_part_view() const
    {
        constexpr uint32_t part_size = _uint<N,N_significant>::words_num / DENOMINATOR;
        static_assert(part_size <= _uint<N,N_significant>::words_num and NUMENATOR <= (_uint<N,N_significant>::words_num-part_size) and __globals::is_power_2(DENOMINATOR));
        constexpr uint32_t offset = (_uint<N,N_significant>::words_num / DENOMINATOR) * NUMENATOR;
        return this->value + NUMENATOR;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_view<N,N_significant>::operator _uint<M,M_significant>() const
    {
        _uint<M,M_significant> res;
        constexpr uint32_t min_significant_bits = std::min(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        for(uint32_t i(0); i < min_significant_bits; ++i)
            res.value[i] = this->value[i];
        std::fill(res.value+min_significant_bits,res.value+_uint<M,M_significant>::words_num,0);

        return res;
    }


    std::string ltrim(const std::string &s)
    {
        size_t start = s.find_first_not_of("0");
        return (start == std::string::npos) ? "0" : s.substr(start);
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant>::operator std::string() const
    {
        std::string res;
        for(int i(_uint<N,N_significant>::significant_words_num-1); i >= 0; --i)
        {
            std::stringstream stream;
            stream.fill('0');
            stream.width(_uint<N,N_significant>::word_bits/4);
            stream <<  std::hex << this->value[i];
            res += stream.str();
        }
        res = ltrim(res);

        return res;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    bool _uint_view<N,N_significant>::operator==(const _uint_view<M,M_significant>& x) const
    {
        bool res = false;
        uint32_t min_size = std::min(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        bool common_part_is_equal = std::equal(this->value,this->value+min_size,x.value);
        bool none_after_common_1 = std::none_of(this->value+min_size,this->value+_uint<N,N_significant>::significant_words_num,[](uint16_t arg) { return arg; });
        bool none_after_common_2 = std::none_of(x.value+min_size,x.value+_uint_view<M,M_significant>::significant_words_num,[](uint16_t arg) { return arg; });
        return common_part_is_equal and none_after_common_1 and none_after_common_2;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant>& _uint_view<N,N_significant>::operator=(const _uint_view<N,N_significant>& x)
    {
        this->value = x.value;
        return *this;
    }

    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_view<N,N_significant>& _uint_view<N,N_significant>::operator=(const _uint_view<M,M_significant>& x)
    {
        static_assert(M >= N);
        this->value = x.value;
        return *this;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_view<N,N_significant>& _uint_view<N,N_significant>::operator+=(const _uint_view<M,M_significant>& b) requires(N_significant == N)
    {
        uint16_t r = 0;
        constexpr uint32_t significant_ops = std::min(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        for(uint32_t i = 0; i < significant_ops; ++i)
        {
            uint32_t word_sum = (uint32_t)this->value[i] + (uint32_t)b.value[i] + (uint32_t)r;
            this->value[i] = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint<N,N_significant>::word_bits;
        }
        for(uint32_t i = significant_ops; i < _uint<N,N_significant>::words_num; ++i)
        {
            if(r == 0)
                break;
            uint32_t word_sum = (uint32_t)this->value[i] + (uint32_t)r;
            this->value[i] = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint<N,N_significant>::word_bits;
        }

        return *this;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_view<N,N_significant>& _uint_view<N,N_significant>::operator-=(const _uint_view<M,M_significant>& b) requires(N_significant == N)
    {
        uint16_t r = 0;
        constexpr uint32_t significant_ops = std::min(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        for(uint32_t i(0); i < significant_ops; ++i)
        {
            uint32_t sub_total = (uint32_t)b.value[i] + (uint32_t)r;
            r = sub_total > this->value[i] ? 1 : 0;
            this->value[i] = ((uint32_t)(r ? _uint_view<N,N_significant>::base : 0) - (uint32_t)sub_total) + (uint32_t)this->value[i];
        }
        uint32_t i = significant_ops;
        for(; i < _uint_view<N>::words_num; ++i)
        {
            if(r == 0)
                break;
            uint32_t sub_total = (uint32_t)r;
            r = sub_total > this->value[i] ? 1 : 0;
            this->value[i] = ((uint32_t)(r ? _uint_view<N,N_significant>::base : 0) - (uint32_t)sub_total) + (uint32_t)this->value[i];
        }

        return *this;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_view<N,N_significant>& _uint_view<N,N_significant>::operator*=(const _uint_view<M,M_significant>& b) requires(N < __globals::karatsuba_bound and N_significant == N)
    {
        uint64_t pseudo_res[_uint<N*2,N_significant*2>::words_num];
        pseudo_res[_uint<2*N,2*N_significant>::words_num-1] = 0;
        std::fill(pseudo_res,pseudo_res+_uint<N*2,N_significant*2>::words_num,0);
        constexpr uint32_t significant_ops = std::min(_uint<2*N,2*N_significant>::words_num,
                                            _uint<2*N,2*N_significant>::significant_words_num+_uint<2*M,2*M_significant>::significant_words_num);
        for(uint32_t i(0); i < significant_ops-1; ++i)
        {
            uint32_t lower_bound = i >= _uint_view<N>::words_num ? i-_uint_view<N>::words_num+1 : 0;
            uint32_t upper_bound = std::min(i,_uint_view<N>::words_num-1);
            for(uint32_t j(lower_bound); j <= upper_bound; ++j)
            {
                uint32_t a_v = this->value[j];
                uint32_t b_v = b.value[upper_bound-j+lower_bound];
                pseudo_res[i] += a_v*b_v;
            }
        }
        for(uint32_t i(1); i < significant_ops; ++i)
            pseudo_res[i] += pseudo_res[i-1] >> 16;
        for(uint32_t i(0); i < _uint_view<N*2>::words_num; ++i)
            this->value[i] = pseudo_res[i] & uint16_t(0xffff);
        return *this;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_view<N,N_significant>& _uint_view<N,N_significant>::operator*=(const _uint_view<M,M_significant>& b) requires(N >= __globals::karatsuba_bound and N_significant == N)
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

    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_add_out_t<N,N_significant,M,M_significant> _uint_view<N,N_significant>::operator+(const _uint_view<M,M_significant>& other) const
    {
        _uint_add_out_t<N,N_significant,M,M_significant> res;
        constexpr uint32_t significant_ops_min = std::min(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        constexpr uint32_t significant_ops_max = std::max(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        uint16_t r = 0;
        for(uint32_t i = 0; i < significant_ops_min; ++i)
        {
            uint32_t word_sum = (uint32_t)this->value[i] + (uint32_t)other.value[i] + (uint32_t)r;
            res.value[i] = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint<N,N_significant>::word_bits;
        }
        for(uint32_t i = significant_ops_min; i < significant_ops_max; ++i)
        {
            uint32_t word_sum = r;
            if constexpr(_uint<N,N_significant>::significant_words_num > significant_ops_min)
                word_sum += (uint32_t)this->value[i];
            else
                word_sum += (uint32_t)other.value[i];
            res.value[i] = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint<N,N_significant>::word_bits;
        }
        if constexpr(significant_ops_max < _uint_add_out_t<N,N_significant,M,M_significant>::words_num)
        {
            res.value[significant_ops_max] = r;
            std::fill(res.value+significant_ops_max+1,res.value+_uint_add_out_t<N,N_significant,M,M_significant>::words_num,0);
        }

        return res;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_sub_out_t<N,N_significant,M,M_significant> _uint_view<N,N_significant>::operator-(const _uint_view<M,M_significant>& other) const
    {
        _uint_sub_out_t<N,N_significant,M,M_significant> res;
        constexpr uint32_t significant_ops_min = std::min(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        constexpr uint32_t significant_ops_max = std::max(_uint<N,N_significant>::significant_words_num,_uint<M,M_significant>::significant_words_num);
        uint16_t r = 0;
        for(uint32_t i(0); i < significant_ops_min; ++i)
        {
            uint32_t sub_total = (uint32_t)other.value[i] + (uint32_t)r;
            r = sub_total > this->value[i] ? 1 : 0;
            res.value[i] = ((uint32_t)(r ? _uint_view<N,N_significant>::base : 0) - (uint32_t)sub_total) + (uint32_t)this->value[i];
        }
        for(uint32_t i = significant_ops_min; i < significant_ops_max; ++i)
        {
            uint32_t sub_total = r;
            if constexpr(_uint<M,M_significant>::significant_words_num > significant_ops_min)
                sub_total += (uint32_t)other.value[i];
            r = sub_total > this->value[i] ? 1 : 0;
            uint32_t sub_val = ((uint32_t)(r ? _uint_view<N,N_significant>::base : 0) - (uint32_t)sub_total);
            if constexpr(_uint<N,N_significant>::significant_words_num > significant_ops_min)
                sub_val += (uint32_t)this->value[i];
            res.value[i] = sub_val;
        }
        std::fill(res.value+significant_ops_max,res.value+_uint_sub_out_t<N,N_significant,M,M_significant>::words_num,(uint32_t)0x10000-r);

        return res;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_mul_out_t<N,N_significant,M,M_significant> _uint_view<N,N_significant>::operator*(const _uint_view<M,M_significant>& other) const requires (N < __globals::karatsuba_bound)
    {
        _uint_mul_out_t<N,N_significant,M,M_significant> res;
        uint64_t pseudo_res[_uint_mul_out_t<N,N_significant,M,M_significant>::words_num];
        constexpr uint32_t significant_ops = std::min(_uint_mul_out_t<N,N_significant,M,M_significant>::words_num,
                                            _uint<N,N_significant>::significant_words_num+_uint<M,M_significant>::significant_words_num);
        std::fill(pseudo_res,pseudo_res+significant_ops,0ULL);
        for(uint32_t i(0); i < _uint<N,N_significant>::significant_words_num; ++i)
        {
            for(uint32_t j(0); j < std::min(_uint<M,M_significant>::significant_words_num,_uint_mul_out_t<N,N_significant,M,M_significant>::words_num-i); ++j)
            {
                uint32_t a_v = this->value[i];
                uint32_t b_v = other.value[j];
                pseudo_res[i+j] += a_v*b_v;
            }
        }

        for(uint32_t i(1); i < _uint_mul_out_t<N,N_significant,M,M_significant>::words_num; ++i)
            pseudo_res[i] += pseudo_res[i-1] >> 16;
        for(uint32_t i(0); i < significant_ops; ++i)
            res.value[i] = pseudo_res[i] & uint16_t(0xffff);
        std::fill(res.value+significant_ops,res.value+_uint_mul_out_t<N,N_significant,M,M_significant>::words_num,0ULL);
        return res;
    }


    template<uint32_t N, uint32_t N_significant>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_mul_out_t<N,N_significant,M,M_significant> _uint_view<N,N_significant>::operator*(const _uint_view<M,M_significant>& other) const requires (N >= __globals::karatsuba_bound)
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


    template<uint32_t N, uint32_t N_significant>
    std::ostream& operator<<(std::ostream& out, const _uint_view<N,N_significant>& x)
    {
        out << (std::string)x;
        return out;
    }


    template<uint32_t N, uint32_t N_significant>
    std::istream& operator>>(std::istream& in, _uint<N,N_significant>& x)
    {
        std::string hex_uint;
        in >> hex_uint;
        x = _uint<N,N_significant>(hex_uint);

        return in;
    }
}

#endif