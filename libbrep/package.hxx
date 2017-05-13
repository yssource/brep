// file      : libbrep/package.hxx -*- C++ -*-
// copyright : Copyright (c) 2014-2017 Code Synthesis Ltd
// license   : MIT; see accompanying LICENSE file

#ifndef LIBBREP_PACKAGE_HXX
#define LIBBREP_PACKAGE_HXX

#include <map>
#include <chrono>

#include <odb/core.hxx>
#include <odb/nested-container.hxx>

#include <libbrep/types.hxx>
#include <libbrep/utility.hxx>

#include <libbrep/common.hxx> // Must be included last (see assert).

// Used by the data migration entries.
//
#define LIBBREP_PACKAGE_SCHEMA_VERSION_BASE 4

#pragma db model version(LIBBREP_PACKAGE_SCHEMA_VERSION_BASE, 4, closed)

namespace brep
{
  // @@ Might make sense to put some heavy members (e.g., description,
  //    containers) into a separate section.
  //
  // @@ Not sure there is a benefit in making tags a full-blown container
  //    (i.e., a separate table). Maybe provide a mapping of vector<string>
  //    to TEXT as a comma-separated list.
  //

  // Forward declarations.
  //
  class repository;
  class package;

  // priority
  //
  using bpkg::priority;

  #pragma db value(priority) definition
  #pragma db member(priority::value) column("")

  // url
  //
  using bpkg::url;

  #pragma db value(url) definition
  #pragma db member(url::value) virtual(string) before access(this) column("")

  // email
  //
  using bpkg::email;

  #pragma db value(email) definition
  #pragma db member(email::value) virtual(string) before access(this) column("")

  // licenses
  //
  using bpkg::licenses;
  using license_alternatives = vector<licenses>;

  #pragma db value(licenses) definition

  // dependencies
  //
  using bpkg::dependency_constraint;

  #pragma db value(dependency_constraint) definition

  // Notes:
  //
  // 1. Will the package be always resolvable? What if it is in
  //    another repository (i.e., a "chained" third-party repo).
  //    The question is then whether we will load such "third-
  //    party packages" (i.e., packages that are not in our
  //    repository). If the answer is yes, then we can have
  //    a pointer here. If the answer is no, then we can't.
  //    Also, if the answer is yes, we probably don't need to
  //    load as much information as for "our own" packages. We
  //    also shouldn't be showing them in search results, etc.
  //    I think all we need is to know which repository this
  //    package comes from so that we can tell the user. How are
  //    we going to capture this? Poly hierarchy of packages?
  //
  // 2. I believe we don't need to use a weak pointer here since
  //    there should be no package dependency cycles (and therefore
  //    ownership cycles).
  //
  // 3. Actually there can be dependency cycle as dependency referes not to
  //    just a package but a specific version, so for the same pair of
  //    packages dependency for different versions can have an opposite
  //    directions. The possible solution is instead of a package we point
  //    to the earliest version that satisfies the constraint. But this
  //    approach requires to ensure no cycles exist before instantiating
  //    package objects which in presense of "foreign" packages can be
  //    tricky. Can stick to just a package name until get some clarity on
  //    "foreign" package resolution.
  //
  // 4. As we left just the package class the dependency resolution come to
  //    finding the best version matching package object. The question is
  //    if to resolve dependencies on the loading phase or in the WEB interface
  //    when required. The arguments in favour of doing that during loading
  //    phase are:
  //
  //    - WEB interface get offloaded from a possibly expensive queries
  //      which otherwise have to be executed multiple times for the same
  //      dependency no matter the result would be the same.
  //
  //    - No need to complicate persisted object model with repository
  //      relations otherwise required just for dependency resolution.
  //

  #pragma db value
  struct dependency
  {
    using package_type = brep::package;

    lazy_shared_ptr<package_type> package;
    optional<dependency_constraint> constraint;

    // Prerequisite package name.
    //
    string
    name () const;

    // Database mapping.
    //
    #pragma db member(package) column("") not_null
    #pragma db member(constraint) column("")
  };

  ostream&
  operator<< (ostream&, const dependency&);

  bool
  operator== (const dependency&, const dependency&);

  bool
  operator!= (const dependency&, const dependency&);

  #pragma db value
  class dependency_alternatives: public vector<dependency>
  {
  public:
    bool conditional;
    bool buildtime;
    string comment;

    dependency_alternatives () = default;
    dependency_alternatives (bool d, bool b, string c)
        : conditional (d), buildtime (b), comment (move (c)) {}
  };

  using dependencies = vector<dependency_alternatives>;

  // requirements
  //
  using bpkg::requirement_alternatives;
  using requirements = vector<requirement_alternatives>;

  #pragma db value(requirement_alternatives) definition

  // repository_location
  //
  using bpkg::repository_location;

  #pragma db map type(repository_location) as(string)     \
    to((?).string ()) from(brep::repository_location (?))

  #pragma db value
  class certificate
  {
  public:
    string fingerprint;  // SHA256 fingerprint.
    string name;         // CN component of Subject.
    string organization; // O component of Subject.
    string email;        // email: in Subject Alternative Name.
    string pem;          // PEM representation.
  };

  #pragma db object pointer(shared_ptr) session
  class repository
  {
  public:
    using email_type = brep::email;
    using certificate_type = brep::certificate;

    // Create internal repository.
    //
    repository (repository_location,
                string display_name,
                repository_location cache_location,
                optional<certificate_type>,
                uint16_t priority);

    // Create external repository.
    //
    explicit
    repository (repository_location);

    string name; // Object id (canonical name).
    repository_location location;
    string display_name;

    // The order in the internal repositories configuration file, starting from
    // 1. 0 for external repositories.
    //
    uint16_t priority;

    optional<string> url;

    // Present only for internal repositories.
    //
    optional<email_type> email;
    optional<string> summary;
    optional<string> description;

    // Location of the repository local cache. Non empty for internal
    // repositories and external ones with a filesystem path location.
    //
    repository_location cache_location;

    // Present only for internal signed repositories.
    //
    optional<certificate_type> certificate;

    // Initialized with timestamp_nonexistent by default.
    //
    timestamp packages_timestamp;

    // Initialized with timestamp_nonexistent by default.
    //
    timestamp repositories_timestamp;

    bool internal;
    vector<lazy_weak_ptr<repository>> complements;
    vector<lazy_weak_ptr<repository>> prerequisites;

    // Database mapping.
    //
    #pragma db member(name) id

    #pragma db member(location)                                  \
      set(this.location = std::move (?);                         \
          assert (this.name == this.location.canonical_name ()))

    #pragma db member(complements) id_column("repository") \
      value_column("complement") value_not_null

    #pragma db member(prerequisites) id_column("repository") \
      value_column("prerequisite") value_not_null

  private:
    friend class odb::access;
    repository () = default;
  };

  // The 'to' expression calls the PostgreSQL to_tsvector(weighted_text)
  // function overload (package-extra.sql). Since we are only interested
  // in "write-only" members of this type, make the 'from' expression
  // always return empty string (we still have to work the placeholder
  // in to keep overprotective ODB happy).
  //
  #pragma db map type("tsvector") as("TEXT")                       \
    to("to_tsvector((?)::weighted_text)") from("COALESCE('',(?))")

  // C++ type for weighted PostgreSQL tsvector.
  //
  #pragma db value type("tsvector")
  struct weighted_text
  {
    string a;
    string b;
    string c;
    string d;
  };

  #pragma db object pointer(shared_ptr) session
  class package
  {
  public:
    using repository_type = brep::repository;
    using version_type = brep::version;
    using priority_type = brep::priority;
    using license_alternatives_type = brep::license_alternatives;
    using url_type = brep::url;
    using email_type = brep::email;
    using dependencies_type = brep::dependencies;
    using requirements_type = brep::requirements;

    // Create internal package object.
    //
    package (string name,
             version_type,
             priority_type,
             string summary,
             license_alternatives_type,
             strings tags,
             optional<string> description,
             string changes,
             url_type,
             optional<url_type> package_url,
             email_type,
             optional<email_type> package_email,
             optional<email_type> build_email,
             dependencies_type,
             requirements_type,
             optional<path> location,
             optional<string> sha256sum,
             shared_ptr<repository_type>);

    // Create external package object.
    //
    // External repository packages can appear on the WEB interface only in
    // dependency list in the form of a link to the corresponding WEB page.
    // The only package information required to compose such a link is the
    // package name, version, and repository location.
    //
    package (string name, version_type, shared_ptr<repository_type>);

    bool
    internal () const noexcept {return internal_repository != nullptr;}

    // Manifest data.
    //
    package_id id;
    upstream_version version;
    priority_type priority;
    string summary;
    license_alternatives_type license_alternatives;
    strings tags;
    optional<string> description;
    string changes;
    url_type url;
    optional<url_type> package_url;
    email_type email;
    optional<email_type> package_email;
    optional<email_type> build_email;
    dependencies_type dependencies;
    requirements_type requirements;
    lazy_shared_ptr<repository_type> internal_repository;

    // Path to the package file. Present only for internal packages.
    //
    optional<path> location;

    // Present only for internal packages.
    //
    optional<string> sha256sum;

    vector<lazy_shared_ptr<repository_type>> other_repositories;

    // Database mapping.
    //
    #pragma db member(id) id column("")
    #pragma db member(version) set(this.version.init (this.id.version, (?)))

    // license
    //
    using _license_key = odb::nested_key<licenses>;
    using _licenses_type = std::map<_license_key, string>;

    #pragma db value(_license_key)
    #pragma db member(_license_key::outer) column("alternative_index")
    #pragma db member(_license_key::inner) column("index")

    #pragma db member(license_alternatives) id_column("") value_column("")
    #pragma db member(licenses)                                       \
      virtual(_licenses_type)                                         \
      after(license_alternatives)                                     \
      get(odb::nested_get (this.license_alternatives))                \
      set(odb::nested_set (this.license_alternatives, std::move (?))) \
      id_column("") key_column("") value_column("license")

    // tags
    //
    #pragma db member(tags) id_column("") value_column("tag")

    // dependencies
    //
    using _dependency_key = odb::nested_key<dependency_alternatives>;
    using _dependency_alternatives_type =
               std::map<_dependency_key, dependency>;

    #pragma db value(_dependency_key)
    #pragma db member(_dependency_key::outer) column("dependency_index")
    #pragma db member(_dependency_key::inner) column("index")

    #pragma db member(dependencies) id_column("") value_column("")
    #pragma db member(dependency_alternatives)                \
      virtual(_dependency_alternatives_type)                  \
      after(dependencies)                                     \
      get(odb::nested_get (this.dependencies))                \
      set(odb::nested_set (this.dependencies, std::move (?))) \
      id_column("") key_column("") value_column("dep_")

    // requirements
    //
    using _requirement_key = odb::nested_key<requirement_alternatives>;
    using _requirement_alternatives_type =
               std::map<_requirement_key, string>;

    #pragma db value(_requirement_key)
    #pragma db member(_requirement_key::outer) column("requirement_index")
    #pragma db member(_requirement_key::inner) column("index")

    #pragma db member(requirements) id_column("") value_column("")
    #pragma db member(requirement_alternatives)               \
      virtual(_requirement_alternatives_type)                 \
      after(requirements)                                     \
      get(odb::nested_get (this.requirements))                \
      set(odb::nested_set (this.requirements, std::move (?))) \
      id_column("") key_column("") value_column("id")

    // other_repositories
    //
    #pragma db member(other_repositories)                  \
      id_column("") value_column("repository") value_not_null

    // search_index
    //
    #pragma db member(search_index) virtual(weighted_text) null \
      access(search_text)

    #pragma db index method("GIN") member(search_index)

  private:
    friend class odb::access;
    package () = default;

    // Save keywords, summary, description, and changes to weighted_text
    // a, b, c, d members, respectively. So a word found in keywords will
    // have a higher weight than if it's found in the summary.
    //
    weighted_text
    search_text () const;

    // Noop as search_index is a write-only member.
    //
    void
    search_text (const weighted_text&) {}
  };

  // Package search query matching rank.
  //
  #pragma db view query("/*CALL*/ SELECT * FROM search_latest_packages(?)")
  struct latest_package_search_rank
  {
    package_id id;
    double rank;
  };

  #pragma db view \
    query("/*CALL*/ SELECT count(*) FROM search_latest_packages(?)")
  struct latest_package_count
  {
    size_t result;

    operator size_t () const {return result;}
  };

  #pragma db view query("/*CALL*/ SELECT * FROM search_packages(?)")
  struct package_search_rank
  {
    package_id id;
    double rank;
  };

  #pragma db view  query("/*CALL*/ SELECT count(*) FROM search_packages(?)")
  struct package_count
  {
    size_t result;

    operator size_t () const {return result;}
  };

  #pragma db view query("/*CALL*/ SELECT * FROM latest_package(?)")
  struct latest_package
  {
    package_id id;
  };

  #pragma db view object(package) \
    object(repository: package::internal_repository)
  struct package_version
  {
    package_id id;
    upstream_version version;

    // Database mapping.
    //
    #pragma db member(version) set(this.version.init (this.id.version, (?)))
  };
}

#endif // LIBBREP_PACKAGE_HXX