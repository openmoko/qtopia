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

#ifndef __QCOMMDEVICESESSION_H__
#define __QCOMMDEVICESESSION_H__

#include <qglobal.h>
#include <qobject.h>

#include <qtopiaglobal.h>

class QByteArray;

class QCommDeviceSession_Private;
class QTOPIACOMM_EXPORT QCommDeviceSession : public QObject
{
    Q_OBJECT

    friend class QCommDeviceSession_Private;

public:
    enum WaitType { Block, BlockWithEventLoop };

    explicit QCommDeviceSession(const QByteArray &deviceId, QObject *parent = 0);
    ~QCommDeviceSession();

    void startSession();
    void endSession();

    const QByteArray &deviceId() const;

    static QCommDeviceSession * session(const QByteArray &deviceId,
                                        WaitType type = BlockWithEventLoop,
                                        QObject *parent = 0);

signals:
    void sessionOpen();
    void sessionFailed();
    void sessionClosed();

private:
    QCommDeviceSession_Private *m_data;
};

#endif
