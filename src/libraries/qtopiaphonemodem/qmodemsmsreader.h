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

#ifndef QMODEMSMSREADER_H
#define QMODEMSMSREADER_H

#include <qsmsreader.h>

class QModemService;
class QModemSMSReaderPrivate;
class QAtResult;
class QSMSTaggedMessage;

class QTOPIAPHONEMODEM_EXPORT QModemSMSReader : public QSMSReader
{
    Q_OBJECT
public:
    explicit QModemSMSReader( QModemService *service );
    ~QModemSMSReader();

public slots:
    void check();
    void firstMessage();
    void nextMessage();
    void deleteMessage( const QString& id );
    void setUnreadCount( int value );

protected:
    virtual QString messageStore() const;
    virtual QString messageListCommand() const;
    virtual void simDownload( const QSMSMessage& message );

private slots:
    void resetModem();
    void smsReady();
    void nmiStatusReceived( bool ok, const QAtResult& result );
    void newMessageArrived();
    void pduNotification( const QString& type, const QByteArray& pdu );
    void cpmsDone( bool ok, const QAtResult& result );
    void storeListDone( bool ok, const QAtResult& result );

private:
    QModemSMSReaderPrivate *d;

    void extractMessages( const QString& store, const QAtResult& result );
    void check(bool force);
    void fetchMessages();
    void joinMessages();
    void updateUnreadCount();
    void updateUnreadList();
    bool joinMessages( QList<QSMSTaggedMessage *>& messages,
                       QStringList& toBeDeleted );
    bool dispatchDatagram( QSMSTaggedMessage *m );
};

#endif
