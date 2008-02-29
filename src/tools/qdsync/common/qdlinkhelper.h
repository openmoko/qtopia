/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef QDLINKHELPER_H
#define QDLINKHELPER_H

#include <qdglobal.h>
#include <QObject>
#include <QPointer>

class QIODevice;
class PingThread;

class QD_EXPORT QDLinkHelper : public QObject
{
    Q_OBJECT
public:
    QDLinkHelper( QIODevice *device, QObject *parent = 0 );
    ~QDLinkHelper();

    QIODevice *socket();
    QIODevice *rawSocket();

signals:
    void timeout();

private:
    void init( QIODevice *device );

    QPointer<QIODevice> rawDevice;
    QPointer<QIODevice> wrapperDevice;
    PingThread *thread;
};

#endif
