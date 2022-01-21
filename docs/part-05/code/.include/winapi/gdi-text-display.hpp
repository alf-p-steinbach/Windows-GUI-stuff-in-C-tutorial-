#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/encoding-conversions.hpp>
#include <cpp/util.hpp>

#include    <assert.h>
#include    <string_view>       // std::(string_view, wstring_view)

namespace winapi::gdi {
    using cpp::util::int_size;
    using std::string_view, std::wstring_view;
    
    constexpr UINT  default_draw_format = DT_LEFT | DT_TOP | DT_NOPREFIX;

    inline auto draw_text(
        const HDC                   canvas,
        const wstring_view&         wide_s,
        RECT&                       area,
        const UINT                  format  = default_draw_format
        ) -> int
    { return DrawTextW( canvas, wide_s.data(), int_size( wide_s ), &area, format ); }

    inline auto draw_text(
        const HDC                   canvas,
        const string_view&          s,
        RECT&                       area,
        const UINT                  format  = default_draw_format
        ) -> int
    { return draw_text( canvas, to_utf16( s ), area, format ); }

}  // namespace winapi::gdi
