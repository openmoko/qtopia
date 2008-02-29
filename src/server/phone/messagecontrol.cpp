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

#include "messagecontrol.h"
#include <qcommservicemanager.h>
#include <QString>
#include <QSettings>
#include <QByteArray>
#include <QDataStream>

/*! \internal */
MessageControl::MessageControl() :
    phoneValueSpace("/Communications/Messages"),
    smsMemoryFull("/Telephony/Status/SMSMemoryFull"),
    smsreq(0), channel("QPE/System"),
    smsCount(0), mmsCount(0), systemCount(0),
    smsIsFull(false), prevSmsMemoryFull(0)
{
    mgr = new QCommServiceManager( this );
    connect( mgr, SIGNAL(servicesChanged()), this,
             SLOT(telephonyServicesChanged()) );
    telephonyServicesChanged(); // Check to see if we already have SMS.

    connect(&channel, SIGNAL(received(const QString&,const QByteArray&)),
            this, SLOT(sysMessage(const QString&,const QByteArray&)) );

    connect(&smsMemoryFull, SIGNAL(contentsChanged()),
            this, SLOT(smsMemoryFullChanged()) );
    prevSmsMemoryFull = smsMemoryFull.value().toInt();

    QSettings setting("Trolltech", "qpe");
    setting.beginGroup("Messages");
    smsCount = setting.value("MissedSMSMessages", 0).toInt();
    mmsCount = setting.value("MissedMMSMessages", 0).toInt();
    systemCount = setting.value("MissedSystemMessages", 0).toInt();

    doNewCount(false);
}

void MessageControl::doNewCount(bool write, bool fromSystem, bool notify)
{
    if(write) {
        QSettings setting("Trolltech", "qpe");
        setting.beginGroup( "Messages" );
        setting.setValue( "MissedSMSMessages", smsCount);
        setting.setValue( "MissedMMSMessages", mmsCount);
        setting.setValue( "MissedSystemMessages", systemCount );
    }

    phoneValueSpace.setAttribute("NewMessages", QVariant(messageCount()));
    if ( !fromSystem )
        emit messageCount(messageCount(), smsFull(), false, notify);
    else
        emit messageCount(messageCount(), false, true, notify);
}

/*! Returns the MessageControl instance. */
MessageControl *MessageControl::instance()
{
    static MessageControl *mc = 0;
    if(!mc)
        mc = new MessageControl;
    return mc;
}

void MessageControl::smsUnreadCountChanged()
{
    // Report on the number of unread SMS messages.
    int c = (smsreq ? smsreq->unreadCount() : 0);
    smsIsFull = ( smsreq && smsreq->usedMessages() != -1 ?
        ( smsreq->usedMessages() >= smsreq->totalMessages() ) : false );
    if (c != smsCount) {
        smsCount = c;
        doNewCount();
    }
}

void MessageControl::telephonyServicesChanged()
{
    // Create the SMS request object if the service has started.
    if ( !smsreq ) {
        if ( mgr->supports<QSMSReader>().size() > 0 ) {
            smsreq = new QSMSReader( QString(), this );
            connect( smsreq, SIGNAL(unreadCountChanged()),
                     this, SLOT(smsUnreadCountChanged()) );
            smsreq->setUnreadCount(smsCount);
        }
    }
}

/*! Returns the number of unread messages */
int MessageControl::messageCount() const
{
    return smsCount + mmsCount + systemCount;
}

/*!
  Returns true if the SMS memory is full.
 */
bool MessageControl::smsFull() const
{
    return smsIsFull;
}

void MessageControl::sysMessage(const QString& message, const QByteArray &data)
{
    QDataStream stream( data );

    if ( message == "newMmsCount(int)") {
         int count;
         stream >> count;
         if (count != mmsCount) {
             mmsCount = count;
             doNewCount(true,false,false);
         }
    }
    else if( message == "newSystemCount(int)" ){
        int count;
        stream >> count;
        if( count != systemCount ){
            systemCount = count;
            doNewCount( true, true, true);
        }
    }
}

void MessageControl::smsMemoryFullChanged()
{
    // Check for the "message rejected state", so we can report it.
    // The "full" state will be reported via messageCount() when new
    // message arrivals are detected.
    int fullState = smsMemoryFull.value().toInt();
    if ( fullState != prevSmsMemoryFull ) {
        prevSmsMemoryFull = fullState;
        if ( fullState == 2 )
            emit messageRejected();
    }
}
