CREATE TABLE appointmentexceptions (
        recid BINARY(8) NOT NULL,
        edate DATE NOT NULL,
        alternateid INT, 
	UNIQUE(recid, edate), 
	FOREIGN KEY(recid) REFERENCES appointments(recid));
