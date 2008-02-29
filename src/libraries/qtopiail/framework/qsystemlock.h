/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QSYSTEMLOCK_H_
#define _QSYSTEMLOCK_H_

#include "qtopiailglobal.h"

class QSystemReadWriteLockPrivate;
class QTOPIAIL_EXPORT QSystemReadWriteLock
{
public:
    QSystemReadWriteLock(unsigned int id, bool own);
    ~QSystemReadWriteLock();

    bool isNull() const;
    unsigned int id() const;

    bool lockForRead(int milliSec);
    bool lockForWrite(int milliSec);
    void unlock();

private:
    QSystemReadWriteLockPrivate * d;
};

class QSystemMutex_Private;
class QTOPIAIL_EXPORT QSystemMutex
{
public:
    QSystemMutex(unsigned int id, bool own);
    ~QSystemMutex();

    bool isNull() const;
    unsigned int id() const;

    bool lock(int milliSec);
    void unlock();

private:
    QSystemMutex_Private *m_data;
};

#endif // _QSYSTEMLOCK_H_

