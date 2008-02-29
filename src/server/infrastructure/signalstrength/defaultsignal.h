/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _DEFAULTSIGNAL_H
#define _DEFAULTSIGNAL_H

#include <QSignalSourceProvider>

class QHardwareManager;
class QSignalSource;
class DefaultSignal : public QSignalSourceProvider
{
Q_OBJECT
public:
    DefaultSignal(QObject *parent = 0);

private slots:
    void accessoryAdded( const QString& newAccessory );
    
    void pAvailabilityChanged(QSignalSource::Availability);
    void pSignalStrengthChanged(int);

private:
    void initSignalSource();
    void syncSignalSource();

    QString m_primary;
    QSignalSource *m_signalSource;
    QHardwareManager *m_accessories;
};

#endif //_DEFAULTSIGNAL_H
