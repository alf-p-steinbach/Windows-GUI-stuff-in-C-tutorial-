#pragma once

#include <random>

namespace cpp_util {
    using   std::random_device, std::mt19937, std::uniform_int_distribution;
    
    template< class T > using T_ = T;
    template< class T > using P_ = T*;
    template< class T > using R_ = T&;
    
    constexpr auto squared( const int v ) -> int { return v*v; }

    struct Range
    {
        int     first;
        int     last;
    };
    
    inline auto is_in( const Range& range, const int v )
        -> bool
    { return range.first <= v and v <= range.last; }

    inline auto random_up_to( const int beyond )
        -> int
    {
        static random_device    entropy;
        static mt19937          bits( entropy() );
        return uniform_int_distribution<>( 0, beyond - 1 )( bits );
    }

}  // namespace cpp_util
