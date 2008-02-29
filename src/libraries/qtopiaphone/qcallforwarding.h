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

#ifndef QCALLFORWARDING_H
#define QCALLFORWARDING_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QCallForwarding : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(Reason)
public:
    explicit QCallForwarding( const QString& service = QString::null,
                              QObject *parent = 0,
                              QCommInterface::Mode mode = Client );
    ~QCallForwarding();

    enum Reason
    {
        Unconditional,
        MobileBusy,
        NoReply,
        NotReachable,
        All,
        AllConditional
    };

    struct Status
    {
        QTelephony::CallClass cls;
        QString number;
        int time;

        template <typename Stream> void serialize(Stream &stream) const;
        template <typename Stream> void deserialize(Stream &stream);
    };

public slots:
    virtual void requestForwardingStatus( QCallForwarding::Reason reason );
    virtual void setForwarding( QCallForwarding::Reason reason,
                                const QCallForwarding::Status& status,
                                bool enable );

signals:
    void forwardingStatus( QCallForwarding::Reason reason,
                           const QList<QCallForwarding::Status>& status );
    void setForwardingResult
            ( QCallForwarding::Reason reason, QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QCallForwarding::Reason)
Q_DECLARE_USER_METATYPE(QCallForwarding::Status)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QCallForwarding::Status>)

#endif /* QCALLFORWARDING_H */
