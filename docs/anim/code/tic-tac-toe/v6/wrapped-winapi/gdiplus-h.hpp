#pragma once
#include <wrapped-winapi/windows-h.hpp>
#include <objbase.h>                // Macro `MIDL_INTERFACE` etc.

#include <algorithm>
namespace Gdiplus {
    using std::min, std::max;       // Substitutes for Windows macros `min` & `max`.
}

#ifdef _MSC_VER
#   pragma warning( disable: 4458 ) // Local name hides class member name.
#endif

#include <gdiplus.h>
