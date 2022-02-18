#pragma once    // Source encoding: utf-8  --  π is (or should be) a lowercase greek pi.
#include <cpp/util.hpp> // cpp::util::(No_copying, Const_)

#include <utility>      // std::exchange

namespace winapi::com {
    namespace cu = cpp::util;
    using   cu::No_copying, cu::Const_;
    using   std::exchange;

    template< class Interface >
    class Ptr_: No_copying
    {
        Interface*  m_ptr;

    public:
        ~Ptr_() { if( m_ptr ) { m_ptr->Release(); } }       // .Release() from IUnknown.
        Ptr_( Const_<Interface*> ptr ): m_ptr( ptr ) {}

        auto raw_ptr() const -> Interface*      { return m_ptr; }
        auto operator->() const -> Interface*   { return raw_ptr(); }
    };
}  // namespace winapi::com
