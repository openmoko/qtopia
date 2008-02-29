CREATE TABLE contactcustom (
        recid INTEGER NOT NULL,
        fieldname VARCHAR(255) NOT NULL,
        fieldvalue VARCHAR(255), 
	UNIQUE(recid, fieldname), 
	FOREIGN KEY(recid) REFERENCES contacts(recid));

-- contactcustom constraints
