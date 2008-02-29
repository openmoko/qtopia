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
#ifndef QSIMTOOLKIT_H
#define QSIMTOOLKIT_H


#include <qcomminterface.h>
#include <qsimterminalresponse.h>
#include <qsimenvelope.h>
#include <qsimcontrolevent.h>

class QTOPIAPHONE_EXPORT QSimToolkit : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSimToolkit( const QString& service = QString(), QObject *parent = 0,
                          QCommInterface::Mode mode = Client );
    ~QSimToolkit();

public slots:
    virtual void begin();
    virtual void end();
    virtual void sendResponse( const QSimTerminalResponse& resp );
    virtual void sendEnvelope( const QSimEnvelope& env );

    // Deprecated functions: use sendResponse() and sendEnvelope() instead.
    void mainMenuSelected( uint item );
    void clearText();
    void key( const QString& value );
    void input( const QString& value );
    void allowCallSetup();
    void denyCallSetup();
    void subMenuSelected( uint item );
    void subMenuExited();
    void idleScreen();
    void userActivity();
    void endSession();
    void moveBackward();
    void cannotProcess();
    void temporarilyUnavailable();
    void noResponseFromUser();
    void aborted();
    void help( QSimCommand::Type command, uint item=0 );

signals:
    void command( const QSimCommand& command );
    void beginFailed();
    void controlEvent( const QSimControlEvent& event );

protected:
    void emitCommandAndRespond( const QSimCommand& command );
};

#endif // QSIMTOOLKIT_H
