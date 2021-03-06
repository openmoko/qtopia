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
#ifndef __QOBEXCLIENTSESSION_P_H__
#define __QOBEXCLIENTSESSION_P_H__

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qobexclientsession.h>
#include <private/qobexsocket_p.h>

#include <QQueue>
#include <QBuffer>
#include <QPointer>

class QObexCommand;

class QObexClientSessionPrivate : public QObject
{
    friend class QObexClientSession;
    Q_OBJECT

public:
    // ---- Functions accessed by QObexSocket: ----

    void requestDone(QObex::ResponseCode response);
    void requestResponseHeaderReceived(QObexHeader &header);
    void requestProgressed();
    void requestAborted();

    void bodyDataRequired(const char **data, qint64 *size);
    void bodyDataAvailable(const char *data, qint64 size);

    void errorOccurred(QObexClientSession::Error error, const QString &msg);

private slots:
    void doPending();
    void performAbort();
    void resendRequestWithAuthResponse();
    void socketDisconnected();

private:
    // ---- Functions accessed by QObexClientSession: ----

    QObexClientSessionPrivate(QIODevice *device, QObexClientSession *parent);
    ~QObexClientSessionPrivate();

    int connect(const QObexHeader &header);
    int disconnect(const QObexHeader &header);
    int put(const QObexHeader &header, QIODevice *dev);
    int put(const QObexHeader &header, const QByteArray &data);
    int putDelete(const QObexHeader &header);
    int get(const QObexHeader &header, QIODevice *dev);
    int setPath(const QObexHeader &header, QObex::SetPathFlags flags);

    void abort();

    int currentId() const;
    QObex::Request currentRequest() const;

    QIODevice *currentDevice() const;

    void clearPendingRequests();
    bool hasPendingRequests() const;

    qint64 read(char *data, qint64 maxlen);
    QByteArray readAll();
    qint64 bytesAvailable() const;

    QPointer<QObexSocket> m_socket;
    QObexClientSession::Error m_error;
    QString m_errorString;
    QObex::ResponseCode m_lastResponseCode;
    QObexHeader m_lastResponseHeader;

    quint32 m_connId;
    bool m_haveConnId;


    // ------- Members for internal use: -------

    int queueCommand(QObexCommand *cmd);
    void performCommand(QObexCommand *cmd);

    bool preparePut(QObexCommand *cmd);
    bool prepareGet(QObexCommand *cmd);

    void finishedAbort(bool success);
    void finishedCurrentOp(QObexClientSession::Error error, const QString &msg);
    void finishedAllOps();

    bool readAuthenticationChallenge(const QByteArray &challengeBytes);
    bool readAuthenticationResponse(const QByteArray &responseBytes);

    void cleanUpDataTransfer();
    void clearAllRequests();

    QObexClientSession *m_parent;
    bool m_socketDisconnected;
    bool m_doneSocketSetup;
    QQueue<QObexCommand *> m_cmdQueue;
    bool m_busyWithRequest;

    // data transfer
    int m_currBytes;    // bytes read/written so far
    int m_totalBytes;   // total bytes to read/write
    char *m_buf;        // buffer for Put if sending from QIODevice
    QBuffer m_recvBuffer;   // for receiving data from Get request
    qint64 m_readSoFar;    // a read pointer for m_recvBuffer
    qint64 m_lastEmittedDone;

    bool m_aborting;

    QByteArray m_challengeNonce;
    QByteArray m_nextAuthResponseBytes;
    bool m_sentAuthResponse;
    bool m_receivedAuthResponse;

    int m_emittedReqFinishedId;
};

#endif
