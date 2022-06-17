#pragma once
#include <wrapped-windows-h.hpp>        // Safer and faster. Safe = e.g. no `small` macro.
#include <cpp_support_machinery.hpp>    // cpp_support_machinery::*

#include <assert.h>             // assert
#include <stdint.h>             // INT16_MAX

namespace winapi_support_machinery:: resource {
    namespace csm = cpp_support_machinery;

    using   csm::is_in, csm::Int_range;

    class Id
    {
        int         m_value;

    public:
        static constexpr int max_value = INT16_MAX;     // INT16_MAX from <stdint.h>.

        Id( const int value ):
            m_value( value )
        { assert( is_in( Int_range{ 1, max_value }, value ) ); }

        auto as_number() const -> int { return m_value; }
        auto as_pseudo_ptr() const -> const char* { return MAKEINTRESOURCE( m_value ); }
        operator int() const { return as_number(); }

        friend auto compare( const Id a, const Id b ) -> int { return a.m_value - b.m_value; }
    };

    inline const HINSTANCE in_this_executable = GetModuleHandle( nullptr );

    class Location
    {
        Id          m_id;
        HINSTANCE   m_module;

    public:
        Location( const Id id, const HINSTANCE module = in_this_executable ):
            m_id( id ), m_module( module )
        {}

        auto id() const     -> const Id&    { return m_id; }
        auto module() const -> HINSTANCE    { return m_module; }
    };
}  // namespace winapi_support_machinery:: resource
