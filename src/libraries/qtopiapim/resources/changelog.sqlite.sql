-- will have better storage in mimer than sqlite (data type support)
CREATE TABLE changelog (recid INTEGER, context INTEGER NOT NULL, created DATETIME, modified DATETIME, removed DATETIME,
        PRIMARY KEY(recid));
