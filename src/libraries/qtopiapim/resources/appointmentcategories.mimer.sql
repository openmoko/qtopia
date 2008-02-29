CREATE TABLE appointmentcategories (
        recid INTEGER NOT NULL,
        categoryid VARCHAR(255) NOT NULL,
	UNIQUE(recid, categoryid), 
	FOREIGN KEY(recid) REFERENCES appointments(recid), 
	FOREIGN KEY (categoryid) REFERENCES categories(categoryid));
