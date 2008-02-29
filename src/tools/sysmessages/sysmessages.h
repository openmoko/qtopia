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

#ifndef SYSMESSAGES_H
#define SYSMESSAGES_H

#include <qobject.h>
#include <qstring.h>
#include <qtopiaabstractservice.h>
#include <qmap.h>
#include <qtopiachannel.h>
#include <qdatetime.h>

#ifdef MAIL_EXISTS
/*!
  \internal
  \class SysMail
  \brief Helper class for SysMessagesService

  The SysMail class encapsulates the information in a system sms message.
*/
class SysMail
{
public:
    SysMail(){}
    SysMail(const QDateTime &time, const QString& subject, const QString &text) :
        _time(time), _subject(subject),_text(text){}

    QDateTime time() { return _time; }
    void setTime( const QDateTime &time ) { this->_time = time; }

    QString subject() { return this->_subject; }
    void setSubject( const QString &subject) {this->_subject = subject; }

    QString text() { return this->_text; }
    void setText( const QString &text) { this->_text = text; }

private:
    QDateTime _time;
    QString _subject;
    QString _text;
};
#endif

class SysMessagesService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class SysMessages;

public:
    SysMessagesService( QObject *parent );
    ~SysMessagesService();

public slots:
    void showDialog( const QString &title, const QString &text );

#ifdef MAIL_EXISTS
    void sendMessage( const QString &subject, const QString &text );

private slots:
    void received(const QString &message, const QByteArray &data);

private:
    void passOnMessage( int messageId );
    void ackMessage( int messageId );
    void collectMessages();

    QtopiaChannel *sysMessagesChannel;
    QMap< int, SysMail > sysMailMap;
    int maxMessageId;
    bool keepRunning;
#endif
};
#endif //SYSMESSAGES_H

