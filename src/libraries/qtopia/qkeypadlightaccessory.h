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

#ifndef QKEYPADLIGHTACCESSORY_H
#define QKEYPADLIGHTACCESSORY_H

#include "qhardwareinterface.h"

class QTOPIA_EXPORT QKeypadLightAccessory : public QHardwareInterface
{
    Q_OBJECT
public:
    explicit QKeypadLightAccessory( const QString& id = QString(), QObject *parent = 0,
                                    QAbstractIpcInterface::Mode mode = Client );
    ~QKeypadLightAccessory();

    bool on() const;

public slots:
    virtual void setOn( const bool value );

signals:
    void onModified();
};

class QTOPIA_EXPORT QKeypadLightAccessoryProvider
            : public QKeypadLightAccessory
{
    Q_OBJECT
public:
    explicit QKeypadLightAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QKeypadLightAccessoryProvider();

public slots:
    void setOn( const bool value );
};

#endif //QKEYPADLIGHTACCESSORY_H
