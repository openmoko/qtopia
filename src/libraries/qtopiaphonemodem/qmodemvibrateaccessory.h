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

#ifndef QMODEMVIBRATEACCESSORY_H
#define QMODEMVIBRATEACCESSORY_H

#include <qvibrateaccessory.h>

class QModemService;

class QTOPIAPHONEMODEM_EXPORT QModemVibrateAccessory
        : public QVibrateAccessoryProvider
{
    Q_OBJECT
public:
    explicit QModemVibrateAccessory( QModemService *service );
    ~QModemVibrateAccessory();

public slots:
    void setVibrateOnRing( const bool value );
    void setVibrateNow( const bool value );

private:
    QModemService *service;
};

#endif // QMODEMVIBRATEACCESSORY_H
