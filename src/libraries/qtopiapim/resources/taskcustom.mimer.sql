CREATE TABLE taskcustom (
        recid BINARY(8) NOT NULL,
        fieldname VARCHAR(255) NOT NULL,
        fieldvalue NVARCHAR(255),
	UNIQUE(recid, fieldname), 
	FOREIGN KEY(recid) REFERENCES tasks(recid));
