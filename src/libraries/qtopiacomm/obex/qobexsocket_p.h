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
#ifndef __QOBEXSOCKET_P_H__
#define __QOBEXSOCKET_P_H__

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

#include <qobexnamespace.h>

#include <QObject>
#include <QPointer>

class QIODevice;
class QObexClientSessionPrivate;
class QObexServerSessionPrivate;
class QObexHeader;
class QByteArray;
class QLatin1String;
typedef void * obex_t;
typedef void * obex_object_t;

class QTOPIA_AUTOTEST_EXPORT QObexSocket : public QObject
{
    Q_OBJECT

public:
    explicit QObexSocket(QIODevice *device, QObject *parent = 0);
    ~QObexSocket();

    inline bool isValid() const { return m_handle != 0 && !m_device.isNull(); }
    QIODevice *device() const;

    void setObexClient(QObexClientSessionPrivate *client);
    void setObexServer(QObexServerSessionPrivate *server);

    bool sendRequest(QObex::Request request,
                     const QObexHeader &header,
                     const char *nonHeaderData = 0,
                     int nonHeaderDataSize = 0);
    bool abortCurrentRequest();

    void connectionEvent(obex_object_t *obj, int mode, int event,
                            int obex_cmd, int obex_rsp);

    void resetObexHandle();
    inline obex_t* handle() const { return m_handle; }

    static QLatin1String responseToString(QObex::ResponseCode response);

private slots:
    void processInput();
    void prepareToClose();

private:
    void handleReqHint(int obex_cmd, obex_object_t *obj);
    void handleReq(int obex_cmd, obex_object_t *obj);
    void handleStreamAvailable(int mode, obex_object_t *obj);
    void handleStreamEmpty(int mode, obex_object_t *obj);

    bool serverAcceptsRequest(obex_object_t *obj, QObex::Request request);
    QObex::Request getCurrentRequest(int obex_cmd);
    QByteArray &readNonHeaderData(obex_object_t *obj, QByteArray &bytes);
    void setNextResponse(obex_object_t *obj, QObex::ResponseCode response);

    QPointer<QIODevice> m_device;
    obex_t *m_handle;
    bool m_cleanedUp;

    QPointer<QObexClientSessionPrivate> m_client;
    QPointer<QObexServerSessionPrivate> m_server;

    bool m_serverRefusedRequest;
    bool m_checkedServerAcceptsRequest;
    bool m_gotPutBodyData;
};

#endif
