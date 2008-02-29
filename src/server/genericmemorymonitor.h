// -*-C++-*-
/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

  private:
    enum VM_Model {
	VMUnknown,
	VMLinux_2_4,
	VMLinux_2_6
    }; 

  private:
    void computeMemoryValues();
    void computePageFaultValues();
    void computeMemoryState();
    void restartTimer();

  private:
    int			m_low_threshhold;
    int			m_verylow_threshhold;
    int			m_critical_threshhold;
    int			m_percent_threshhold;
    int			m_memory_available;
    int			m_percent_available;
    int			m_samples;
    int			m_long_interval;
    int			m_short_interval;
    int			m_normal_count;
    int			m_critical_count;
    int			m_verylow_count;
    int			m_low_count;

    VM_Model		m_vmStatType;
    QTimer* 		m_vmMonitor;
    bool 		m_useLongInterval;
    short 		m_count;
    long 		m_previousFaults;
    QVector<int> 	m_pageFaults;
    QVector<int> 	m_memStates;
    MemState 		m_memoryState;
    QDateTime 		m_lastChanged;
    QValueSpaceObject*	m_vso;
};

#endif // _GENERICMEMORYMONITOR_H_
