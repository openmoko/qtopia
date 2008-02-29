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

#ifndef QCALLBARRING_H
#define QCALLBARRING_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QCallBarring : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(BarringType)
public:
    explicit QCallBarring( const QString& service = QString::null,
                           QObject *parent = 0,
                           QCommInterface::Mode mode = Client );
    ~QCallBarring();

    enum BarringType
    {
        OutgoingAll,
        OutgoingInternational,
        OutgoingInternationalExceptHome,
        IncomingAll,
        IncomingWhenRoaming,
        IncomingNonTA,
        IncomingNonMT,
        IncomingNonSIM,
        IncomingNonMemory,
        AllBarringServices,
        AllOutgoingBarringServices,
        AllIncomingBarringServices
    };

public slots:
    virtual void requestBarringStatus( QCallBarring::BarringType type );
    virtual void setBarringStatus( QCallBarring::BarringType type,
                                   const QString& password,
                                   QTelephony::CallClass cls,
                                   bool lock );
    virtual void unlockAll( const QString& password );
    virtual void unlockAllIncoming( const QString& password );
    virtual void unlockAllOutgoing( const QString& password );
    virtual void changeBarringPassword( QCallBarring::BarringType type,
                                        const QString& oldPassword,
                                        const QString& newPassword );

signals:
    void barringStatus( QCallBarring::BarringType type,
                        QTelephony::CallClass cls );
    void setBarringStatusResult( QTelephony::Result result );
    void unlockResult( QTelephony::Result result );
    void changeBarringPasswordResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QCallBarring::BarringType)

#endif /* QCALLBARRING_H */
