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
#ifndef QPHONECALLMANAGER_P_H
#define QPHONECALLMANAGER_P_H

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

#include <qphonecallmanager.h>
#include <qphonecallprovider.h>
#include <qtopiaipcadaptor.h>
#include <qvaluespace.h>
#include <qmap.h>

class QPhoneCallManagerPrivate : public QObject
{
    Q_OBJECT
    friend class QPhoneCall;
    friend class QPhoneCallPrivate;
public:
    explicit QPhoneCallManagerPrivate( QObject *parent = 0 );
    ~QPhoneCallManagerPrivate();

    static QPhoneCallManagerPrivate *instance();

    QPhoneCall create( const QString& service, const QString& callType );
    void loadCallTypes();

    QList<QPhoneCall> calls;
    QValueSpaceItem *item;
    QStringList callTypes;
    QMap< QString, QStringList > callTypeMap;

signals:
    void newCall( const QPhoneCall& call );
    void callTypesChanged();
    void statesChanged( const QList<QPhoneCall>& calls );

private slots:
    void callStateChanged( const QString& identifier, QPhoneCall::State state,
                           const QString& number, const QString& service,
                           const QString& callType, int actions );
    void callStateTransaction( const QByteArray& transaction );
    void trackStateChanged( const QPhoneCall& call );
    void contentsChanged();

private:
    QtopiaIpcAdaptor *request;
    QtopiaIpcAdaptor *response;
};

#endif // QPHONECALLMANAGER_P_H
