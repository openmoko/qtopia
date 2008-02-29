
CREATE TABLE appointmentcustom (recid BLOB NOT NULL, fieldname TEXT NOT NULL COLLATE NOCASE, fieldvalue TEXT COLLATE NOCASE, 
	UNIQUE(recid, fieldname), 
	FOREIGN KEY(recid) REFERENCES appointments(recid));
