// file      : brep/diagnostics.cxx -*- C++ -*-
// copyright : Copyright (c) 2014-2015 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <brep/diagnostics>

#include <cassert>
#include <exception>

using namespace std;

namespace build
{
  diag_record::
  ~diag_record () noexcept(false)
  {
    // Don't flush the record if this destructor was called as part of
    // the stack unwinding. Right now this means we cannot use this
    // mechanism in destructors, which is not a big deal, except for
    // one place: exception_guard. So for now we are going to have
    // this ugly special check which we will be able to get rid of
    // once C++17 uncaught_exceptions() becomes available.
    //
    if (!data_.empty () &&
        (!std::uncaught_exception () /*|| exception_unwinding_dtor*/))
    {
      data_.back ().msg = os_.str (); // Save last message.

      assert (epilogue_ != nullptr);
      (*epilogue_) (move (data_)); // Can throw.
    }
  }
}