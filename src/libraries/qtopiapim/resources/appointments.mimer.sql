
CREATE TABLE appointments (
        recid BINARY(8) NOT NULL PRIMARY KEY,
        description NVARCHAR(32),
        location NVARCHAR(32),
        start DATETIME,
        end DATETIME,
        allday BOOL,
        starttimezone VARCHAR(32),
        endtimezone VARCHAR(32),
        alarm INT,
        alarmdelay INT,
        repeatrule INT,
        repeatfrequency INT,
        repeatenddate DATE,
        repeatweekflags INT,
	notes NVARCHAR(255));

CREATE INDEX appointments_descrption ON appointments (description, start, recid);
