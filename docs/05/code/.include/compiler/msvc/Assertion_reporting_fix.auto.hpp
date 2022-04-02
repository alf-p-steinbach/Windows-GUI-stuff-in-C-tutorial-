#pragma once    // Source encoding: UTF-8 with BOM (π is a lowercase Greek "pi").
#ifdef _MSC_VER
#   include <compiler/msvc/Assertion_reporting_fix.hpp>
    const bool msvc_arf = compiler::msvc::Assertion_reporting_fix::global_instantiation();
#endif
