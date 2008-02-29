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

#ifndef QSERVICENUMBERS_H
#define QSERVICENUMBERS_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QServiceNumbers : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(NumberId)
public:
    explicit QServiceNumbers( const QString& service = QString(),
                              QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QServiceNumbers();

    enum NumberId
    {
        VoiceMail,
        SmsServiceCenter,
        SubscriberNumber
    };

public slots:
    virtual void requestServiceNumber( QServiceNumbers::NumberId id );
    virtual void setServiceNumber
        ( QServiceNumbers::NumberId id, const QString& number );

signals:
    void serviceNumber( QServiceNumbers::NumberId id, const QString& number );
    void setServiceNumberResult
            ( QServiceNumbers::NumberId id, QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QServiceNumbers::NumberId)

#endif /* QSERVICENUMBERS_H */
