#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi-headers/windows-h.hpp>

namespace winapi::gdi {
    struct Color{ COLORREF value; Color( const COLORREF c ): value( c ) {} };

    struct Pen_color: Color
    {
        using Color::Color;
        void set_in( const HDC canvas ) const { SetDCPenColor( canvas, value ); }
        static auto in( const HDC canvas ) -> Pen_color { return GetDCPenColor( canvas ); }
    };

    struct Brush_color: Color
    {
        using Color::Color;
        void set_in( const HDC canvas ) const { SetDCBrushColor( canvas, value ); }
        static auto in( const HDC canvas ) -> Brush_color { return GetDCBrushColor( canvas ); }
    };

    struct Gap_color: Color     // Gaps in pattern lines, and bg in text presentation.
    {
        using Color::Color;
        void set_in( const HDC canvas ) const
        {
            SetBkColor( canvas, value );  SetBkMode( canvas, OPAQUE );
        }
        static auto in( const HDC canvas ) -> Pen_color { return GetBkColor( canvas ); }
    };

    struct Transparent_gaps
    {
        void set_in( const HDC canvas ) const { SetBkMode( canvas, TRANSPARENT ); }
        
        static auto in( const HDC canvas )
            -> bool
        { return (GetBkMode( canvas ) == TRANSPARENT); }
    };

    struct Text_color: Color
    {
        using Color::Color;
        void set_in( const HDC canvas ) const { SetTextColor( canvas, value ); }
        static auto in( const HDC canvas ) -> Text_color { return GetTextColor( canvas ); }
    };
}  // namespace winapi::gdi
