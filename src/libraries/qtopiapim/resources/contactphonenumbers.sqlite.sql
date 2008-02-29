CREATE TABLE contactphonenumbers (
    phone_number TEXT NOT NULL,
    recid INTEGER,
    phone_type INT,
    FOREIGN KEY(recid) REFERENCES contacts(recid)
);

CREATE INDEX contactphonenumbersindex ON contactphonenumbers (recid);
