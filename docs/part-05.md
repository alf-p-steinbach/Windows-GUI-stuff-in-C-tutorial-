# Windows GUI-stuff in C++: a tutorial.

## Part 5 – Primitive graphics: the GDI.

We’ll now use the [**GDI**](https://en.wikipedia.org/wiki/Graphics_Device_Interface), Windows’ original *graphics device interface*, to explore basic graphics: drawing shapes, lines and text, and learning how graphics drawing relates to Windows’ GUI mechanisms, such as drawing the background image of a window.

The GDI is simple and C-oriented, which is nice.

On the other hand it’s slow and produces low quality graphics. In particular the GDI doesn’t support [anti-aliasing](https://en.wikipedia.org/wiki/Spatial_anti-aliasing), and it doesn’t support [alpha channel transparency](https://en.wikipedia.org/wiki/Alpha_compositing), which are both strong reasons to later move on to the successor technologies [GDI+](https://en.wikipedia.org/wiki/Graphics_Device_Interface#Windows_XP) and [Direct 2D](https://en.wikipedia.org/wiki/Direct2D).

And unfortunately, also, the GDI doesn’t yet support UTF-8 based text for *drawing* text as graphics, as opposed to using controls to present text as we did in part 4. UTF-8 based text drawing is a must for leveraging the previous part’s discussion of how to use UTF-8 as the `char` based text encoding. We’ll just write our own wrappers over GDI’s wide text drawing functions, i.e. providing the missing support ourselves.

Happily Windows does support conversion between UTF-8 and UTF-16 via API functions such as [`MultiByteToWideChar`](https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar), in addition to, since Windows 10, providing the feature rich C API of the main Unicode library [**ICU**](https://docs.microsoft.com/en-us/windows/win32/intl/international-components-for-unicode--icu-).

[some figure]

<!-- START doctoc generated TOC please keep comment here to allow auto update -->

<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [5.1. Draw directly on the screen to learn GDI basics.](#51-draw-directly-on-the-screen-to-learn-gdi-basics)
- [5.2 Use pseudo-mutable `DC_PEN` and `DC_BRUSH` stock objects to reduce verbosity.](#52-use-pseudo-mutable-dc_pen-and-dc_brush-stock-objects-to-reduce-verbosity)
- [5.3. Automate cleanup for device contexts and GDI objects.](#53-automate-cleanup-for-device-contexts-and-gdi-objects)
- [5.4. Save the generated graphics to an image file.](#54-save-the-generated-graphics-to-an-image-file)
- [5.x. Draw UTF-8 text by converting to UTF-16 and using the wide text API.](#5x-draw-utf-8-text-by-converting-to-utf-16-and-using-the-wide-text-api)
- [5.3. In passing: support `assert` messages in a GUI program built with Visual C++.](#53-in-passing-support-assert-messages-in-a-gui-program-built-with-visual-c)
- [5.4. A potpourri of GDI things introduced via a C curve example.](#54-a-potpourri-of-gdi-things-introduced-via-a-c-curve-example)

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

### 5.2 Use pseudo-mutable `DC_PEN` and `DC_BRUSH` stock objects to reduce verbosity.

Instead of creating, selecting, using, unselecting and destroying pen and brush objects, as long as you don’t need fancy effects such as line patterns you can just change the device context’s **DC pen color** and **DC brush color**, via respectively `SetDCPenColor` and `SetDCBrushColor`. These colors are only *used* when the **stock objects** you get from respectively `GetStockObject(DC_PEN)` and `GetStockObject(DC_BRUSH)` are selected in the device context.

My experimentation showed that in Windows 11 these are not the default objects in a DC from `GetDC(0)`, so it’s necessary to explicitly select them:

*[part-05/code/on-screen-graphics/v2/main.cpp](part-05/code/on-screen-graphics/v2/main.cpp)*:

```cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>

void draw_on( const HDC canvas, const RECT& area )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );

    // Clear the background to blue.
    SetDCBrushColor( canvas, blue );
    FillRect( canvas, &area, 0 );

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, yellow );
    SetDCBrushColor( canvas, orange );
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

The [current documentation of `SelectObject`](https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectobject) states that the object one selects “must have been created” by one of the functions listed in a table there, which does not include `GetStockObject`. But of course that’s just the usual Microsoft documentation SNAFU. The stock objects would not be useful for anything if they couldn’t be used.

However, the stock objects are special in that they don’t need to and shouldn’t be destroyed via `DeleteObject` (or any other way).

Result: same as before, just with shorter & more clear code.

---

### 5.3. Automate cleanup for device contexts and GDI objects.

<img title="" src="part-05/images/yoda.png" alt="">

To unleash the full power of the GDI, such as using pattern pens and brushes, it's necessary to deal with dynamic creation and destruction of GDI objects. Doing it in C style, as in the first example, is however fragile and verbose. But you can automate the `DeleteObject` object destruction calls via C++ destructors, the C++ [RAII](https://en.wikipedia.org/wiki/Resource_acquisition_is_initialization) technique, to make such code shorter and safer. Essentially it means defining small handle ownership classes such as `Brush` and `Pen`. For simplicity and efficiency these classes can be made non-copyable.

The RAII idea of leveraging C++ construction and destruction can also be applied to device contexts, e.g. (because device context destruction depends on the kind of DC) via a general abstract base class `Dc` with concrete derived classes such as `Screen_dc` and `Bitmap_dc`.

For exception safety — to be able to use exceptions freely — even the `SelectObject` call pairs can/should be automated via C++ construction and destruction, e.g. a class `Dc::Selection` whose instances retain the requisite information to undo the selection.

Finally, most of these objects will ordinarily be very short lived ones, created for single calls of graphics primitives such as `FillRect` and `Ellipse`.

To reduce or eliminate a phletora of named short lived variables one can support *implicit creation* of the objects via operators such `+` or `->`, using the same return-reference-to-self *call chaining* idea as with iostream `<<` expressions.

For example, “adding” a device context and e.g. a temporary GDI pen object handle with `+` can result in a temporary `Pen` object (ensuring `DeleteObject`) and a `Dc::Selection` instance that converts implicitly to `HDC`, so that such an expression can be used where an `HDC` is required, and where further addition can be chained on the first one. The key C++ language support for this is that a temporary object is destroyed at the end of the full-expression, not immediately after the function call it appears as argument to. So the temporary objects created by `+` arguments persist until the full expression, e.g. with the outermost level a call of `FillRect` or `Ellipse`, has been evaluated.

Using such machinery the example with dynamic creation of brushes and pens can be rewritten as much shorter and now exception safe C++ level code:

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

Oh, the Yoda picture is really about absorbing a great destructive force rather than generating a constructive force. But it looks forceful. And I like Yoda. ☺

---

For the implementation of GDI object RAII wrappers I found it useful to define a little **type list** class template,

in *[part-05/code/.include/cpp/util.hpp](part-05/code/.include/cpp/util.hpp)*:

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
            // Deletion may possibly fail if the object is selected in a device context.
            const bool deleted = ::DeleteObject( m_handle );
            assert(( "DeleteObject", deleted ));  (void) deleted;
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

Start of *[part-05/code/.include/winapi/gui/device-contexts.hpp](part-05/code/.include/winapi/gui/device-contexts.hpp)*:

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


    class Bitmap_dc: public Memory_dc
    {
        Bitmap* m_p_bitmap;

    public:
        Bitmap_dc( const Explicit_ref_<Bitmap> bitmap ):
            Memory_dc(),
            m_p_bitmap( &bitmap.get() )
        {
            SelectObject( handle(), m_p_bitmap->handle() );
        }

        auto bitmap() const -> const Bitmap& { return *m_p_bitmap; }
    };


    ⋮
```

&hellip; where, in the `Bitmap_dc` constructor, `Explicit_ref_` is a just tiny helper class that derives from and is initialized with a `std::reference_wrapper`, that makes clear in using code that the argument (e.g. `std::ref(image)`) bitmap is referenced, not copied.

---

The temporary GDI object automation via `+` is very closely tied to the `Dc::Selection` class, so I present it all together in one little package. This class is full of subtleties and I’m not sure my design choices here were the best. I’m not even sure that I managed to make it difficult to use incorrectly, but anyway, the complexity is in large part due to that *goal*.

End of *[part-05/code/.include/winapi/gui/device-contexts.hpp](part-05/code/.include/winapi/gui/device-contexts.hpp)*:

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

### 5.4. Save the generated graphics to an image file.

A modern “.jpg” or “.png” image would be perfect for communicating one's result to others, or for using it in other work. However, the only file format supported by the GDI is the archaic [“.wmf”](https://en.wikipedia.org/wiki/Windows_Metafile) and possibly its cousin “.emf”. This is a Windows-specific binary vector graphics format with little to no support today, not even in Windows Explorer.

The GDI successor technologies GDI+ and DirectX are for later parts of the tutorial (hopefully). For now let’s stick to the basic original, simple GDI. What  options does one then have for saving a graphics result — other than “.wmf”?

Well, there are two ways to save a GDI graphics result as a now reasonably [portable](https://en.wikipedia.org/wiki/BMP_file_format#Usage_of_BMP_format) “**.bmp**” image file, namely

* directly [generate the binary contents](https://docs.microsoft.com/en-us/windows/win32/gdi/storing-an-image) of such a file (the DIY approach), or
* use the [**`OleSavePictureFile`**](https://docs.microsoft.com/en-us/windows/win32/api/olectl/nf-olectl-olesavepicturefile) function.

Here we’ll use the API function because that involves *learning about how to use such Windows API functions*, which is generally useful knowledge, and because this is information that’s difficult to come by nowadays.

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
    draw_on( Bitmap_dc( ref( image ) ), RECT{ 0, 0, w, h } );
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

A call of the `OleSavePictureFile` function drags in a lot of complexity because it’s strongly coupled to things in the [**OLE library**](https://en.wikipedia.org/wiki/Object_Linking_and_Embedding). Since OLE is now mostly old irrelevant technology (except for `OleSavePictureFile` and few other still useful functions) I only discuss how to use it. OLE sits or sat on top of the [**COM infra-structure**](https://en.wikipedia.org/wiki/Component_Object_Model), the same object/component infra-structure that as of 2022 serves as foundation for the [**Windows Runtime**](https://en.wikipedia.org/wiki/Windows_Runtime) a.k.a. WinRT.

COM and OLE were designed as callable from both C and C++, in the 1990’s. Every function returns a 32-bit structured **result code** of type [**`HRESULT`**](https://en.wikipedia.org/wiki/HRESULT)that tells whether the call failed or succeeded, and roughly how. A great many of these codes have names, like `E_FAIL` (a general failure), `S_OK` and `S_FALSE` (two different success codes, respectively 0 and 1). Essentially such a code denotes failure if the most significant bit, bit 31, the sign bit, is 1. But instead of checking whether a “hr” code is negative one can and should use the macros **`FAILED`** and **`SUCCEEDED`**:

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

For completeness, the — or a possible — definition:

*[part-05/code/.include/winapi/com/failure-checking.hpp](part-05/code/.include/winapi/com/failure-checking.hpp)*:

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>                     // cpp::util::Success
#include <wrapped-winapi/windows-h.hpp>     // HRESULT, SetLastError, GetLastError

namespace winapi::com {
    inline namespace failure_checking {
        using cpp::util::Success;

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



asdasd



Before calling most any COM function the COM library must have been initialized via a call to **`CoInitialize`**. Happily a function that needs some COM-based functionality can do that locally, because nested calls of `CoInitialize` can succeed if the parameters are compatible with the original call. Each call of `CoInitialize` must eventually be paired with a corresponding call of **`CoUninitialize`**.

When you use OLE there is correspondingly **`OleInitialize`** and **`OleUninitialize`**, which are used in the same way, and which take care of calling respectively `CoInitialize` and `CoUninitialize`:







The `gdi::save_to` function used in the above main program code is a quite thin wrapper over a function `ole::save_to`, that in turn wraps use of the Windows API function 

---

### 5.x. Draw UTF-8 text by converting to UTF-16 and using the wide text API.

As of early 2022 GDI’s `char` based text drawing functions unfortunately assume the global Windows ANSI encoding instead of the process’ ANSI encoding. The `wchar_t` based functions work. But with the incorrect encoding assumption the result of drawing `char` based international text can be a lot of gobbledygook:

![Default result of drawing UTF8 text](file://D:\root\current work\Windows GUI-stuff in C++ (tutorial)\docs\part-05\images\sshot-4.mangled-utf-8.cropped.png)

Not only is the text “Every 日本国 кошка loves Norwegian blåbærsyltetøy!” mangled, but it’s also drawn with an old 1980’s raster font. As I see it this double whammy of problems is due to a Microsoft business tactic of not fixing or updating what they’ve made, but just moving on to ever more fancy stuff (like GDI+ and DirectX, with their own problems). However, we obtained a handle to the modern GUI font [in part 3](part-03.md#34-fix-gross-imperfections-standard-font-window-just-on-top-modern-appearance), and all that remains is a call of `SelectObject`, so the main problem is the text mangling.

Full code for this example:

*[part-05/code/on-screen-graphics/v3/resources/app-manifest.xml](part-05/code/on-screen-graphics/resources/app-manifest.xml)*:

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
    <assemblyIdentity
        version="0.3.0.0"
        processorArchitecture="*"
        name="Alfs.CppInPractice.GDI-stuff"
        type="win32"
    />
    <description>Exemplifies that GDI text drawing doesn't support UTF-8 code page.</description>
    <application>
        <windowsSettings>
            <activeCodePage xmlns="http://schemas.microsoft.com/SMI/2019/WindowsSettings"
                >UTF-8</activeCodePage>
        </windowsSettings>
    </application>
</assembly>
```

*part-05/code/on-screen-graphics/v3/resources.rc*:

```c
#pragma code_page( 65001 )  // UTF-8
#include <windows.h>


/////////////////////////////////////////////////////////////////////////////
// Neutral resources
LANGUAGE LANG_NEUTRAL, SUBLANG_NEUTRAL

CREATEPROCESS_MANIFEST_RESOURCE_ID      RT_MANIFEST "resources/app-manifest.xml"
```

*part-05/code/on-screen-graphics/v3/main.cpp*:

```cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <string_view>      // std::string_view
#include <iterator>         // std::size

#include <assert.h>

using   std::string_view, std::size;

template< class T > auto int_size( const T& o ) -> int { return static_cast<int>( size( o ) ); }

void draw_on( const HDC canvas, const RECT& area )
{
    constexpr auto  white       = COLORREF( RGB( 0xFF, 0xFF, 0xFF ) );  (void) white;   // Unused.
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  black       = COLORREF( RGB( 0, 0, 0 ) );

    // Clear the background to blue.
    SetDCBrushColor( canvas, blue );
    FillRect( canvas, &area, 0 );

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, yellow );
    SetDCBrushColor( canvas, orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );

    // Draw some international text. Note: non-ASCII UTF-8 characters are incorrectly rendered.
    constexpr auto text = string_view( "Every 日本国 кошка loves\nNorwegian blåbærsyltetøy!" );
    auto text_rect = RECT{ area.left + 40, area.top + 150, area.right, area.bottom };
    SetTextColor( canvas, black );              // This is also the default, but making it explicit.
    SetBkMode( canvas, TRANSPARENT );           // Don't fill in the background of the text, please.
    DrawText( canvas, text.data(), int_size( text ), &text_rect, DT_LEFT | DT_TOP | DT_NOPREFIX );
}

auto main() -> int
{
    assert( GetACP() == CP_UTF8 );
    constexpr auto  no_window   = HWND( 0 );

    const HDC canvas = GetDC( no_window );
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );

    draw_on( canvas, RECT{ 10, 10, 10 + 400, 10 + 400 } );

    ReleaseDC( no_window, canvas );
}
```

And for completeness, building with Visual C++:

```txt
[T:\part-05\code\on-screen-graphics\v3\.build]
> rc /nologo /c 65001 /fo r.res ..\resources.rc

[T:\part-05\code\on-screen-graphics\v3\.build]
> set common-code=t:\part-05\code\.include

[T:\part-05\code\on-screen-graphics\v3\.build]
> cl /I %common-code% ..\main.cpp user32.lib gdi32.lib r.res /Feb
main.cpp
```

… and with MinGW g++:

```txt
[T:\part-05\code\on-screen-graphics\v3\.build]
> windres ..\resources.rc -o res.o

[T:\part-05\code\on-screen-graphics\v3\.build]
> set common-code=t:\part-05\code\.include

[T:\part-05\code\on-screen-graphics\v3\.build]
> g++ -std=c++17 -I %common-code% ..\main.cpp res.o -lgdi32
```

❁ ❁ ❁

In the above code `<windows.h>` defined the `DrawText` macro as `DrawTextA`, the `char` based wrapper version of this function. The basic `wchar_t` based version, `DrawTextW`, doesn’t have an encoding assumption problem because it deals with only one encoding, namely UTF-16. So we just need the text re-encoded as UTF-16.

Windows provides the `MultiByteToWideChar` and `WideCharToMultiByte` functions to convert to and from UTF-16. These functions assume that the input is a sequence of complete code point specifications, i.e. that the input doesn’t start or end in the middle of a UTF-8 code point sequence or in the middle of a UTF-16 surrogate pair. However that’s usually the case and anyway easy to arrange, and this assumption makes the functions stateless, easy to use.

The functions can be used to just determine the necessary minimum size of the output buffer, quite common in C-oriented libraries. However for conversion UTF-8 → UTF-16 that preparation is not necessary, it would just introduce an inefficiency, because the UTF-8 number of bytes is a known sufficient buffer size for the UTF-16 text. More precisely because all Unicode code points that are single byte as UTF-8 are single value as UTF-16, and UTF-16 never uses more than two values per code point.

*part-05/code/convert-to-wide-text/main.cpp*:

```cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <stdlib.h>         // EXIT_FAILURE
#include <string>           // std::wstring;
#include <string_view>      // std::string_view
using   std::wstring, std::string_view;

auto main() -> int
{
    constexpr auto& text        = "Every 日本国 кошка loves\nNorwegian blåbærsyltetøy!";
    constexpr auto  text_length = static_cast<int>( string_view( text ).length() );

    constexpr auto buffer_size = text_length;
    auto wide_text = wstring( buffer_size, L'\0' );
    const auto flags = DWORD( 0 );
    const int n_wide_values = MultiByteToWideChar(
        CP_UTF8, flags, text, text_length, &wide_text[0], buffer_size
        );
    if( n_wide_values == 0 ) { return EXIT_FAILURE; }
    wide_text.resize( n_wide_values );

    MessageBoxW( 0, wide_text.c_str(), L"UTF-16 text:", MB_ICONINFORMATION | MB_SETFOREGROUND );
}
```

![Wide text message box](file://D:\root\current work\Windows GUI-stuff in C++ (tutorial)\docs\part-05\images\sshot-5.wide-text-messagebox.png)

❁ ❁ ❁

With now confidence that the convert-to-UTF-16 approach works we can express the text drawing as reusable machinery,

In *part-05/code/.include/cpp/util.hpp*:

```cpp
    #define CPPUTIL_FAIL( s ) ::cpp::util::fail( std::string( __func__ ) + " - " + (s) )

    template< class T >
    auto int_size( const T& c ) -> int { return static_cast<int>( size( c ) ); }
```

*part-05/code/.include/winapi/encoding-conversions.hpp*:

```cpp
#pragma once // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <cpp/util.hpp>     // CPPUTIL_FAIL, cpp::util::(hopefully, int_size)

#include <string>           // std::wstring
#include <string_view>      // std::string_view
#include <utility>          // std::move

namespace winapi {
    namespace cu = cpp::util;
    using   cu::hopefully, cu::fail, cu::int_size;
    using   std::wstring,
            std::string_view,
            std::move;

    inline auto to_utf16( const string_view& s, wstring result_buffer = {} )
        -> wstring
    {
        const auto s_length = int_size( s );
        if( s_length == 0 ) { return L""; }

        const auto buffer_size = s_length;  // May be a litte too large, but that's OK.
        result_buffer.resize( buffer_size );
        constexpr auto flags = DWORD( 0 );
        const int n_wide_values = MultiByteToWideChar(
            CP_UTF8, flags, s.data(), s_length, &result_buffer[0], buffer_size
            );
        hopefully( n_wide_values != 0 ) or CPPUTIL_FAIL( "MultiByteToWideChar failed" );
        result_buffer.resize( n_wide_values );
        return move( result_buffer );
    }
}  // namespace winapi
```

*part-05/code/.include/winapi/gdi-text-display.hpp*:

```cpp
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
```

I’ve intentionally refrained from adressing the problem with introducing arbitrary implementation stuff such as standard library names, in the namespace that exports things. Fixing that is trivial but would reduce clarity by introducing additional COBOL-like verbosity, because C++17 lacks support for namespace [information hiding](https://en.wikipedia.org/wiki/Information_hiding). Alternatively one could use C++20 modules.

However, this code introduces the *possibility of failure*, i.e. an exception. But this is purely a technical possibility because per the documentation in modern Windows `MultiByteToWideChar` doesn’t ignore malformed UTF-8 sequences but replaces each with Unicode code point `L'\uFFFD'`, i.e. *some* output. Thus, except for the really remote possibility of failure due to internal resource exhaustion or the like, there’s no way that the result can be an empty string.

And thus the new version of the main program just assumes that there is no exception:

*part-05/code/on-screen-graphics/v4/main.cpp*:

```cpp
# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/encoding-conversions.hpp>  // winapi::to_utf16
#include <winapi/gdi-text-display.hpp>      // winapi::gdi::draw_text
#include <winapi/gui-util.hpp>              // winapi::gui::std_gui_font

#include <string_view>      // std::string_view
#include <iterator>         // std::size

#include <assert.h>

namespace gdi   = winapi::gdi;
using   std::string_view, std::size;

void draw_on( const HDC canvas, const RECT& area )
{
    constexpr auto  white       = COLORREF( RGB( 0xFF, 0xFF, 0xFF ) );  (void) white;   // Unused.
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  black       = COLORREF( RGB( 0, 0, 0 ) );

    // Clear the background to blue.
    SetDCBrushColor( canvas, blue );
    FillRect( canvas, &area, 0 );

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, yellow );
    SetDCBrushColor( canvas, orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );

    // Draw some international (English, Russian, Chinese, Norwegian) text.
    constexpr auto text = string_view( "Every 日本国 кошка loves\nNorwegian blåbærsyltetøy!" );
    SetTextColor( canvas, black );              // This is also the default, but making it explicit.
    auto text_rect = RECT{ area.left + 40, area.top + 150, area.right, area.bottom };
    gdi::draw_text( canvas, text, text_rect );
}

void init( const HDC canvas )
{
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );
    SetBkMode( canvas, TRANSPARENT );           // Don't fill in the background of text, please.
    SelectObject( canvas, winapi::gui::std_gui_font.handle );
}

auto main() -> int
{
    assert( GetACP() == CP_UTF8 );
    constexpr auto  no_window   = HWND( 0 );

    const HDC canvas = GetDC( no_window );
    init( canvas );

    draw_on( canvas, RECT{ 10, 10, 10 + 400, 10 + 400 } );

    ReleaseDC( no_window, canvas );
}
```

Building examples were provided earlier; just note, if you don’t look at them, that since this code uses the `char` based Windows API functions with UTF-8 encoding, i.e. since it assumes and requires UTF-8 as the process’ ANSI code page, it’s necessary to compile and link with the resources file that provides a minimal application manifest.

![Presentation of international text](file://D:\root\current work\Windows GUI-stuff in C++ (tutorial)\docs\part-05\images\sshot-6.international-text.cropped.png)

---

asdasd

---

---

### 5.3. In passing: support `assert` messages in a GUI program built with Visual C++.

To be sure that the `assert` statements really do their job you can intentionally trigger an assert, make it “fire”, e.g. by changing

```cpp
    assert(( m_dc != 0 ));
```

… to

```cpp
    assert(( m_dc != 0, false ));
```

With a MinGW g++ console subsystem build this works fine; in that the assertion text is reported in the console:

```txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> g++ -std=c++17 -I %common-code% ..\main.cpp -lgdi32

[T:\part-05\code\on-screen-graphics\v2\.build]
> a
Assertion failed: ( m_dc != 0, false ), file t:\part-05\code\.include/winapi/gdi.hpp, line 23
```

With a MinGW g++ GUI subsystem build it also works fine, producing an **assertion failure box**:

```txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> g++ -std=c++17 -I %common-code% ..\main.cpp -lgdi32 -mwindows

[T:\part-05\code\on-screen-graphics\v2\.build]
> a
```

![The assertion box](part-05/images/sshot-2.assertion-box.png)

And, it works fine with a Visual C++ console program build:

```txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> cl /I %common-code% ..\main.cpp user32.lib gdi32.lib /Feb
main.cpp

[T:\part-05\code\on-screen-graphics\v2\.build]
> b
Assertion failed: ( m_dc != 0, false ), file t:\part-05\code\.include\winapi/gdi.hpp, line 23
```

But with a Visual C++ GUI subsystem build the assertion message is not reported in any way. Indeed there’s ***nothing to tell you that an assertion fired***. All you can see is that the program mysteriously fails to have any effect:

```txt
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
```

The technical reason is that Microsoft’s runtime library decides how to present the assertion message, as text output or in an assertion failure box, based on ***which entry point function*** one linked with:

| Entry point:         | Calls startup function: | Assumed subsystem: | Presentation of assertion message: |
| -------------------- | ----------------------- | ------------------ | ---------------------------------- |
| `mainCRTStartup`     | `main`                  | Console            | Text output.                       |
| `wmainCRTStartup`    | Microsoft `wmain`       | Console            | Text output.                       |
| `WinMainCRTStartup`  | Microsoft `WinMain`     | GUI                | Assertion failure box.             |
| `wWinMainCRTStartup` | Microsoft `wWinMain`    | GUI                | Assertion failure box.             |

From a design level perspective, if the intent was to support the programmer, then a decision to base the presentation mode on the executable’s subsystem is sub-optimal. For example, a GUI subsystem program may be running with its standard error stream tied to a console, so that presentation as text output is desirable. And in most cases it supports the programmer best to have both presentation modes, both text output that can be logged and an assertion failure box that can be viewed when the assertion failure happens; a restriction of the presentation to a single mode very seldom has any advantage.

**\<rant\>** Then, a decision to *infer* the subsystem from the entry point, instead of checking the subsystem, is a bit of lunacy. But perhaps the intent was not to support but instead to add yet another vendor lock in, and after all, this is tied to the `WinMain` monstrosity, which itself was a vendor lock in device. Microsoft got infamous for its vendor lock in business tactics when some [internal mails were revealed](https://www.cnet.com/tech/services-and-software/eu-report-takes-microsoft-to-task/) in the legal dispute between Microsoft and Sun Corporation. **\</rant\>**

Anyway, the assertion message wasn’t actually suppressed: it was just erroneously presented as text output on the program’s standard error stream, that wasn’t connected to anything. So a simple fix is to connect that error stream to the console. To do that in Cmd, simply redirecting the standard error stream via `2>con` doesn’t work, but fusing it into the standard output stream via `2>&1` and then piping the output to e.g. `find /v ""`, works nicely:

```txt
[T:\part-05\code\on-screen-graphics\v2\.build]
> b 2>&1 | find /v ""
Assertion failed: ( m_dc != 0, false ), file t:\part-05\code\.include\winapi/gdi.hpp, line 23
```

It would be much better if the program itself chose a more reasonable practically useful presentation mode. For example, depending on whether its standard error stream is connected to something, as it is with the above command, or not. Doing this involves ***lying*** to the Microsoft runtime about the executable’s subsystem (like with a sabotage-minded donkey that always goes in the opposite direction of where you indicate you want to go, so you just lie to it), but happily that Just Works&trade; with as of Visual C++ 2022 no ill effects:

[*part-05/code/.include/compiler/msvc/Assertion_reporting_fix.hpp*](part-05/code/.include/compiler/msvc/Assertion_reporting_fix.hpp)

```cpp
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
```

Here [**`_set_app_type`**](https://docs.microsoft.com/en-us/cpp/c-runtime-library/set-app-type?view=msvc-170) is an internal function in Microsoft’s runtime library.

The above is intended to be used like this:

```cpp
#ifdef _MSC_VER
#   include <compiler/msvc/Assertion_reporting_fix.hpp>
    const bool msvc_arf = compiler::msvc::Assertion_reporting_fix::global_instantiation();
#endif
```

The [Meyers’ singleton](https://stackoverflow.com/a/17712497) ensures that this results in a single call of `_set_app_type` even if this code appears in several translation units. I chose to exemplify this general `global_instantiation` technique even though the `_set_app_type` calls are idempotent, because it’s a good way to provide such fixes in general. A C++17 `inline` variable’s initialization could alternatively have done the job, but `inline` variables appear to still be somewhat unreliable in Visual C++.

With this, if you don’t redirect standard error, then with GUI subsystem you get the assertion failure box (the very same as shown earlier for MinGW g++, because it’s the same library in both cases), but if you do connect up the standard error stream then you get the message as text output, so you can decide the reporting mode in the command to run the program.

However, for more [DRY code](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself) the above usage is placed in a convenience header:

*[part-05/code/.include/compiler/msvc/Assertion_reporting_fix.auto.hpp](part-05/code/.include/compiler/msvc/Assertion_reporting_fix.auto.hpp)*

```cpp
#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#ifdef _MSC_VER
#   include <compiler/msvc/Assertion_reporting_fix.hpp>
    const bool msvc_arf = compiler::msvc::Assertion_reporting_fix::global_instantiation();
#endif
```

And so all that appears in v3 of the program is an include of the above file:

*In [part-05/code/on-screen-graphics/v3/main.cpp](part-05/code/on-screen-graphics/v3/main.cpp)*

```cpp
#include <compiler/msvc/Assertion_reporting_fix.auto.hpp>
```

---

### 5.4. A potpourri of GDI things introduced via a C curve example.

One of my pet pedagogical ideas is that the things that are most important are the things one ends up using or needing in concrete examples. So, as a concrete example I chose a simple recursive figure called the [C curve](https://en.wikipedia.org/wiki/L%C3%A9vy_C_curve). One way to create it is to start with a single line segment **–**, replace each line segment with an angle bracket **∨**, do that again, and so on:

<img src="part-05/images/C-curve-construction.png" width="340">

The above figure is based on [an image from Wikimedia](https://en.wikipedia.org/wiki/File:Levy_C_construction.png).

Using the above conceptual construction directly in code has the advantage that the starting line defines the direction and size of the result curve, but it has the problems that the generated (line start and end-) points are not necessarily at integer pixel coordinates, and that one may end up with diagonal lines that don’t look so good without anti-aliasing (GDI).

An alternative is to generate the curve with essentially [turtle graphics](https://en.wikipedia.org/wiki/Turtle_graphics), just tracing the curve from start to end, recreating its logical structure  — the left and right turnings — without regard for absolute orientation or size. This yields pixel perfect line segments that are either horizontal or vertical and always start and end on integer pixel coordinates, but the curve gets larger and rotates 45° when the number of levels is increased by 1. Anyway, with this approach one doesn’t need any graphics library or geometry maths to generate the curve points, just an understanding of recursive functions:

```cpp
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
```

That code gets a little obfuscated, sorry, by having the details filled in and being expressed as something easy to *use*, as opposed to easy to grok:

*[part-05\code\c-curve\v1\c_curve.hpp](part-05\code\c-curve\v1\c_curve.hpp)*

```cpp
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
```

```cpp

```
