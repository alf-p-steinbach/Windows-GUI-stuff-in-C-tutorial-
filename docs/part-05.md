# Windows GUI-stuff in C++: a tutorial.

## Part 5 – GDI: drawing, saving & presenting.

We’ll now use the [**GDI**](https://en.wikipedia.org/wiki/Graphics_Device_Interface), Windows’ original *graphics device interface*, to explore basic graphics: drawing shapes, lines and text, and learning how graphics drawing relates to Windows’ GUI mechanisms, such as drawing the background image of a window.

The GDI is simple and C-oriented, which is nice.

On the other hand it’s slow and produces low quality graphics. In particular the GDI doesn’t support [anti-aliasing](https://en.wikipedia.org/wiki/Spatial_anti-aliasing), and it doesn’t support [alpha channel transparency](https://en.wikipedia.org/wiki/Alpha_compositing), which are both strong reasons to later move on to the successor technologies [GDI+](https://en.wikipedia.org/wiki/Graphics_Device_Interface#Windows_XP) and [Direct 2D](https://en.wikipedia.org/wiki/Direct2D).

And unfortunately, also, the GDI doesn’t yet support UTF-8 based text for *drawing* text as graphics, as opposed to using controls to present text as we did in part 4.

UTF-8 based text drawing is a must for leveraging the previous part’s discussion of how to use UTF-8 as the `char` based text encoding. Unfortunately this part got too long to cover the issues, so international text drawing has to be postponed to the next part. Then we’ll just write our own wrappers over GDI’s UTF-16 based wide text drawing functions.

[some figure]

<!-- START doctoc generated TOC please keep comment here to allow auto update -->

<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [5.1. Draw directly on the screen to learn GDI basics.](#51-draw-directly-on-the-screen-to-learn-gdi-basics)
- [5.2 Use pseudo-mutable `DC_PEN` and `DC_BRUSH` stock objects to reduce verbosity.](#52-use-pseudo-mutable-dc_pen-and-dc_brush-stock-objects-to-reduce-verbosity)
- [5.3. Automate cleanup for device contexts and GDI objects.](#53-automate-cleanup-for-device-contexts-and-gdi-objects)
- [5.4. GDI + COM/OLE: save graphics to an image file.](#54-gdi--comole-save-graphics-to-an-image-file)
- [5.x. GDI + GUI: present graphics in a window.](#5x-gdi--gui-present-graphics-in-a-window)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

---

### 5.1. Draw directly on the screen to learn GDI basics.

With the GDI you can draw more or less directly on the screen.

Drawing directly on the screen is just a special case of drawing in a window, the slightly paradoxical case of “no window”.

This involves first calling `GetDC(0)` (with `0` for “no window”) to get a handle to a drawing surface covering the screen. In Windows terminology that’s called a **device context** for the screen, and the handle type is a `HDC`, handle to device context. In more general programming the equivalent of a Windows DC is often called a **canvas**, emphasizing that it’s used for painting.

On the *output side* — the right side in the figure below — a device context generates graphics in windows, in bitmap images, to printers, and to now archaic “.wmf” image files. So, you can use roughly the same code to generate graphics for all these destinations, much like with a C++ abstract base class with `virtual` functions. There is probably some historical reason why Microsoft incongruously refers to all these graphics destinations as “devices”; maybe originally only screens and printers were supported.

The side of a device context that such code relates to, its *input side*, mainly executes drawing commands such as calls of the `Ellipse` function, but it also receives and retains drawing attributes such as a **pen** that specifies attributes of lines (e.g. color, width and pattern), and such as a **brush** that specifies attributes of color fills, in particular the fill color.

<img alt="DC inputs and outputs" src="part-05/images/dc.png" width="500">

By default a device context typically has a black pen and a white brush. To draw a yellow circle filled with orange the code below uses the general GDI approach of (1) creating pen and brush objects, respectively yellow and orange; (2) **selecting** them in the device context; (3) drawing, which implicitly uses the selected objects; (4) deselecting the objects by selecting back in the original objects; and finally (5) destroying the objects. This is not necessarily inefficient, but, which is a general problem with the GDI, it’s quite verbose:

*[part-05/code/on-screen-graphics/v1/main.cpp](part-05/code/on-screen-graphics/v1/main.cpp)*:

```cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

// COLORREF is 32-bit unsigned.
namespace color {
    constexpr COLORREF  orange      = RGB( 0xFF, 0x80, 0x20 );
    constexpr COLORREF  yellow      = RGB( 0xFF, 0xFF, 0x20 );
    constexpr COLORREF  blue        = RGB( 0x00, 0x00, 0xFF );
}  // namespace color

auto main() -> int
{
    constexpr auto  no_window   = HWND( 0 );
    constexpr auto  area        = RECT{ 10, 10, 10 + 400, 10 + 400 };

    const HDC canvas = GetDC( no_window );
        // Fill the background with blue.
        const HBRUSH blue_brush = CreateSolidBrush( color::blue );
            FillRect( canvas, &area, blue_brush );
        DeleteObject( blue_brush );

        // Draw a yellow circle filled with orange.
        const HBRUSH    orange_brush                = CreateSolidBrush( color::orange );
            const HGDIOBJ   original_brush          = SelectObject( canvas, orange_brush );
                const HPEN      yellow_pen          = CreatePen( PS_SOLID, 1, color::yellow );
                    const HGDIOBJ   original_pen    = SelectObject( canvas, yellow_pen );
                        Ellipse( canvas, area.left, area.top, area.right, area.bottom );
                    SelectObject( canvas, original_pen );
                DeleteObject( yellow_pen );
            SelectObject( canvas, original_brush );
        DeleteObject( orange_brush );
    ReleaseDC( no_window, canvas );
}
```

Originally the effect was probably to actually draw directly on the screen, bypassing all the window management, and messing up the screen Real Good&trade;. But in Windows 11 there are layers of indirection and management interposed between the drawing calls and the screen output, in particular the [Desktop Window Manager](https://docs.microsoft.com/en-us/windows/win32/dwm/dwm-overview). There are some weird effects such as the graphics partially intruding in console windows, but such code still “works” and supports explorative programming.

![A filled ellips drawn directly on the screen](part-05/images/sshot-1.graphics-on-screen.cropped.png)

The effect is not entirely consistent between runs. Usually, if one doesn’t fill in the background, there’s a black background around the disk; but sometimes (rarely) there’s only the disk, then with essentially transparent background; and with some other graphics I’ve seen the background from one run of one program being retained as background for the graphics from another program, which was pretty confusing, huh where did *that* come from, before I understood what was going on. This is much like the rest of Windows 11’s functionality, i.e. it’s pretty shaky, not very reliable, depending on the phase of the moon, but the unreliability doesn’t really matter here.

For completeness, the `COLORREF` type is a 32-bit [RGB](https://en.wikipedia.org/wiki/RGB_color_model) **color** specification (3×8 = 24 bits used), and the `RECT` type is a simple struct with `left`, `top`, `right` and `bottom` integer value members.

In the default pixel coordinate system *x* increases left to right so that `left` ≤ `right` and *y* increases top to bottom so that `top` ≤ `bottom`.

Worth knowing: the `right` pixel column and the `bottom` pixel row are not part of a `RECT` rectangle. These are “beyond” values just like a C++ `o.end()` iterator. Thus the initialization `RECT{ 10, 10, 10 + 400, 10 + 400 }` creates a 400×400 rectangle.

Also, for completeness, here’s how to build and run with the Microsoft toolchain, Visual C++ (needs linking with “**gdi32**.lib”):

```txt
[T:\part-05\code\on-screen-graphics\v1\.build]
> set common-code=t:\part-05\code\.include

[T:\part-05\code\on-screen-graphics\v1\.build]
> cl /I %common-code% ..\main.cpp user32.lib gdi32.lib /Feb
main.cpp

[T:\part-05\code\on-screen-graphics\v1\.build]
> b_
```

Ditto, building and running with the MinGW toolchain, g++:

```txt
[T:\part-05\code\on-screen-graphics\v1\.build]
> set common-code=t:\part-05\code\.include

[T:\part-05\code\on-screen-graphics\v1\.build]
> g++ -I %common-code% ..\main.cpp -lgdi32

[T:\part-05\code\on-screen-graphics\v1\.build]
> a_
```

### 5.2 Use “DC colors” to reduce verbosity.

Instead of creating, selecting, using, unselecting and destroying pen and brush objects, as long as you don’t need fancy effects such as line patterns you can just change the device context’s **DC pen color** and **DC brush color**, via respectively `SetDCPenColor` and `SetDCBrushColor`. These colors are only *used* when the **stock objects** you get from respectively `GetStockObject(DC_PEN)` and `GetStockObject(DC_BRUSH)` are selected in the device context.

My experimentation showed that in Windows 11 these are not the default objects in a DC from `GetDC(0)`, so it’s necessary to explicitly select them:

*[part-05/code/on-screen-graphics/v2/main.cpp](part-05/code/on-screen-graphics/v2/main.cpp)*:

```cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/color_names.hpp>
#include <wrapped-winapi/windows-h.hpp>
namespace color = winapi::gdi::color_names;

void draw_on( const HDC canvas, const RECT& area )
{
    // Clear the background to blue.
    SetDCBrushColor( canvas, color::blue );
    FillRect( canvas, &area, 0 );

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, color::yellow );
    SetDCBrushColor( canvas, color::orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );
}

auto main() -> int
{
    constexpr auto  no_window   = HWND( 0 );

    const HDC canvas = GetDC( no_window );
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );

        draw_on( canvas, RECT{ 10, 10, 10 + 400, 10 + 400 } );

    ReleaseDC( no_window, canvas );
}
```

… where

*[part-05/code/.include/winapi/gdi/color_names.hpp](part-05/code/.include/winapi/gdi/color_names.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

namespace winapi::gdi {

    // COLORREF is 32-bit unsigned.
    namespace color_names {
        constexpr COLORREF  orange      = RGB( 0xFF, 0x80, 0x20 );
        constexpr COLORREF  yellow      = RGB( 0xFF, 0xFF, 0x20 );
        constexpr COLORREF  blue        = RGB( 0x00, 0x00, 0xFF );
    }  // namespace color_names

}  // namespace winapi::gdi
```

The [current documentation of `SelectObject`](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectobject) states that the object one selects “must have been created” by one of the functions listed in a table there, which does not include `GetStockObject`. But of course that’s just the usual Microsoft documentation SNAFU. The stock objects would not be useful for anything if they couldn’t be used.

However, the stock objects are special in that they don’t need to and shouldn’t be destroyed via `DeleteObject` (or any other way).

Result: same as before, just with shorter & more clear code.

---

### 5.3. A C++ fluent style wrapper for “DC color” usage.

<img title="" src="part-05/images/yoda.png" alt="">

The drawing code

```cpp
using namespace winapi::gdi::color_names;

// Clear the background to blue.
SetDCBrushColor( canvas, blue );
FillRect( canvas, &area, 0 );

// Draw a yellow circle filled with orange.
SetDCPenColor( canvas, yellow );
SetDCBrushColor( canvas, orange );
Ellipse( canvas, area.left, area.top, area.right, area.bottom );
```

… is a definite improvement on the first example’s ultra-verbose general GDI code, but ideally it should look more like this:

```cpp
using namespace winapi::gdi::color_names;
canvas.use( Brush_color( blue ) ).fill( area );
canvas.use( Brush_color( orange ), Pen_color( yellow ) ).draw( Ellipse, area );
```

Here `canvas` is an instance of a C++ class that wraps an `HDC`, and its `.use` member function returns a reference to the instance so that one can call e.g. `.fill` or `.draw`. This is the same principle as with the iostreams `<<` operator. It's called [**fluent style**](https://en.wikipedia.org/wiki/Fluent_interface).

Oh, the Yoda picture is really about absorbing a great destructive force rather than generating a constructive force. But it looks forceful. And I like Yoda. ☺

---

The device context class’  `.use` member function takes an arbitrary number of arguments that represent colors to set in the device context. What kind of colorization a given argument is used for — e.g. pen or brush — is determined by the argument type’s `.set_in` member function (examples below). All but the `Color` base class are such argument types:

*[part-05/code/.include/winapi/gdi/color-usage-classes.hpp](part-05/code/.include/winapi/gdi/color-usage-classes.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

namespace winapi::gdi {
    struct Color{ COLORREF value; Color( const COLORREF c ): value( c ) {} };

    struct Brush_color: Color
    {
        using Color::Color;
        void set_in( const HDC canvas ) const { SetDCBrushColor( canvas, value ); }
    };

    struct Pen_color: Color
    {
        using Color::Color;
        void set_in( const HDC canvas ) const { SetDCPenColor( canvas, value ); }
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
```

The device context class' `.use` member function that uses the above:

```cpp
template< class... Args >
auto use( const Args&... colors ) const
    -> const Dc&
{
    (colors.set_in( m_handle ), ...);
    return *this;
}
```

The first statement is a C++17 [**fold expression**](https://en.cppreference.com/w/cpp/language/fold), which expands to one *c*`.set_in(m_handle)` call for each actual argument *c*.

---

There are several kinds of device context so I chose to define the common features in an abstract base class `Dc`:

*Start of [part-05/code/.include/winapi/gdi/device-contexts.hpp](part-05/code/.include/winapi/gdi/device-contexts.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                         // cpp::util::(hopefully, No_copying, Types_)
#include <winapi/gdi/color-usage-classes.hpp>   // winapi::gdi::(Brush_color, Pen_color, Gap_color)
#include <winapi/gui/std_font.hpp>              // winapi::gui::std_font
#include <wrapped-winapi/windows-h.hpp>         // Geneal Windows API.

#include <stddef.h>         // size_t

#include <tuple>            // std::(get, tie, tuple)
#include <type_traits>      // std::is_same_v
#include <utility>          // std::(index_sequence, 

namespace winapi::gdi {
    namespace cu = cpp::util;
    using   cu::hopefully, cu::No_copying, cu::Types_;
    using   std::get, std::tie, std::tuple,
            std::index_sequence,
            std::make_index_sequence;

    inline void make_practical( const HDC dc )
    {
        SelectObject( dc, GetStockObject( DC_PEN ) );
        SelectObject( dc, GetStockObject( DC_BRUSH ) );
        SetBkMode( dc, TRANSPARENT );   // Don't fill in background of text, please.
        SelectObject( dc, gui::std_font );
    }

    class Dc: No_copying
    {
        HDC     m_handle;

        ⋮ // An internal helper function here.

    protected:
        inline virtual ~Dc() = 0;                           // Derived-class responsibility.

        Dc( HDC&& handle, const bool do_extended_init = true ):
            m_handle( handle )
        {
            hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Device context handle is 0." );
            if( do_extended_init ) { make_practical( m_handle ); }
        }

    public:
        template< class... Args >
        auto use( const Args&... colors ) const
            -> const Dc&
        {
            (colors.set_in( m_handle ), ...);
            return *this;
        }

        auto fill( const RECT& area ) const
            -> const Dc&
        {
            FillRect( m_handle, &area, 0 );
            return *this;
        }

        template< class Api_func, class... Args >
        auto simple_draw( const Api_func api_func, const Args&... args ) const
            -> const Dc&
        {
            api_func( m_handle, args... );
            return *this;
        }

        ⋮ // Some more advanced functionality here.
    };

    inline Dc::~Dc() {}

    ⋮
```

For the last source code line: the fully abstract destructor is implemented and needs to be implemented because it’s called non-virtually by the destructors of derived classes. Unfortunately there’s no syntax for defining it inline in the class definition. Bjarne Stroustrup chose the `= 0` syntax for pure virtual function because it indicated that the function has no body, which is usually true, just not for the case of pure virtual destructors.

The `simple_draw` function,

```cpp
template< class Api_func, class... Args >
auto simple_draw( const Api_func api_func, const Args&... args ) const
    -> const Dc&
{
    api_func( m_handle, args... );
    return *this;
}
```

… shows how simple the basic fluid style support can be. It just takes the API function to call (e.g. `Ellipse`) and the call arguments; calls that function with the specified arguments; and returns a reference to self so that one can fluidly tack on more calls. With the relevant API function as argument one avoids having to define a separate such wrapper for each API function.

However, since the Windows API is designed to alway be usable from C, with `simple_draw` the position and size of the ellipse must be specified as the individual member values of a `RECT`, like this:

```cpp
canvas.use( Brush_color( orange ), Pen_color( yellow ) ).simple_draw(
    Ellipse, area.left, area.top, area.right, area.bottom
    );
```

asd



### 5.4 Automate creation of temporary GDI objects.

xxx

To unleash the full power of the GDI, such as using pattern pens and brushes, it's necessary to deal with dynamic creation and destruction of GDI objects. Doing it in C style, as in the first example, is however fragile and verbose. But you can automate the `DeleteObject` object destruction calls via C++ destructors, the C++ [RAII](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization) technique, to make such code shorter and safer. Essentially it means defining small handle ownership classes such as `Brush` and `Pen`. For simplicity and efficiency these classes can be made non-copyable.

The RAII idea of leveraging C++ construction and destruction can also be applied to device contexts, e.g. (because device context destruction depends on the kind of DC) via a general abstract base class `Dc` with concrete derived classes such as `Screen_dc` and `Bitmap_dc`.

For exception safety — to be able to use exceptions freely — even the `SelectObject` call pairs can/should be automated via C++ construction and destruction, e.g. a class `Dc::Selection` whose instances retain the requisite information to undo the selection.

xxx

 Finally, most of these objects will ordinarily be very short lived ones, created for single calls of graphics primitives such as `FillRect` and `Ellipse`.

To reduce or eliminate a phletora of named short lived variables one can support *implicit creation* of the objects via operators such `+` or `->`, using the same return-reference-to-self *call chaining* idea as with iostream `<<` expressions.

For example, “adding” a device context and e.g. a temporary GDI pen object handle with `+` can result in a temporary `Pen` object (ensuring `DeleteObject`) and a `Dc::Selection` instance that converts implicitly to `HDC`, so that such an expression can be used where an `HDC` is required, and where further addition can be chained on the first one. The key C++ language support for this is that a temporary object is destroyed at the end of the full-expression, not immediately after the function call it appears as argument to. So the temporary objects created by `+` arguments persist until the full expression, e.g. with the outermost level a call of `FillRect` or `Ellipse`, has been evaluated.

Using such machinery the example with dynamic creation of brushes and pens can be rewritten as much shorter and now exception safe C++ level code:

asdasd

*[part-05/code/on-screen-graphics/v3/main.cpp](part-05/code/on-screen-graphics/v3/main.cpp)*:

```cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::*

namespace gdi = winapi::gdi;
using   gdi::Dc, gdi::Screen_dc;

void draw_on( const Dc& canvas, const RECT& area )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );

    FillRect( canvas + CreateSolidBrush( blue ), &area, 0 );
    Ellipse(
        canvas + CreatePen( PS_SOLID, 1, yellow ) + CreateSolidBrush( orange ),
        area.left, area.top, area.right, area.bottom
        );
}

auto main() -> int
{
    draw_on( Screen_dc(), RECT{ 10, 10, 10 + 400, 10 + 400 } );
}
```

<img title="" src="part-05/images/sshot-2.cpp-graphics-on-screen.cropped.png" alt="">

This program does exactly the same as the first and second C style examples, in order to make it easy to compare the three programs.

---

For the implementation of GDI object RAII wrappers I found it useful to define a little **type list** class template,

*in [part-05/code/.include/cpp/util.hpp](part-05/code/.include/cpp/util.hpp)*:

```cpp
    template< class... Types >
    struct Types_
    {
        static constexpr int count = static_cast<int>( sizeof...( Types ) );

        template< class T >
        static constexpr bool contain_ = (... or is_same_v<T, Types>);
    };
```

… which supports definitions like

```cpp
using Object_handle_types = Types_<HGDIOBJ, HPEN, HBRUSH, HFONT, HBITMAP, HRGN, HPALETTE>;
```

The `Pen`, `Brush`, `Font`, `Bitmap`, `Region` and `Palette` classes below do not inherit from `Object` because, while that would faithfully reproduce the conceptual GDI class hierarchy it would require e.g. adding handle type downcasts for no real advantage.

*[part-05/code/.include/winapi/gdi/Object_.hpp](part-05/code/.include/winapi/gdi/Object_.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <cpp/util.hpp>

#include    <assert.h>
#include    <utility>       // std::(enable_if_t, exchange)

namespace winapi::gdi {
    namespace cu = cpp::util;
    using   cu::No_copying, cu::Types_;
    using   std::enable_if_t, std::exchange;

    // The handle types below are the types that have destruction via GDI `DestroyObject`.
    // The `HGDIOBJ` handle type is like a base class for the other handle types.
    // Note: the GDI `GetObjectType` function supports some additional GDI object kinds.
    using Object_handle_types = Types_<HGDIOBJ, HPEN, HBRUSH, HFONT, HBITMAP, HRGN, HPALETTE>;

    template<
        class Handle_type,
        class = enable_if_t< Object_handle_types::contain_< Handle_type > >
        >
    class Object_: No_copying
    {
    public:
        using Handle = Handle_type;

    private:
        Handle      m_handle;

    public:
        ~Object_()
        {
            if( m_handle ) {
                // Deletion may possibly fail if the object is selected in a device context.
                const bool deleted = ::DeleteObject( m_handle );
                assert(( "DeleteObject", deleted ));  (void) deleted;
            }
        }

        Object_( Handle&& handle ): m_handle( handle )
        {
            hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Handle is 0." );
        }

        auto released() -> Handle { return exchange( m_handle, Handle( 0 ) ); }

        auto handle() const -> Handle { return m_handle; }
        operator Handle() const { return handle(); }
    };

    using   Object      = Object_<HGDIOBJ>;

    using   Pen         = Object_<HPEN>;
    using   Brush       = Object_<HBRUSH>;
    using   Font        = Object_<HFONT>;
    using   Bitmap      = Object_<HBITMAP>;
    using   Region      = Object_<HRGN>;
    using   Palette     = Object_<HPALETTE>;
}  // namespace winapi::gdi
```

Fine detail: the rvalue reference `Handle&&` for the `Object_` constructor parameter expresses an **ownerhip transfer**. It helps to avoid incorrect usage by allowing only a temporary handle such as from a call of `CreatePen` or `CreateSolidBrush`.

---

The [part 3 machinery to obtain a handle to the standard GUI font](part-03.md#34-fix-gross-imperfections-standard-font-window-just-on-top-modern-appearance) can now be re-expressed in terms of the `Font` class, and to reduce coupling I now put it in its own little header:

*[part-05/code/.include/winapi/gui/std_font.hpp](part-05/code/.include/winapi/gui/std_font.hpp)*:

```cpp
#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <winapi/gdi/Object_.hpp>           // winapi::gdi::Font
#include <wrapped-winapi/windowsx-h.hpp>    // windows.h + SetWindowFont    

namespace winapi:gui {
    inline auto create_std_font()
        -> HFONT
    {
        // Get the system message box font
        const auto ncm_size = sizeof( NONCLIENTMETRICS );
        NONCLIENTMETRICS metrics = {ncm_size};
        SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm_size, &metrics, 0 );
        return CreateFontIndirect( &metrics.lfMessageFont );
    }

    class Standard_font: public gdi::Font
    {
    public:
        Standard_font(): gdi::Font( create_std_font() ) {}
    };

    inline const auto std_font = Standard_font();       // Converts implicitly to HFONT.

    inline void set_font( const HWND window, const HFONT font )
    {
        const auto callback = []( const HWND control, const LPARAM font ) noexcept -> BOOL
        {
            SetWindowFont( control, reinterpret_cast<HFONT>( font ), true );
            return true;
        };

        SetWindowFont( window, font, true );
        EnumChildWindows( window, callback, reinterpret_cast<LPARAM>( font ) );
    }

    inline void set_standard_font( const HWND window )
    {
        set_font( window, std_font );
    }
}  // namespace winapi::gui
```

---

The standard GUI font is used in the `Dc` class’ default initialization of a device context:

*Start of [part-05/code/.include/winapi/gui/device-contexts.hpp](part-05/code/.include/winapi/gui/device-contexts.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // cpp::util::(hopefully, fail)
#include <winapi/gdi/Object_.hpp>           // winapi::gdi::(Brush, Pen, Bitmap)
#include <winapi/gui/std_font.hpp>          // winapi::gui::std_font
#include <wrapped-winapi/windows-h.hpp>

namespace winapi::gdi {
    namespace cu = cpp::util;
    using cu::hopefully, cu::No_copying, cu::Explicit_ref_;

    inline void make_practical( const HDC dc )
    {
        SelectObject( dc, GetStockObject( DC_PEN ) );
        SelectObject( dc, GetStockObject( DC_BRUSH ) );
        SetBkMode( dc, TRANSPARENT );       // Don't fill in the background of text, please.
        SelectObject( dc, gui::std_font );
    }

    class Dc: No_copying
    {
        HDC     m_handle;

    protected:
        inline virtual ~Dc() = 0;                           // Derived-class responsibility.

        Dc( HDC&& handle, const bool do_extended_init = true ):
            m_handle( handle )
        {
            hopefully( m_handle != 0 ) or CPPUTIL_FAIL( "Device context handle is 0." );
            if( do_extended_init ) { make_practical( m_handle ); }
        }

    public:
        class Selection;                                    // RAII for SelectObject, separate.

        auto handle() const -> HDC { return m_handle; }
        operator HDC() const { return handle(); }
    };

    inline Dc::~Dc() {}


    class Screen_dc: public Dc
    {
    public:        
        ~Screen_dc() override { ReleaseDC( 0, handle() ); }
        Screen_dc(): Dc( GetDC( 0 ) ) {}                    // Main screen specified implicitly.
    };


    class Memory_dc: public Dc
    {
    public:
        ~Memory_dc() override { DeleteDC( handle() ); }
        Memory_dc(): Dc( CreateCompatibleDC( 0 ) ) {}       // Implicitly DC for main screen.
    };


    inline auto bitmap_in( const HDC dc )
        -> HBITMAP
    { return static_cast<HBITMAP>( GetCurrentObject( dc, OBJ_BITMAP ) ); }

    class Bitmap_dc: public Memory_dc
    {
    public:
        Bitmap_dc( const HBITMAP bitmap ):
            Memory_dc()
        {
            SelectObject( handle(), bitmap );
        }

        auto bitmap() const -> HBITMAP { return bitmap_in( handle() ); }
    };


    ⋮
```

---

The temporary GDI object automation via `+` is very closely tied to the `Dc::Selection` class, so I present it all together in one little package. This class is full of subtleties and I’m not sure my design choices here were the best. I’m not even sure that I managed to make it difficult to use incorrectly, but anyway, the complexity is in large part due to that *goal*.

*End of [part-05/code/.include/winapi/gui/device-contexts.hpp](part-05/code/.include/winapi/gui/device-contexts.hpp)*:

```cpp
    ⋮

    class Dc::Selection: No_copying
    {
        const Dc&   m_dc;
        HGDIOBJ     m_original_object;

    public:
        ~Selection() { SelectObject( m_dc, m_original_object ); }

        struct From_api_handle {};  // Intentionally explicit, verbose & ugly interface.
        Selection( From_api_handle, const HGDIOBJ object_handle, const Dc& dc ):
            m_dc( dc ),  m_original_object( SelectObject( dc, object_handle ) )
        {}

        template< class Handle >    // Easy to use interface.
        Selection( const Dc& dc, const Object_<Handle>& object ):
            Selection( From_api_handle(), object.handle(), dc )
        {}

        auto dc() const -> const Dc& { return m_dc; }

        operator const Dc&() const { return dc(); }     // Supports `+` operator.
        operator HDC() const { return m_dc.handle(); }  // Supports using Selection as DC.
    };

    inline auto operator+( const Dc& dc, const Brush& object )
        -> Dc::Selection
    { return { Dc::Selection::From_api_handle(), object.handle(), dc }; }

    inline auto operator+( const Dc& dc, const Pen& object )
        -> Dc::Selection
    { return { Dc::Selection::From_api_handle(), object.handle(), dc }; }
}  // namespace winapi::gdi
```

### 5.4. GDI + COM/OLE: save graphics to an image file.

A modern “.jpg” or “.png” image would be perfect for communicating one's result to others, or for using it in other work. However, the only file format supported by the GDI is the archaic [“.wmf”](https://en.wikipedia.org/wiki/Windows_Metafile) and possibly its cousin “.emf”. This is a Windows-specific binary vector graphics format with little to no support today, not even in Windows Explorer.

The GDI successor technologies GDI+ and DirectX are for later parts of the tutorial (hopefully). For now let’s stick to the basic original, simple GDI. What  options does one then have for saving a graphics result — other than “.wmf”?

Well, there are two ways to save a GDI graphics result as a now reasonably [portable](https://en.wikipedia.org/wiki/BMP_file_format#Usage_of_BMP_format) “**.bmp**” image file, namely

* directly [generate the binary contents](https://docs.microsoft.com/en-us/windows/win32/gdi/storing-an-image) of such a file (the DIY approach), or
* use the [**`OleSavePictureFile`**](https://docs.microsoft.com/en-us/windows/win32/api/olectl/nf-olectl-olesavepicturefile) function.

A potential third way could have been to use the `CreateDIBSection` function to create a bitmap at the right place in a memory mapped file, namely after the BMP file header. However, that right place is at offset 14 in the file, and when I tried that the function call just failed with no indication why. Offset 16 worked fine though, which indicated an alignment requirement. Indeed, *then* reading the documentation — like reading the user manual for something after failing — it says that ❝*dwOffset* must be a multiple of the size of a `DWORD`❞. And unfortunately that particular placement is one that can’t be adjusted.

Here we’ll use the `OleSavePictureFile` API function because that involves *learning about how to use such Windows API functions*, which is generally useful knowledge, and because this is information that’s difficult to come by nowadays.

The Microsoft documentation linked to above is one of many sources that show how to generate binary “.bmp” file contents, if you want to go that route.

<p align="center">❁ ❁ ❁</p>

The `OleSavePictureFile` function expects the graphics image as an `IPictureDisp` abstract C++ object.

An `IPictureDisp` object, in turn, is created from a **bitmap**, essentially an array of pixel values plus some meta-information such as the image size.

Our drawing code draws to a device context, and as mentioned a device context can generate the graphics in a bitmap, so the flow is roughly DC → bitmap → `IPictureDisp` → file:

<img src="part-05/images/data-flow-for-saving.png" title="" alt="asdlkj" width="391"> 

<p align="center">❁ ❁ ❁</p>

We already have a `Bitmap_dc` class suitable for drawing in a bitmap, and if the GDI had offered a simple to use function to create a bitmap with a specified format, then that would have sufficed.

Creating a bitmap with the same format (bits per color value, layout in memory) as the main screen *is* easy via `CreateCompatibleBitmap`. But e.g. I have one screen with a mode with 10 bits per R, G and B. So, using the simple and for-most-screens-working `CreateCompatibleBitmap` falls in the category of “oh, but it worked on *my* machine…”.

Creating a *device independent bitmap* with a known common format, a **DIB**, can be done via a function called `CreateDIBSection` but is more complex, with lots of details involved; it can go like this:

*[part-05/code/.include/winapi/gdi/Bitmap_32.hpp](part-05/code/.include/winapi/gdi/Bitmap_32.hpp)*

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/Object_.hpp>       // winapi::gdi::Bitmap

#include <utility>      // std::move

namespace winapi::gdi {
    namespace cu = cpp::util;
    using cu::hopefully, cu::No_copying;
    using std::move;

    namespace bitmap {
        struct Format               // See documentation of `BITMAPINFOHEADER::biBitCount`.
        { enum Enum{
            implied                 = 0,
            monochrome              = 1,
            palette_16_colors       = 4,
            palette_256_colors      = 8,
            rgb_compressed          = 16,
            rgb_24_bits             = 24,
            rgb_32_bits             = 32
        }; };

        struct Handle_and_memory
        {
            HBITMAP     handle;
            void*       p_bits;     // Owned by but cannot be obtained from the handle.
        };

        inline auto create_rgb32( const int width, const int height )
            -> Handle_and_memory
        {
            BITMAPINFO params  = {};
            BITMAPINFOHEADER& info = params.bmiHeader;
            info.biSize             = {sizeof( info )};
            info.biWidth            = width;
            info.biHeight           = height;
            info.biPlanes           = 1;
            info.biBitCount         = Format::rgb_32_bits;
            info.biCompression      = BI_RGB;

            void* p_bits;
            const HBITMAP handle = CreateDIBSection(
                HDC(),              // Not needed because no DIB_PAL_COLORS palette.
                ¶ms,
                DIB_RGB_COLORS,     // Irrelevant, but.
                &p_bits,
                HANDLE(),           // Section.
                0                   // Section offset.
                );
            hopefully( handle != 0 ) or CPPUTIL_FAIL( "CreateDibSection failed" );
            return Handle_and_memory{ handle, p_bits };
        }
    }  // namespace bitmap

    class Bitmap_32: public Bitmap
    {
        void*       m_p_bits;

    public:
        Bitmap_32( bitmap::Handle_and_memory&& pieces ):
            Bitmap( move( pieces.handle ) ),
            m_p_bits( pieces.p_bits )
        {}

        Bitmap_32( const int w, const int h ):
            Bitmap_32( bitmap::create_rgb32( w, h ) )
        {}

        auto bits() const -> void* { return m_p_bits; }
    };
}  // namespace winapi::gdi
```

The above is all of the new GDI functionality needed for the saving. Let’s look at the generally non-GDI details of the saving function, the mainly OLE stuff, after the main program. The main new thing is that since the saving can ***fail*** with an exception, the previous `main` function body is now the body of a function `cpp_main` that is  called from an exception-handling `main`:

*[part-05/code/on-screen-graphics/v4/main.cpp](part-05/code/on-screen-graphics/v4/main.cpp)*:

```cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::(Dc, Bitmap_dc, `+`)
#include <winapi/gdi/Bitmap_32.hpp>         // winapi::gdi::Bitmap_32
#include <winapi/gdi/bitmap-util.hpp>       // winapi::gdi::save_to

#include <stdlib.h>     // EXIT_..., system
#include <stdio.h>      // fprintf

#include <functional>   // std::ref
#include <stdexcept>    // std::exception
#include <string>       // std::string

namespace gdi = winapi::gdi;
using   gdi::Dc, gdi::Bitmap_32, gdi::Bitmap_dc, gdi::save_to;
using   std::ref, std::exception, std::string;
const auto s = string();    // For string concatenation.

void draw_on( const Dc& canvas, const RECT& area )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );

    FillRect( canvas + CreateSolidBrush( blue ), &area, 0 );
    Ellipse(
        canvas + CreatePen( PS_SOLID, 1, yellow ) + CreateSolidBrush( orange ),
        area.left, area.top, area.right, area.bottom
        );
}

void cpp_main()
{
    const auto [w, h] = SIZE{ 400, 400 };
    auto image = Bitmap_32( w, h );
    draw_on( Bitmap_dc( image ), RECT{ 0, 0, w, h } );
    const auto filename = string( "image-saving-result.bmp" );
    gdi::save_to( filename, image );

    auto cmd = ::s + "start \"\" " + filename + "\"";   // “start” for non-blocking command.
    system( cmd.c_str() );  // Open file in “.bmp”-associated program, e.g. an image viewer.
}

auto main( int, char** args ) -> int
{
    const auto error_box = []( const string& title, string const& text )
    {
        const auto as_errorbox  = MB_ICONERROR | MB_SYSTEMMODAL;    // Sys-modal for Win 11.
        MessageBox( 0, text.c_str(), title.c_str(), as_errorbox );
    };

    using std::exception;
    try {
        cpp_main();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        fprintf( stderr, "!%s\n", x.what() );
        error_box( ::s + args[0] + " failed:", ::s + "Because:\n" + x.what() );
    }
    return EXIT_FAILURE;
}
```

My default image viewer is a free program called FastStone, so I get this result:

![Image in FastStone viewer](part-05/images/sshot-3.image-file-in-viewer.png)

<p align="center">❁ ❁ ❁</p>

A call of the `OleSavePictureFile` function drags in a lot of complexity because it’s strongly coupled to things in the [**OLE library**](https://en.wikipedia.org/wiki/Object_Linking_and_Embedding).

Since OLE is now mostly old irrelevant technology I only discuss how to use it, and about that only what’s needed for our use of `OleSavePictureFile`.

OLE sits or sat on top of the [**COM infra-structure**](https://en.wikipedia.org/wiki/Component_Object_Model), the same object/component infra-structure that as of 2022 serves as foundation for the [**Windows Runtime**](https://en.wikipedia.org/wiki/Windows_Runtime) a.k.a. WinRT.

Every COM and therefore also OLE function returns a 32-bit structured **result code** of type [**`HRESULT`**](https://en.wikipedia.org/wiki/HRESULT)that tells whether the call failed or succeeded, and roughly how. C++ exceptions are not used even though most of the COM stuff is in the form of C++ objects. A main reason is that COM is designed to make everything accessible from C, and result codes are very C friendly, very C-ish, accessible.

A great many of the `HRESULT` codes have names, like `E_FAIL` (a general failure, defined as the for most unexpectedly non-simple value 80004005₁₆), and like `S_OK` and `S_FALSE` (two different success codes, defined as respectively 0 and 1). Essentially such a code denotes failure if the most significant bit, bit 31, the sign bit, is 1. But instead of checking whether a “hr” code is negative one can and should use the macros **`FAILED`** and **`SUCCEEDED`**:

```cpp
const HRESULT hr = SomeOleFunction( "blah", 42 );
if( FAILED( hr ) ) {
    throw std::runtime_error( "SomeOleFunction failed!" );
}
```

This verbose C++ call pattern can be streamlined a bit by defining a class type constant `success` and a `>>` operator that returns `true` (only) if the `HRESULT` denotes success, so that, using the `fail` throwing function defined earlier, the code becomes

```cpp
SomeOleFunction( "blah", 42 )
    >> success or fail( "SomeOleFunction failed!" );
```

Note: since the `or` only evaluates the right argument when the left argument is `true`, the failure handling after `>>` can involve arbitrary complexity such as complex string argument building, without cost for normal successful code execution; the same as with the `if`.

For completeness, here’s the — or a possible — definition:

*[part-05/code/.include/winapi/com/failure-checking.hpp](part-05/code/.include/winapi/com/failure-checking.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // cpp::util::Success
#include <wrapped-winapi/windows-h.hpp>     // HRESULT, SetLastError, GetLastError

namespace winapi::com {
    namespace cu = cpp::util;
    inline namespace failure_checking {
        using cu::Success, cu::success;

        constexpr auto denotes_success( const HRESULT hr )
            -> bool
        { return SUCCEEDED( hr ); }         // Essentially that hr >= 0.

        inline auto operator>>( const HRESULT hr, Success )
            -> bool
        {
            const bool is_success = denotes_success( hr );
            if( not is_success ) { SetLastError( hr ); }    // Can be retrieved via GetLastError.
            return is_success;
        }
    }  // namespace failure_checking
}  // namespace winapi::com
```

<p align="center">❁ ❁ ❁</p>

Before calling most any COM function the COM library must have been initialized via a call to **`CoInitialize`**. Nested calls of `CoInitialize` can succeed if the parameters are compatible with the original call. Each call of `CoInitialize` must eventually be paired with a correspondingly nested call of **`CoUninitialize`**.

When you use OLE there is correspondingly **`OleInitialize`** and **`OleUninitialize`** which are used in the same way. In particular these calls can be nested, and therefore, can be used locally.  `OleInitialize` and `OleUninitialize` take care of calling respectively `CoInitialize` and `CoUninitialize`.

Encapsulating that with exception safe C++ RAII:

*[part-05/code/.include/winapi/ole/Library_usage.hpp](part-05/code/.include/winapi/ole/Library_usage.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // CPPUTIL_FAIL, cpp::util::hopefully
#include <winapi/com/failure-checking.hpp>  // winapi::com::failure_checking::(success, >>)
#include <wrapped-winapi/ole2-h.hpp>        // OleInitialize, OleUninitialize

namespace winapi::ole {
    using namespace winapi::com::failure_checking;
    using cpp::util::No_copying;

    struct Library_usage: No_copying
    {
        Library_usage()
        {
            OleInitialize( {} )
                >> success or CPPUTIL_FAIL( "OleInitialize failed" );
        }

        ~Library_usage()
        {
            OleUninitialize();  // Failure handling here would be advanced, e.g. logging.
        }
    };
}  // namespace winapi::ole
```

This is used locally in `gdi::save_to`, relying on the nestability of such call pairs so that `gdi::save_to` can be used without the calling code having to know about OLE things or at all that OLE is involved:

*[part-05/code/.include/winapi/gdi/bitmap-util.hpp](part-05/code/.include/winapi/gdi/bitmap-util.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/ole/Library_usage.hpp>     // winapi::ole::Library_usage
#include <winapi/ole/picture-util.hpp>      // winapi::ole::save_to

#include <string_view>

namespace winapi::gdi {
    namespace ole = winapi::ole;
    using std::string_view;

    inline void save_to( const string_view& file_path, const HBITMAP bitmap )
    {
        const ole::Library_usage _;     // RAII OleInitialize + OleUninitialize.
        ole::save_to( file_path, ole::picture_from( bitmap ).raw_ptr() );
    }
}  // namespace winapi::gdi
```

The class is *not* used in the OLE library wrappers such as `ole::save_to` and `ole::picture_from`_. The OLE wrappers assume — they have as precondition — that the library has been initialized. There would be no point in hiding the use of OLE in the OLE wrappers, and doing library initialization and cleanup in every function, e.g. for convenience to callers, would constitute needless inefficiency and verbosity, plus needlessly added failure modes.

<p align="center">❁ ❁ ❁</p>

COM objects like the OLE `IPictureDisp` object we need for saving and that `ole::picture_from` produces, are reference counted via calls to the **`.AddRef()`** and **`.Release()`** methods inherited from the [**`IUnknown`**](https://en.wikipedia.org/wiki/IUnknown) interface. When the last reference is removed the object self-destroys, just like an object managed by a C++ `shared_ptr`. To ensure correct and exception safe reference counting, in C++ COM objects are usually handled via smart pointers that automate the `.AddRef` and `.Release` calls.

Unfortunately the Windows API headers, while supplying some other C++ specific functionality, do not supply a **COM object smart pointer**. The Visual C++ compiler does supply a COM object smart pointer called [**`_com_ptr_t`**](https://docs.microsoft.com/en-us/cpp/cpp/com-ptr-t-class?view=msvc-170) via the `<comdef.h>` header. The MinGW g++ compiler that I have installed also provides `_com_ptr_t`, but in a brittle way: it requires that one includes `<stdio.h>` before the `<comdef.h>` header.

Rather than relying on such compiler specific functionality I chose to define a minimal COM object smart pointer, one that just ensures a final `.Release()` call, which is all that we need:

*[part-05/code/.include/winapi/com/Ptr_.hpp](part-05/code/.include/winapi/com/Ptr_.hpp)*:

```cpp
#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <cpp/util.hpp> // cpp::util::(No_copying, Const_)

#include <utility>      // std::exchange

namespace winapi::com {
    namespace cu = cpp::util;
    using   cu::No_copying, cu::Const_;
    using   std::exchange;

    template< class Interface >
    class Ptr_: No_copying
    {
        Interface*  m_ptr;

    public:
        ~Ptr_() { if( m_ptr ) { m_ptr->Release(); } }       // .Release() from IUnknown.
        Ptr_( Const_<Interface*> ptr ): m_ptr( ptr ) {}

        auto raw_ptr() const -> Interface*      { return m_ptr; }
        auto operator->() const -> Interface*   { return raw_ptr(); }
    };
}  // namespace winapi::com
```

Crucial for correctness of the above: that `No_copying` prohibits also move construction and move assignment.

Detail: `Const_` is defined as just `template< class T > using Const_ = const T;`, which enables uniform “left `const`” a.k.a. “west `const`” syntax. Arguably the `Ptr_` constructor is so ultra-simple that the `const` on the parameter serves no direct practical purpose. It’s there for uniform rules, namely to *always* declare things that can be `const`, as `const`.

<p align="center">❁ ❁ ❁</p>

The `.raw_ptr()` in the call that `gdi::save_to` wraps, shown earlier,

```cpp
ole::save_to( file_path, ole::picture_from( bitmap ).raw_ptr() );
```

… is because `ole::picture_from` returns a safe `com::Ptr_<IPictureDisp>` (expressing the ownership transfer, exception safe) instead of an unsafe raw `IPictureDisp*` pointer:

*Start of [part-05/code/.include/winapi/ole/picture-util.hpp](part-05/code/.include/winapi/ole/picture-util.hpp)*:

```cpp
#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <cpp/util.hpp>                         // cpp::util::(success, Const_)
#include <winapi/com/failure-checking.hpp>      // winapi::com::failure_checking::operator>>
#include <winapi/com/Ptr_.hpp>                  // winpai::com::Ptr_
#include <winapi/ole/B_string.hpp>              // winapi::ole::B_string
#include <wrapped-winapi/ocidl-h.hpp>           // IPictureDisp
#include <wrapped-winapi/olectl-h.hpp>          // OleCreatePictureIndirect

namespace winapi::ole {
    namespace com   = winapi::com;
    namespace cu    = cpp::util;
    using   com::success, com::failure_checking::operator>>;
    using   cu::Const_;

    inline auto picture_from( const HBITMAP bitmap )
        -> com::Ptr_<IPictureDisp>
    {
        PICTDESC params = { sizeof( PICTDESC ) };
        params.picType      = PICTYPE_BITMAP;
        params.bmp.hbitmap  = bitmap;

        IPictureDisp* p_picture_disp;
        OleCreatePictureIndirect(
            &params, __uuidof( IPictureDisp ), false, reinterpret_cast<void**>( &p_picture_disp )
            )
            >> success or CPPUTIL_FAIL( "OleCreatePictureIndirect failed" );
        return p_picture_disp;
    }

    ⋮
```

The [**`__uuidof`**](https://docs.microsoft.com/en-us/cpp/cpp/uuidof-operator?view=msvc-170) pseudo-function produces a reference to a 128-bit [**UUID**](https://en.wikipedia.org/wiki/Universally_unique_identifier), a *universally unique id*, for the specified interface. Such interface identifiers are used to make COM and OLE usable from C. If it was all pure C++ then `OleCreatePictureIndirect` would presumably have been a function template, with the desired interface as a template parameter, and access of an interface would have been a `dynamic_cast`.

*Digression (sorry! but worth knowing): the MinGW g++ compiler implements `__uuidof` via standard C++, involving macros and templates, but unfortunately Visual C++ implements it as a **language extension**, in a family of such extensions for COM, so that portable code that defines COM interfaces needs to be carefully designed to be compatible with all the various relevant compilers.*

<p align="center">❁ ❁ ❁</p>

asdasd

The `gdi::save_to` function used in the above main program code is a quite thin wrapper over a function `ole::save_to`, that in turn wraps use of the Windows API function 

---

### 5.x. GDI + GUI: present graphics in a window.

asdasd
