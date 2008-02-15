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

#ifndef QCELLBROADCAST_H
#define QCELLBROADCAST_H

#include <qcomminterface.h>
#include <qtelephonynamespace.h>
#include <qcbsmessage.h>

class QTOPIAPHONE_EXPORT QCellBroadcast : public QCommInterface
{
    Q_OBJECT
    Q_PROPERTY(QList<int> channels READ channels WRITE setChannels)
public:
    explicit QCellBroadcast( const QString& service = QString(),
                             QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QCellBroadcast();

    QList<int> channels() const;

public slots:
    virtual void setChannels( const QList<int>& list );

signals:
    void setChannelsResult( QTelephony::Result result );
    void broadcast( const QCBSMessage& message );
};

Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<int>)

#endif /* QCELLBROADCAST_H */
