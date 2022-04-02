#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.

#include <assert.h>         // assert
#include <functional>       // std::reference_wrapper
#include <iterator>         // std::size
#include <random>           // std::(random_device, mt19937, uniform_int_distribution)
#include <stdexcept>        // std::(exception, runtime_error)
#include <string>           // std::string
#include <type_traits>      // std::(enable_if_t, is_same_v)

#define CPPUTIL_FAIL( s ) ::cpp::util::fail( std::string( __func__ ) + " - " + (s) )

#define CPPUTIL_WITH( name, initializer ) \
    if( auto&& name = initializer; ((void) name, true) )

namespace cpp::util {
    using   std::reference_wrapper,
            std::size,
            std::random_device, std::mt19937, std::uniform_int_distribution,
            std::exception, std::runtime_error,
            std::string,
            std::enable_if_t, std::is_same_v;

    constexpr auto utf8_is_the_execution_character_set()
        -> bool
    {
        constexpr auto& slashed_o = "ø";
        return (sizeof( slashed_o ) == 3 and slashed_o[0] == '\xC3' and slashed_o[1] == '\xB8');
    }

    constexpr auto hopefully( const bool condition ) -> bool { return condition; }
    inline auto fail( const string& message ) -> bool { throw runtime_error( message ); }

    struct Success{} success;
    
    template<
        class Value,
        class = enable_if_t< is_same_v< Value, bool > >     // Don't want implicit conversions.
        >
    inline auto operator>>( Success, const Value v ) -> bool { return v; }

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
    
    template< class... Types >
    struct Is_one_of_
    {
        template< class U >
        struct Result_
        {
            static constexpr bool value = (... or is_same_v< U, Types >);
        };
    };
        
    namespace impl::types {
        // Logic to find the index of the first T in a list of types, or -1 if none.

        constexpr auto successor_if_not_negative( const int v ) -> int { return (v < 0? v : 1 + v); }

        template< template <class> class Is_match_, class... Args > struct First_match_;
        
        template< template <class> class Is_match_ > struct First_match_< Is_match_> { enum{ index = -1 }; };

        template< template <class> class Is_match_, class First, class... More_args >
        struct First_match_< Is_match_, First, More_args... >
        {
            enum
            { index = Is_match_< First >::value
                ? 0
                : successor_if_not_negative( First_match_< Is_match_, More_args... >::index )
            };
        };

        template< class T, class... Args >
        constexpr int index_of_first_ = First_match_< Is_one_of_<T>::template Result_, Args... >::index;

        template< template<class> class Is_match_, class... Args >
        constexpr int index_of_first_match_ = First_match_< Is_match_, Args... >::index;
    }  // namespace impl::types

    template< class... Types >
    struct Types_
    {
        static constexpr int count = static_cast<int>( sizeof...( Types ) );

        template< class T >
        static constexpr bool contain_ = (... or is_same_v< T, Types >);

        template< class T >
        static constexpr int index_of_first_ = impl::types::index_of_first_< T, Types... >;

        template< class... T >
        static constexpr int index_of_first_of_ =
            impl::types::index_of_first_match_< Is_one_of_<T...>::template Result_, Types... >;
    };

    template< class T, class Arg >
    constexpr auto is_of_type_( Arg ) -> bool { return is_same_v< T, Arg >; }

    template< class T, class U >
    constexpr bool is_same_type_ = is_same_v< T, U >;

    template< class T >
    class Explicit_ref_:
        public reference_wrapper<T>
    {
    public:
        template< class U >
        Explicit_ref_( reference_wrapper<U>&& ref ): reference_wrapper<T>( ref.get() ) {}
    };
}  // namespace cpp::util
