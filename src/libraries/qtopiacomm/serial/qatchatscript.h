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

#ifndef QATCHATSCRIPT_H
#define QATCHATSCRIPT_H

#include <qatchat.h>
#include <qatresult.h>

class QAtChatScriptPrivate;

class QTOPIACOMM_EXPORT QAtChatScript : public QObject
{
    Q_OBJECT
public:
    explicit QAtChatScript( QAtChat *atchat, QObject *parent = 0 );
    ~QAtChatScript();

    int totalCommands() const;
    int successfulCommands() const;

public slots:
    void runChatFile( const QString& filename );
    void runChat( const QString& chatScript );
    void runChat( const QStringList& commands );
    void stop();

signals:
    void done( bool ok, const QAtResult& result );

private slots:
    void commandDone( bool ok, const QAtResult& result );

private:
    QAtChatScriptPrivate *d;

    void sendNext();
    QString expandEscapes( const QString& value );
    void word( const QString& value );
};

#endif // QATCHATSCRIPT_H
