CREATE TABLE contactcategories (recid BLOB NOT NULL, categoryid TEXT NOT NULL COLLATE NOCASE, 
	UNIQUE(recid, categoryid), 
	FOREIGN KEY(recid) REFERENCES contacts(recid), 
	FOREIGN KEY (categoryid) REFERENCES categories(categoryid));

-- create index on joined column.
CREATE INDEX contactcategoriesindex ON contactcategories (categoryid);
