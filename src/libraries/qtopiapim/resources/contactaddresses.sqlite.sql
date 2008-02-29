CREATE TABLE contactaddresses (
    recid BLOB NOT NULL,
    addresstype INT NOT NULL,
    street TEXT COLLATE NOCASE,
    city TEXT COLLATE NOCASE,
    state TEXT COLLATE NOCASE,
    zip TEXT COLLATE NOCASE,
    country TEXT COLLATE NOCASE,
    FOREIGN KEY(recid) REFERENCES contacts(recid)
);
