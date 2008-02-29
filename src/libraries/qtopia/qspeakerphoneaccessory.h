/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef QSPEAKERPHONEACCESSORY_H
#define QSPEAKERPHONEACCESSORY_H

#include "qhardwareinterface.h"

class QTOPIA_EXPORT QSpeakerPhoneAccessory : public QHardwareInterface
{
    Q_OBJECT
public:
    explicit QSpeakerPhoneAccessory
        ( const QString& id = QString(), QObject *parent = 0,
          QAbstractIpcInterface::Mode mode = Client );
    ~QSpeakerPhoneAccessory();

    bool onSpeaker() const;

public slots:
    virtual void setOnSpeaker( bool value );

signals:
    void onSpeakerModified();
};

class QTOPIA_EXPORT QSpeakerPhoneAccessoryProvider
            : public QSpeakerPhoneAccessory
{
    Q_OBJECT
public:
    explicit QSpeakerPhoneAccessoryProvider
        ( const QString& id, QObject *parent = 0 );
    ~QSpeakerPhoneAccessoryProvider();

public slots:
    void setOnSpeaker( bool value );
};

#endif //QSPEAKERPHONEACCESSORY_H
