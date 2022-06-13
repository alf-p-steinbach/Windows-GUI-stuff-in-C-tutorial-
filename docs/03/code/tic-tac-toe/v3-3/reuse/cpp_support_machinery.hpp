#pragma once
#include <algorithm>        // std::find
#include <iterator>         // std::(begin, end)
#include <type_traits>      // std::is_same_v;

// Convenience include:
#include <initializer_list> // For using range-based `for` over initializer list.

namespace cpp_support_machinery {
    using   std::find,
            std::begin, std::end,
            std::is_same_v;

    struct Int_range { int first; int last; };
    
    constexpr auto is_in( const Int_range& range, const int v )
        -> bool
    { return (range.first <= v and v <= range.last); }

    template< class Value, class Container >
    inline auto contains_( const Value& v, const Container& c )
        -> bool
    {
        const auto it_first     = begin( c );
        const auto it_beyond    = end( c );
        return (find( it_first, it_beyond, v ) != it_beyond);
    }

    template< class Desired, class Actual >
    constexpr void assert_type_is_( const Actual& )
    {
        char _[2*is_same_v<Desired, Actual> - 1];   // Negative size if unequal types.
        (void) _;
    }
}  // namespace cpp_support_machinery
