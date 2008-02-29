CREATE TABLE contactcustom (
    recid INTEGER NOT NULL,
    fieldname VARCHAR(255) COLLATE NOCASE NOT NULL,
    fieldvalue VARCHAR(255) COLLATE NOCASE, 
    UNIQUE(recid, fieldname), 
    FOREIGN KEY(recid) REFERENCES contacts(recid));
