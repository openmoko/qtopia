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

#ifndef QSMSSENDER_H
#define QSMSSENDER_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <qsmsmessage.h>

class QTOPIAPHONE_EXPORT QSMSSender : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSMSSender( const QString& service = QString(),
                         QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QSMSSender();

    QString send( const QSMSMessage& msg );

public slots:
    virtual void send( const QString& id, const QSMSMessage& msg );

signals:
    void finished( const QString& id, QTelephony::Result result );
};

#endif /* QSMSSENDER_H */
