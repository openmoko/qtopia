CREATE TABLE simcardidmap (sqlid BLOB, cardid TEXT, cardindex INT, PRIMARY KEY(sqlid), UNIQUE(cardid, cardindex));
