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

#ifndef QATCHAT_P_H
#define QATCHAT_P_H

#include <qatresult.h>
#include <qobject.h>
#include <qstring.h>
#include <qbytearray.h>

class QAtChatCommandPrivate;

class QAtChatCommand : public QObject
{
    Q_OBJECT
    friend class QAtChat;
    friend class QAtChatPrivate;
public:
    QAtChatCommand( const QString& command, QAtResult::UserData *data );
    QAtChatCommand( const QString& command, const QByteArray& pdu,
                    QAtResult::UserData *data );
    ~QAtChatCommand();

signals:
    void done( bool ok, const QAtResult& result );

private:
    QAtChatCommandPrivate *d;

    void emitDone();
};

class QAtChatLineRequest : public QObject
{
    Q_OBJECT
public:
    QAtChatLineRequest( QObject *target, const char *slot )
    {
        connect( this, SIGNAL(line(QString)), target, slot );
    }
    ~QAtChatLineRequest() {}

    void sendLine( const QString& value ) { emit line( value ); }

signals:
    void line( const QString& value );
};

#endif // QATCHAT_P_H
