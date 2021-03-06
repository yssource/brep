// file      : libbrep/build-package.hxx -*- C++ -*-
// copyright : Copyright (c) 2014-2019 Code Synthesis Ltd
// license   : MIT; see accompanying LICENSE file

#ifndef LIBBREP_BUILD_PACKAGE_HXX
#define LIBBREP_BUILD_PACKAGE_HXX

#include <odb/core.hxx>

#include <libbrep/types.hxx>
#include <libbrep/utility.hxx>

#include <libbrep/common.hxx> // Must be included last (see assert).

namespace brep
{
  // These are "foreign objects" that are mapped to subsets of the package
  // database objects using the PostgreSQL foreign table mechanism. Note that
  // since we maintain the pair in sync by hand, we should only have a minimal
  // subset of "core" members (ideally just the primary key) that are unlikly
  // to disappear or change.
  //
  // The mapping is established in build-extra.sql. We also explicitly mark
  // non-primary key foreign-mapped members in the source object.
  //
  // Foreign object that is mapped to a subset of the tenant object.
  //
  #pragma db object table("build_tenant") pointer(shared_ptr) readonly
  class build_tenant
  {
  public:
    string id;

    bool archived;

    // Database mapping.
    //
    #pragma db member(id) id

  private:
    friend class odb::access;
    build_tenant () = default;
  };

  // Foreign object that is mapped to a subset of the repository object.
  //
  #pragma db object table("build_repository") pointer(shared_ptr) readonly
  class build_repository
  {
  public:
    repository_id id;

    const string& canonical_name;             // Tracks id.canonical_name.
    repository_location location;
    optional<string> certificate_fingerprint;

    // Database mapping.
    //
    #pragma db member(id) id column("")

    #pragma db member(canonical_name) transient

    #pragma db member(location)                                            \
      set(this.location = std::move (?);                                   \
          assert (this.canonical_name == this.location.canonical_name ()))

  private:
    friend class odb::access;
    build_repository (): canonical_name (id.canonical_name) {}
  };

  // Foreign object that is mapped to a subset of the package object.
  //
  #pragma db object table("build_package") pointer(shared_ptr) readonly
  class build_package
  {
  public:
    package_id id;
    upstream_version version;
    lazy_shared_ptr<build_repository> internal_repository;

    // Mapped to the package object builds member using the PostgreSQL foreign
    // table mechanism.
    //
    build_class_exprs builds;

    // Mapped to the package object build_constraints member using the
    // PostgreSQL foreign table mechanism.
    //
    build_constraints constraints;

    // Database mapping.
    //
    #pragma db member(id) id column("")
    #pragma db member(version) set(this.version.init (this.id.version, (?)))
    #pragma db member(builds) id_column("") value_column("")
    #pragma db member(constraints) id_column("") value_column("")

  private:
    friend class odb::access;
    build_package () = default;
  };

  // Packages that can potentially be built (internal non-stub).
  //
  // Note that ADL can't find the equal operator, so we use the function call
  // notation.
  //
  #pragma db view                                                      \
    object(build_package)                                              \
    object(build_repository inner:                                     \
           brep::operator== (build_package::internal_repository,       \
                             build_repository::id) &&                  \
           brep::compare_version_ne (build_package::id.version,        \
                                     brep::wildcard_version,           \
                                     false))                           \
    object(build_tenant: build_package::id.tenant == build_tenant::id)
  struct buildable_package
  {
    package_id id;
    upstream_version version;

    // Database mapping.
    //
    #pragma db member(version) set(this.version.init (this.id.version, (?)))
  };

  #pragma db view                                                      \
    object(build_package)                                              \
    object(build_repository inner:                                     \
           brep::operator== (build_package::internal_repository,       \
                             build_repository::id) &&                  \
           brep::compare_version_ne (build_package::id.version,        \
                                     brep::wildcard_version,           \
                                     false))                           \
    object(build_tenant: build_package::id.tenant == build_tenant::id)
  struct buildable_package_count
  {
    size_t result;

    operator size_t () const {return result;}

    // Database mapping.
    //
    #pragma db member(result) column("count(" + build_package::id.name + ")")
  };
}

#endif // LIBBREP_BUILD_PACKAGE_HXX
