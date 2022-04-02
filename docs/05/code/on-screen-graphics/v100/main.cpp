# // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi"). v100
#include <cpp/util.hpp>                     // hopefully, fail
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/com/failure-checking.hpp>  // winapi::com::operator>>
#include <winapi/com/Ptr_.hpp>              // winapi::com::Ptr_
#include <winapi/kernel/encoding-conversions.hpp>  // winapi::to_utf16
#include <winapi/gdi/Bitmap_32.hpp>         // winapi::gdi::Bitmap_32
#include <winapi/gdi/bitmap-util.hpp>       // winapi::gdi::save_to
#include <winapi/gdi/device-contexts.hpp>   // winapi::gdi::(Screen_dc, Memory_dc, Bitmap_dc)
#include <winapi/gui/util.hpp>              // winapi::gui::std_gui_font
#include <winapi/ole/B_string.hpp>          // winapi::ole::B_string
#include <winapi/ole/Library_usage.hpp>     // winapi::ole::Library_usage
#include <winapi/ole/picture-util.hpp>      // winapi::ole::picture_from

// #include <shlwapi.h>        // SHCreateStreamOnFileEx
#include <io.h>             // _get_osfhandle
#include <stdio.h>          // fprintf, _fileno
#include <stdlib.h>         // EXIT_...

#include <fstream>          // std::ofstream
#include <stdexcept>        // std::exception&
#include <string>           // std::string
#include <string_view>      // std::(string_view, wstring_view)
#include <utility>          // std::ref

namespace cu  = cpp::util;
namespace com = winapi::com;
namespace ole = winapi::ole;
namespace gdi = winapi::gdi;
using   cu::hopefully, cu::fail, cu::success, cu::No_copying, cu::int_size, cu::Const_, cu::Range;
using   com::failure_checking::operator>>;
using   ole::save_to;
using   gdi::Bitmap, gdi::Bitmap_32, gdi::Bitmap_dc, gdi::bitmap_in, gdi::save_to;
using   std::string, std::to_string,
        std::string_view, std::wstring_view,
        std::ref;

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

void init( const HDC canvas )
{
    SelectObject( canvas, GetStockObject( DC_PEN ) );
    SelectObject( canvas, GetStockObject( DC_BRUSH ) );
    SetBkMode( canvas, TRANSPARENT );       // Don't fill in the background of text, please.
    SelectObject( canvas, winapi::gui::std_font.handle() );
}

using C_str = const char*;

struct Dib_format               // See documentation of `BITMAPINFOHEADER::biBitCount`.
{ enum Enum{
    implied                 = 0,
    monochrome              = 1,
    palette_16_colors       = 4,
    palette_256_colors      = 8,
    rgb_compressed          = 16,
    rgb_24_bits             = 24,
    rgb_32_bits             = 32
}; };

auto create_rgb24_in_mmfile( const HANDLE mm_section, const int offset, const int width, const int height )
    -> HBITMAP
{
    BITMAPINFO params  = {};
    BITMAPINFOHEADER& info = params.bmiHeader;
    info.biSize         = {sizeof( info )};
    info.biWidth        = width;
    info.biHeight       = height;
    info.biPlanes       = 1;
    info.biBitCount     = Dib_format::rgb_24_bits;
    info.biCompression  = BI_RGB;

    // void* p_bits;
    SetLastError( 666 );
    const HBITMAP handle = CreateDIBSection(
        HDC(),              // Not needed because no DIB_PAL_COLORS palette.
        &params,
        DIB_RGB_COLORS,     // Irrelevant, but.
        nullptr, //&p_bits,
        mm_section,         // Section.
        offset              // Section offset.
        );
    using namespace std::string_literals;
    hopefully( handle != 0 ) or CPPUTIL_FAIL( "CreateDibSection failed, code "s + to_string( GetLastError() ) );
    // (void) p_bits;
    return handle;
}

auto size( const HBITMAP bitmap )
    -> SIZE
{
    BITMAP info;
    GetObject( bitmap, int( sizeof( BITMAP ) ), &info );
    return {info.bmWidth, info.bmHeight};
}

auto file_size( const HBITMAP bitmap )
    -> int
{
    BITMAP info;
    GetObject( bitmap, int( sizeof( BITMAP ) ), &info );
    const int bytes_per_row = info.bmWidthBytes;
    const int h = info.bmHeight;
    return int( sizeof( BITMAPFILEHEADER ) + sizeof( BITMAPINFOHEADER ) + bytes_per_row*h );
}

struct Bitmap_info
{
    const int   width;
    const int   height;
    const int   bytes_per_pixel;
    
    Bitmap_info( const int w, const int h, const int bypp ):
        width( w ), height( h ), bytes_per_pixel( bypp )
    { assert( is_in( Range{ 3, 4 }, bytes_per_pixel ) ); }

    auto bytes_per_row() const
        -> int
    { return 2*((bytes_per_pixel*width + 1)/2); }

    auto n_bytes_for_data() const
        -> int
    { return height*bytes_per_row(); }

    auto image_size() const
        -> int
    { return int( sizeof( BITMAPINFOHEADER ) ) + n_bytes_for_data(); }

    static auto file_offset_of_image()
        -> int
    { return int( sizeof( BITMAPFILEHEADER ) ); }

    auto file_size() const
        -> int
    { return file_offset_of_image() + image_size(); }
};

namespace bitmap {
    struct File_header: BITMAPFILEHEADER
    {
        File_header(): BITMAPFILEHEADER()
        {
            memcpy( &bfType, "BM", 2 );
            // TODO: offset bits
        }
    };
    static_assert( sizeof( File_header ) == sizeof( BITMAPFILEHEADER ) );
}

class C_file
{
    FILE*   m_f;
    
public:
    ~C_file() { fclose( m_f ); }
    C_file( Const_<FILE*> f ): m_f( f ) {}

    operator FILE*() const { return m_f; }
};

class File_mapping
{
    HANDLE  m_handle;

    static auto create_mapping( const HANDLE f, const int size )
        -> HANDLE
    {
        return CreateFileMapping(
            f,                          // HANDLE                hFile,
            nullptr,                    // LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
            PAGE_READWRITE,             // DWORD                 flProtect,
            // PAGE_WRITECOPY,             // DWORD                 flProtect,
            0,                          // DWORD                 dwMaximumSizeHigh,
            size,                       // DWORD                 dwMaximumSizeLow,
            "mm-generated-image" //nullptr                     // LPCSTR                lpName
            );
    }

public:
    ~File_mapping() { CloseHandle( m_handle ); }
    
    File_mapping( const HANDLE f, const int size ):
        m_handle( create_mapping( f, size ) )
    {
        using namespace std::string_literals;
        hopefully( m_handle != 0 )
            or CPPUTIL_FAIL( "CreateFileMapping failed, code "s + to_string( GetLastError() ) );
    }
    
    auto handle() const -> HANDLE { return m_handle; }
};

void mm_save_to( const C_str file_path, const HBITMAP bitmap )
{
    // printf( "%dx%d, %d bytes.\n", w, h, file_size( bitmap ) );
    // printf( "%dx%d, %d bytes.\n", w, h, Bitmap_info( w, h, 3 ).file_size() );
    // (void) file_path; (void) bitmap;
    
    const auto& binary_read_write = "w+b";
    fprintf( stderr, "%s\n", "/ Before fopen." );
    const auto f = C_file( fopen( file_path, binary_read_write ) );
    fprintf( stderr, "%s\n", "/ After fopen." );
    hopefully( f ) or CPPUTIL_FAIL( "Failed to create output file" );

    const auto f_handle = reinterpret_cast<HANDLE>( _get_osfhandle( _fileno( f ) ) );
    hopefully( f_handle ) or CPPUTIL_FAIL( "Failed to obtain Windows file handle." );

    const auto header = bitmap::File_header();
    fwrite( &header, int( sizeof( header ) ), 1, f )
        == 1 or CPPUTIL_FAIL( "Failed to write file header." );

    const auto [w, h]       = size( bitmap );
    const auto bitmap_info  = Bitmap_info( w, h, 3 );
    const auto mmf = File_mapping( f_handle, bitmap_info.file_size() + 1000 ); // Expands file.
    auto image_in_file = Bitmap( create_rgb24_in_mmfile(
        mmf.handle(), Bitmap_info::file_offset_of_image(), w, h
        ) );
    auto src = Bitmap_dc( bitmap );
    auto dst = Bitmap_dc( image_in_file );
    BitBlt( dst, 0, 0, w, h, src, 0, 0, SRCCOPY )
        or CPPUTIL_FAIL( "Copying the bitmap via BitBlt failed." );
}

void display_graphics()
{
    const int width     = 400;
    const int height    = 400;
    auto bitmap = Bitmap_32( width, height );
    const auto dc = winapi::gdi::Bitmap_dc( bitmap );
    init( dc.handle() );
    display_graphics_on( dc.handle() );
    //BitBlt( Screen_dc().handle, 15, 15, width, height, dc.handle(), 0, 0, SRCCOPY );
    mm_save_to( "mm-generated-image.bmp", bitmap );
}

auto main( int, char** args ) -> int
{
    using   std::exception;
    try {
        const ole::Library_usage _;
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
