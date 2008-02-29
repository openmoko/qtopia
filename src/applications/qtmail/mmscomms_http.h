/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#ifndef MMSCOMMS_HTTP_H
#define MMSCOMMS_HTTP_H

#include "mmscomms.h"
#include <qlist.h>

class MailAccount;
class MMSMessage;
class QTimer;
class QUrl;
class QHttp;
class QHttpResponseHeader;
class QHttpRequestHeader;

class MmsCommsHttp : public MmsComms
{
    Q_OBJECT
public:
    MmsCommsHttp(MailAccount *acc, QObject *parent=0);
    ~MmsCommsHttp();

    virtual void sendMessage(MMSMessage &msg);
    virtual void retrieveMessage(const QUrl &url);
    virtual bool isActive() const;
    virtual void clearRequests();

private slots:
    void appMessage(const QString &msg, const QByteArray &data);
    void dataReadProgress(int done, int total);
    void dataSendProgress(int done, int total);
    void done(bool error);
    void cleanup();
    void requestFinished(int id, bool error);
    void requestStarted(int id);
    void responseHeaderReceived(const QHttpResponseHeader &resp);
    void stateChanged(int state);

private:
    void addAuth(QHttpRequestHeader &header);
    QHttp *createHttpConnection(const QString &host, int port);
    void destroyHttpConnection(QHttp *http);

private:
    QHttp *rhttp;
    QHttp *shttp;
    int rId;
    int sId;
    int rStatus;
    int sStatus;
    QTimer *timer;
    QList<QHttp*> deleteList;
};

#endif
