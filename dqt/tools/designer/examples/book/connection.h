/****************************************************************************
** $Id: qt/connection.h   3.3.5   edited Aug 31 12:17 $
**
** Copyright (C) 1992-2005 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

// Add your own connection parameters here
#define DB_BOOKS_DRIVER "QSQLITE"
#define DB_BOOKS 	":memory:"
#define DB_BOOKS_USER 	""
#define DB_BOOKS_PASSWD ""
#define DB_BOOKS_HOST 	""

bool createConnections();

