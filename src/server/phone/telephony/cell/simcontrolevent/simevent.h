/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef SIMEVENT_H
#define SIMEVENT_H

#include <QObject>
#include <QSimControlEvent>

class QCommServiceManager;
class QSimToolkit;

class SimControlEventHandlerTask : public QObject
{
    Q_OBJECT
public:
    SimControlEventHandlerTask( QObject *parent = 0 );

private slots:
    void servicesChanged();
    void simControlEvent( const QSimControlEvent& event );

private:
    QSimToolkit *simToolkit;
    QCommServiceManager *serviceManager;
};

#endif
