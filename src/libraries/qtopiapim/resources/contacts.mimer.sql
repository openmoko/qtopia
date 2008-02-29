CREATE TABLE contacts (
        recid INTEGER NOT NULL PRIMARY KEY,
        title NVARCHAR(16),
        firstname NVARCHAR(32),
        middlename NVARCHAR(32),
        lastname NVARCHAR(32),
        suffix NVARCHAR(32),
        fileas NVARCHAR(32),

        default_email NVARCHAR(128),

        jobtitle NVARCHAR(64),
        department NVARCHAR(128),
        company NVARCHAR(128),

        b_phone VARCHAR(16),
        b_fax VARCHAR(16),
        b_mobile VARCHAR(16),

        b_street NVARCHAR(64),
        b_city NVARCHAR(64),
        b_state NVARCHAR(32),
        b_zip VARCHAR(32),
        b_country NVARCHAR(64),

        b_pager VARCHAR(16),

        b_webpage NVARCHAR(64),

        office NVARCHAR(64),
        profession NVARCHAR(64),
        assistant NVARCHAR(64),
        manager NVARCHAR(64),

        h_phone VARCHAR(16),
        h_fax VARCHAR(16),
        h_mobile VARCHAR(16),

        h_street NVARCHAR(64),
        h_city NVARCHAR(64),
        h_state NVARCHAR(32),
        h_zip NVARCHAR(16),
        h_country NVARCHAR(64),
        h_webpage NVARCHAR(64),

        spouse NVARCHAR(64),
        gender INT,
        birthday DATE,
        anniversary DATE,
        nickname NVARCHAR(32),
        children NVARCHAR(128),

        portrait VARCHAR(255),

        lastname_pronunciation NVARCHAR(32),
        firstname_pronunciation NVARCHAR(32),
        company_pronunciation NVARCHAR(32),

        notes NVARCHAR(128)
);

CREATE UNIQUE INDEX contactsflcindex ON contacts (firstname, lastname, company, recid);
CREATE UNIQUE INDEX contactslfcindex ON contacts (lastname, firstname, company, recid);

CREATE INDEX contactsfindex ON contacts (firstname);
CREATE INDEX contactslindex ON contacts (lastname);
CREATE INDEX contactscindex ON contacts (company);
