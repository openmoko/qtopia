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

#ifndef QSIMGENERICACCESS_H
#define QSIMGENERICACCESS_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>

class QTOPIAPHONE_EXPORT QSimGenericAccess : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSimGenericAccess
            ( const QString& service = QString(), QObject *parent = 0,
              QCommInterface::Mode mode = Client );
    ~QSimGenericAccess();

    QString command( const QByteArray& data );

public slots:
    virtual void command( const QString& reqid, const QByteArray& data );

signals:
    void response( const QString& reqid, QTelephony::Result result,
                   const QByteArray& data );
};

#endif /* QSIMGENERICACCESS_H */
