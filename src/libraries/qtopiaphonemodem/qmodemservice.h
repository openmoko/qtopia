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

#ifndef QMODEMSERVICE_H
#define QMODEMSERVICE_H

#include <qtelephonyservice.h>
#include <qserialiodevice.h>
#include <qserialiodevicemultiplexer.h>
#include <qatchat.h>
#include <qatresult.h>

class QModemServicePrivate;
class QModemCallProvider;
class QModemIndicators;

class QTOPIAPHONEMODEM_EXPORT QModemService : public QTelephonyService
{
    Q_OBJECT
    friend class QModemSimToolkit;
public:
    explicit QModemService( const QString& service,
                   const QString& device = QString(),
                   QObject *parent = 0 );
    QModemService( const QString& service, QSerialIODeviceMultiplexer *mux,
                   QObject *parent = 0 );
    ~QModemService();

    virtual void initialize();

    QSerialIODeviceMultiplexer *multiplexer() const;
    QAtChat *primaryAtChat() const;
    QAtChat *secondaryAtChat() const;

    void chat( const QString& command );
    void chat( const QString& command, QObject *target, const char *slot,
               QAtResult::UserData *data = 0 );

    void retryChat( const QString& command );

    static QModemService *createVendorSpecific
                ( const QString& service = "modem",
                  const QString& device = QString(),
                  QObject *parent = 0 );

    void post( const QString& item );
    void connectToPost
        ( const QString& item, QObject *target, const char *slot );

    QModemIndicators *indicators() const;

    QString defaultCharset() const;

protected slots:
    virtual void needSms();
    virtual void suspend();
    virtual void wake();
    void suspendDone();
    void wakeDone();

signals:
    void posted( const QString& item );
    void resetModem();

protected:
    void setDefaultCharset(const QString&);

private:
    void init( QSerialIODeviceMultiplexer *mux );

private slots:
    void cmgfDone( bool ok );
    void sendNeedSms();
    void postItems();
    void firstTimeInit();
    void phoneBookPreload();
    void stkInitDone();

private:
    QModemServicePrivate *d;
};

#endif /* QMODEMSERVICE_H */
