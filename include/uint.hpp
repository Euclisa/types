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
#include <type_traits>
#include "globals.hpp"
#include "view_ref_storage.hpp"

namespace lrf
{
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    class _uint_view;

    // ===BEGIN TYPEDEFS===

    typedef _uint_view<128,128,false> uint128_t;
    typedef _uint_view<256,256,false> uint256_t;
    typedef _uint_view<512,512,false> uint512_t;
    typedef _uint_view<1024,1024,false> uint1024_t;
    typedef _uint_view<2048,2048,false> uint2048_t;
    typedef _uint_view<4096,4096,false> uint4096_t;


    template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
    using _uint_add_out_t = _uint_view<__globals::max_addition_output_bits<N,M>(),__globals::max_addition_output_significant_bits<N,N_significant,M,M_significant>(),false>;

    template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
    using _uint_sub_out_t = _uint_view<__globals::max_subtraction_output_bits<N,M>(),__globals::max_subtraction_output_significant_bits<N,N_significant,M,M_significant>(),false>;

    template<uint32_t N, uint32_t N_significant, uint32_t M, uint32_t M_significant>
    using _uint_mul_out_t = _uint_view<__globals::max_multiplication_output_bits<N,M>(),__globals::max_multiplication_output_significant_bits<N,N_significant,M,M_significant>(),false>;

    // ===END TYPEDEFS===

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    class _uint_view
    {
    public:
        typedef uint16_t word_type;
        static constexpr uint8_t word_bits = sizeof(word_type) * 8;
        static constexpr uint32_t words_num = N / word_bits;
        static constexpr uint32_t significant_words_num = N_significant / word_bits;
        static constexpr uint32_t base = 1 << word_bits;
        static constexpr uint8_t parts_num = sizeof...(ViewsMask);
	    static constexpr uint32_t parts_size = words_num / parts_num;
        static constexpr uint32_t parts_size_bits = parts_size * word_bits;

    private:
	    void init(ViewRefStorage<false,std::string_view> hex_str, uint8_t part_i);

        template<uint32_t M_significant, bool ViewMaskOther>
	    void init(ViewRefStorage<true,_uint_view<parts_size_bits,M_significant,ViewMaskOther>> arg, uint8_t part_i);

        template<uint32_t M_significant, bool... ViewMaskOther>
	    void init(ViewRefStorage<false,_uint_view<parts_size_bits,M_significant,ViewMaskOther...>> arg, uint8_t part_i);

	    void init(ViewRefStorage<false,uint64_t> val, uint8_t part_i);

        template<__globals::Iterator _Iterator>
        void init(ViewRefStorage<false,std::pair<_Iterator,_Iterator>> iters, uint8_t part_i);

        template<bool View, typename T>
	    void init(ViewRefStorage<View,T> ptr);
        template<bool View, typename T, typename... InitTail>
	    void init(ViewRefStorage<View,T> ptr, InitTail... tail);

        template<bool View, bool... ViewsMaskTail>
        void destroy();

        template<bool View, bool... ViewsMaskTail>
        void alloc();

        //template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        //void copy(const _uint_view<M,M_significant,ViewsMaskOther...>& other);

        template<uint8_t ViewInd, bool View, bool... ViewsMaskTail>
        static constexpr bool get_view();

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        struct uint_other
        {
            template<bool View, bool... ViewsMaskOtherTail>
            static void copy(_uint_view<N,N_significant,ViewsMask...>& origin, const _uint_view<M,M_significant,ViewsMaskOther...>& other);

            template<bool View, bool... ViewsMaskTail>
            static void check_and_init_remaining(_uint_view<N,N_significant,ViewsMask...>& origin) requires(N <= M);
        };

    protected:
        uint16_t& at(uint32_t i);

        template<uint8_t ViewInd>
        static constexpr bool get_view();

        _uint_view();
    public:
        uint16_t **value;

        template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
        uint16_t *get_part_view();
        template<uint32_t DENOMINATOR, uint32_t NUMENATOR>
        const uint16_t *get_part_view() const;

        template<typename... InitArgs>
        _uint_view(InitArgs... args);
        _uint_view(const _uint_view<N,N_significant,ViewsMask...>&);
        _uint_view(_uint_view<N,N_significant,ViewsMask...>&&);
        ~_uint_view();

        template<uint32_t M, uint32_t M_significant>
        operator _uint_view<M,M_significant,ViewsMask...>() const;

        operator std::string() const;

        _uint_view<N,N_significant,ViewsMask...>& operator=(const _uint_view<N,N_significant,ViewsMask...>& x);

        template<uint32_t M, uint32_t M_significant>
        _uint_view<N,N_significant>& operator=(const _uint_view<M,M_significant>& x);

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_view<N,N_significant,ViewsMask...>&
        operator+=(const _uint_view<M,M_significant,ViewsMaskOther...>& x) requires(N_significant == N);

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_view<N,N_significant,ViewsMask...>&
        operator-=(const _uint_view<M,M_significant,ViewsMaskOther...>& x) requires(N_significant == N);

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_view<N,N_significant,ViewsMask...>&
        operator*=(const _uint_view<M,M_significant,ViewsMaskOther...>& b) requires(N < __globals::karatsuba_bound and N_significant == N);

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_view<N,N_significant,ViewsMask...>&
        operator*=(const _uint_view<M,M_significant,ViewsMaskOther...>& b) requires(N >= __globals::karatsuba_bound and N_significant == N);

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_add_out_t<N,N_significant,M,M_significant>
        operator+(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const;

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_sub_out_t<N,N_significant,M,M_significant>
        operator-(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const;

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_mul_out_t<N,N_significant,M,M_significant>
        operator*(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const requires (N < __globals::karatsuba_bound);
        
        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_mul_out_t<N,N_significant,M,M_significant>
        operator*(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const requires (N >= __globals::karatsuba_bound);

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        bool operator==(const _uint_view<M,M_significant,ViewsMaskOther...>& x) const;
    };


    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
	void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,std::string_view> hex_str, uint8_t part_i)
    {
        constexpr uint32_t significant_words_in_curr_part = this->significant_words_num - part_i*this->parts_size;
        constexpr uint32_t hexes_in_word = this->word_bits/4;
        constexpr std::size_t max_str_len = this->parts_size*hexes_in_word;
        std::size_t str_len = hex_str.value.length();
        uint64_t significant_symbols_offset = std::max(int64_t(this->parts_size - significant_words_in_curr_part)*hexes_in_word - int64_t(max_str_len - (int64_t)str_len),0L);
        uint32_t significant_words_offset_end = (max_str_len - str_len + significant_symbols_offset) / hexes_in_word;
        uint32_t significant_words_num = this->parts_size - significant_words_offset_end;
        uint8_t init_alignment = (str_len-significant_symbols_offset) % 4;
        std::fill(this->value+significant_words_num,this->value+this->parts_size,0);
        if(init_alignment)
        {
            std::stringstream stream;
            stream << hex_str.value.substr(significant_symbols_offset,init_alignment);
            uint32_t curr_word_i = significant_words_num - significant_symbols_offset/hexes_in_word - 1;
            stream >> std::hex >> this->value[part_i][curr_word_i];
            --significant_words_num;
            significant_symbols_offset += init_alignment;
        }
        for(uint64_t i(significant_symbols_offset); i < hex_str.value.length(); i += hexes_in_word)
        {
            std::stringstream stream;
            stream << hex_str.value.substr(i,hexes_in_word);
            uint32_t curr_word_i = significant_words_num - i/hexes_in_word - 1;
            stream >> std::hex >> this->value[part_i][curr_word_i];
        }
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M_significant, bool ViewMaskOther>
	void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<true,_uint_view<parts_size_bits,M_significant,ViewMaskOther>> arg, uint8_t part_i)
    {
        this->value[part_i] = arg.value.value[0];
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M_significant, bool... ViewMaskOther>
	void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,_uint_view<parts_size_bits,M_significant,ViewMaskOther...>> arg, uint8_t part_i)
    {
        for(uint32_t i(0); i < this->parts_size; ++i)
            this->value[part_i][i] = arg.value.at(part_i);
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,uint64_t> value, uint8_t part_i)
    {
        constexpr uint32_t significant_words_in_curr_part = this->significant_words_num - part_i*this->parts_size;
        constexpr uint32_t significant_bits_in_part = significant_words_in_curr_part * this->word_bits;
        for(uint32_t shift(0); shift < significant_bits_in_part; shift += this->word_bits)
        {
            uint16_t l = shift < 64 ? (value.value >> shift) & uint64_t(0xffff) : 0;
            this->value[part_i][shift / this->word_bits] = l;
        }
        std::fill(this->value[part_i]+significant_words_in_curr_part,this->value[part_i]+this->parts_size,0);
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<__globals::Iterator _Iterator>
    void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,std::pair<_Iterator,_Iterator>> iters, uint8_t part_i)
    {
        constexpr uint32_t significant_words_in_curr_part = this->significant_words_num - part_i*this->parts_size;
        auto begin = iters.value.first;
        auto end = iters.value.second;
        auto curr = begin;
        std::size_t bit_i = 0;
        std::fill(this->value[part_i],this->value[part_i]+this->parts_size,0);
        while(curr != end and bit_i < significant_words_in_curr_part)
        {
            this->value[part_i][bit_i/this->word_bits] += *(curr++) ? this->pow2(bit_i%this->word_bits) : 0;
            ++bit_i;
        }
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<bool View, typename T>
	void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<View,T> arg)
    {
        constexpr uint8_t part_i = parts_num-1;
        this->init(arg,part_i);
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<bool View, typename T, typename... InitTail>
	void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<View,T> arg, InitTail... tail)
    {
        constexpr uint8_t tail_size = sizeof...(tail);
        constexpr uint8_t part_i = parts_num-tail_size-1;
        this->init(arg,part_i);
        this->init(tail...);
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<bool View, bool... ViewsMaskTail>
    void _uint_view<N,N_significant,ViewsMask...>::destroy()
    {
        constexpr uint8_t tail_size = sizeof...(ViewsMaskTail);
        constexpr uint8_t part_i = parts_num-tail_size-1;
        if(!View)
            delete[] this->value[part_i];
        if constexpr(tail_size > 0)
            this->destroy<ViewsMaskTail...>();
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<bool View, bool... ViewsMaskTail>
    void _uint_view<N,N_significant,ViewsMask...>::alloc()
    {
        constexpr uint8_t tail_size = sizeof...(ViewsMaskTail);
        constexpr uint8_t part_i = parts_num-tail_size-1;
        if(!View)
            this->value[part_i] = new typename _uint_view<N,N_significant,ViewsMask...>::word_type[this->parts_size];
        if constexpr(tail_size > 0)
            this->destroy<ViewsMaskTail...>();
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    template<bool View, bool... ViewsMaskOtherTail>
    void _uint_view<N,N_significant,ViewsMask...>::uint_other<M,M_significant,ViewsMaskOther...>::copy(_uint_view<N,N_significant,ViewsMask...>& origin, const _uint_view<M,M_significant,ViewsMaskOther...>& other)
    {
        constexpr uint8_t other_tail_size = sizeof...(ViewsMaskOtherTail);
        constexpr uint8_t other_part_i = parts_num-other_tail_size-1;
        constexpr uint32_t current_word_i = _uint_view<M,M_significant,ViewsMaskOther...>::parts_size*other_part_i;

        if constexpr(current_word_i >= _uint_view<N,N_significant,ViewsMask...>::words_num)
            return;

        constexpr uint8_t origin_part_i = current_word_i / _uint_view<N,N_significant,ViewsMask...>::parts_size;
        constexpr uint32_t origin_word_in_part_i = current_word_i % _uint_view<N,N_significant,ViewsMask...>::parts_size;
        constexpr bool origin_current_view = _uint_view<N,N_significant,ViewsMask...>::get_view<origin_part_i>();
        constexpr uint32_t words_can_be_copied = std::min(
                                                    _uint_view<N,N_significant,ViewsMask...>::words_num - current_word_i,
                                                    _uint_view<M,M_significant,ViewsMaskOther...>::parts_size
                                                );
        if constexpr(origin_current_view)
        {
            static_assert(_uint_view<M,M_significant,ViewsMaskOther...>::parts_size == _uint_view<N,N_significant,ViewsMask...>::parts_size, "Can set view only on part with the same size.");
            origin.value[origin_part_i] = other.value[other_part_i];
        }
        else
        {
            constexpr uint8_t origin_parts_in_one_other_num = words_can_be_copied / _uint_view<N,N_significant,ViewsMask...>::parts_size;
            if constexpr(origin_parts_in_one_other_num <= 1)
            {
                constexpr uint32_t part_to_copy_size = words_can_be_copied;
                std::copy(
                    other.value[other_part_i],
                    other.value[other_part_i]+part_to_copy_size,
                    origin.value[origin_part_i]+origin_word_in_part_i
                );
            }
            else
            {
                constexpr uint32_t part_to_copy_size = _uint_view<N,N_significant,ViewsMask...>::parts_size;
                for(uint8_t i(0); i < origin_parts_in_one_other_num; ++i)
                {
                    std::copy(
                        other.value[other_part_i],
                        other.value[other_part_i]+part_to_copy_size,
                        origin.value[origin_part_i]+origin_word_in_part_i
                    );
                }
            }
        }
        if constexpr(other_tail_size > 0)
            _uint_view<N,N_significant,ViewsMask...>::uint_other<M,M_significant,ViewsMaskOther...>::copy<ViewsMaskOtherTail...>(origin,other);
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    template<bool View, bool... ViewsMaskTail>
    void _uint_view<N,N_significant,ViewsMask...>::uint_other<M,M_significant,ViewsMaskOther...>::check_and_init_remaining(_uint_view<N,N_significant,ViewsMask...>& origin) requires(N <= M)
    {
        constexpr uint8_t tail_size = sizeof...(ViewsMaskTail);
        constexpr uint8_t part_i = parts_num-tail_size-1;
        constexpr uint32_t current_word_i = _uint_view<N,N_significant,ViewsMask...>::parts_size*part_i;
        
        if constexpr(current_word_i >= _uint_view<M,M_significant,ViewsMaskOther...>::words_num)
        {
            static_assert(!View,"Can't set view. Sizes mismatch.");
            std::fill(
                origin.value[part_i],
                origin.value[part_i]+_uint_view<N,N_significant,ViewsMask...>::parts_size,
                0
            );
        }

        if constexpr(tail_size > 0)
            _uint_view<N,N_significant,ViewsMask...>::uint_other<M,M_significant,ViewsMaskOther...>::check_and_init_remaining<ViewsMaskTail...>(origin);
    }


    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    uint16_t& _uint_view<N,N_significant,ViewsMask...>::at(uint32_t i)
    {
        return this->value[i/this->parts_size][i%this->parts_size];
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint8_t ViewInd, bool View, bool... ViewsMaskTail>
    constexpr bool _uint_view<N,N_significant,ViewsMask...>::get_view()
    {
        static_assert(ViewInd < _uint_view<N,N_significant,ViewsMask...>::parts_num,"Index out of range");
        constexpr uint8_t tail_size = sizeof...(ViewsMaskTail);
        constexpr uint8_t curr_part_i = _uint_view<N,N_significant,ViewsMask...>::parts_num-tail_size-1;
        if constexpr(curr_part_i == ViewInd)
            return View;
        return _uint_view<N,N_significant,ViewsMask...>::get_view<ViewInd,ViewsMaskTail...>();
    }

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint8_t ViewInd>
    constexpr bool _uint_view<N,N_significant,ViewsMask...>::get_view()
    {
        static_assert(ViewInd < _uint_view<N,N_significant,ViewsMask...>::parts_num,"Index out of range");
        return _uint_view<N,N_significant,ViewsMask...>::get_view<ViewInd,ViewsMask...>();
    }


    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>::_uint_view()
    {
        this->value = new std::add_pointer_t<typename _uint_view<N,N_significant,ViewsMask...>::word_type>[this->parts_num];
        this->alloc<ViewsMask...>();
    }


    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<typename... InitArgs>
    _uint_view<N,N_significant,ViewsMask...>::_uint_view(InitArgs... args)
    {
        this->init(ViewRefStorage<ViewsMask,__globals::unqualified_t<InitArgs>>(args)...);
    }


    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>::~_uint_view()
    {
        this->destroy<ViewsMask...>();
    }


    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>::_uint_view(const _uint_view<N,N_significant,ViewsMask...>& x) : _uint_view<N,N_significant>()
    {
        
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