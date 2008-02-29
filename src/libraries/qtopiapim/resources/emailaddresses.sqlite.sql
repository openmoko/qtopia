-- relationships, addr, phone could all be split out into other tables,
-- keeping like this as matches current UI.

CREATE TABLE emailaddresses (
    addr TEXT NOT NULL COLLATE NOCASE,
    label TEXT COLLATE NOCASE,
    recid BLOB,
    PRIMARY KEY(addr),
    FOREIGN KEY(recid) REFERENCES contacts(recid)
);
