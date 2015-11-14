// file      : brep/package-details.cxx -*- C++ -*-
// copyright : Copyright (c) 2014-2015 Code Synthesis Ltd
// license   : MIT; see accompanying LICENSE file

#include <brep/package-details>

#include <string>
#include <memory>  // make_shared(), shared_ptr
#include <cstddef> // size_t
#include <cassert>

#include <xml/serializer>

#include <odb/session.hxx>
#include <odb/database.hxx>
#include <odb/transaction.hxx>

#include <web/xhtml>
#include <web/module>
#include <web/mime-url-encoding>

#include <brep/page>
#include <brep/options>
#include <brep/package>
#include <brep/package-odb>
#include <brep/shared-database>

using namespace std;
using namespace odb::core;

namespace brep
{
  using namespace cli;

  void package_details::
  init (scanner& s)
  {
    MODULE_DIAG;

    options_ = make_shared<options::package_details> (
      s, unknown_mode::fail, unknown_mode::fail);

    db_ = shared_database (options_->db_host (), options_->db_port ());
  }

  template <typename T>
  static inline query<T>
  search_params (const string& n, const string& q)
  {
    using query = query<T>;

    return "(" +
      (q.empty ()
       ? query ("NULL")
       : "plainto_tsquery (" + query::_val (q) + ")") +
      "," +
      query::_val (n) +
      ")";
  }

  void package_details::
  handle (request& rq, response& rs)
  {
    using namespace xml;
    using namespace web;
    using namespace web::xhtml;

    MODULE_DIAG;

    // The module options object is not changed after being created once per
    // server process.
    //
    static const size_t rp (options_->results_on_page ());
    static const dir_path& rt (options_->root ());

    const string& name (*rq.path ().rbegin ());
    const string en (mime_url_encode (name));

    params::package_details pr;

    try
    {
      param_scanner s (rq.parameters ());
      pr = params::package_details (s, unknown_mode::fail, unknown_mode::fail);
    }
    catch (const unknown_argument& e)
    {
      throw invalid_request (400, e.what ());
    }

    const string& sq (pr.query ()); // Search query.
    size_t pg (pr.page ());
    bool f (pr.full ());

    auto url (
      [&en](bool f = false,
            const string& q = "",
            size_t p = 0,
            const string& a = "") -> string
      {
        string s ("?");
        string u (en);

        if (f)           { u += "?full"; s = "&"; }
        if (!q.empty ()) { u += s + "q=" +  mime_url_encode (q); s = "&"; }
        if (p > 0)       { u += s + "p=" + to_string (p); s = "&"; }
        if (!a.empty ()) { u += '#' + a; }
        return u;
      });

    serializer s (rs.content (), name);
    const string& title (sq.empty () ? name : name + " " + sq);
    static const path sp ("package-details.css");

    s << HTML
      <<   HEAD
      <<     TITLE << title << ~TITLE
      <<     CSS_LINKS (sp, rt)
      <<   ~HEAD
      <<   BODY
      <<     DIV_HEADER (rt)
      <<     DIV(ID="content");

    if (f)
      s << CLASS("full");

    s <<       DIV(ID="heading")
      <<         H1 << A(HREF=url ()) << name << ~A << ~H1
      <<         A(HREF=url (!f, sq, pg)) << (f ? "[brief]" : "[full]") << ~A
      <<       ~DIV;

    session sn;
    transaction t (db_->begin ());

    shared_ptr<package> p;
    {
      latest_package lp;
      if (!db_->query_one<latest_package> (
            query<latest_package>(
              "(" + query<latest_package>::_val (name) + ")"), lp))
      {
        throw invalid_request (404, "Package '" + name + "' not found");
      }

      p = db_->load<package> (lp.id);
    }

    const auto& ll (p->license_alternatives);

    if (pg == 0)
    {
      // Display package details on the first page only.
      //
      s << H2 << p->summary << ~H2;

      static const size_t dl (options_->description_length ());

      if (const auto& d = p->description)
        s << (f
              ? P_DESCRIPTION (*d)
              : P_DESCRIPTION (*d, dl, url (!f, sq, pg, "description")));

      s << TABLE(CLASS="proplist", ID="package")
        <<   TBODY
        <<     TR_LICENSE (ll)
        <<     TR_URL (p->url)
        <<     TR_EMAIL (p->email)
        <<     TR_TAGS (p->tags, rt)
        <<   ~TBODY
        << ~TABLE;
    }

    auto pc (
      db_->query_value<package_count> (
        search_params<package_count> (name, sq)));

    auto r (
      db_->query<package_search_rank> (
        search_params<package_search_rank> (name, sq) +
        "ORDER BY rank DESC, version_epoch DESC, "
        "version_canonical_upstream DESC, version_revision DESC" +
        "OFFSET" + to_string (pg * rp) +
        "LIMIT" + to_string (rp)));

    s << FORM_SEARCH (sq)
      << DIV_COUNTER (pc, "Version", "Versions");

    // Enclose the subsequent tables to be able to use nth-child CSS selector.
    //
    s << DIV;
    for (const auto& pr: r)
    {
      shared_ptr<package> p (db_->load<package> (pr.id));

      s << TABLE(CLASS="proplist version")
        <<   TBODY
        <<     TR_VERSION (name, p->version.string (), rt)

        // @@ Shouldn't we skip low priority row ? Don't think so, why?
        //
        <<     TR_PRIORITY (p->priority);

      // Comparing objects of the license_alternatives class as being of the
      // vector<vector<string>> class, so comments are not considered.
      //
      if (p->license_alternatives != ll)
        s << TR_LICENSE (p->license_alternatives);

      assert (p->internal ());

      // @@ Shouldn't we make package location to be a link to the proper
      //    place of the About page, describing corresponding repository?
      //    Yes, I think that's sounds reasonable, once we have about.
      //
      // @@ In most cases package location will be the same for all versions
      //    of the same package. Shouldn't we put package location to the
      //    package summary part and display it here only if it differes
      //    from the one in the summary ?
      //
      //    Hm, I am not so sure about this. Consider: stable/testing/unstable.
      //
      s <<     TR_LOCATION (p->internal_repository.object_id ())
        <<     TR_DEPENDS (p->dependencies, rt)
        <<     TR_REQUIRES (p->requirements)
        <<   ~TBODY
        << ~TABLE;
    }
    s << ~DIV;

    t.commit ();

    static const size_t pp (options_->pages_in_pager ());

    s <<       DIV_PAGER (pg, pc, rp, pp, url (f, sq))
      <<     ~DIV
      <<   ~BODY
      << ~HTML;
  }
}