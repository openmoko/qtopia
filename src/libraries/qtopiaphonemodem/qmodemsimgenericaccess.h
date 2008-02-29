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

#ifndef QMODEMSIMGENERICACCESS_H
#define QMODEMSIMGENERICACCESS_H

#include <qsimgenericaccess.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT QModemSimGenericAccess : public QSimGenericAccess
{
    Q_OBJECT
public:
    explicit QModemSimGenericAccess( QModemService *service );
    ~QModemSimGenericAccess();

public slots:
    void command( const QString& reqid, const QByteArray& data );

private slots:
    void csim( bool ok, const QAtResult& result );

private:
    QModemService *service;
};

#endif /* QMODEMSIMGENERICACCESS_H */
