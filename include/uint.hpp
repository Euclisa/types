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

        template<uint8_t ViewIndBegin, uint8_t ViewIndEnd, bool View, bool... ViewsMaskTail>
        static constexpr bool any_view();

        static constexpr uint32_t significant_words_in_part(uint8_t part_i);

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
        uint16_t atc(uint32_t i);

        template<uint8_t ViewInd>
        static constexpr bool get_view();

        template<uint8_t ViewIndBegin, uint8_t ViewIndEnd>
        static constexpr bool any_view();

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
        Service function that resolves absolute index to pair (part_index, word_inside_part_index) and outputs refernce to corresponding word.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    uint16_t& _uint_view<N,N_significant,ViewsMask...>::at(uint32_t i)
    {
        return this->value[i/this->parts_size][i%this->parts_size];
    }

    /*
        Same as above but returns a copy.
    */
    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    uint16_t _uint_view<N,N_significant,ViewsMask...>::atc(uint32_t i)
    {
        return this->value[i/this->parts_size][i%this->parts_size];
    }



    /*
        These two static functions are used to get a particular element of 'ViewsMask'.
        First one is private and should only be used wrapped with second one, which only takes index as an argument.
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

    template<uint32_t N, uint32_t N_significant, bool... ViewsMask>
        requires(N >= 16 and __globals::is_power_2(N) and N_significant <= N and N_significant % 16 == 0)
    template<uint8_t ViewInd>
    constexpr bool _uint_view<N,N_significant,ViewsMask...>::get_view()
    {
        static_assert(ViewInd < _uint_view<N,N_significant,ViewsMask...>::parts_num,"Index out of range");
        return _uint_view<N,N_significant,ViewsMask...>::get_view<ViewInd,ViewsMask...>();
    }


    /*
        These two static functions check if at least one entry in 'ViewsMask' is 'true'.
        First one is private and should only be used wrapped with second one, which only takes indices as arguments.
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
    _uint_view<N,N_significant,ViewsMask...>::_uint_view(InitArgs... args)
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
        if(!View)
            delete[] this->value[part_i];
        if constexpr(tail_size > 0)
            this->destroy<ViewsMaskTail...>();
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