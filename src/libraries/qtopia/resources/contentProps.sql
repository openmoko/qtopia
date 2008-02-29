-- Document Model sub-system SQL Schema

-- This file is intended to be used by "all other" RDBMS back-ends
-- including MySQL.  
-- See contentlnk.sql for portability notes

-- User/Programmer defined freeform properties information
CREATE TABLE contentProps
(
    cid INTEGER NOT NULL
        FOREIGN KEY REFERENCES content( cid ) 
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    grp VARCHAR(255),
    name VARCHAR(255) NOT NULL,
    value BLOB,
    PRIMARY KEY (cid, grp, name)
);
