/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info\@trolltech.com if any conditions of this licensing are
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

signals:
    void tasksUpdated();

private:
    AccessMode amode;
};

#endif
