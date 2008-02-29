
CREATE TABLE appointmentexceptions (recid BLOB NOT NULL, edate DATE NOT NULL, alternateid INT, 
	UNIQUE(recid, edate), 
	FOREIGN KEY(recid) REFERENCES appointments(recid));
