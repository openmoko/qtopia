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

#ifndef _MESSAGECONTROL_H_
#define _MESSAGECONTROL_H_

#include <QObject>
#include <qvaluespace.h>
#include <qsmsreader.h>
#include <qtopiaipcenvelope.h>
class QString;

class QCommServiceManager;

class MessageControl : public QObject
{
Q_OBJECT
public:
    static MessageControl *instance();

    int messageCount() const;
    bool smsFull() const;

signals:
    void messageCount(int, bool, bool, bool);
    void newMessage(const QString &type, const QString &from,
                    const QString &subject);
    void messageRejected();

private slots:
    void smsUnreadCountChanged();
    void telephonyServicesChanged();
    void sysMessage(const QString& message, const QByteArray&);
    void smsMemoryFullChanged();

private:
    QValueSpaceObject phoneValueSpace;
    QValueSpaceItem smsMemoryFull;
    void doNewCount(bool write=true, bool fromSystem=false, bool notify=true);
    MessageControl();
    QCommServiceManager *mgr;
    QSMSReader *smsreq;
    QtopiaChannel channel;

    int smsCount;
    int mmsCount;
    int systemCount;
    bool smsIsFull;
    int prevSmsMemoryFull;
};

#endif // _MESSAGECONTROL_H_
