-- Document Model sub-system SQL Schema

-- SQL file optimised for SQL Lite.  Please see the comments in the
-- file location.sql for SQL92/99 standard sql.

-- Note that SQLite will convert all column types with the substring "CHAR"
-- into the SQLite type "TEXT" which is a pointer offset string type

CREATE TABLE locationLookup
(
    pKey INTEGER PRIMARY KEY,
    location varchar(255) NOT NULL
);

CREATE UNIQUE INDEX cLocationLookupLocation ON locationLookup ( location );

CREATE TABLE mimeTypeLookup
(
    pKey INTEGER PRIMARY KEY,
    mimeType varchar(100) NOT NULL
);

CREATE UNIQUE INDEX cMimeTypeLookupMimeType ON mimeTypeLookup ( mimeType );