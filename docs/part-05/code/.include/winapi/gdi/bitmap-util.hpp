#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#include <wrapped-winapi/windows-h.hpp>
#include <winapi/ole/picture-util.hpp>      // winapi::ole::save_to

namespace winapi::gdi {
    namespace ole = winapi::ole;

    inline void save_to( const string_view& file_path, const HBITMAP bitmap )
    {
        ole::save_to( file_path, ole::picture_from( bitmap ) );
    }
}  // namespace winapi::gdi
