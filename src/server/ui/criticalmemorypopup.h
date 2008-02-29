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

#ifndef _CRITICALMEMORYPOPUP_H_
#define _CRITICALMEMORYPOPUP_H_

#include <QMessageBox>
#include <QTimer>
#include "memorymonitor.h"

class CriticalMemoryPopup : public QMessageBox
{
Q_OBJECT
public:
    CriticalMemoryPopup();

private slots:
    void showToUser();
    void memoryStateChanged(MemoryMonitor::MemState state);

private:
    QTimer m_delayedVMInfo;
    MemoryMonitor *m_monitor;
};

#endif // _CRITICALMEMORYPOPUP_H_
