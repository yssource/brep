// file      : libbrep/package.cxx -*- C++ -*-
// copyright : Copyright (c) 2014-2019 Code Synthesis Ltd
// license   : MIT; see accompanying LICENSE file

#include <libbrep/package.hxx>

#include <odb/database.hxx>

#include <libbrep/package-odb.hxx>

using namespace std;
using namespace odb::core;

namespace brep
{
  // dependency
  //
  ostream&
  operator<< (ostream& o, const dependency& d)
  {
    o << d.name;

    if (d.constraint)
      o << ' ' << *d.constraint;

    return o;
  }

  bool
  operator== (const dependency& x, const dependency& y)
  {
    return x.name == y.name && x.constraint == y.constraint;
  }

  bool
  operator!= (const dependency& x, const dependency& y)
  {
    return !(x == y);
  }

  // tenant
  //
  tenant::
  tenant (string i)
      : id (move (i)),
        creation_timestamp (timestamp::clock::now ())
  {
  }

  // package
  //
  package::
  package (package_name nm,
           version_type vr,
           package_name pn,
           priority_type pr,
           string sm,
           license_alternatives_type la,
           strings tg,
           optional<string> ds,
           string ch,
           optional<url_type> ur,
           optional<url_type> du,
           optional<url_type> su,
           optional<url_type> pu,
           optional<email_type> em,
           optional<email_type> pe,
           optional<email_type> be,
           optional<email_type> bwe,
           optional<email_type> bee,
           dependencies_type dp,
           requirements_type rq,
           build_class_exprs bs,
           build_constraints_type bc,
           optional<path> lc,
           optional<string> fr,
           optional<string> sh,
           shared_ptr<repository_type> rp)
      : id (rp->tenant, move (nm), vr),
        tenant (id.tenant),
        name (id.name),
        version (move (vr)),
        project (move (pn)),
        priority (move (pr)),
        summary (move (sm)),
        license_alternatives (move (la)),
        tags (move (tg)),
        description (move (ds)),
        changes (move (ch)),
        url (move (ur)),
        doc_url (move (du)),
        src_url (move (su)),
        package_url (move (pu)),
        email (move (em)),
        package_email (move (pe)),
        build_email (move (be)),
        build_warning_email (move (bwe)),
        build_error_email (move (bee)),
        dependencies (move (dp)),
        requirements (move (rq)),
        builds (move (bs)),
        build_constraints (version.compare (wildcard_version, true) != 0
                           ? move (bc)
                           : build_constraints_type ()),
        internal_repository (move (rp)),
        location (move (lc)),
        fragment (move (fr)),
        sha256sum (move (sh))
  {
    assert (internal_repository->internal);
  }

  package::
  package (package_name nm,
           version_type vr,
           shared_ptr<repository_type> rp)
      : id (rp->tenant, move (nm), vr),
        tenant (id.tenant),
        name (id.name),
        version (move (vr))
  {
    assert (!rp->internal);
    other_repositories.emplace_back (move (rp));
  }

  weighted_text package::
  search_text () const
  {
    if (!internal ())
      return weighted_text ();

    // Derive keywords from the basic package information: name,
    // version.
    //
    //@@ What about 'stable' from cppget.org/stable? Add path of
    //   the repository to keywords? Or is it too "polluting" and
    //   we will handle it in some other way (e.g., by allowing
    //   the user to specify repo location in the drop-down box)?
    //   Probably drop-box would be better as also tells what are
    //   the available internal repositories.
    //
    string k (project.string () + " " + name.string () + " " +
              version.string () + " " + version.string (true));

    // Add tags to keywords.
    //
    for (const auto& t: tags)
      k += " " + t;

    // Add licenses to keywords.
    //
    for (const auto& la: license_alternatives)
    {
      for (const auto& l: la)
      {
        k += " " + l;

        // If license is say LGPLv2 then LGPL is also a keyword.
        //
        size_t n (l.size ());
        if (n > 2 && l[n - 2] == 'v' && l[n - 1] >= '0' && l[n - 1] <= '9')
          k += " " + string (l, 0, n - 2);
      }
    }

    return {move (k), summary, description ? *description : "", changes};
  }

  // repository
  //
  repository::
  repository (string t,
              repository_location l,
              string d,
              repository_location h,
              optional<certificate_type> c,
              uint16_t r)
      : id (move (t), l.canonical_name ()),
        tenant (id.tenant),
        canonical_name (id.canonical_name),
        location (move (l)),
        display_name (move (d)),
        priority (r),
        cache_location (move (h)),
        certificate (move (c)),
        internal (true)
  {
  }

  repository::
  repository (string t, repository_location l)
      : id (move (t), l.canonical_name ()),
        tenant (id.tenant),
        canonical_name (id.canonical_name),
        location (move (l)),
        priority (0),
        internal (false)
  {
  }
}
