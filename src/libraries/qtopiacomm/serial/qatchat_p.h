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

#ifndef QATCHAT_P_H
#define QATCHAT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

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
