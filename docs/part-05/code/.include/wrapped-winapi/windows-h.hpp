#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#ifdef UNICODE
#   error "`UNICODE` should not be defined for a `char` based application."
#   include <stop-compilation>
#endif

#undef STRICT
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

#define STRICT                  // C++-compatible declarations (the default now).
#define NOMINMAX                // No `min` and `max` macros, please.
#define WIN32_LEAN_AND_MEAN     // Excludes about half of `<windows.h>` = faster & safer.

#include <windows.h>

#ifdef small
#   error "The macro `small` was defined by <windows.h>. Oops."
#   include <stop-compilation>
#endif
