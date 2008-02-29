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

#ifndef MMSCOMMS_H
#define MMSCOMMS_H

#include <QObject>

class QMailAccount;
class MMSMessage;
class QUrl;

class MmsComms : public QObject
{
    Q_OBJECT
public:
    MmsComms(QMailAccount *acc, QObject *parent=0);
    virtual ~MmsComms();

    virtual void sendMessage(MMSMessage &msg, const QByteArray& encoded) = 0;
    virtual void retrieveMessage(const QUrl &url) = 0;
    virtual bool isActive() const = 0;
    virtual void clearRequests() = 0;

    QString networkConfig() const;

signals:
    void notificationInd(const MMSMessage &msg);
    void deliveryInd(const MMSMessage &msg);
    void sendConf(const MMSMessage &msg);
    void retrieveConf(const MMSMessage &msg, int size);
    void statusChange(const QString &status);
    void error(int code, const QString &msg);
    void transferSize(int size);
    void transfersComplete();

protected:
    QMailAccount *account;
};

#endif
