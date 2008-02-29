-- Document Model sub-system SQL Schema

-- This file is intended to be used by "all other" RDBMS back-ends
-- including MySQL.  

-- Portability notes:  SQLite always stores a ROWID, and if a field
-- is declared "INTEGER PRIMARY KEY" it becomes an alias for the 
-- ROWID, which is an auto-incrementing, non-null column.  MySQL does
-- require the AUTO_INCREMENT keyword, but will automatically make
-- the column non-null as its a primary key.
--
-- SQLite ignores most column definitions, instead coercing everything
-- into one of TEXT, INTEGER, REAL, BLOB or NULL on an *item by item*
-- basis (not column by column).  MySQL VARCHAR(50) would actually be
-- simply TEXT in SQLite, but for better portability the full definitions
-- are used here.  In standard SQL TEXT is a large dynamic storage type
-- with special indexing considerations, and storage overheads.  Note that
-- in MySQL VARCHAR(n) where n <= 255 is stored in L+1 bytes, where L is
-- the *actual* length of the data.
--
-- SQLite does not have an enum class.  Enum effectively gives insert
-- verification and small storage.  Instead we have to code as a single
-- char, and do the conversion ourselves in code for SQLite.  (enum is
-- a MySQL extension to SQL99).
--
-- SQLite does not have FOREIGN KEY constraint enforcement, although an
-- approximation of it can be achieved with triggers.
--
-- For portability and transparency, foreign keys can be maintained in
-- the client code (see the contentsql.* source).  This would also save
-- the performance cost of the triggers/foreign key enforcement:
-- http://dev.mysql.com/doc/mysql/en/ansi-diff-foreign-keys.html
--
-- Since the decision has been made to put foreign key constraint enforcement
-- into the database, rather than the code, this approach will be followed here.
-- Performance problems detected later might be addressed by changing this.
--
-- Note that MySQL requires TYPE=InnoDB in order to have transaction support
-- and foreign keys so the table type must be altered after creation with a
-- "ALTER TABLE content TYPE=INNODB", although this may be automatic with
-- the embedded version which AIUI does not ship with MyISAM only InnoDB

-- Main Content Class
CREATE TABLE content 
(
    cid          INTEGER PRIMARY KEY AUTO_INCREMENT, 
    uiName       VARCHAR(255) NOT NULL,
                     INDEX( uiName ),
    mType     INTEGER,
                     INDEX( mType ),
    drmFlags     INTEGER,
    docStatus    CHAR(1),                   -- ENUM( 'doc', 'bin' ),
                     INDEX( docStatus ),
    path         VARCHAR(255),
                     INDEX( path ),
    location     INTEGER,
                     INDEX( location ),
    icon         VARCHAR(255),              -- path + filename
    linkFile     VARCHAR(255),              -- location of original .desktop file
                     INDEX( linkFile ),
    linkLocation INTEGER,
                     INDEX( linkLocation ),
    mimeTypes    VARCHAR(255),
    lastUpdated INTEGER
    UNIQUE INDEX (path, location, linkFile, linkLocation)
);
