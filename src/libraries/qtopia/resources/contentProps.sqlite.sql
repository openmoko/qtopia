-- Document Model sub-system SQL Schema

-- SQL file optimised for SQL Lite.  Please see the comments in the
-- file contentProps.sql for SQL92/99 standard sql.

-- Note that SQLite will convert all column types with the substring "CHAR"
-- into the SQLite type "TEXT" which is a pointer offset string type

-- User/Programmer defined freeform properties information
CREATE TABLE contentProps
(
    cid INTEGER NOT NULL,
    grp VARCHAR(255) COLLATE NOCASE,
    name VARCHAR(255) NOT NULL COLLATE NOCASE,
    value BLOB,
    PRIMARY KEY (cid, grp, name)
);