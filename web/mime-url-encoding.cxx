// file      : web/mime-url-encoding.cxx -*- C++ -*-
// copyright : Copyright (c) 2014-2019 Code Synthesis Ltd
// license   : MIT; see accompanying LICENSE file

#include <web/mime-url-encoding.hxx>

#include <string>
#include <iterator> // back_inserter

#include <libbutl/url.mxx>

using namespace std;
using namespace butl;

namespace web
{
  inline static bool
  encode_query (char& c)
  {
    if (c == ' ')
    {
      c = '+';
      return false;
    }

    return !url::unreserved (c);
  }

  string
  mime_url_encode (const char* v, bool query)
  {
    return query ? url::encode (v, encode_query) : url::encode (v);
  }

  string
  mime_url_encode (const string& v, bool query)
  {
    return query ? url::encode (v, encode_query) : url::encode (v);
  }

  string
  mime_url_decode (const char* b, const char* e, bool trim, bool query)
  {
    if (trim)
    {
      for (; b != e && *b == ' '; ++b) ;

      if (b == e)
        return string ();

      while (*--e == ' ');
      ++e;
    }

    string r;
    if (!query)
      url::decode (b, e, back_inserter (r));
    else
      url::decode (b, e, back_inserter (r),
                   [] (char& c)
                   {
                     if (c == '+')
                       c = ' ';
                   });
    return r;
  }
}
