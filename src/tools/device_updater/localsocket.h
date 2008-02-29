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

#ifndef DEVICE_UPDATER_LOCALSOCKET_H
#define DEVICE_UPDATER_LOCALSOCKET_H

#define LOCAL_SOCKET_PATH ".device_updater.sock"

#include <QObject>

class LocalSocket : public QObject
{
    Q_OBJECT
public:
    LocalSocket();
    virtual ~LocalSocket();
    virtual void sendRequest( const QString & );
    bool isErrorCondition() const { return mDescriptor == -1; }
protected:
    void setDescriptor(int desc) { mDescriptor = desc; }
    int descriptor() { return mDescriptor; }
private:
    void connect();
    int mDescriptor;
};

#endif
