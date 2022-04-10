#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <wrapped-winapi-headers/windows-h.hpp>

namespace winapi::kernel {
    inline const HINSTANCE this_exe = GetModuleHandle( nullptr );

    struct Resource_id
    {
        int value;
        auto as_pseudo_ptr() const -> const char* { return MAKEINTRESOURCE( value ); }
    };
}  // namespace winapi::kernel
