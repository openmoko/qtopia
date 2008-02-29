/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef _GENERICMEMORYMONITOR_H_
#define _GENERICMEMORYMONITOR_H_

#include "memorymonitor.h"
#include <QVector>
#include <QDateTime>
#include <QValueSpaceObject>
class QTimer;

class GenericMemoryMonitorTask : public MemoryMonitor
{
Q_OBJECT
public:
    GenericMemoryMonitorTask();

    virtual MemState memoryState() const;
    virtual unsigned int timeInState() const;

private slots:
    void memoryMonitor();
    void evalMemory();

private:
    void setMemState(MemState);
    void readMemInfo(int *, int *);
    enum { VMUnknown, VMLinux_2_4, VMLinux_2_6 } m_vmStatType;
    QTimer *m_vmMonitor;
    bool m_slowVMMonitor;
    short m_count;
    long m_prevFaults;
    QVector<int> m_pgFaults;
    MemState m_memstate;
    QDateTime m_lastChanged;
    QValueSpaceObject * m_vso;
};

#endif // _GENERICMEMORYMONITOR_H_
