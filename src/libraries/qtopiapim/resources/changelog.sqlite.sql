-- will have better storage in mimer than sqlite (data type support)
CREATE TABLE changelog (recid BLOB, created DATETIME, modified DATETIME, removed DATETIME,
        PRIMARY KEY(recid));
