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

#ifndef QMODEMCELLBROADCAST_H
#define QMODEMCELLBROADCAST_H

#include <qcellbroadcast.h>

class QModemService;
class QModemCellBroadcastPrivate;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemCellBroadcast : public QCellBroadcast
{
    Q_OBJECT
public:
    explicit QModemCellBroadcast( QModemService *service );
    ~QModemCellBroadcast();

public slots:
    void setChannels( const QList<int>& list );

private slots:
    void sendChange();
    void sendAdd();
    void sendRemove();
    void cscb( bool ok, const QAtResult& result );
    void cscbQuery( bool ok, const QAtResult& result );
    void pduNotification( const QString& type, const QByteArray& pdu );
    void registrationStateChanged();
    void smsReady();

protected:
    void groupInitialized( QAbstractIpcInterfaceGroup *group );
    virtual bool removeBeforeChange() const;
    virtual bool removeOneByOne() const;

private:
    QModemCellBroadcastPrivate *d;

    QString command( int mode, const QList<int>& channels ) const;
};

#endif /* QMODEMCELLBROADCAST_H */
