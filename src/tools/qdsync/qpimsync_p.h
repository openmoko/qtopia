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
#ifndef QPIMSYNCPROTOCOL_H
#define QPIMSYNCPROTOCOL_H

#include <QObject>

class QSyncProtocolController;
class QSyncProtocol : public QObject
{
    friend class QSyncProtocolController;
    Q_OBJECT
public:
    QSyncProtocol(QObject *parent = 0);
    ~QSyncProtocol();

    void startSync(const QString &);
    void abortSync();

    int timeoutDuration() const;
    void setTimeoutDuration(int);

signals:
    void syncError(const QString &);
    void syncComplete();
    void timeout();

public slots:
    void resetTimeout();

private:
    QSyncProtocolController *d;
};

#endif // QPIMSYNCPROTOCOL_H
