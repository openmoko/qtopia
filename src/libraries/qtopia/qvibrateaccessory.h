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

#ifndef QVIBRATEACCESSORY_H
#define QVIBRATEACCESSORY_H

#include "qhardwareinterface.h"

class QTOPIA_EXPORT QVibrateAccessory : public QHardwareInterface
{
    Q_OBJECT
public:
    explicit QVibrateAccessory( const QString& id = QString(), QObject *parent = 0,
                                QAbstractIpcInterface::Mode mode = Client );
    ~QVibrateAccessory();

    bool vibrateOnRing() const;
    bool vibrateNow() const;

    bool supportsVibrateOnRing() const;
    bool supportsVibrateNow() const;

public slots:
    virtual void setVibrateOnRing( const bool value );
    virtual void setVibrateNow( const bool value );

signals:
    void vibrateOnRingModified();
    void vibrateNowModified();
};

class QTOPIA_EXPORT QVibrateAccessoryProvider : public QVibrateAccessory
{
    Q_OBJECT
public:
    explicit QVibrateAccessoryProvider( const QString& id, QObject *parent = 0 );
    ~QVibrateAccessoryProvider();

protected:
    void setSupportsVibrateOnRing( bool value );
    void setSupportsVibrateNow( bool value );

public slots:
    void setVibrateOnRing( const bool value );
    void setVibrateNow( const bool value );

};

#endif //QVIBRATEACCESSORY_H
