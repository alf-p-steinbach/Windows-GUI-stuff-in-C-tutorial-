﻿# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <cpp/util.hpp>     // hopefully, fail
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/ole2.hpp>  // Ole_library_usage

#include <stdio.h>          // fprintf
#include <stdlib.h>         // EXIT_...
#include <olectl.h>         // OleCreatePictureIndirect

#include <stdexcept>        // std::exception&
#include <string>           // std::string

namespace cu = cpp::util;
using   cu::hopefully, cu::fail;
using   std::string;

void display_graphics_on( const HDC canvas )
{
    constexpr auto  orange      = COLORREF( RGB( 0xFF, 0x80, 0x20 ) );
    constexpr auto  yellow      = COLORREF( RGB( 0xFF, 0xFF, 0x20 ) );
    constexpr auto  blue        = COLORREF( RGB( 0, 0, 0xFF ) );
    constexpr auto  area        = RECT{ 0, 0, 400, 400 };
    
    SetDCBrushColor( canvas, blue );
    FillRect( canvas, &area, 0 );   // `0` works for me, but should perhaps be the DC brush.

    // Draw a yellow circle filled with orange.
    SetDCPenColor( canvas, yellow );
    SetDCBrushColor( canvas, orange );
    Ellipse( canvas, area.left, area.top, area.right, area.bottom );
}

void display_graphics()
{
    constexpr auto no_window = HWND( 0 );
 
    struct Screen_dc: cu::No_copying
    {
        HDC     handle;
            
        ~Screen_dc() { ReleaseDC( no_window, handle ); }

        Screen_dc():
            handle( GetDC( no_window ) )
        {
            hopefully( handle != 0 )
                or fail( "GetDC() failed" );
        }
    };

    struct Memory_dc: cu::No_copying
    {
        HDC     handle;
            
        ~Memory_dc() { DeleteDC( handle ); }

        Memory_dc( const HDC properties ):
            handle( CreateCompatibleDC( properties ) )
        {
            hopefully( handle != 0 )
                or fail( "CreateCompatibleDC() failed" );
            SelectObject( handle, GetStockObject( DC_PEN ) );
            SelectObject( handle, GetStockObject( DC_BRUSH ) );
        }
    };

    struct Bitmap_dc: Memory_dc
    {
        ~Bitmap_dc()
        {
            DeleteObject( GetCurrentObject( handle, OBJ_BITMAP ) );
        }

        Bitmap_dc( const HDC properties, const int width, const int height ):
            Memory_dc( properties )
        {
            const HBITMAP bmp = CreateCompatibleBitmap( properties, width, height );
            BITMAP info = {};
            GetObject( bmp, sizeof( info ), &info );
            fprintf( stderr, "planes = %ld, bits-per-pixel = %ld, width = %ld\n", info.bmPlanes, info.bmBitsPixel, info.bmWidth );
            hopefully( bmp != 0 )
                or fail( "CreateCompatibleBitmap failed" );
            SelectObject( handle, bmp );
        }
    };

    const auto bitmap_dc = Bitmap_dc( Screen_dc().handle, 400, 400 );
    display_graphics_on( bitmap_dc.handle );
    BitBlt( Screen_dc().handle, 15, 15, 400, 400, bitmap_dc.handle, 0, 0, SRCCOPY );
}

auto main( int, char** args ) -> int
{
    using   std::exception;
    try {
        const winapi::ole2::Ole_library_usage _;
        display_graphics();
        return EXIT_SUCCESS;
    } catch( const exception& x ) {
        fprintf( stderr, "!%s\n", x.what() );
        const auto as_errorbox  = MB_ICONERROR | MB_SYSTEMMODAL;
        const auto title        = string() + args[0] + " failed:";
        const auto text         = string() + "Because:\n" + x.what();
        MessageBox( 0, text.c_str(), title.c_str(), as_errorbox );
    }
    return EXIT_FAILURE;
}
