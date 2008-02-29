
CREATE TABLE tasks (recid INTEGER NOT NULL, description TEXT COLLATE NOCASE, priority INT, 
	status INT, percentcompleted INT, due DATE, started DATE, completed DATE, 
	context INT NOT NULL,
        PRIMARY KEY(recid));

CREATE INDEX tasks_descrption ON tasks (description, priority, percentcompleted, recid);
CREATE INDEX tasks_completed ON tasks (completed, priority, description, recid);
