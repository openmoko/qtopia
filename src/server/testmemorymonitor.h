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

#ifndef _TESTMEMORYMONITOR_H_
#define _TESTMEMORYMONITOR_H_

#include "memorymonitor.h"
#include <QDateTime>
class QValueSpaceObject;

class TestMemoryMonitor : public MemoryMonitor
{
Q_OBJECT
public:
    TestMemoryMonitor();

    virtual MemState memoryState() const;
    virtual unsigned int timeInState() const;

private slots:
    void setValue(const QByteArray &, const QVariant &);

private:
    void refresh();

    QValueSpaceObject *m_vso;
    MemState m_memstate;
    QDateTime m_lastDateTime;
};

#endif // _TESTMEMORYMONITOR_H_
