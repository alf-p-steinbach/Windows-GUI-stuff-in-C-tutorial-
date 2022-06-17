#pragma once
#include <type_traits>      // std::is_same_v;

namespace cpp_support_machinery {
    using   std::is_same_v;
    
    template< class Desired, class Actual >
    constexpr void assert_type_is_( const Actual& )
    {
        char _[2*is_same_v<Desired, Actual> - 1];   // Negative size if unequal types.
        (void) _;
    }

}  // namespace cpp_support_machinery
