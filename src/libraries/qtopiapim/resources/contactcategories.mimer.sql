CREATE TABLE contactcategories (
        recid BINARY(8) NOT NULL,
        categoryid VARCHAR(255) NOT NULL, 
	UNIQUE(recid, categoryid), 
	FOREIGN KEY(recid) REFERENCES contacts(recid), 
	FOREIGN KEY (categoryid) REFERENCES categories(categoryid));
