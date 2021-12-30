#pragma once

#include <assert.h>
#include <random>

namespace cpp_util {
    using   std::random_device, std::mt19937, std::uniform_int_distribution;
    
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
    { return random_in({ 0, beyond + 1 }); }

}  // namespace cpp_util
