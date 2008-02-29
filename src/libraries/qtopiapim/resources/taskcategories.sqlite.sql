CREATE TABLE taskcategories (recid BLOB NOT NULL, categoryid TEXT NOT NULL COLLATE NOCASE, 
	UNIQUE(recid, categoryid), 
	FOREIGN KEY(recid) REFERENCES tasks(recid), 
	FOREIGN KEY (categoryid) REFERENCES categories(categoryid));
