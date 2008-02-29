
CREATE TABLE gcal_appointmentexceptions (recid INTEGER NOT NULL, edate DATE NOT NULL, alternateid INT, 
	UNIQUE(recid, edate), 
	FOREIGN KEY(recid) REFERENCES gcal_appointments(recid));
