# Windows GUI-stuff in C++: a tutorial.

## Part 5 – Primitive graphics: the GDI.

Last part’s discussion of how to use UTF-8 as the `char` based text encoding in both resource definitions and GUI code established a limited but useful framework for exploratory C++ Windows desktop programming. We’ll now use that to explore basic graphics: drawing shapes, lines and text. For this we’ll use the  [**GDI**](https://en.wikipedia.org/wiki/Graphics_Device_Interface), Windows’ original *graphics device interface*.

The GDI is simple and C-oriented, which is nice.
 On the other hand it’s slow and produces low quality graphics. In particular the GDI doesn’t support [anti-aliasing](https://en.wikipedia.org/wiki/Spatial_anti-aliasing), which is a strong reason to later move on to the successor technologies [GDI+](https://en.wikipedia.org/wiki/Graphics_Device_Interface#Windows_XP) and [Direct 2D](https://en.wikipedia.org/wiki/Direct2D).

Unfortunately Windows doesn’t yet support UTF-8 based text for *drawing* text as graphics, as opposed to using controls to present text as we did in part 4. We’ll work around that by writing our own wrappers over Windows’ wide text drawing functions. Happily Windows does support conversion between UTF-8 and UTF-16 via API functions such as [`MultiByteToWideChar`](https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar), in addition to, since Windows 10, providing the feature rich C API of the main Unicode library [**ICU**](https://docs.microsoft.com/en-us/windows/win32/intl/international-components-for-unicode--icu-).


[some figure]

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
*Contents (table generated with [DocToc](https://github.com/thlorenz/doctoc)):*

- [5.1. Draw directly on the screen to learn GDI basics.](#51-draw-directly-on-the-screen-to-learn-gdi-basics)
- [5.2. Use C++ RAII to automate GDI object destruction.](#52-use-c-raii-to-automate-gdi-object-destruction)
- [5.3. In passing: support `assert` messages in a GUI program built with Visual C++.](#53-in-passing-support-assert-messages-in-a-gui-program-built-with-visual-c)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


---
### 5.1. Draw directly on the screen to learn GDI basics.

You don’t need a window to draw graphics: with GDI you can draw more or less directly on the screen.

Drawing directly on the screen is just a special case of drawing in a window, the slightly paradoxical case of “no window”.

This involves first calling `GetDC(0)` (with `0` for “no window”) to get a handle to a drawing surface covering the screen. In Windows terminology that’s called a **device context** for the screen, and the handle type is a `HDC`, handle to device context. In more general programming the equivalent of a Windows DC is often called a **canvas**, emphasizing that it’s used for painting.

On the “output side” — the right side in the figure below — a device context works as a common abstraction of various quite different devices. It generates graphics in windows, in bitmap images, to printers, and to now archaic “.wmf” Windows binary vector graphics “meta-files”. So, you can use roughly the same code to generate graphics for all these devices.

The side of a device context that such code relates to, the “input side”, mainly executes drawing commands such as calls of the `Ellipse` function, but it also receives and retains drawing attributes such as a **pen** that specifies attributes of lines (e.g. color, width and pattern), and such as a **brush** that specifies attributes of color fills, in particular the fill color.

<img alt="DC inputs and outputs" src="part-05/images/dc.png" width="500">

By default a device context typically has a black pen and a white brush. To draw a yellow circle filled with orange the code below uses the general GDI approach of (1) creating pen and brush objects, respectively yellow and orange; (2) **selecting** them in the device context; (3) drawing, which implicitly uses the selected objects; (4) deselecting the objects by selecting back in the original objects; and finally (5) destroying the objects. This is not necessarily inefficient, but, which is a general problem with the GDI, it’s quite verbose:

[*part-05/code/on-screen-graphics/v1/main.cpp*](part-05/code/on-screen-graphics/v1/main.cpp)
~~~cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

auto main() -> int
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  no_window   = HWND( 0 );
    constexpr auto  area        = RECT{ 10, 10, 10 + 400, 10 + 400 };
    
    const HDC canvas = GetDC( no_window );

    // Fill the background with blue.
    const HBRUSH blue_brush = CreateSolidBrush( blue );
    FillRect( canvas, &area, blue_brush );
    DeleteObject( blue_brush );

    // Draw a yellow circle filled with orange.
    const HBRUSH    orange_brush    = CreateSolidBrush( orange );
    const HGDIOBJ   original_brush  = SelectObject( canvas, orange_brush );
    const HPEN      yellow_pen      = CreatePen( PS_SOLID, 1, yellow );
    const HGDIOBJ   original_pen    = SelectObject( canvas, yellow_pen );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );
    SelectObject( canvas, original_pen );
    DeleteObject( yellow_pen );
    SelectObject( canvas, original_brush );
    DeleteObject( orange_brush );

    ReleaseDC( no_window, canvas );
}
~~~

Originally the effect was probably to actually draw directly on the screen, bypassing all the window management, and messing up the screen Real Good&trade;. But in Windows 11 there are layers of indirection and management interposed between the drawing calls and the screen output, in particular the [Desktop Window Manager](https://docs.microsoft.com/en-us/windows/win32/dwm/dwm-overview). There are some weird effects such as the graphics partially intruding in console windows, but such code still “works” and supports explorative programming.

![A filled ellips drawn directly on the screen](part-05/images/sshot-1.graphics-on-screen.cropped.png)

The effect is not entirely consistent between runs. Sometimes, if one doesn’t fill in the background, there’s a black background around the disk; sometimes (but rarely) there’s only the disk, then with essentially transparent background; and with some other graphics I’ve seen the background from one run of one program being retained as background for the graphics from another program, which was pretty confusing, huh where did *that* come from, before I understood what was going on. This is much like the rest of Windows 11’s functionality, i.e. it’s pretty shaky, not very reliable, depending on the phase of the moon, but the unreliability doesn’t really matter here.

For completeness, the `COLORREF` type is a 32-bit [RGB](https://en.wikipedia.org/wiki/RGB_color_model) **color** specification (3×8 = 24 bits used), and the `RECT` type is a simple struct with `left`, `top`, `right` and `bottom` integer value members.

In the default pixel coordinate system *x* increases left to right so that `left` ≤ `right` and *y* increases top to bottom so that `top` ≤ `bottom`.

Worth knowing: the `right` pixel column and the `bottom` pixel row are not part of a `RECT` rectangle. These are “beyond” values just like a C++ `o.end()` iterator. Thus the initialization `RECT{ 10, 10, 10 + 400, 10 + 400 }` creates a 400×400 rectangle.

Also, for completeness, here’s how to build and run with the Microsoft toolchain, Visual C++ (needs linking with “**gdi32**.lib”):

~~~txt
[T:\part-05\code\on-screen-graphics\v1\.build]
> set common-code=t:\part-05\code\.include

[T:\part-05\code\on-screen-graphics\v1\.build]
> cl /I %common% ..\main.cpp user32.lib gdi32.lib /Feb
main.cpp

[T:\part-05\code\on-screen-graphics\v1\.build]
> b_
~~~

Ditto, building and running with the MinGW toolchain, g++:

~~~txt
[T:\part-05\code\on-screen-graphics\v1\.build]
> set common-code=t:\part-05\code\.include

[T:\part-05\code\on-screen-graphics\v1\.build]
> g++ -I %common-code% ..\main.cpp -lgdi32

[T:\part-05\code\on-screen-graphics\v1\.build]
> a_
~~~


### 5.2 Use pseudo-mutable `DC_PEN` and `DC_BRUSH` stock objects to reduce verbosity.

Instead of creating, selecting, using, unselecting and destroying pen and brush objects, as long as you don’t need fancy effects such as line patterns you can just change the device context’s **DC pen color** and **DC brush color**, via respectively `SetDCPenColor` and `SetDCBrushColor`. These colors are only *used* when the **stock objects** you get from respectively `GetStockObject(DC_PEN)` and `GetStockObject(DC_BRUSH)` are selected in the device context.

My experimentation showed that in Windows 11 these are not the default objects in a DC from `GetDC(0)`, so it’s necessary to explicitly select them:

[*part-05/code/on-screen-graphics/v2/main.cpp*](part-05/code/on-screen-graphics/v2/main.cpp)
~~~cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

auto main() -> int
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  no_window   = HWND( 0 );
    constexpr auto  area        = RECT{ 10, 10, 10 + 400, 10 + 400 };
    
    const HDC canvas = GetDC( no_window );
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );

    SetDCBrushColor( canvas, blue );
    FillRect( canvas, &area, 0 );   // `0` works for me, but should perhaps be the DC brush.

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, yellow );
    SetDCBrushColor( canvas, orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );

    ReleaseDC( no_window, canvas );
}
~~~

The [current documentation of `SelectObject`](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectobject) states that the object one selects “must have been created” by one of the functions listed in a table there, which does not include `GetStockObject`. But of course that’s just the usual Microsoft documentation SNAFU. The stock objects would not be useful for anything if they couldn’t be used.

However, the stock objects are special in that they don’t need to and shouldn’t be destroyed via `DeleteObject` (or any other way).

Result: same as before, just with shorter & more clear code.


---
### 5.x. Use C++ RAII to automate GDI object destruction.

The preceding section’s code exemplified how GDI usage, and for that matter many other areas of Windows programming, involves establishing and tearing down local state, with these pairs of calls at least logically in nested scopes:

~~~cpp
const HDC canvas = GetDC( no_window );
{
    const HBRUSH red_brush = CreateSolidBrush( red );
    {
        const HGDIOBJ original_brush = SelectObject( canvas, red_brush );
        {
            Ellipse( canvas, 10, 10, 10 + 400, 10 + 400 );
        }
        SelectObject( canvas, original_brush );
    }
    DeleteObject( red_brush );
}
ReleaseDC( no_window, canvas );
~~~

| What: | Create: | Destroy: |
|-------|----------|-----------|
| Window device context: | `GetDC` | `ReleaseDC` |
| Color brush: | `CreateSolidBrush` | `DeleteObject` |
| Selection state: | `SelectObject` | `SelectObject` |


These pairwise nested *create* + *destroy* call pairs are as made for applying the C++ [**RAII** technique](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization), the idea of using a C++ constructor to create something, so that the something’s eventual destruction can be automated and guaranteed via the corresponding C++ destructor. The nice thing about RAII is that it ensures destruction even when an exception occurs in the code using the something. But there is a snag, for the [documentation of the `DeleteObject` function](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-deleteobject) warns that

> ❞ Do not delete a drawing object (pen or brush) while it is still selected into a DC.

This means that `DeleteObject` probably can fail, even though some simple experimentation with not unselecting didn’t provoke such destruction failure, and therefore that the destructor used for RAII can fail, and if one chooses to let it report failure via an exception then it’s not unlikely that this will result in an exception in the stack unwinding of some other exception, resulting in a call of `std::terminate`… Which is seriously undesireable. Alternatives include:

* Ignore cleanup failures. This is likely to result in an ongoing GDI **resource leak**. A possible consequence is that instead of graphics the program ends up displaying white (or possibly black) areas, say.
* Ensure that the particular failure mode, `DeleteObject` of a pen or brush that’s still selected in a device context, cannot happen. Doing this in a reliable, robust way involves defining a full abstraction layer where client code doesn’t have access to the GDI object handles, so that it can’t mess up things. That can be a lot of work.
* Make it very likely that the problem, if any, is detected by testing, e.g. via `assert` statements.

The code below uses the last bullet point’s approach, the “just support discovery of the problem” approach:

[*part-05/code/.include/winapi/gdi.hpp*](part-05/code/.include/winapi/gdi.hpp)
~~~cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <cpp/util.hpp>

#include    <assert.h>

namespace winapi::gdi {
    class Window_dc: private cpp::util::No_copying
    {
        HWND    m_window;
        HDC     m_dc;
        
    public:
        ~Window_dc()
        {
            ::ReleaseDC( m_window, m_dc );
        }
        
        explicit Window_dc( const HWND window ):
            m_window( window ),
            m_dc( ::GetDC( window ) )
        {
            assert(( m_dc != 0 ));
        }
        
        auto handle() const -> HDC { return m_dc; }
        operator HDC() const { return handle(); }
    };

    template< class Handle >
    class Object_: private cpp::util::No_copying
    {
        Handle      m_object;
        
    public:
        ~Object_()
        {
            const bool ok = !!::DeleteObject( m_object );
            assert(( "DeleteObject", ok ));  (void) ok;
        }
        
        Object_( const Handle object ): m_object( object ) {}
        
        auto handle() const -> Handle { return m_object; }
        operator Handle() const { return handle(); }
    };

    class Selection: private cpp::util::No_copying
    {
        HDC         m_dc;
        HGDIOBJ     m_original_object;
        
    public:
        ~Selection() { ::SelectObject( m_dc, m_original_object ); }
        
        Selection( const HDC dc, const HGDIOBJ object ):
            m_dc( dc ),
            m_original_object( ::SelectObject( dc, object ) )
        {}
    };
}  // namespace winapi::gdi
~~~

In the v2 main program below this machinery is used to do exactly the same as the v1 program did:

[*part-05/code/on-screen-graphics/v2/main.cpp*](part-05/code/on-screen-graphics/v2/main.cpp)
~~~cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/gdi.hpp>
namespace gdi = winapi::gdi;

auto main() -> int
{
    constexpr auto  red         = COLORREF( RGB( 0xFF, 0, 0 ) );
    constexpr auto  no_window   = HWND( 0 );
    
    const auto canvas       = gdi::Window_dc( no_window );
    const auto red_brush    = gdi::Object_( CreateSolidBrush( red ) );

    { // Using the red brush.
        const auto _ = gdi::Selection( canvas, red_brush );
        Ellipse( canvas, 10, 10, 10 + 400, 10 + 400 );
    }
}
~~~

This can be further streamlined by defining a `WITH` macro, e.g.

~~~cpp
#define WITH( initializer )  if( const auto& _ = initializer; ((void) _, true) )
~~~

… so that one can write, for example,

~~~cpp
WITH( gdi::Selection( canvas, red_brush ) ) {
    Ellipse( canvas, 10, 10, 10 + 400, 10 + 400 );
}
~~~

… but I’ve found that I don’t actually use a `WITH` macro after definining it in some project (this has happened a number of times), so while it looks elegant it’s perhaps a waste of time.

Anyway the C++ RAII classes reduce the client code compared to direct use of the GDI API, but that’s just a nice bonus: they exist to provide *correctness* via guaranteed cleanup. This is however paid for by incurring some needless *inefficiency*, namely that *n* selection effect calls of `SelectObject` are paired with *n* corresponding guaranteed unselection calls, when just 1 final unselection call would suffice… The GDI API provides the `SaveDC` and `RestoreDC` functions to address that efficiency concern, where `SaveDC` saves the current selections, somewhere, and `RestoreDC` restores the last saved state.

However, from a C++ RAII automation point of view `SaveDC`+`RestoreDC` are slightly problematic because the unselection in `RestoreDC` should ideally be done before any possibly selected pen or brush is destroyed, which practically requires partially overlapping lifetimes for the RAII objects, and partially overlapping lifetimes don’t match C++ scopes very well.


---
### 5.3. In passing: support `assert` messages in a GUI program built with Visual C++.

To be sure that the `assert` statements really do their job you can intentionally trigger an assert, make it “fire”, e.g. by changing

~~~cpp
    assert(( m_dc != 0 ));
~~~

… to

~~~cpp
    assert(( m_dc != 0, false ));
~~~

With a MinGW g++ console subsystem build this works fine; in that the assertion text is reported in the console:

~~~txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> g++ -std=c++17 -I %common-code% ..\main.cpp -lgdi32

[T:\part-05\code\on-screen-graphics\v2\.build]
> a
Assertion failed: ( m_dc != 0, false ), file t:\part-05\code\.include/winapi/gdi.hpp, line 23
~~~

With a MinGW g++ GUI subsystem build it also works fine, producing an **assertion failure box**:

~~~txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> g++ -std=c++17 -I %common-code% ..\main.cpp -lgdi32 -mwindows

[T:\part-05\code\on-screen-graphics\v2\.build]
> a
~~~

![The assertion box](part-05/images/sshot-2.assertion-box.png)

And, it works fine with a Visual C++ console program build:

~~~txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> cl /I %common-code% ..\main.cpp user32.lib gdi32.lib /Feb
main.cpp

[T:\part-05\code\on-screen-graphics\v2\.build]
> b
Assertion failed: ( m_dc != 0, false ), file t:\part-05\code\.include\winapi/gdi.hpp, line 23
~~~

But with a Visual C++ GUI subsystem build the assertion message is not reported in any way. Indeed there’s ***nothing to tell you that an assertion fired***. All you can see is that the program mysteriously fails to have any effect:

~~~txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> set LINK=/entry:mainCRTStartup

[T:\part-05\code\on-screen-graphics\v2\.build]
> cl /I %common-code% ..\main.cpp user32.lib gdi32.lib /Feb /link /subsystem:windows
main.cpp

[T:\part-05\code\on-screen-graphics\v2\.build]
> b

[T:\part-05\code\on-screen-graphics\v2\.build]
> echo %errorlevel%
0
~~~

The technical reason is that Microsoft’s runtime library decides how to present the assertion message, as text output or in an assertion failure box, based on ***which entry point function*** one linked with:

| Entry point: | Calls startup function: | Assumed subsystem: | Presentation of assertion message: |
|-------------------|---------------|----------------|-----------------|
| `mainCRTStartup`  | `main` | Console | Text output. |
| `wmainCRTStartup` | Microsoft `wmain` | Console | Text output. |
| `WinMainCRTStartup` | Microsoft `WinMain` | GUI | Assertion failure box. |
| `wWinMainCRTStartup` | Microsoft `wWinMain` | GUI | Assertion failure box. |

From a design level perspective, if the intent was to support the programmer, then a decision to base the presentation mode on the executable’s subsystem is sub-optimal. For example, a GUI subsystem program may be running with its standard error stream tied to a console, so that presentation as text output is desirable. And in most cases it supports the programmer best to have both presentation modes, both text output that can be logged and an assertion failure box that can be viewed when the assertion failure happens; a restriction of the presentation to a single mode very seldom has any advantage.

**\<rant\>** Then, a decision to *infer* the subsystem from the entry point, instead of checking the subsystem, is a bit of lunacy. But perhaps the intent was not to support but instead to add yet another vendor lock in, and after all, this is tied to the `WinMain` monstrosity, which itself was a vendor lock in device. Microsoft got infamous for its vendor lock in business tactics when some [internal mails were revealed](https://www.cnet.com/tech/services-and-software/eu-report-takes-microsoft-to-task/) in the legal dispute between Microsoft and Sun Corporation. **\</rant\>**

Anyway, the assertion message wasn’t actually suppressed: it was just erroneously presented as text output on the program’s standard error stream, that wasn’t connected to anything. So a simple fix is to connect that error stream to the console. To do that in Cmd, simply redirecting the standard error stream via `2>con` doesn’t work, but fusing it into the standard output stream via `2>&1` and then piping the output to e.g. `find /v ""`, works nicely:

~~~txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> b 2>&1 | find /v ""
Assertion failed: ( m_dc != 0, false ), file t:\part-05\code\.include\winapi/gdi.hpp, line 23
~~~

It would be much better if the program itself chose a more reasonable practically useful presentation mode. For example, depending on whether its standard error stream is connected to something, as it is with the above command, or not. Doing this involves ***lying*** to the Microsoft runtime about the executable’s subsystem (like with a sabotage-minded donkey that always goes in the opposite direction of where you indicate you want to go, so you just lie to it), but happily that Just Works&trade; with as of Visual C++ 2022 no ill effects:

[*part-05/code/.include/compiler/msvc/Assertion_reporting_fix.hpp*](part-05/code/.include/compiler/msvc/Assertion_reporting_fix.hpp)
~~~cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#ifndef     _MSC_VER
#   error   "This header is for the Visual C++ compiler only."
#   include <stop-compilation>
#endif

#include <wrapped-winapi/windows-h.hpp>

#include <process.h>            // Microsoft - _set_app_type

namespace compiler::msvc {
    class  Assertion_reporting_fix
    {
        static auto is_connected( const HANDLE stream )
            -> bool
        {
            DWORD dummy;
            if( ::GetConsoleMode( stream, &dummy ) ) { return true; }
            switch( ::GetFileType( stream ) ) {
                case FILE_TYPE_DISK:    [[fallthrough]];
                case FILE_TYPE_PIPE:    { return true; }
            }
            return false;       // E.g. stream is redirected to `nul`.
        }

        Assertion_reporting_fix()
        {
            // Prevent assertion text being sent to The Big Bit Bucket In The Sky™.
            const HANDLE error_stream = ::GetStdHandle( STD_ERROR_HANDLE );
            _set_app_type( static_cast<_crt_app_type>(
                is_connected( error_stream )? _crt_console_app : _crt_gui_app
                ) );
        }

    public:
        static auto global_instantiation()
            -> bool
        {
            static Assertion_reporting_fix  the_instance;
            return true;
        };
    };
}  // namespace compiler::msvc
~~~

Here [**`_set_app_type`**](https://docs.microsoft.com/en-us/cpp/c-runtime-library/set-app-type?view=msvc-170) is an internal function in Microsoft’s runtime library.

The above is intended to be used like this:

~~~cpp
#ifdef _MSC_VER
#   include <compiler/msvc/Assertion_reporting_fix.hpp>
    const bool msvc_arf = compiler::msvc::Assertion_reporting_fix::global_instantiation();
#endif
~~~

The [Meyers’ singleton](https://stackoverflow.com/a/17712497) ensures that this results in a single call of `_set_app_type` even if this code appears in several translation units. I chose to exemplify this general `global_instantiation` technique even though the `_set_app_type` calls are idempotent, because it’s a good way to provide such fixes in general. A C++17 `inline` variable’s initialization could alternatively have done the job, but `inline` variables appear to still be somewhat unreliable in Visual C++.

With this, if you don’t redirect standard error, then with GUI subsystem you get the assertion failure box (the very same as shown earlier for MinGW g++, because it’s the same library in both cases), but if you do connect up the standard error stream then you get the message as text output, so you can decide the reporting mode in the command to run the program.

However, for more [DRY code](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself) the above usage is placed in a convenience header:

*[part-05/code/.include/compiler/msvc/Assertion_reporting_fix.auto.hpp](part-05/code/.include/compiler/msvc/Assertion_reporting_fix.auto.hpp)*
~~~cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#ifdef _MSC_VER
#   include <compiler/msvc/Assertion_reporting_fix.hpp>
    const bool msvc_arf = compiler::msvc::Assertion_reporting_fix::global_instantiation();
#endif
~~~

And so all that appears in v3 of the program is an include of the above file:

*In [part-05/code/on-screen-graphics/v3/main.cpp](part-05/code/on-screen-graphics/v3/main.cpp)*
~~~cpp
#include <compiler/msvc/Assertion_reporting_fix.auto.hpp>
~~~


---
### 5.4. A potpourri of GDI things introduced via a C curve example.

One of my pet pedagogical ideas is that the things that are most important are the things one ends up using or needing in concrete examples. So, as a concrete example I chose a simple recursive figure called the [C curve](https://en.wikipedia.org/wiki/L%C3%A9vy_C_curve). One way to create it is to start with a single line segment **–**, replace each line segment with an angle bracket **∨**, do that again, and so on:

<img src="part-05/images/C-curve-construction.png" width="340">

The above figure is based on [an image from Wikimedia](https://en.wikipedia.org/wiki/File:Levy_C_construction.png).

Using the above conceptual construction directly in code has the advantage that the starting line defines the direction and size of the result curve, but it has the problems that the generated (line start and end-) points are not necessarily at integer pixel coordinates, and that one may end up with diagonal lines that don’t look so good without anti-aliasing (GDI).

An alternative is to generate the curve with essentially [turtle graphics](https://en.wikipedia.org/wiki/Turtle_graphics), just tracing the curve from start to end, recreating its logical structure  — the left and right turnings — without regard for absolute orientation or size. This yields pixel perfect line segments that are either horizontal or vertical and always start and end on integer pixel coordinates, but the curve gets larger and rotates 45° when the number of levels is increased by 1. Anyway, with this approach one doesn’t need any graphics library or geometry maths to generate the curve points, just an understanding of recursive functions:

~~~cpp
void generate( const int order )
{
    if( order == 0 ) {
        current_position += step;
        output_curve_point( current_position );
    } else {
        generate( order - 1 );
        rotate_left( step );
        generate( order - 1 );
        rotate_right( step );
    }
}
~~~

That code gets a little obfuscated, sorry, by having the details filled in and being expressed as something easy to *use*, as opposed to easy to grok:

*[part-05\code\c-curve\v1\c_curve.hpp](part-05\code\c-curve\v1\c_curve.hpp)*
~~~cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <functional>       // std::function
#include <utility>          // std::move
#include <vector>           // std::vector

namespace c_curve {
    using std::function, std::move, std::vector;
    
    struct Point { int x; int y; };
    void rotate_left( Point& pt )   { pt = {-pt.y, pt.x}; }
    void rotate_right( Point& pt )  { pt = {pt.y, -pt.x}; }

    void operator+=( Point& pt, const Point& d )
    {
        pt.x += d.x; pt.y += d.y;
    }

    using Callback = function<void( const Point& )>;

    class Impl
    {
        Point       m_current_position;
        Point       m_step;
        Callback    m_output_curve_point;

    public:
        Impl( Callback output_curve_point, const int step ):
            m_current_position{ 0, 0 },
            m_step{ 0, -step },
            m_output_curve_point( move( output_curve_point ) )
        {
            m_output_curve_point( m_current_position );
        }
        
        void generate( const int order )
        {
            if( order == 0 ) {
                m_current_position += m_step;
                m_output_curve_point( m_current_position );
            } else {
                generate( order - 1 );
                rotate_left( m_step );
                generate( order - 1 );
                rotate_right( m_step );
            }
        }
    };

    inline void generate( const int order, Callback output_curve_point, const int step = 4 )
    {
        Impl( move( output_curve_point ), step ).generate( order );
    }
    
    template< class Point_type >
    auto as_vector_of_( const int order, const int step = 4 )
        -> vector<Point_type>
    {
        vector<Point_type> points;
        points.reserve( 1 + (1 << order) );     // Just because this is C++ => efficiency.
        generate(
            order,
            [&]( const Point& pt ){ points.push_back( Point_type{ pt.x, pt.y } ); },
            step
            );
        return points;
    }
}  // namespace c_curve
~~~

