CREATE TABLE appointmentcustom (
        recid BINARY(8) NOT NULL,
        fieldname VARCHAR(255) NOT NULL,
        fieldvalue NVARCHAR(255), -- translatable
	UNIQUE(recid, fieldname), 
	FOREIGN KEY(recid) REFERENCES appointments(recid));
