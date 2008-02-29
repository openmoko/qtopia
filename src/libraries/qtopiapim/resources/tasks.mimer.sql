CREATE TABLE tasks (
        recid INTEGER NOT NULL,
        description NVARCHAR(255),
        priority INT, 
	status INT,
        percentcompleted INT,
        due DATE,
        started DATE,
        completed DATE, 
	notes NVARCHAR(1024),
        PRIMARY KEY(recid));

CREATE INDEX tasks_descrption ON tasks (description, priority, percentcompleted, recid);
