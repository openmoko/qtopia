-- relationships, addr, phone could all be split out into other tables,
-- keeping like this as matches current UI.

CREATE TABLE emailaddresses (
    addr NVARCHAR(255) NOT NULL,
    label NVARCHAR(255),
    recid BINARY(8),
    PRIMARY KEY(addr),
    FOREIGN KEY(recid) REFERENCES contacts(recid)
);
