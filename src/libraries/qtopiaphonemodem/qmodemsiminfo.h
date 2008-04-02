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
    enum SimNotInsertedReasons {
        Reason_SimFailure = 0x1
    };

    explicit QModemSimInfo( QModemService *service );
    ~QModemSimInfo();

    void setSimNotInsertedReason(enum SimNotInsertedReasons);

protected slots:
    void simInserted();
    void simRemoved();

private slots:
    void requestIdentity();
    void cimi( bool ok, const QAtResult& result );
    void serviceItemPosted( const QString& item );

private:
    QModemSimInfoPrivate *d;

    static QString extractIdentity( const QString& content );
};

#endif /* QMODEMSIMINFO_H */
