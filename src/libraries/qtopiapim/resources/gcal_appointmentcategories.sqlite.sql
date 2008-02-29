CREATE TABLE gcal_appointmentcategories (recid INTEGER NOT NULL, categoryid TEXT NOT NULL COLLATE NOCASE, 
	UNIQUE(recid, categoryid), 
	FOREIGN KEY(recid) REFERENCES gcal_appointments(recid), 
	FOREIGN KEY (categoryid) REFERENCES categories(categoryid));
