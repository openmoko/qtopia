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
#ifndef __QOBEXSERVERSESSION_P_H__
#define __QOBEXSERVERSESSION_P_H__

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

#include <qobexserversession.h>
#include <qobexheader.h>
#include <private/qobexsocket_p.h>

#include <QByteArray>
#include <QBuffer>
#include <QSet>
#include <QPointer>

class QObexSocketPrivate;

class QObexServerSessionPrivate : public QObject
{
    friend class QObexServerSession;
    Q_OBJECT

public:
    // ---- Functions accessed by QObexSocket: ----

    QObex::ResponseCode acceptIncomingRequest(QObex::Request request);
    QObex::ResponseCode receivedRequestFirstPacket(QObex::Request request,
                                                   QObexHeader &header,
                                                   QObexHeader *responseHeader);
    QObex::ResponseCode receivedRequest(QObex::Request request,
                                        QObexHeader &requestHeader,
                                        const QByteArray &nonHeaderData,
                                        QObexHeader *responseHeader);
    void requestDone(QObex::Request request);

    QObex::ResponseCode bodyDataAvailable(const char *data, qint64 size);
    QObex::ResponseCode bodyDataRequired(const char **data, qint64 *size);

    void errorOccurred(QObexServerSession::Error error, const QString &errorString);

private:
    // ---- Members accessed by QObexServerSession: ----

    QObexServerSessionPrivate(QIODevice *device, QObexServerSession *parent);
    ~QObexServerSessionPrivate();
    void close();

    void setNextResponseHeader(const QObexHeader &header);

    QPointer<QObexSocket> m_socket;


private slots:
    void socketDisconnected();

private:
    // ---- Internal memebers: ----
    QObex::ResponseCode processRequestHeader(QObex::Request request,
                                             QObexHeader &requestHeader,
                                             const QByteArray &nonHeaderData);
    QObex::ResponseCode processAuthenticationChallenge(QObexAuthenticationChallenge &challenge);
    QObex::ResponseCode readAuthenticationResponse(const QByteArray &responseBytes);
    bool invokeSlot(const QString &methodName,
                    QObex::ResponseCode *responseCode = 0,
                    QGenericArgument arg1 = QGenericArgument(),
                    QGenericArgument arg2 = QGenericArgument());
    void resetOpData();
    QObex::SetPathFlags getSetPathFlags(const QByteArray &nonHeaderData);
    void initAvailableCallbacks();

    static QLatin1String getRequestSlot(QObex::Request request);

    QObexServerSession *m_parent;
    bool m_closed;
	bool m_socketDisconnected;

    QObexHeader m_nextResponseHeader;
    bool m_invokedRequestSlot;

    QSet<QString> m_implementedCallbacks;

    QByteArray m_challengeNonce;
};

#endif
