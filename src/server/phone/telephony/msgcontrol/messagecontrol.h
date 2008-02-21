/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _MESSAGECONTROL_H_
#define _MESSAGECONTROL_H_

#include <QObject>
#include <qvaluespace.h>
#ifdef QTOPIA_CELL
#include <qsmsreader.h>
#endif
#include <qtopiaipcenvelope.h>
#include <QtopiaIpcAdaptor>
class QString;

class QCommServiceManager;

class MessageControl : public QObject
{
Q_OBJECT
public:
    static MessageControl *instance();

    int messageCount() const;
    bool smsFull() const;
    QString lastSmsId() const;

signals:
    void messageCount(int, bool, bool, bool);
    void newMessage(const QString &type, const QString &from, const QString &subject);
    void smsMemoryFull(bool);
    void messageRejected();

private slots:
    void smsUnreadCountChanged();
    void telephonyServicesChanged();
    void sysMessage(const QString& message, const QByteArray&);
    void smsMemoryFullChanged();
    void messageCountChanged();

private:
    QValueSpaceObject phoneValueSpace;
    QValueSpaceItem smsMemFull;
    void doNewCount(bool write=true, bool fromSystem=false, bool notify=true);
    MessageControl();
    QCommServiceManager *mgr;
#ifdef QTOPIA_CELL
    QSMSReader *smsreq;
#endif
    QtopiaIpcAdaptor *messageCountUpdate;
    QtopiaChannel channel;
    QString smsId;

    int smsCount;
    int mmsCount;
    int systemCount;
    bool smsIsFull;
    int prevSmsMemoryFull;
};

#endif // _MESSAGECONTROL_H_
