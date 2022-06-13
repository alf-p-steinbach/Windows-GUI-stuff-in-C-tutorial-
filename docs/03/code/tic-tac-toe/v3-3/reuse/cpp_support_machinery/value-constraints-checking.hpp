#pragma once
#include <algorithm>        // std::find
#include <iterator>         // std::(begin, end)

namespace cpp_support_machinery {
    using   std::find,
            std::begin, std::end;

    template< class Value, class Container >
    inline auto contains( const Value& v, const Container& c )
        -> bool
    {
        const auto it_first     = begin( c );
        const auto it_beyond    = end( c );
        return (find( it_first, it_beyond, v ) != it_beyond);
    }

    struct Int_range { int first; int last; };
    
    constexpr auto is_in( const Int_range& range, const int v )
        -> bool
    { return (range.first <= v and v <= range.last); }
}  // namespace cpp_support_machinery
