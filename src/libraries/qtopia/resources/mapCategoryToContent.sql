-- Document Model sub-system SQL Schema

-- This file is intended to be used by "all other" RDBMS back-ends
-- including MySQL.  
-- See contentlnk.sql for portability notes

-- Map Categories selected by user onto Content items
CREATE TABLE mapCategoryToContent
(
    mid INTEGER PRIMARY KEY AUTO_INCREMENT,
    cat VARCHAR(255) NOT NULL
        FOREIGN KEY REFERENCES category( categoryid ) 
        ON DELETE CASCADE
        ON UPDATE CASCADE,
    cid INTEGER NOT NULL
        FOREIGN KEY REFERENCES content( cid ) 
        ON DELETE CASCADE
        ON UPDATE CASCADE,
);
