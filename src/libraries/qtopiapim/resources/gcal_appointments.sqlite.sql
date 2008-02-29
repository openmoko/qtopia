
CREATE TABLE gcal_appointments (recid BLOB NOT NULL, description TEXT COLLATE NOCASE, location TEXT COLLATE NOCASE,
        start DATETIME, end DATETIME, allday BOOL, starttimezone TEXT COLLATE NOCASE, endtimezone TEXT COLLATE NOCASE,
        alarm INT, alarmdelay INT, repeatrule INT, repeatfrequency INT,
        repeatenddate DATE, repeatweekflags INT,
	notes TEXT COLLATE NOCASE, PRIMARY KEY(recid));

CREATE INDEX gcal_appointments_descrption ON gcal_appointments (description, start, recid);
