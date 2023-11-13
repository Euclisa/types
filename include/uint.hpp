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

        template<uint32_t M, uint32_t M_significant, bool... ViewMaskOther>
        void init(ViewRefStorage<true,std::pair<_uint_view<M,M_significant,ViewMaskOther...>,uint8_t>> other, uint8_t part_i) requires(_uint_view<M,M_significant,ViewMaskOther...>::parts_size == _uint_view<N,N_significant,ViewsMask...>::words_num);

        template<uint32_t M, uint32_t M_significant, bool... ViewMaskOther>
        void init(ViewRefStorage<false,std::pair<_uint_view<M,M_significant,ViewMaskOther...>,uint8_t>> other, uint8_t part_i) requires(_uint_view<M,M_significant,ViewMaskOther...>::parts_size == _uint_view<N,N_significant,ViewsMask...>::words_num);

        template<bool View, bool... ViewsMaskTail>
        void destroy();

        template<bool View, bool... ViewsMaskTail>
        void alloc();

        template<bool View, bool... ViewsMaskTail>
        void move(_uint_view<N,N_significant,ViewsMask...>&&);

        //template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        //void copy(const _uint_view<M,M_significant,ViewsMaskOther...>& other);

        template<uint8_t ViewInd, bool View, bool... ViewsMaskTail>
        static constexpr bool get_view();

        template<uint8_t ViewIndBegin, uint8_t ViewIndEnd, bool View, bool... ViewsMaskTail>
        static constexpr bool any_view();
        
        static constexpr uint32_t significant_words_in_part(uint8_t part_i);
        static constexpr uint32_t significant_bits_in_part(uint8_t part_i);

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
        const uint16_t& at(uint32_t i) const;
        uint16_t atc(uint32_t i) const;

        template<uint8_t ViewInd>
        static constexpr bool get_view();

        template<uint8_t ViewIndBegin, uint8_t ViewIndEnd>
        static constexpr bool any_view();

        _uint_view();
    public:
        uint16_t **value;

        template<uint8_t part_i>
        using _uint_part_view_t = _uint_view<_uint_view<N,N_significant,ViewsMask...>::parts_size_bits,_uint_view<N,N_significant,ViewsMask...>::significant_bits_in_part(part_i),true>;

        template<uint8_t part_i>
        using _uint_part_copy_t = _uint_view<_uint_view<N,N_significant,ViewsMask...>::parts_size_bits,_uint_view<N,N_significant,ViewsMask...>::significant_bits_in_part(part_i),false>;

        template<uint32_t part_i>
        _uint_part_view_t<part_i> get_part_view();
        template<uint32_t part_i>
        _uint_part_copy_t<part_i> get_part_view() const;

        template<typename... InitArgs>
        _uint_view(InitArgs... args);
        _uint_view(const _uint_view<N,N_significant,ViewsMask...>&);
        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_view(const _uint_view<M,M_significant,ViewsMaskOther...>&);

        _uint_view(_uint_view<N,N_significant,ViewsMask...>&&);

        ~_uint_view();

        template<uint32_t M, uint32_t M_significant>
        operator _uint_view<M,M_significant,false>() const;

        operator std::string() const;

        _uint_view<N,N_significant,ViewsMask...>& operator=(const _uint_view<N,N_significant,ViewsMask...>& x);

        template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
        _uint_view<N,N_significant,ViewsMask...>& operator=(const _uint_view<M,M_significant,ViewsMaskOther...>& x);

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

    /*
        Service function that resolves absolute index to pair (part_index, word_inside_part_index) and returns refernce to corresponding word.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    uint16_t& _uint_view<N,N_significant,ViewsMask...>::at(uint32_t i)
    {
        return this->value[i/this->parts_size][i%this->parts_size];
    }

    /*
        Service function that resolves absolute index to pair (part_index, word_inside_part_index) and returns refernce to corresponding word.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    const uint16_t& _uint_view<N,N_significant,ViewsMask...>::at(uint32_t i) const
    {
        return this->value[i/this->parts_size][i%this->parts_size];
    }

    /*
        Service function that resolves absolute index to pair (part_index, word_inside_part_index) and returns a copy to corresponding word.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    uint16_t _uint_view<N,N_significant,ViewsMask...>::atc(uint32_t i) const
    {
        return this->value[i/this->parts_size][i%this->parts_size];
    }



    /*
        Returns a particular element of 'ViewsMask'.
        Private and should only be used wrapped with one beneath.
    */
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

    /*
        Returns a particular element of 'ViewsMask'.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint8_t ViewInd>
    constexpr bool _uint_view<N,N_significant,ViewsMask...>::get_view()
    {
        static_assert(ViewInd < _uint_view<N,N_significant,ViewsMask...>::parts_num,"Index out of range");
        return _uint_view<N,N_significant,ViewsMask...>::get_view<ViewInd,ViewsMask...>();
    }


    /*
        Checks if at least one entry in 'ViewsMask' is 'true'.
        Private and should only be used wrapped with one beneath.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint8_t ViewIndBegin, uint8_t ViewIndEnd, bool View, bool... ViewsMaskTail>
    constexpr bool _uint_view<N,N_significant,ViewsMask...>::any_view()
    {
        static_assert(ViewIndBegin < _uint_view<N,N_significant,ViewsMask...>::parts_num,"Begin index out of range");
        static_assert(ViewIndEnd <= _uint_view<N,N_significant,ViewsMask...>::parts_num,"End index out of range");
        static_assert(ViewIndBegin <= ViewIndEnd,"Begin index must be not greater than end.");
        constexpr uint8_t tail_size = sizeof...(ViewsMaskTail);
        constexpr uint8_t curr_part_i = _uint_view<N,N_significant,ViewsMask...>::parts_num-tail_size-1;
        if constexpr(curr_part_i >= ViewIndBegin)
        {
            if constexpr(curr_part_i >= ViewIndEnd)
                return false;
            if constexpr(View)
                return true;
        }
        return _uint_view<N,N_significant,ViewsMask...>::any_view<ViewIndBegin,ViewIndEnd,ViewsMaskTail...>();
    }

    /*
        Checks if at least one entry in 'ViewsMask' is 'true'.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint8_t ViewIndBegin, uint8_t ViewIndEnd>
    constexpr bool _uint_view<N,N_significant,ViewsMask...>::any_view()
    {
        static_assert(ViewIndBegin < _uint_view<N,N_significant,ViewsMask...>::parts_num,"Begin index out of range.");
        static_assert(ViewIndEnd <= _uint_view<N,N_significant,ViewsMask...>::parts_num,"End index out of range.");
        static_assert(ViewIndBegin <= ViewIndEnd,"Begin index must be not greater than end.");
        return _uint_view<N,N_significant,ViewsMask...>::any_view<ViewIndBegin,ViewIndEnd,ViewsMask...>();
    }



    /*
        Service static function that returns number of significant words in the particular part (with index 'part_i')
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    constexpr uint32_t _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(uint8_t part_i)
    {
        return std::max(
            (int64_t)_uint_view<N,N_significant,ViewsMask...>::significant_words_num - (int64_t)_uint_view<N,N_significant,ViewsMask...>::parts_size*part_i,
            0L
            );
    }

    /*
        Service static function that returns number of significant bits in the particular part (with index 'part_i')
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    constexpr uint32_t _uint_view<N,N_significant,ViewsMask...>::significant_bits_in_part(uint8_t part_i)
    {
        return std::max(
            (int64_t)_uint_view<N,N_significant,ViewsMask...>::significant_words_num - (int64_t)_uint_view<N,N_significant,ViewsMask...>::parts_size*part_i,
            0L
            ) * _uint_view<N,N_significant,ViewsMask...>::word_bits;
    }



    /*
        Default constructor.
        In order to stay as flexible as possible this class define uses 'word_type' field.
        This allocates space for all parts as type 'word_type*' and then calls for 'alloc'.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>::_uint_view()
    {
        this->value = new std::add_pointer_t<typename _uint_view<N,N_significant,ViewsMask...>::word_type>[this->parts_num];
        this->alloc<ViewsMask...>();
    }



    /*
        This routine unrolls 'ViewsMask' and for every 'false' ("not a view") entry inside allocates space.
    */
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
            this->alloc<ViewsMaskTail...>();
    }



    /*
        Entry point of every constructor calls.
        Removes all qualifiers from 'args', unroll it and put all elements to 'ViewRefStorage'. Then passes all this to the 'init' overload right below.
        'ViewRefStorage's purpose is to provide flexible mechanism of managing write access to certain arguments according to 'ViewMask'.
        If 'ViewMask's i-th entry is false then i-th element of 'args' will not be modified through out whole lifetime of this object instance.
        Otherwise non-const reference to i-th element will be used inside this object instance.
        This mechanism allows user to reduce unnecessary copy overhead when needed.
        Moreover, one can use different initialization argument for every single part.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<typename... InitArgs>
    _uint_view<N,N_significant,ViewsMask...>::_uint_view(InitArgs... args) : _uint_view()
    {
        this->init(ViewRefStorage<ViewsMask,__globals::unqualified_t<InitArgs>>(args)...);
    }


    /*
        These two functions are wrappers for all remaining 'init' functions that take non-template arguments.
        These call necessary 'init' functions part-by-part.
    */
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

    // === BEGIN 'init' FUNCTIONS COLLECTION ===

    /*
        Initialization of a 'part_i' part from hex-string.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
	void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,std::string_view> hex_str, uint8_t part_i)
    {
        // Calculated as number of all significant words except ones in lower parts
        constexpr uint32_t significant_words_in_curr_part = _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(part_i);
        constexpr uint32_t hexes_in_word = this->word_bits/4;
        constexpr std::size_t max_str_len = this->parts_size*hexes_in_word;

        // Unforunately, I couldn't get string length in compile time. If someone reads this and knows how to fix it, please, do it.
        // This would allow to perform this whole routine in compile time.
        /*constexpr*/ std::size_t str_len = hex_str.value.length();

        // Number of hex symbols needed to fill all unsignificant words of the current part
        constexpr int64_t symbols_num_for_unsignificant_words = (this->parts_size - significant_words_in_curr_part)*hexes_in_word;
        // Number of ommitted '0' symbols in 'hex_str'.
        // For example, if current part holds two significant bytes then canonical form of 'hex_str' would be "0234", but user can pass just "234" with last zero omitted
        // but also user can pass something like this: "001234", which is valid but we need to truncate last two zeros. Though I said that it is 'valid' you can see that if input is "ff1234" we won't throw an exception but just truncate 'ff'.
        int64_t unsignificant_symbols_omitted = (int64_t)max_str_len - (int64_t)str_len;
        // Index of the first symbol of 'hex_str' that can be mapped on significant words of the current part
        uint64_t significant_symbols_offset = std::max(symbols_num_for_unsignificant_words - unsignificant_symbols_omitted,0L);

        // Words within parts are stored in little-endian so the most significant words are at the end and, hence, unsignificant words are at the end too
        uint32_t significant_words_offset_end = (max_str_len - (str_len - significant_symbols_offset)) / hexes_in_word;
        uint32_t significant_words_num = this->parts_size - significant_words_offset_end;
        std::fill(this->value+significant_words_num,this->value+this->parts_size,0);

        // This is important because number of hex symbols in 'hex_str' might not be equal to number of hexes in a word.
        uint8_t init_alignment = (str_len-significant_symbols_offset) % (_uint_view<N,N_significant,ViewsMask...>::word_bits/4);
        if(init_alignment)
        {
            std::stringstream stream;
            stream << hex_str.value.substr(significant_symbols_offset,init_alignment);
            uint32_t curr_word_i = significant_words_num - significant_symbols_offset/hexes_in_word - 1;
            stream >> std::hex >> this->value[part_i][curr_word_i];
            --significant_words_num;
            significant_symbols_offset += init_alignment;
        }
        for(uint64_t i(significant_symbols_offset); i < str_len; i += hexes_in_word)
        {
            std::stringstream stream;
            stream << hex_str.value.substr(i,hexes_in_word);
            uint32_t curr_word_i = significant_words_num - i/hexes_in_word - 1;
            stream >> std::hex >> this->value[part_i][curr_word_i];
        }
    }

    /*
        Initialization of a 'part_i' part as a view on some other one.
        If you want to a specific part to be a view on some other class, it must contain only one part.
        Basically, it would be feasible to create a simillar constructor that takes class with variadic 'ViewMaskOther' and explicitly demand it to be of size 'parts_size_bits*sizeof...(ViewMaskOther)' but it would make code much more sophisticated and I don't need it.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M_significant, bool ViewMaskOther>
	void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<true,_uint_view<parts_size_bits,M_significant,ViewMaskOther>> arg, uint8_t part_i)
    {
        constexpr uint32_t significant_words_in_curr_part = _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(part_i);
        static_assert(significant_words_in_curr_part <= _uint_view<parts_size_bits,M_significant,ViewMaskOther>::significant_words_num, "Can't set view on instance with number of significant words less than in current part.");
        this->value[part_i] = arg.value.value[0];
    }

    /*
        Initialization of a 'part_i' part as a copy of some other one.
        Other instance can have any number of parts.
        'atc' ("at-copy") function is used to acquire copies of words from absolute indices.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M_significant, bool... ViewMaskOther>
	void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,_uint_view<parts_size_bits,M_significant,ViewMaskOther...>> arg, uint8_t part_i)
    {
        for(uint32_t i(0); i < this->parts_size; ++i)
            this->value[part_i][i] = arg.value.atc(part_i);
    }

    /*
        Initialization of a 'part_i' part from unsigned integer.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,uint64_t> value, uint8_t part_i)
    {
        constexpr uint32_t significant_words_in_curr_part = _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(part_i);
        constexpr uint32_t significant_bits_in_part = significant_words_in_curr_part * this->word_bits;
        for(uint32_t shift(0); shift < significant_bits_in_part; shift += this->word_bits)
        {
            uint16_t l = shift < 64 ? (value.value >> shift) & uint64_t(0xffff) : 0;
            this->value[part_i][shift / this->word_bits] = l;
        }
        std::fill(this->value[part_i]+significant_words_in_curr_part,this->value[part_i]+this->parts_size,0);
    }

    /*
        Initialization of a 'part_i' part from a pair of iterators [begin, end).
        Iterators must be able to be resolved to boolean values.
        Each element of this iterators range is treated as a single bit.
        First iterator must be corresponded to the least significant bit.
        Last unsignificant bits can be safely omitted. Exceeding bits will be discarded.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<__globals::Iterator _Iterator>
    void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,std::pair<_Iterator,_Iterator>> iters, uint8_t part_i)
    {
        constexpr uint32_t significant_words_in_curr_part =  _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(part_i);
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

    /*
        Initialization of a 'part_i' part from another instance and part index.
        Constructed instance is a view on a corresponding part of given instance.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewMaskOther>
    void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<true,std::pair<_uint_view<M,M_significant,ViewMaskOther...>,uint8_t>> other, uint8_t part_i) requires(_uint_view<M,M_significant,ViewMaskOther...>::parts_size == _uint_view<N,N_significant,ViewsMask...>::words_num)
    {
        constexpr uint32_t significant_words_in_curr_part = _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(part_i);
        constexpr uint32_t significant_words_in_part_other = _uint_view<M,M_significant,ViewMaskOther...>::significant_words_in_part(part_i);
        static_assert(significant_words_in_curr_part <= significant_words_in_part_other, "Can't set view on instance with number of significant words less than in current part.");
        this->value[part_i] = other.value.first.value[other.value.second];
    }

    /*
        Initialization of a 'part_i' part from another instance and part index.
        Constructed instance is a copy of a corresponding part of given instance.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewMaskOther>
    void _uint_view<N,N_significant,ViewsMask...>::init(ViewRefStorage<false,std::pair<_uint_view<M,M_significant,ViewMaskOther...>,uint8_t>> other, uint8_t part_i) requires(_uint_view<M,M_significant,ViewMaskOther...>::parts_size == _uint_view<N,N_significant,ViewsMask...>::words_num)
    {
        constexpr uint32_t significant_words_in_curr_part = _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(part_i);
        constexpr uint32_t significant_words_in_part_other = _uint_view<M,M_significant,ViewMaskOther...>::significant_words_in_part(part_i);
        constexpr uint32_t significant_words_to_copy = std::min(significant_words_in_curr_part,significant_words_in_part_other);
        std::copy(other.value.first.value[other.value.second],other.value.first.value[other.value.second]+significant_words_to_copy,this->value[part_i]);
    }

    // === END 'init' FUNCTIONS COLLECTION ===



    /*
        Destructor.
        Basically, a wrapper for 'destroy' that handles 'ViewsMask' unrolling.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>::~_uint_view()
    {
        this->destroy<ViewsMask...>();
    }


    /*
        Unrolls 'ViewsMask' and for every 'false' ("not a view") entry in it deletes corresponding part.
        This ensures that only owned memory will be destroyed.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<bool View, bool... ViewsMaskTail>
    void _uint_view<N,N_significant,ViewsMask...>::destroy()
    {
        constexpr uint8_t tail_size = sizeof...(ViewsMaskTail);
        constexpr uint8_t part_i = parts_num-tail_size-1;
        if(!View && this->value[part_i])
            delete[] this->value[part_i];
        if constexpr(tail_size > 0)
            this->destroy<ViewsMaskTail...>();
    }


    /*
        Copy constructor.
        Wrapper for'copy' routine (see bellow).
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>::_uint_view(const _uint_view<N,N_significant,ViewsMask...>& x)
    {
        _uint_view<N,N_significant,ViewsMask...>::template uint_other<N,N_significant,ViewsMask...>::template copy(*this,x);
    }


    /*
        Copy constructor but template.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_view<N,N_significant,ViewsMask...>::_uint_view(const _uint_view<M,M_significant,ViewsMaskOther...>& x)
    {
        _uint_view<N,N_significant,ViewsMask...>::template uint_other<M,M_significant,ViewsMaskOther...>::template copy<ViewsMaskOther...>(*this,x);
    }


    /*
        Copy assignment operator.
        Wrapper for'copy' routine (see bellow).
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>& _uint_view<N,N_significant,ViewsMask...>::operator=(const _uint_view<N,N_significant,ViewsMask...>& x)
    {
        _uint_view<N,N_significant,ViewsMask...>::template uint_other<N,N_significant,ViewsMask...>::template copy<ViewsMask...>(*this,x);
        return *this;
    }

    
    /*
        Copy assignment operator but template.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_view<N,N_significant,ViewsMask...>& _uint_view<N,N_significant,ViewsMask...>::operator=(const _uint_view<M,M_significant,ViewsMaskOther...>& x)
    {
        _uint_view<N,N_significant,ViewsMask...>::template uint_other<M,M_significant,ViewsMaskOther...>::template copy(*this,x);
        return *this;
    }


    /*
        This function performs copying from one template instance to another.
        Does it according to 'origin's 'ViewsMask', so parts with view flag set receive data from 'other' as references.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    template<bool View, bool... ViewsMaskOtherTail>
    void _uint_view<N,N_significant,ViewsMask...>::uint_other<M,M_significant,ViewsMaskOther...>::copy(_uint_view<N,N_significant,ViewsMask...>& origin, const _uint_view<M,M_significant,ViewsMaskOther...>& other)
    {
        constexpr uint8_t other_tail_size = sizeof...(ViewsMaskOtherTail);
        constexpr uint8_t other_part_i = _uint_view<M,M_significant,ViewsMaskOther...>::parts_num-other_tail_size-1;
        constexpr uint32_t current_word_i = _uint_view<M,M_significant,ViewsMaskOther...>::parts_size*other_part_i;

        if constexpr(current_word_i >= _uint_view<N,N_significant,ViewsMask...>::words_num)
            return;

        constexpr uint8_t origin_part_i = current_word_i / _uint_view<N,N_significant,ViewsMask...>::parts_size;
        constexpr uint32_t origin_word_in_part_i = current_word_i % _uint_view<N,N_significant,ViewsMask...>::parts_size;

        // This routine is able to copy from instance with any template arguments to instance with any template arguments.
        // We copy from 'other' part-by-part and this is the number of 'origin's parts inside one 'other's part.
        constexpr uint8_t origin_parts_in_one_other_num = std::max(_uint_view<M,M_significant,ViewsMaskOther...>::parts_size / _uint_view<N,N_significant,ViewsMask...>::parts_size,1UL);

        // Checks whether 'view' flag is set for any origin parts that will be populated with values from 'other' or not.
        // If yes, then parts must have equal sizes
        // if no, then we can safely copy from 'other'
        constexpr bool origin_current_view = _uint_view<N,N_significant,ViewsMask...>::any_view<origin_part_i,origin_part_i+origin_parts_in_one_other_num>();

        if constexpr(origin_current_view)
        {
            static_assert(_uint_view<M,M_significant,ViewsMaskOther...>::parts_size == _uint_view<N,N_significant,ViewsMask...>::parts_size, "Can set view only on part with the same size.");
            origin.value[origin_part_i] = other.value[other_part_i];
        }
        else
        {
            // Could write it in more compact form but supposed that it would be better to compile 'for' loop only when it is necessary
            if constexpr(origin_parts_in_one_other_num == 1)
            {
                // If 'origin's part is equal or larger, then just copy from other
                constexpr uint32_t part_to_copy_size = _uint_view<M,M_significant,ViewsMaskOther...>::parts_size;
                std::copy(
                    other.value[other_part_i],
                    other.value[other_part_i]+part_to_copy_size,
                    origin.value[origin_part_i]+origin_word_in_part_i
                );
            }
            else
            {
                // If 'origin's is smaller, then
                constexpr uint32_t part_to_copy_size = _uint_view<N,N_significant,ViewsMask...>::parts_size;
                // This is just for debugging purposes. Number of parts is always a power of 2, hence if 'origin's part size is smaller then i
                static_assert(origin_word_in_part_i == 0, "If origin part is smaller than copying must start at the base of its part");
                // Iterate over 'origin's parts filling them with conten of (one) current 'other's part
                for(uint8_t i(0); i < origin_parts_in_one_other_num; ++i)
                {
                    std::copy(
                        other.value[other_part_i]+part_to_copy_size*i,
                        other.value[other_part_i]+part_to_copy_size*(i+1),
                        origin.value[origin_part_i+i]
                    );
                }
            }
        }
        if constexpr(other_tail_size > 0)
            _uint_view<N,N_significant,ViewsMask...>::uint_other<M,M_significant,ViewsMaskOther...>::copy<ViewsMaskOtherTail...>(origin,other);
    }



    /*
        Move constructor.
        Wrapper for 'move'.
        Only allowed for instances with the same template arguments
        Could be implemented with 'for' loop. Possibly it would be even better.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>::_uint_view(_uint_view<N,N_significant,ViewsMask...>&& other)
    {
        this->move<ViewsMask...>(other);
    }


    /*
        Just moves content from 'other' to 'this' regardless 'ViewsMask'.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<bool View, bool... ViewsMaskTail>
    void _uint_view<N,N_significant,ViewsMask...>::move(_uint_view<N,N_significant,ViewsMask...>&& other)
    {
        constexpr uint8_t tail_size = sizeof...(ViewsMaskTail);
        constexpr uint8_t part_i = _uint_view<N,N_significant,ViewsMask...>::parts_num-tail_size-1;
        this->value[part_i] = other.value[part_i];
        other.value[part_i] = nullptr;
        if constexpr(tail_size > 0)
            this->move<ViewsMaskTail...>(other);
    }


    /*
        Returns actual view on a part of non-const instance, so one can edit actual part through acquired uint instance.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t part_i>
    _uint_view<N,N_significant,ViewsMask...>::_uint_part_view_t<part_i>
    _uint_view<N,N_significant,ViewsMask...>::get_part_view()
    {
        constexpr uint8_t parts_total = sizeof...(ViewsMask);
        static_assert(part_i < parts_total, "Part index out of range");

        constexpr uint32_t view_bits = _uint_view<N,N_significant,ViewsMask...>::parts_size_bits;
        constexpr uint32_t view_significant_bits = _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(part_i);

        return _uint_part_view_t<part_i>(std::pair<_uint_view<N,N_significant,ViewsMask...>,uint8_t>{*this,part_i});
    }

    /*
        Returns copy of part of const instance.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t part_i>
    _uint_view<N,N_significant,ViewsMask...>::_uint_part_copy_t<part_i> 
    _uint_view<N,N_significant,ViewsMask...>::get_part_view() const
    {
        constexpr uint8_t parts_total = sizeof...(ViewsMask);
        static_assert(part_i < parts_total, "Part index out of range");

        constexpr uint32_t view_bits = _uint_view<N,N_significant,ViewsMask...>::parts_size_bits;
        constexpr uint32_t view_significant_bits = _uint_view<N,N_significant,ViewsMask...>::significant_words_in_part(part_i);

        return _uint_part_copy_t<part_i>(std::pair<_uint_view<N,N_significant,ViewsMask...>,uint8_t>{*this,part_i});
    }


    /*
        Casts instance to another with another shape and one non-view part
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant>
    _uint_view<N,N_significant,ViewsMask...>::operator _uint_view<M,M_significant,false>() const
    {
        _uint_view<M,M_significant,false> res;
        constexpr uint32_t min_significant_bits = std::min(_uint_view<N,N_significant,ViewsMask...>::significant_words_num,_uint_view<M,M_significant,false>::significant_words_num);
        for(uint32_t i(0); i < min_significant_bits; ++i)
            res.at(i) = this->atc(i);
        std::fill(res.value[0]+min_significant_bits,res.value[0]+_uint_view<M,M_significant,false>::words_num,0);

        return res;
    }


    /*
        Returns a string with all zeros on the left removed.
    */
    std::string ltrim(const std::string &s)
    {
        size_t start = s.find_first_not_of("0");
        return (start == std::string::npos) ? "0" : s.substr(start);
    }


    /*
        Converts the instance to hex-string.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    _uint_view<N,N_significant,ViewsMask...>::operator std::string() const
    {
        std::string res;
        for(int i(_uint_view<N,N_significant,ViewsMask...>::significant_words_num-1); i >= 0; --i)
        {
            std::stringstream stream;
            stream.fill('0');
            stream.width(_uint_view<N,N_significant,ViewsMask...>::word_bits/4);
            stream <<  std::hex << this->atc[i];
            res += stream.str();
        }
        res = ltrim(res);

        return res;
    }


    /*
        Checks if 'this' and another instance are equal.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    bool _uint_view<N,N_significant,ViewsMask...>::operator==(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const
    {
        bool res = false;
        constexpr uint32_t common_part_size = std::min(_uint_view<N,N_significant,ViewsMask...>::significant_words_num,_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);
        // Check if common significant part is equal
        bool common_part_is_equal = true;
        for(uint32_t i(0); i < common_part_size; ++i)
        {
            if(other.at(i) != this->at(i))
            {
                common_part_is_equal = false;
                break;
            }
        }
        // Check if in the current instance there are no non-zero values after common significant part
        bool none_after_common_1 = true;
        for(uint32_t i(common_part_size); i < _uint_view<N,N_significant,ViewsMask...>::significant_words_num; ++i)
        {
            if(this->at(i))
            {
                none_after_common_1 = false;
                break;
            }
        }
        // Check if in anohter instance there are no non-zero values after common significant part
        bool none_after_common_2 = true;
        for(uint32_t i(common_part_size); i < _uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num; ++i)
        {
            if(other.at(i))
            {
                none_after_common_1 = false;
                break;
            }
        }
        return common_part_is_equal and none_after_common_1 and none_after_common_2;
    }


    /*
        Performs addition of 'this' and another template instance in-place.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_view<N,N_significant,ViewsMask...>& _uint_view<N,N_significant,ViewsMask...>::operator+=(const _uint_view<M,M_significant,ViewsMaskOther...>& b) requires(N_significant == N)
    {
        uint16_t r = 0;
        constexpr uint32_t significant_ops = std::min(_uint_view<N,N_significant,ViewsMask...>::significant_words_num,_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);
        for(uint32_t i = 0; i < significant_ops; ++i)
        {
            uint32_t word_sum = (uint32_t)this->at(i) + (uint32_t)b.at(i) + (uint32_t)r;
            this->at(i) = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint_view<N,N_significant,ViewsMask...>::word_bits;
        }
        for(uint32_t i = significant_ops; i < _uint_view<N,N_significant,ViewsMask...>::words_num; ++i)
        {
            if(r == 0)
                break;
            uint32_t word_sum = (uint32_t)this->at(i) + (uint32_t)r;
            this->at(i) = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint_view<N,N_significant,ViewsMask...>::word_bits;
        }

        return *this;
    }


    /*
        Performs subtraction of 'this' and another template instance in-place.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_view<N,N_significant,ViewsMask...>& _uint_view<N,N_significant,ViewsMask...>::operator-=(const _uint_view<M,M_significant,ViewsMaskOther...>& b) requires(N_significant == N)
    {
        uint16_t r = 0;
        constexpr uint32_t significant_ops = std::min(_uint_view<N,N_significant,ViewsMask...>::significant_words_num,_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);
        for(uint32_t i(0); i < significant_ops; ++i)
        {
            uint32_t sub_total = (uint32_t)b.at(i) + (uint32_t)r;
            r = sub_total > this->at(i) ? 1 : 0;
            this->at(i) = ((uint32_t)(r ? _uint_view<N,N_significant,ViewsMask...>::base : 0) - (uint32_t)sub_total) + (uint32_t)this->at(i);
        }
        for(uint32_t i(significant_ops); i < _uint_view<N,N_significant,ViewsMask...>::words_num; ++i)
        {
            if(r == 0)
                break;
            uint32_t sub_total = (uint32_t)r;
            r = sub_total > this->at(i) ? 1 : 0;
            this->at(i) = ((uint32_t)(r ? _uint_view<N,N_significant,ViewsMask...>::base : 0) - (uint32_t)sub_total) + (uint32_t)this->at(i);
        }

        return *this;
    }


    /*
        Performs multiplication of 'this' and another template instance in-place using school-grade algorithm.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_view<N,N_significant,ViewsMask...>& _uint_view<N,N_significant,ViewsMask...>::operator*=(const _uint_view<M,M_significant,ViewsMaskOther...>& other) requires(N < __globals::karatsuba_bound and N_significant == N)
    {
        constexpr uint32_t res_words_num = _uint_view<N,N_significant,ViewsMask...>::words_num;
        uint64_t pseudo_res[_uint_view<N,N_significant,ViewsMask...>::words_num];
        std::fill(pseudo_res,pseudo_res+_uint_view<N,N_significant,ViewsMask...>::words_num,0);
        constexpr uint32_t significant_ops = std::min(_uint_view<N,N_significant,ViewsMask...>::words_num,
                                            _uint_view<N,N_significant,ViewsMask...>::significant_words_num + _uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);

        for(uint32_t i(0); i < _uint_view<N,N_significant,ViewsMask...>::significant_words_num; ++i)
        {
            for(uint32_t j(0); j < std::min(_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num,_uint_view<N,N_significant,ViewsMask...>::words_num-i); ++j)
            {
                uint32_t a_v = this->at(i);
                uint32_t b_v = other.at(j);
                pseudo_res[i+j] += a_v*b_v;
            }
        }
        for(uint32_t i(1); i < significant_ops; ++i)
            pseudo_res[i] += pseudo_res[i-1] >> 16;
        for(uint32_t i(0); i < _uint_view<N,N_significant,ViewsMask...>::words_num; ++i)
            this->at(i) = pseudo_res[i] & uint16_t(0xffff);
        return *this;
    }


    /*
        Performs multiplication of 'this' and another template instance in-place using Karatsuba algorithm.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_view<N,N_significant,ViewsMask...>& _uint_view<N,N_significant,ViewsMask...>::operator*=(const _uint_view<M,M_significant,ViewsMaskOther...>& other) requires(N >= __globals::karatsuba_bound and N_significant == N)
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


    /*
        Performs addition of 'this' and another template instance.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_add_out_t<N,N_significant,M,M_significant> _uint_view<N,N_significant,ViewsMask...>::operator+(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const
    {
        _uint_add_out_t<N,N_significant,M,M_significant> res;
        constexpr uint32_t significant_ops_min = std::min(_uint_view<N,N_significant,ViewsMask...>::significant_words_num,_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);
        constexpr uint32_t significant_ops_max = std::max(_uint_view<N,N_significant,ViewsMask...>::significant_words_num,_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);
        uint16_t r = 0;
        for(uint32_t i = 0; i < significant_ops_min; ++i)
        {
            uint32_t word_sum = (uint32_t)this->at(i) + (uint32_t)other.at(i) + (uint32_t)r;
            res.at(i) = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint_view<N,N_significant,ViewsMask...>::word_bits;
        }
        for(uint32_t i = significant_ops_min; i < significant_ops_max; ++i)
        {
            uint32_t word_sum = r;
            if constexpr(_uint_view<N,N_significant,ViewsMask...>::significant_words_num > significant_ops_min)
                word_sum += (uint32_t)this->at(i);
            else
                word_sum += (uint32_t)other.at(i);
            res.at(i) = word_sum & uint16_t(0xffff);
            r = word_sum >> _uint_view<N,N_significant,ViewsMask...>::word_bits;
        }
        if constexpr(significant_ops_max < _uint_add_out_t<N,N_significant,M,M_significant>::words_num)
        {
            res.at(significant_ops_max) = r;
            for(uint32_t i(significant_ops_max+1); i < _uint_add_out_t<N,N_significant,M,M_significant>::words_num; ++i)
                res.at(i) = 0;
        }

        return res;
    }


    /*
        Performs subtraction of 'this' and another template instance.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_sub_out_t<N,N_significant,M,M_significant> _uint_view<N,N_significant,ViewsMask...>::operator-(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const
    {
        _uint_sub_out_t<N,N_significant,M,M_significant> res;
        constexpr uint32_t significant_ops_min = std::min(_uint_view<N,N_significant,ViewsMask...>::significant_words_num,_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);
        constexpr uint32_t significant_ops_max = std::max(_uint_view<N,N_significant,ViewsMask...>::significant_words_num,_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);
        uint16_t r = 0;
        for(uint32_t i(0); i < significant_ops_min; ++i)
        {
            uint32_t sub_total = (uint32_t)other.at(i) + (uint32_t)r;
            r = sub_total > this->at(i) ? 1 : 0;
            res.at(i) = ((uint32_t)(r ? _uint_view<N,N_significant,ViewsMask...>::base : 0) - (uint32_t)sub_total) + (uint32_t)this->at(i);
        }
        for(uint32_t i = significant_ops_min; i < significant_ops_max; ++i)
        {
            uint32_t sub_total = r;
            if constexpr(_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num > significant_ops_min)
                sub_total += (uint32_t)other.at(i);
            r = sub_total > this->at(i) ? 1 : 0;
            uint32_t sub_val = ((uint32_t)(r ? _uint_view<N,N_significant,ViewsMask...>::base : 0) - (uint32_t)sub_total);
            if constexpr(_uint_view<N,N_significant,ViewsMask...>::significant_words_num > significant_ops_min)
                sub_val += (uint32_t)this->at(i);
            res.at(i) = sub_val;
        }
        for(uint32_t i(significant_ops_max); i < _uint_sub_out_t<N,N_significant,M,M_significant>::words_num; ++i)
            res.at(i) = (uint32_t)0x10000-r;

        return res;
    }


    /*
        Performs multiplication of 'this' and another template instance using school-grade algorithm.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_mul_out_t<N,N_significant,M,M_significant> _uint_view<N,N_significant,ViewsMask...>::operator*(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const requires (N < __globals::karatsuba_bound)
    {
        _uint_mul_out_t<N,N_significant,M,M_significant> res;
        uint64_t pseudo_res[_uint_mul_out_t<N,N_significant,M,M_significant>::words_num];
        constexpr uint32_t significant_ops = std::min(_uint_mul_out_t<N,N_significant,M,M_significant>::words_num,
                                            _uint_view<N,N_significant,ViewsMask...>::significant_words_num+_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num);
        std::fill(pseudo_res,pseudo_res+significant_ops,0ULL);
        for(uint32_t i(0); i < _uint_view<N,N_significant,ViewsMask...>::significant_words_num; ++i)
        {
            for(uint32_t j(0); j < std::min(_uint_view<M,M_significant,ViewsMaskOther...>::significant_words_num,_uint_mul_out_t<N,N_significant,M,M_significant>::words_num-i); ++j)
            {
                uint32_t a_v = this->at(i);
                uint32_t b_v = other.at(j);
                pseudo_res[i+j] += a_v*b_v;
            }
        }

        for(uint32_t i(1); i < _uint_mul_out_t<N,N_significant,M,M_significant>::words_num; ++i)
            pseudo_res[i] += pseudo_res[i-1] >> 16;
        for(uint32_t i(0); i < significant_ops; ++i)
            res.value[i] = pseudo_res[i] & uint16_t(0xffff);

        return res;
    }


    /*
        Performs multiplication of 'this' and another template instance using Karatsuba algorithm.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint32_t M, uint32_t M_significant, bool... ViewsMaskOther>
    _uint_mul_out_t<N,N_significant,M,M_significant> _uint_view<N,N_significant,ViewsMask...>::operator*(const _uint_view<M,M_significant,ViewsMaskOther...>& other) const requires (N >= __globals::karatsuba_bound)
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


    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
    std::ostream& operator<<(std::ostream& out, const _uint_view<N,N_significant,ViewsMask...>& x)
    {
        out << (std::string)x;
        return out;
    }


    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
    std::istream& operator>>(std::istream& in, _uint_view<N,N_significant,ViewsMask...>& x)
    {
        std::string hex_uint;
        in >> hex_uint;
        x = _uint_view<N,N_significant,ViewsMask...>(hex_uint);

        return in;
    }
}

#endif