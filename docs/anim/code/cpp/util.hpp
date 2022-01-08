#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.

#include <assert.h>
#include <random>
#include <stdexcept>
#include <string>

#define CPPUTIL_FAIL( s ) ::cpp::util::fail( std::string( __func__ ) + " - " + (s) )

namespace cpp::util {
    using   std::random_device, std::mt19937, std::uniform_int_distribution,
            std::exception, std::runtime_error,
            std::string;

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

}  // namespace cpp::util
