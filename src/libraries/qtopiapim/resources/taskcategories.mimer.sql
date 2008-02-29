CREATE TABLE taskcategories (
        recid INTEGER NOT NULL,
        categoryid VARCHAR NOT NULL,
	UNIQUE(recid, categoryid), 
	FOREIGN KEY(recid) REFERENCES tasks(recid), 
	FOREIGN KEY (categoryid) REFERENCES categories(categoryid));
