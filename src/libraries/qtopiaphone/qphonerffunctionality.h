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

#ifndef QPHONERFFUNCTIONALITY_H
#define QPHONERFFUNCTIONALITY_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QPhoneRfFunctionality : public QCommInterface
{
    Q_OBJECT
    Q_ENUMS(Level)
    Q_PROPERTY(QPhoneRfFunctionality::Level level READ level WRITE setLevel)
public:
    explicit QPhoneRfFunctionality( const QString& service = QString(),
                                    QObject *parent = 0,
                                    QCommInterface::Mode mode = Client );
    ~QPhoneRfFunctionality();

    enum Level
    {
        Minimum,
        Full,
        DisableTransmit,
        DisableReceive,
        DisableTransmitAndReceive
    };

    QPhoneRfFunctionality::Level level() const;

public slots:
    virtual void forceLevelRequest();
    virtual void setLevel( QPhoneRfFunctionality::Level level );

signals:
    void levelChanged();
    void setLevelResult( QTelephony::Result result );
};

Q_DECLARE_USER_METATYPE_ENUM(QPhoneRfFunctionality::Level)

#endif /* QPHONERFFUNCTIONALITY_H */
