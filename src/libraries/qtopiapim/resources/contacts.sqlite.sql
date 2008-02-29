CREATE TABLE contacts (
	recid BLOB NOT NULL,
	title TEXT COLLATE NOCASE,
	firstname TEXT COLLATE NOCASE,
	middlename TEXT COLLATE NOCASE,
	lastname TEXT COLLATE NOCASE,
	suffix TEXT COLLATE NOCASE,
	
	default_email TEXT COLLATE NOCASE,
	default_phone TEXT COLLATE NOCASE,

        b_webpage TEXT COLLATE NOCASE,

	jobtitle TEXT COLLATE NOCASE,
	department TEXT COLLATE NOCASE,
	company TEXT COLLATE NOCASE,
	office TEXT COLLATE NOCASE,
	profession TEXT COLLATE NOCASE,
	assistant TEXT COLLATE NOCASE,
	manager TEXT COLLATE NOCASE,


        h_webpage TEXT COLLATE NOCASE,

	spouse TEXT COLLATE NOCASE,
	gender TEXT COLLATE NOCASE,
	birthday DATE,
	anniversary DATE,
	nickname TEXT COLLATE NOCASE,
	children TEXT COLLATE NOCASE, -- should be table of relationships?
	
	portrait TEXT COLLATE NOCASE,
	
	lastname_pronunciation TEXT COLLATE NOCASE,
	firstname_pronunciation TEXT COLLATE NOCASE,
	company_pronunciation TEXT COLLATE NOCASE,

        context INT NOT NULL,

	PRIMARY KEY(recid)
);

-- two most common sort orderings.  If likely to have other common sort orderings
-- highly recommended to create and maintain indexes.
CREATE INDEX contactsflcindex ON contacts (firstname, lastname, company, recid);
CREATE INDEX contactslfcindex ON contacts (lastname, firstname, company, recid);

CREATE INDEX contactsfindex ON contacts (firstname);
CREATE INDEX contactslindex ON contacts (lastname);
CREATE INDEX contactscindex ON contacts (company);
