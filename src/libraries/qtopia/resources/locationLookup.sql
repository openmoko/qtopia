-- Document Model sub-system SQL Schema

-- This file is intended to be used by "all other" RDBMS back-ends
-- including MySQL.  
-- See contentlnk.sql for portability notes

CREATE TABLE locationLookup
(
    pKey INTEGER NOT NULL,
    location varchar(255) NOT NULL,
        UNIQUE INDEX (location),
    PRIMARY KEY (pkey)
);

CREATE TABLE mimeTypeLookup
(
    pKey INTEGER NOT NULL,
    mimeType varchar(100) NOT NULL,
        UNIQUE INDEX (mimeType),
    PRIMARY KEY (pkey)
);
