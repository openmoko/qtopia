/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include "mmscomms_http.h"
#include "account.h"
#include "mmsmessage.h"
#include <qtopia/mail/mailmessage.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <QUrl>
#include <QFile>
#include <QBuffer>
#include <QTimer>
#include <QHttp>
#include <QSettings>
#include <QDSActionRequest>
#include <QDSData>
#include <QDrmContent>

// This is an example of how to implement a comms driver for MMS.

MmsCommsHttp::MmsCommsHttp(MailAccount *acc, QObject *parent)
    : MmsComms(acc, parent), rhttp(0), shttp(0), rId(0), sId(0),
      rStatus(200), sStatus(200)
{
    connect(qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
            this, SLOT(appMessage(const QString&,const QByteArray&)));
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(cleanup()));
}

MmsCommsHttp::~MmsCommsHttp()
{
}

bool MmsCommsHttp::isActive() const
{
    if ((rhttp && rhttp->currentId()) || (shttp && shttp->currentId()))
        return true;

    return false;
}

void MmsCommsHttp::clearRequests()
{
    deleteList.append(shttp);
    shttp = 0;
    sId = 0;
    sStatus = 200;
    deleteList.append(rhttp);
    rhttp = 0;
    rId = 0;
    rStatus = 200;
}

void MmsCommsHttp::sendMessage(MMSMessage &msg)
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    msg.encode(&buffer);
    buffer.close();

    //### DEBUG
    /*
    QFile f("sentmms.mms");
    if (f.open(QIODevice::WriteOnly)) {
        f.writeBlock(data);
    }
    */

    QSettings conf(networkConfig(), QSettings::IniFormat);
    conf.beginGroup("MMS");
    QString server = conf.value("Server").toString();

    if (!server.startsWith("http://"))
        server = "http://" + server;
    QUrl url(server);

    int port = 80;
    if (!shttp) {
        port = url.port() > 0 ? url.port() : 80;
        shttp = createHttpConnection(url.host(), port);
    }

    sStatus = 200;
    QHttpRequestHeader header("POST", url.toString());
    header.setValue("User-Agent", "Trolltech-Qtopia-MMS-Client/4.1");
    header.setValue("Host", url.host() + ":" + QString::number(port));
    header.setContentType("application/vnd.wap.mms-message");
    header.setContentLength(data.size());
//    addAuth(header);
    qLog(Messaging) << "MmsCommsHttp: Sending" << header.toString();

    int id = shttp->request(header, data);
    if (msg.type() == MMSMessage::MSendReq)
        sId = id;
}

void MmsCommsHttp::retrieveMessage(const QUrl &url)
{
    int port = 80;
    if (url.port() > 0)
        port = url.port();
    rStatus = 200;
    if (!rhttp)
        rhttp = createHttpConnection(url.host(), port);
    QHttpRequestHeader header("GET", url.toString());
    header.setValue("User-Agent", "Trolltech-Qtopia-MMS-Client/4.2");
    header.setValue("Host", url.host() + ":" + QString::number(port));

    QPair< QString, QString > drmHeader;
    foreach( drmHeader, QDrmContent::httpHeaders() )
        header.setValue( drmHeader.first, drmHeader.second );
    QStringList types = QDrmContent::supportedTypes();

    if( !types.isEmpty() )
        header.setValue( "Accept", types.join( ", " ) );

    rId = rhttp->request(header);
}

void MmsCommsHttp::appMessage(const QString &msg, const QByteArray &data)
{
    if (msg == "SMS::pushMmsMessage(QDSActionRequest)") {
        qLog(Messaging) << "Received SMS::pushMmsMessage";
        QDataStream stream(data);
        QDSActionRequest request;
        stream >> request;

        QByteArray pushData(request.requestData().data());
        QBuffer buffer(&pushData);
        buffer.open(QIODevice::ReadOnly);

        MMSMessage mmsMsg;
        mmsMsg.decode(&buffer);
        if (mmsMsg.type() == MMSMessage::MNotificationInd)
            emit notificationInd(mmsMsg);
        else if (mmsMsg.type() == MMSMessage::MDeliveryInd)
            emit deliveryInd(mmsMsg);

        // Tell the requestor that we are finished with the QDS request.
        QDSActionRequest( request ).respond();
    }
}

void MmsCommsHttp::dataReadProgress(int done, int /*total*/)
{
    emit transferSize(done);
}

void MmsCommsHttp::dataSendProgress(int done, int /*total*/)
{
    emit transferSize(done);
}

void MmsCommsHttp::done(bool error)
{
    if (sender() == shttp)
        qLog(Messaging) << "MmsClient::done POST:" << error;
    else
        qLog(Messaging) << "MmsClient::done GET:" << error;
    timer->start(1000);
}

void MmsCommsHttp::cleanup()
{
    if (shttp && !shttp->currentId() && !shttp->hasPendingRequests()) {
        destroyHttpConnection(shttp);
        shttp = 0;
    }
    if (rhttp && !rhttp->currentId() && !rhttp->hasPendingRequests()) {
        destroyHttpConnection(rhttp);
        rhttp = 0;
    }

    if (!shttp && !rhttp) {
        emit statusChange(QString::null);
        emit transfersComplete();
    }

    foreach (QHttp *http, deleteList)
        destroyHttpConnection(http);
    deleteList.clear();
}

void MmsCommsHttp::requestFinished(int id, bool err)
{
    QHttp *h = (QHttp *)sender();
    QByteArray data = h->readAll();
    QString tmp(data);
    if (err) {
        if (h->error() != QHttp::Aborted) {
            qWarning() << "MmsClient::requestFinished: Error:" << h->error() << h->errorString();
            emit statusChange(tr("Error occurred"));
            emit error(h->error(), h->errorString());
        }
        return;
    }
    if (id == rId && rStatus != 200 || id == sId && sStatus != 200) {
        emit statusChange(tr("Error occurred"));
        emit error(id == rId ? rStatus : sStatus, tr("Transfer Failed"));
        return;
    }
    QBuffer buffer(&data);
    buffer.open(QIODevice::ReadOnly);
    MMSMessage mmsMsg;
    mmsMsg.decode(&buffer);
    if (id == rId) {
        // m-retrieve-conf
        emit retrieveConf(mmsMsg, data.size());
        rId = 0;
    } else if (id == sId) {
        // m-send-conf.
        emit sendConf(mmsMsg);
        sId = 0;
    } else {
        // other responses ignored.
    }
}

void MmsCommsHttp::requestStarted(int id)
{
    Q_UNUSED(id);
}

void MmsCommsHttp::responseHeaderReceived(const QHttpResponseHeader &resp)
{
    if (resp.statusCode() != 200) {
        if (sender() == shttp)
            sStatus = resp.statusCode();
        else
            rStatus = resp.statusCode();
    }
}

void MmsCommsHttp::stateChanged(int state)
{
    switch (state) {
        case QHttp::Unconnected:
            break;
        case QHttp::HostLookup:
            emit statusChange(tr("Lookup host"));
            break;
        case QHttp::Connecting:
            emit statusChange(tr("Connecting..."));
            break;
        case QHttp::Sending:
            emit statusChange(tr("Sending..."));
            break;
        case QHttp::Reading:
            emit statusChange(tr("Receiving..."));
            break;
        case QHttp::Closing:
            emit statusChange(tr("Closing Connection"));
            break;
        default:
            break;
    }
}

void MmsCommsHttp::addAuth(QHttpRequestHeader &header)
{
    QString user = account->mailUserName();
    if (user.isNull())
        user = "";
    QString pass = account->mailPassword();
    if (pass.isNull())
        pass = "";
    QString auth = user + ':' + pass;
    QByteArray ba = auth.toLatin1();
    auth = "Basic " + MailMessage::encodeBase64(ba);
    header.setValue("Authorization", auth);
}

QHttp *MmsCommsHttp::createHttpConnection(const QString &host, int port)
{
    QHttp *http = new QHttp(host, port);
    connect(http, SIGNAL(dataReadProgress(int,int)),
            this, SLOT(dataReadProgress(int,int)));
    connect(http, SIGNAL(dataSendProgress(int,int)),
            this, SLOT(dataSendProgress(int,int)));
    connect(http, SIGNAL(done(bool)), this, SLOT(done(bool)));
    connect(http, SIGNAL(requestFinished(int,bool)),
            this, SLOT(requestFinished(int,bool)));
    connect(http, SIGNAL(requestStarted(int)),
            this, SLOT(requestStarted(int)));
    connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader&)),
            this, SLOT(responseHeaderReceived(const QHttpResponseHeader&)));
    connect(http, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));

    return http;
}

void MmsCommsHttp::destroyHttpConnection(QHttp *http)
{
    if (!http)
        return;
    disconnect(http, SIGNAL(dataReadProgress(int,int)),
            this, SLOT(dataReadProgress(int,int)));
    disconnect(http, SIGNAL(dataSendProgress(int,int)),
            this, SLOT(dataSendProgress(int,int)));
    disconnect(http, SIGNAL(done(bool)), this, SLOT(done(bool)));
    disconnect(http, SIGNAL(requestFinished(int,bool)),
            this, SLOT(requestFinished(int,bool)));
    disconnect(http, SIGNAL(requestStarted(int)),
            this, SLOT(requestStarted(int)));
    disconnect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader&)),
            this, SLOT(responseHeaderReceived(const QHttpResponseHeader&)));
    disconnect(http, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
    delete http;
}

