CREATE TABLE appointmentcategories (
        recid BINARY(8) NOT NULL,
        categoryid VARCHAR(255) NOT NULL,
	UNIQUE(recid, categoryid), 
	FOREIGN KEY(recid) REFERENCES appointments(recid), 
	FOREIGN KEY (categoryid) REFERENCES categories(categoryid));
