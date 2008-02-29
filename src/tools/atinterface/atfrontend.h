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

#ifndef ATFRONTEND_H
#define ATFRONTEND_H

#include <qserialiodevice.h>
#include <qatresult.h>

class AtFrontEndPrivate;
class AtOptions;
class QAtResult;

class AtFrontEnd : public QObject
{
    Q_OBJECT
public:
    AtFrontEnd( const QString& startupOptions, QObject *parent=0 );
    ~AtFrontEnd();

    QSerialIODevice *device() const;
    void setDevice( QSerialIODevice *device );
    void setDataSource( QIODevice* dataSourceDevice );

    enum State
    {
        Offline,
        OnlineData,
        OnlineCommand
    };

    AtFrontEnd::State state() const;
    AtOptions *options() const;

    bool canMux() const;
    void setCanMux( bool value );

public slots:
    void setState( AtFrontEnd::State state );
    void requestExtra();
    void send( const QString& line );
    void send( QAtResult::ResultCode result );
    void stopRepeatLast();

signals:
    void commands( const QStringList& cmds );
    void extra( const QString& line, bool cancel );
    void remoteHangup();
    void enterCommandState();

private slots:
    void raiseDtr();
    void readyRead();
    void dsrChanged( bool value );
    void dataSourceReadyRead();
    void dataSourceClosed();

private:
    AtFrontEndPrivate *d;

    void writeCRLF();
    void writeBackspace();
    void parseCommandLine( const QString& line );
};

#endif // ATFRONTEND_H
