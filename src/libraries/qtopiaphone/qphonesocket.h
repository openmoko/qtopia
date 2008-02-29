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
#ifndef QPHONESOCKET_H
#define QPHONESOCKET_H

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qstringlist.h>

class QPhoneSocketPrivate;

class QTOPIAPHONE_EXPORT QPhoneSocket : public QObject
{
    Q_OBJECT
public:
    explicit QPhoneSocket( QObject *parent = 0 );
    explicit QPhoneSocket( int fd, QObject *parent = 0 );
    ~QPhoneSocket();

public slots:
    void send( const QStringList& cmd );
    void send( const QString& cmd );
    void send( const QString& cmd, const QString& arg1 );
    void send( const QString& cmd, const QString& arg1, const QString& arg2 );
    void send( const QString& cmd, const QString& arg1,
               const QString& arg2, const QString& arg3 );

signals:
    void received( const QStringList& cmd );
    void incoming( QPhoneSocket *socket );
    void closed();

private slots:
    void readyRead();
    void readyAccept();

private:
    void init();
    void processLine( const char *line, int len );
    QPhoneSocketPrivate *d;
};

#endif // QPHONESOCKET_H
