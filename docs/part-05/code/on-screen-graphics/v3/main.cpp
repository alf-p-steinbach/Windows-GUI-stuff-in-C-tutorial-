﻿# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi"). v3
#include <winapi/gdi/color_names.hpp>
#include <winapi/gdi/device-contexts.hpp>
#include <wrapped-winapi/windows-h.hpp>
namespace gdi = winapi::gdi;
using gdi::Dc, gdi::Screen_dc, gdi::Pen_color, gdi::Brush_color;

void draw_on( const Dc& canvas, const RECT& area )
{
    using namespace winapi::gdi::color_names;
    canvas.use( Brush_color( blue ) ).fill( area );
    canvas.use( Brush_color( orange ), Pen_color( yellow ) ).draw( Ellipse, area );
}

auto main() -> int
{
    draw_on( Screen_dc(), RECT{ 10, 10, 10 + 400, 10 + 400 } );
}
