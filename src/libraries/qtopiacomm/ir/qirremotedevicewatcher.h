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

#ifndef __QIRREMOTEDEVICEWATCHER_H__
#define __QIRREMOTEDEVICEWATCHER_H__

#include <QObject>
#include <qirnamespace.h>
#include <qtopiaglobal.h>

class QIrRemoteDeviceWatcher_Private;

class QTOPIACOMM_EXPORT QIrRemoteDeviceWatcher : public QObject
{
    Q_OBJECT
    friend class QIrRemoteDeviceWatcher_Private;

public:
    explicit QIrRemoteDeviceWatcher(QObject *parent = 0);
    ~QIrRemoteDeviceWatcher();

    bool watch(int ms, QIr::DeviceClasses = QIr::All);

signals:
    void deviceFound();

private:
    Q_DISABLE_COPY(QIrRemoteDeviceWatcher)
    QIrRemoteDeviceWatcher_Private *m_data;
};

#endif
