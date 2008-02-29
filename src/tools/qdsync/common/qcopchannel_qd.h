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
#ifndef QCOPCHANNEL_QD_H
#define QCOPCHANNEL_QD_H

#include <qdglobal.h>

#include <QObject>
#include <QString>
#include <QByteArray>

namespace qdsync {

class QD_EXPORT QCopChannel : public QObject
{
    Q_OBJECT
public:
    QCopChannel( const QString &channel, QObject *parent = 0 );
    ~QCopChannel();

    QString channel() const;

    static bool isRegistered( const QString &channel );
    static void send( const QString &ch, const QString &msg, const QByteArray &data = QByteArray() );

signals:
    void received(const QString &msg, const QByteArray &data);

private:
    void init(const QString &channel);
    void receive(const QString &msg, const QByteArray &data);

    QString mChannel;
};

};

using namespace qdsync;

#endif // QCOPCHANNEL_QWS_H
