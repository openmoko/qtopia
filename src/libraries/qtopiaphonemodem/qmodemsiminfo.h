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

#ifndef QMODEMSIMINFO_H
#define QMODEMSIMINFO_H

#include <qsiminfo.h>

class QModemService;
class QAtResult;
class QModemSimInfoPrivate;

class QTOPIAPHONEMODEM_EXPORT QModemSimInfo : public QSimInfo
{
    Q_OBJECT
public:
    explicit QModemSimInfo( QModemService *service );
    ~QModemSimInfo();

protected slots:
    void simInserted();
    void simRemoved();

private slots:
    void requestIdentity();
    void cimi( bool ok, const QAtResult& result );

private:
    QModemSimInfoPrivate *d;

    static QString extractIdentity( const QString& content );
};

#endif /* QMODEMSIMINFO_H */
