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

#ifndef ATSMSCOMMANDS_H
#define ATSMSCOMMANDS_H

#include <QTelephony>
#include <QObject>

class AtCommands;
class QSMSSender;
class QSMSReader;
class QSMSMessage;

class AtSmsCommands : public QObject
{
    Q_OBJECT

public:
    AtSmsCommands(AtCommands * parent);
    ~AtSmsCommands();

public slots:
    void atcmgd( const QString& params );
    void atcmgf( const QString& params );
    void atcmgl( const QString& params );
    void atcmgr( const QString& params );
    void atcmgs( const QString& params );
    void atcmgw( const QString& params );
    void atcmms( const QString& params );
    void atcnmi( const QString& params );
    void atcpms( const QString& params );
    void atcres(  );
    void atcsas(  );
    void atcsca( const QString& params );
    void atcsdh( const QString& params );

private slots:
    void extraLine( const QString& line, bool cancel );
    void smsFetched( const QString & id, const QSMSMessage & m );
    void smsFinished( const QString& id, QTelephony::Result result );

private:
    AtCommands *atc;

    // sms related
    QString smsNumber;
    QString smsMessageId;
    bool sendingSms;
    QSMSSender *smsSender;
    int smsMessageReference;
    QSMSReader *smsReader;
    bool readingSms;
    int readingSmsCount;
    int wantedSmsIndex;
    bool writingSms;
    uint cmgw_address_type;
    QString cmgw_status;
};

#endif // ATSMSCOMMANDS_H
