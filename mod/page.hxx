// file      : mod/page.hxx -*- C++ -*-
// copyright : Copyright (c) 2014-2019 Code Synthesis Ltd
// license   : MIT; see accompanying LICENSE file

#ifndef MOD_PAGE_HXX
#define MOD_PAGE_HXX

#include <libstudxml/forward.hxx>

#include <libbbot/manifest.hxx>

#include <web/xhtml-fragment.hxx>

#include <libbrep/types.hxx>
#include <libbrep/utility.hxx>

#include <libbrep/build.hxx>
#include <libbrep/package.hxx>

#include <mod/options-types.hxx> // page_menu

namespace brep
{
  // Page common building blocks.
  //

  // Generates CSS link elements.
  //
  class CSS_LINKS
  {
  public:
    CSS_LINKS (const path& p, const dir_path& r): path_ (p), root_ (r) {}

    void
    operator() (xml::serializer&) const;

  private:
    const path& path_;
    const dir_path& root_;
  };

  // Generates page header element.
  //
  class DIV_HEADER
  {
  public:
    DIV_HEADER (const web::xhtml::fragment& logo,
                const vector<page_menu>& menu,
                const dir_path& root,
                const string& tenant):
        logo_ (logo), menu_ (menu), root_ (root), tenant_ (tenant) {}

    void
    operator() (xml::serializer&) const;

  private:
    const web::xhtml::fragment& logo_;
    const vector<page_menu>& menu_;
    const dir_path& root_;
    const string& tenant_;
  };

  // Generates package search form element with the specified query input
  // element name.
  //
  class FORM_SEARCH
  {
  public:
    FORM_SEARCH (const string& q, const string& n): query_ (q), name_ (n) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& query_;
    const string& name_;
  };

  // Generates counter element.
  //
  // It could be redunant to distinguish between singular and plural word forms
  // if it wouldn't be so cheap in English, and phrase '1 Packages' wouldn't
  // look that ugly.
  //
  class DIV_COUNTER
  {
  public:
    DIV_COUNTER (size_t c, const char* s, const char* p)
        : count_ (c), singular_ (s), plural_ (p) {}

    void
    operator() (xml::serializer&) const;

  private:
    size_t count_;
    const char* singular_;
    const char* plural_;
  };

  // Generates table row element, that has the 'label: value' layout.
  //
  class TR_VALUE
  {
  public:
    TR_VALUE (const string& l, const string& v)
        : label_ (l), value_ (v) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& label_;
    const string& value_;
  };

  // Generates table row element, that has the 'label: <input type="text"/>'
  // layout.
  //
  class TR_INPUT
  {
  public:
    TR_INPUT (const string& l,
              const string& n,
              const string& v,
              const string& p = string (),
              bool a = false)
        : label_ (l), name_ (n), value_ (v), placeholder_ (p), autofocus_ (a)
    {
    }

    void
    operator() (xml::serializer&) const;

  private:
    const string& label_;
    const string& name_;
    const string& value_;
    const string& placeholder_;
    bool autofocus_;
  };

  // Generates table row element, that has the 'label: <select></select>'
  // layout. Option elements are represented as a list of value/inner-text
  // pairs.
  //
  class TR_SELECT
  {
  public:
    TR_SELECT (const string& l,
               const string& n,
               const string& v,
               const vector<pair<string, string>>& o)
        : label_ (l), name_ (n), value_ (v), options_ (o) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& label_;
    const string& name_;
    const string& value_;
    const vector<pair<string, string>>& options_;
  };

  // Generates tenant id element.
  //
  // Displays a link to the service page for the specified tenant.
  //
  class TR_TENANT
  {
  public:
    TR_TENANT (const string& n,
               const string& s,
               const dir_path& r,
               const string& t)
        : name_ (n), service_ (s), root_ (r), tenant_ (t) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& name_;
    const string& service_;
    const dir_path& root_;
    const string& tenant_;
  };

  // Generates package name element with an optional search criteria. The
  // search string should be url-encoded, if specified.
  //
  class TR_NAME
  {
  public:
    TR_NAME (const package_name& n,
             const string& q,
             const dir_path& r,
             const string& t)
        : name_ (n), query_ (q), root_ (r), tenant_ (t) {}

    void
    operator() (xml::serializer&) const;

  private:
    const package_name& name_;
    const string& query_;
    const dir_path& root_;
    const string& tenant_;
  };

  // Generates package version element.
  //
  class TR_VERSION
  {
  public:
    // Display the version as a link to the package version details page.
    //
    TR_VERSION (const package_name& p,
                const version& v,
                const dir_path& r,
                const string& t)
        : package_ (&p),
          version_ (v.string ()),
          stub_ (v.compare (wildcard_version, true) == 0),
          root_ (&r),
          tenant_ (&t)
    {
    }

    // Display the version as a regular text.
    //
    TR_VERSION (const version& v)
        : package_ (nullptr),
          version_ (v.string ()),
          stub_ (v.compare (wildcard_version, true) == 0),
          root_ (nullptr),
          tenant_ (nullptr)
    {
    }

    void
    operator() (xml::serializer&) const;

  private:
    const package_name* package_;
    string version_;
    bool stub_;
    const dir_path* root_;
    const string* tenant_;
  };

  // Generates package project name element.
  //
  // Displays a link to the package search page with the project name
  // specified as a keyword.
  //
  class TR_PROJECT
  {
  public:
    TR_PROJECT (const package_name& p, const dir_path& r, const string& t)
        : project_ (p), root_ (r), tenant_ (t) {}

    void
    operator() (xml::serializer&) const;

  private:
    const package_name& project_;
    const dir_path& root_;
    const string& tenant_;
  };

  // Generates package summary element.
  //
  class TR_SUMMARY
  {
  public:
    TR_SUMMARY (const string& s): summary_ (s) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& summary_;
  };

  // Generates package license alternatives element.
  //
  class TR_LICENSE
  {
  public:
    TR_LICENSE (const license_alternatives& l): licenses_ (l) {}

    void
    operator() (xml::serializer&) const;

  private:
    const license_alternatives& licenses_;
  };

  // Generates package license alternatives elements. Differs from TR_LICENSE
  // by producing multiple rows instead of a single one.
  //
  class TR_LICENSES
  {
  public:
    TR_LICENSES (const license_alternatives& l): licenses_ (l) {}

    void
    operator() (xml::serializer&) const;

  private:
    const license_alternatives& licenses_;
  };

  // Generates package tags element.
  //
  class TR_TAGS
  {
  public:
    // Display the tag link list.
    //
    TR_TAGS (const strings& ts, const dir_path& r, const string& t)
        : project_ (nullptr), tags_ (ts), root_ (r), tenant_ (t) {}

    // As above but prepend the list with a tag link produced from the project
    // name, if it differs from other tags.
    //
    TR_TAGS (const package_name& pr,
             const strings& ts,
             const dir_path& r,
             const string& t)
        : project_ (&pr), tags_ (ts), root_ (r), tenant_ (t) {}

    void
    operator() (xml::serializer&) const;

  private:
    const package_name* project_;
    const strings& tags_;
    const dir_path& root_;
    const string& tenant_;
  };

  // Generates package dependencies element.
  //
  class TR_DEPENDS
  {
  public:
    TR_DEPENDS (const dependencies& d, const dir_path& r, const string& t)
        : dependencies_ (d), root_ (r), tenant_ (t) {}

    void
    operator() (xml::serializer&) const;

  private:
    const dependencies& dependencies_;
    const dir_path& root_;
    const string& tenant_;
  };

  // Generates package requirements element.
  //
  class TR_REQUIRES
  {
  public:
    TR_REQUIRES (const requirements& r): requirements_ (r) {}

    void
    operator() (xml::serializer&) const;

  private:
    const requirements& requirements_;
  };

  // Generates url element.
  //
  class TR_URL
  {
  public:
    TR_URL (const url& u, const char* l = "url"): url_ (u), label_ (l) {}

    void
    operator() (xml::serializer&) const;

  private:
    const url& url_;
    const char* label_;
  };

  // Generates email element.
  //
  class TR_EMAIL
  {
  public:
    TR_EMAIL (const email& e, const char* l = "email")
        : email_ (e), label_ (l) {}

    void
    operator() (xml::serializer&) const;

  private:
    const email& email_;
    const char* label_;
  };

  // Generates package version priority element.
  //
  class TR_PRIORITY
  {
  public:
    TR_PRIORITY (const priority& p): priority_ (p) {}

    void
    operator() (xml::serializer&) const;

  private:
    const priority& priority_;
  };

  // Generates repository name element.
  //
  class TR_REPOSITORY
  {
  public:
    TR_REPOSITORY (const string& n, const dir_path& r, const string& t)
        : name_ (n), root_ (r), tenant_ (t) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& name_;
    const dir_path& root_;
    const string& tenant_;
  };

  // Generates repository location element.
  //
  class TR_LOCATION
  {
  public:
    TR_LOCATION (const repository_location& l): location_ (l) {}

    void
    operator() (xml::serializer&) const;

  private:
    const repository_location& location_;
  };

  // Generates package download URL element.
  //
  class TR_DOWNLOAD
  {
  public:
    TR_DOWNLOAD (const string& u): url_ (u) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& url_;
  };

  // Generates sha256sum element.
  //
  class TR_SHA256SUM
  {
  public:
    TR_SHA256SUM (const string& s): sha256sum_ (s) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& sha256sum_;
  };

  // Generates build results element.
  //
  class TR_BUILD_RESULT
  {
  public:
    TR_BUILD_RESULT (const build& b, const string& h, const dir_path& r):
        build_ (b), host_ (h), root_ (r) {}

    void
    operator() (xml::serializer&) const;

  private:
    const build& build_;
    const string& host_;
    const dir_path& root_;
  };

  // Generates comment element.
  //
  class SPAN_COMMENT
  {
  public:
    SPAN_COMMENT (const string& c): comment_ (c) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& comment_;
  };

  // Generates package build result status element.
  //
  class SPAN_BUILD_RESULT_STATUS
  {
  public:
    SPAN_BUILD_RESULT_STATUS (const bbot::result_status& s): status_ (s) {}

    void
    operator() (xml::serializer&) const;

  private:
    const bbot::result_status& status_;
  };

  // Generates paragraph elements converting a plain text into XHTML5 applying
  // some heuristics (see implementation for details). Truncate the text if
  // requested.
  //
  // Note that there is no way to specify that some text fragment must stay
  // pre-formatted. Thus, don't use this type for text that can contain such
  // kind of fragments and consider using PRE_TEXT instead.
  //
  class P_TEXT
  {
  public:
    // Generate full text elements.
    //
    P_TEXT (const string& t, const string& id = "")
        : text_ (t), length_ (t.size ()), url_ (nullptr), id_ (id) {}

    // Generate brief text elements.
    //
    P_TEXT (const string& t, size_t l, const string& u, const string& id = "")
        : text_ (t), length_ (l), url_ (&u), id_ (id) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& text_;
    size_t length_;
    const string* url_; // Full page url.
    string id_;
  };

  // Generates pre-formatted text element. Truncate the text if requested.
  //
  class PRE_TEXT
  {
  public:
    // Generate a full text element.
    //
    PRE_TEXT (const string& t, const string& id = "")
        : text_ (t), length_ (t.size ()), url_ (nullptr), id_ (id) {}

    // Generate a brief text element.
    //
    PRE_TEXT (const string& t,
              size_t l,
              const string& u,
              const string& id = "")
        : text_ (t), length_ (l), url_ (&u), id_ (id) {}

    void
    operator() (xml::serializer&) const;

  private:
    const string& text_;
    size_t length_;
    const string* url_; // Full page url.
    string id_;
  };

  // Generates paging element.
  //
  class DIV_PAGER
  {
  public:
    DIV_PAGER (size_t current_page,
               size_t item_count,
               size_t item_per_page,
               size_t page_number_count,
               const string& url);

    void
    operator() (xml::serializer&) const;

  private:
    size_t current_page_;
    size_t item_count_;
    size_t item_per_page_;
    size_t page_number_count_;
    const string& url_;
  };

  // Convert the argument to a string representing the valid HTML 5 'id'
  // attribute value.
  //
  string
  html_id (const string&);
}

#endif // MOD_PAGE_HXX
