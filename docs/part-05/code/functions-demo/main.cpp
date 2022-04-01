# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi"). v3

// Demo of GDI drawing functions.
#include <winapi/gdi/color_names.hpp>
#include <winapi/gdi/device-contexts.hpp>
namespace gdi = winapi::gdi;
using gdi::Dc, gdi::Screen_dc, gdi::Pen_color, gdi::Brush_color;

#include <string_view>
using namespace std::string_view_literals;

void draw_on( Dc& canvas, const RECT& area )
{
    using namespace gdi::color_names;
    using gdi::Text_color;
    canvas.bg( blue ).fill( area );
    const POINT position = {area.left + 10, area.top + 10};
    canvas.use( Text_color( yellow ) ).draw( TextOut, position, "Hello, world!"sv );
}

auto main() -> int
{
    Screen_dc dc;
    draw_on( dc, RECT{ 10, 10, 10 + 400, 10 + 400 } );
}
