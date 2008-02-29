
CREATE TABLE appointments (recid INTEGER NOT NULL, description TEXT COLLATE NOCASE, location TEXT COLLATE NOCASE,
        start DATETIME, end DATETIME, allday BOOL, starttimezone TEXT COLLATE NOCASE, endtimezone TEXT COLLATE NOCASE,
        alarm INT, alarmdelay INT, repeatrule INT, repeatfrequency INT,
        repeatenddate DATE, repeatweekflags INT,
        context INT NOT NULL,
        PRIMARY KEY(recid));

CREATE INDEX appointments_descrption ON appointments (description, start, recid);
