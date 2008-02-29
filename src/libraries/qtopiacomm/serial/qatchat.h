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

#ifndef QATCHAT_H
#define QATCHAT_H

#include <qtopiaglobal.h>
#include <qatresult.h>
#include <qbytearray.h>


class QAtChatPrivate;
class QAtChatCommand;
class QSerialIODevice;

class QTOPIACOMM_EXPORT QAtChat : public QObject
{
    Q_OBJECT
    friend class QSerialIODevice;
private:
    explicit QAtChat( QSerialIODevice *device );
    ~QAtChat();

public:
    void chat( const QString& command );
    void chat( const QString& command, QObject *target, const char *slot,
               QAtResult::UserData *data = 0 );
    void chatPDU( const QString& command, const QByteArray& pdu,
                  QObject *target, const char *slot,
                  QAtResult::UserData *data = 0 );

    void registerNotificationType
        ( const QString& type, QObject *target,
          const char *slot, bool mayBeCommand = false );

    void abortDial();

    void suspend();
    void resume();

    void setDebugChars( char from, char to, char notify, char unknown );

    int deadTimeout() const;
    void setDeadTimeout( int msec );

    int retryOnNonEcho() const;
    void setRetryOnNonEcho( int msec );

    void setCPINTerminator();

    void requestNextLine( QObject *target, const char *slot );

    void send( const QString& command );

signals:
    void pduNotification( const QString& type, const QByteArray& pdu );
    void callNotification( const QString& type );
    void dead();

private slots:
    void incoming();
    void timeout();
    void failCommand();
    void retryTimeout();

private:
    QAtChatPrivate *d;

    void queue( QAtChatCommand *command );
    void done();
    bool writeLine( const QString& line );
    void writePduLine( const QString& line );
    bool processLine( const QString& line );

    void prime();
    void primeIfNecessary();
};

#endif // QATCHAT_H
