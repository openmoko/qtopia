/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef TASKIO_PRIVATE_H
#define TASKIO_PRIVATE_H

#include <qvector.h>
#include <qvaluelist.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qobject.h>
#include <qtopia/pim/qtopiapim.h>
#include <qtopia/pim/task.h>

class TaskIO;
class QTOPIAPIM_EXPORT PrTask : public PimTask {
public:
    PrTask() : PimTask() {}
    PrTask(const PrTask &t) : PimTask(t) {}

    void setUid(QUuid u) { p_setUid(u); }
};


class QTOPIAPIM_EXPORT TaskIteratorMachine : public QShared
{
public:
    virtual ~TaskIteratorMachine() {}

    virtual bool atFirst() const = 0;
    virtual bool atLast() const = 0;
    virtual const PrTask *toFirst() = 0;
    virtual const PrTask *toLast() = 0;

   virtual const PrTask *next() = 0;
    virtual const PrTask *prev() = 0;

    virtual const PrTask *current() const = 0;
};

class QTOPIAPIM_EXPORT TaskIO : public QObject {

Q_OBJECT

public:
    enum AccessMode {
	ReadOnly,
	ReadWrite,
	WriteOnly
    };

    TaskIO(AccessMode m) : amode(m) { }
    virtual ~TaskIO() {}

    AccessMode accessMode() const { return amode; }

    virtual TaskIteratorMachine *begin() const = 0;

    virtual PrTask taskForId( const QUuid &, bool *ok ) const = 0;

signals:
    void tasksUpdated();

private:
    AccessMode amode;
};

#endif
