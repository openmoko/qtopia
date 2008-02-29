CREATE TABLE contactcustom (recid INTEGER NOT NULL, fieldname TEXT NOT NULL COLLATE NOCASE, fieldvalue TEXT COLLATE NOCASE, 
	UNIQUE(recid, fieldname), 
	FOREIGN KEY(recid) REFERENCES contacts(recid));
