# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi"). v3
#include <winapi/gdi/color_names.hpp>
#include <winapi/gdi/device-contexts.hpp>
namespace gdi = winapi::gdi;
using gdi::Dc, gdi::Screen_dc, gdi::Pen_color, gdi::Brush_color;

void draw_on( Dc& canvas, const RECT& area )
{
    using namespace winapi::gdi::color_names;
    canvas.bg( blue ).fill( area );
    canvas.bg( orange ).fg( yellow ).draw( Ellipse, area );
}

auto main() -> int
{
    Screen_dc dc;
    draw_on( dc, RECT{ 10, 10, 10 + 400, 10 + 400 } );
}
