#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.

#include <assert.h>         // assert
#include <iterator>         // std::size
#include <random>           // std::(random_device, mt19937, uniform_int_distribution)
#include <stdexcept>        // std::(exception, runtime_error)
#include <string>           // std::string
#include <type_traits>      // std::is_same_v

#define CPPUTIL_FAIL( s ) ::cpp::util::fail( std::string( __func__ ) + " - " + (s) )

namespace cpp::util {
    using   std::size,
            std::random_device, std::mt19937, std::uniform_int_distribution,
            std::exception, std::runtime_error,
            std::string,
            std::is_same_v;

    constexpr auto utf8_is_the_execution_character_set()
        -> bool
    {
        constexpr auto& slashed_o = "ø";
        return (sizeof( slashed_o ) == 3 and slashed_o[0] == '\xC3' and slashed_o[1] == '\xB8');
    }

    constexpr auto hopefully( const bool condition ) -> bool { return condition; }
    inline auto fail( const string& message ) -> bool { throw runtime_error( message ); }

    struct No_copying
    {
        No_copying( const No_copying& ) = delete;
        auto operator=( const No_copying& ) -> No_copying& = delete;
        No_copying() {}
        
        No_copying( No_copying&& ) {}
        auto operator=( No_copying&& ) -> No_copying& { return *this; }
    };

    constexpr auto squared( const int v ) -> int { return v*v; }

    struct Range
    {
        int     first;
        int     last;
    };
    
    inline auto is_in( const Range& range, const int v )
        -> bool
    { return range.first <= v and v <= range.last; }


    inline auto random_in( const Range& range )
        -> int
    {
        static random_device    entropy;
        static mt19937          bits( entropy() );
        return uniform_int_distribution<>( range.first, range.last )( bits );
    }

    inline auto random_up_to( const int beyond )
        -> int
    { return random_in({ 0, beyond - 1 }); }

    template< class T >
    auto int_size( const T& c ) -> int { return static_cast<int>( size( c ) ); }

    template< class T >
    using Const_ = const T;
    
    template< class T, class... Types >
    constexpr bool includes_type_ = (... or is_same_v<T, Types>);

}  // namespace cpp::util
