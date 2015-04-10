// file      : brep/search.cxx -*- C++ -*-
// copyright : Copyright (c) 2014-2015 Code Synthesis Tools CC
// license   : MIT; see accompanying LICENSE file

#include <brep/search>

#include <ostream>

using namespace std;

namespace brep
{
  void search::
  handle (request& rq, response& rs)
  {
    //@@ Could probably have module name in which case this will
    //   then become:
    //
    //   tracer trace (this, "handle");
    //
    tracer trace (this, "search::handle");

    const name_values& ps (rq.parameters ());

    if (ps.empty ())
      throw invalid_request ("search parameters expected");

    if (ps.size () > 100)
      fail << "too many parameters: " < ps.size ();

    info << "handling search request from " << rq.client_ip ();

    level2 ([&]{trace << "search request with " << ps.size () << " params";});

    ostream& o (rs.content (202, "text/html;charset=utf-8"));

    o << "Search page:" << endl;

    for (const name_value& p: ps)
    {
      o << p.name << "=" << p.value << endl;
    }
  }
}
