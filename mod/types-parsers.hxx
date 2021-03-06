// file      : mod/types-parsers.hxx -*- C++ -*-
// copyright : Copyright (c) 2014-2019 Code Synthesis Ltd
// license   : MIT; see accompanying LICENSE file

// CLI parsers, included into the generated source files.
//

#ifndef MOD_TYPES_PARSERS_HXX
#define MOD_TYPES_PARSERS_HXX

#include <libbpkg/manifest.hxx> // repository_location

#include <web/xhtml-fragment.hxx>

#include <libbrep/types.hxx>
#include <libbrep/utility.hxx>

#include <mod/options-types.hxx>

namespace brep
{
  namespace cli
  {
    class scanner;

    template <typename T>
    struct parser;

    template <>
    struct parser<path>
    {
      static void
      parse (path&, bool&, scanner&);
    };

    template <>
    struct parser<dir_path>
    {
      static void
      parse (dir_path&, bool&, scanner&);
    };

    template <>
    struct parser<bpkg::repository_location>
    {
      static void
      parse (bpkg::repository_location&, bool&, scanner&);
    };

    template <>
    struct parser<page_form>
    {
      static void
      parse (page_form&, bool&, scanner&);
    };

    template <>
    struct parser<page_menu>
    {
      static void
      parse (page_menu&, bool&, scanner&);
    };

    template <>
    struct parser<web::xhtml::fragment>
    {
      static void
      parse (web::xhtml::fragment&, bool&, scanner&);
    };
  }
}

#endif // MOD_TYPES_PARSERS_HXX
