#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

namespace winapi::gdi {
    struct Color{ COLORREF value; Color( const COLORREF c ): value( c ) {} };

    struct Pen_color: Color
    {
        using Color::Color;
        void set_in( const HDC canvas ) const { SetDCPenColor( canvas, value ); }
    };

    struct Brush_color: Color
    {
        using Color::Color;
        void set_in( const HDC canvas ) const { SetDCBrushColor( canvas, value ); }
    };

    struct Gap_color: Color     // Gaps in pattern lines, and bg in text presentation.
    {
        using Color::Color;
        void set_in( const HDC canvas ) const
        {
            SetBkColor( canvas, value );  SetBkMode( canvas, OPAQUE );
        }
    };

    struct Transparent_gaps
    {
        void set_in( const HDC canvas ) const { SetBkMode( canvas, TRANSPARENT ); }
    };
}  // namespace winapi::gdi
