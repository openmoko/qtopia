-- Document Model sub-system SQL Schema

-- SQL file optimised for SQL Lite.  Please see the comments in the
-- file mapCategoryToContent.sql for SQL92/99 standard sql.

-- Note that SQLite will convert all column types with the substring "CHAR"
-- into the SQLite type "TEXT" which is a pointer offset string type

-- Map Categories selected by user onto Content items
CREATE TABLE mapCategoryToContent
(
    mid        INTEGER PRIMARY KEY,
    categoryid VARCHAR(100) NOT NULL COLLATE NOCASE,      -- category index, foreign key
    cid        INTEGER NOT NULL            -- contentId index, foreign key
);

CREATE INDEX mCatIndex ON mapCategoryToContent ( categoryid );

CREATE INDEX mCidIndex ON mapCategoryToContent ( cid );

CREATE UNIQUE INDEX mCidCatIndex ON mapCategoryToContent ( cid, categoryid );