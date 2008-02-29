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

#ifndef QMODEMRFFUNCTIONALITY_H
#define QMODEMRFFUNCTIONALITY_H

#include <qphonerffunctionality.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemRfFunctionality
        : public QPhoneRfFunctionality
{
    Q_OBJECT
public:
    explicit QModemRfFunctionality( QModemService *service );
    ~QModemRfFunctionality();

public slots:
    void forceLevelRequest();
    void setLevel( QPhoneRfFunctionality::Level level );

private slots:
    void cfun( bool ok, const QAtResult& result );
    void cfunSet( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif /* QMODEMRFFUNCTIONALITY_H */
