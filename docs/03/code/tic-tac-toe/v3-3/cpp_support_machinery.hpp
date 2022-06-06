#pragma once
#include <algorithm>        // std::find
#include <iterator>         // std::(begin, end)

namespace cpp_support_machinery {
    using   std::find,
            std::begin, std::end;

    template< class Value, class Sequence >
    inline auto contains( const Value& v, const Sequence& seq )
        -> bool
    {
        const auto it_first     = begin( seq );
        const auto it_beyond    = end( seq );
        return (find( it_first, it_beyond, v ) != it_beyond);
    }

}  // namespace cpp_support_machinery
