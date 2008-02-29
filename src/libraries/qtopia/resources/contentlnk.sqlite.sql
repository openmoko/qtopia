-- Document Model sub-system SQL Schema

-- SQL file optimised for SQL Lite.  Please see the comments in the
-- file contentlnk.sql for SQL92/99 standard sql.

-- Note that SQLite will convert all column types with the substring "CHAR"
-- into the SQLite type "TEXT" which is a pointer offset string type

-- Main Content Class
CREATE TABLE content 
(
    cid INTEGER PRIMARY KEY,        -- SQLite chokes on "AUTO_INCREMENT", will make this auto inc anyway
    uiName VARCHAR(100) NOT NULL COLLATE NOCASE,   -- indexed
    mType INTEGER,          -- indexed
    drmFlags INTEGER,              -- enum "[p]lain", "[c]ontrolled", or "[u]nrenderable"
    docStatus CHAR(1),              -- enum "[d]oc", or "[b]in", indexed
    path VARCHAR(255) COLLATE NOCASE,              -- path + filename, unique
    location INTEGER,
    icon VARCHAR(255) COLLATE NOCASE,              -- path + filename
    linkFile VARCHAR(255) COLLATE NOCASE,
    linkLocation INTEGER,
    mimeTypes VARCHAR(255) COLLATE NOCASE,
    lastUpdated INTEGER
);

CREATE INDEX cNameIndex ON content ( uiName );

CREATE INDEX cMimeIndex ON content ( mType );

CREATE INDEX cDocStatusIndex ON content ( docStatus );

CREATE UNIQUE INDEX cLinkPathIndex ON content ( path, location, linkFile, linkLocation );

CREATE INDEX cLocationIndex ON content ( location );

CREATE INDEX cLinkLocationIndex ON content ( linkLocation );

CREATE INDEX cPath on content (path);

CREATE INDEX cLinkFile on content (linkFile);

PRAGMA user_version=109;
