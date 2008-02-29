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

#ifndef QSMSREADER_H
#define QSMSREADER_H

#include <qcomminterface.h>
#include <qsmsmessage.h>

class QTOPIAPHONE_EXPORT QSMSReader : public QCommInterface
{
    Q_OBJECT
public:
    explicit QSMSReader( const QString& service = QString(),
                         QObject *parent = 0, QCommInterface::Mode mode = Client );
    ~QSMSReader();

    int unreadCount() const;
    QStringList unreadList() const;

    int usedMessages() const;
    int totalMessages() const;

    bool ready() const;

public slots:
    virtual void check();
    virtual void firstMessage();
    virtual void nextMessage();
    virtual void deleteMessage( const QString& id );
    virtual void setUnreadCount( int value );

protected:
    void setReady( bool value );

signals:
    void unreadCountChanged();
    void messageCount( int total );
    void fetched( const QString& id, const QSMSMessage& m );
    void readyChanged();
};

#endif
