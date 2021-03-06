-- This file should be parsable by the brep-migrate utility. To decrease the
-- parser complexity, there is a number of restrictions, see package-extra.sql
-- file for details.
--
-- Increment the database 'build' schema version when update this file, see
-- package-extra.sql file for details.
--

DROP FOREIGN TABLE IF EXISTS build_package_constraints;

DROP FOREIGN TABLE IF EXISTS build_package_builds;

DROP FOREIGN TABLE IF EXISTS build_package;

DROP FOREIGN TABLE IF EXISTS build_repository;

DROP FOREIGN TABLE IF EXISTS build_tenant;

-- The foreign table for build_tenant object.
--
--
CREATE FOREIGN TABLE build_tenant (
  id TEXT NOT NULL,
  archived BOOLEAN NOT NULL)
SERVER package_server OPTIONS (table_name 'tenant');

-- The foreign table for build_repository object.
--
--
CREATE FOREIGN TABLE build_repository (
  tenant TEXT NOT NULL,
  canonical_name TEXT NOT NULL,
  location_url TEXT NOT NULL,
  location_type TEXT NOT NULL,
  certificate_fingerprint TEXT NULL)
SERVER package_server OPTIONS (table_name 'repository');

-- The foreign table for build_package object.
--
--
CREATE FOREIGN TABLE build_package (
  tenant TEXT NOT NULL,
  name CITEXT NOT NULL,
  version_epoch INTEGER NOT NULL,
  version_canonical_upstream TEXT NOT NULL,
  version_canonical_release TEXT NOT NULL COLLATE "C",
  version_revision INTEGER NOT NULL,
  version_upstream TEXT NOT NULL,
  version_release TEXT NULL,
  internal_repository_tenant TEXT NULL,
  internal_repository_canonical_name TEXT NULL)
SERVER package_server OPTIONS (table_name 'package');

-- The foreign table for the build_package object builds member (that is of a
-- container type).
--
--
CREATE FOREIGN TABLE build_package_builds (
  tenant TEXT NOT NULL,
  name CITEXT NOT NULL,
  version_epoch INTEGER NOT NULL,
  version_canonical_upstream TEXT NOT NULL,
  version_canonical_release TEXT NOT NULL COLLATE "C",
  version_revision INTEGER NOT NULL,
  index BIGINT NOT NULL,
  expression TEXT NOT NULL,
  comment TEXT NOT NULL)
SERVER package_server OPTIONS (table_name 'package_builds');

-- The foreign table for the build_package object constraints member (that is
-- of a container type).
--
--
CREATE FOREIGN TABLE build_package_constraints (
  tenant TEXT NOT NULL,
  name CITEXT NOT NULL,
  version_epoch INTEGER NOT NULL,
  version_canonical_upstream TEXT NOT NULL,
  version_canonical_release TEXT NOT NULL COLLATE "C",
  version_revision INTEGER NOT NULL,
  index BIGINT NOT NULL,
  exclusion BOOLEAN NOT NULL,
  config TEXT NOT NULL,
  target TEXT NULL,
  comment TEXT NOT NULL)
SERVER package_server OPTIONS (table_name 'package_build_constraints');
