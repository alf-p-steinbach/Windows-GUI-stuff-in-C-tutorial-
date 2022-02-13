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

        auto create_rgb32(
            const int                   width,
            const int                   height
            ) -> Handle_and_memory
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
                &params,
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
