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

#ifndef QSERVICECHECKER_H
#define QSERVICECHECKER_H

#include <qcomminterface.h>

class QTOPIAPHONE_EXPORT QServiceChecker : public QCommInterface
{
    Q_OBJECT
public:
    explicit QServiceChecker( const QString& service = QString(), QObject *parent = 0,
                              QCommInterface::Mode mode = Client );
    ~QServiceChecker();

    bool isValid();

protected:
    void setValid( bool value );
};

class QTOPIAPHONE_EXPORT QServiceCheckerServer : public QServiceChecker
{
    Q_OBJECT
public:
    QServiceCheckerServer( const QString& service, bool valid,
                           QObject *parent = 0 );
    ~QServiceCheckerServer();

};

#endif /* QSERVICECHECKER_H */
