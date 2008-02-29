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

#include "cellbroadcastcontrol.h"
#include <QSettings>
#include <QString>
#include <QCellBroadcast>
#include <QTimer>

/*!
  \class CellBroadcastControl
  \ingroup QtopiaServer
  \brief The CellBroadcastControl class monitors incoming cell broadcast messages.
  
  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
 */

/*!
  \enum CellBroadcastControl::Type

  Represents the type of a received cell broadcast message.

  \value Popup A popup message.  The text should be shown to the user immediately.
  \value Background A background message.  The text should be shown on the homescreen.
 */

/*!
  Constructs a new CellBroadcastControl instance with the specified \a parent.
  */
CellBroadcastControl::CellBroadcastControl(QObject *parent)
: QObject(parent)
{
    cb = new QCellBroadcast(QString(), this);
    QObject::connect(cb, SIGNAL(broadcast(QCBSMessage)),
                    this, SLOT(cellBroadcast(QCBSMessage)));

    firstSubscribe = false;

    CellModemManager *cellModem = qtopiaTask<CellModemManager>();
    connect(cellModem,
            SIGNAL(registrationStateChanged(QTelephony::RegistrationState)),
            this,
            SLOT(registrationChanged(QTelephony::RegistrationState)));
}

/*!
  Return the single instance of CellBroadcastControl.
*/
CellBroadcastControl *CellBroadcastControl::instance()
{
    static CellBroadcastControl *inst = 0;
    if ( !inst )
        inst = new CellBroadcastControl();
    return inst;
}

/*!
  \fn void CellBroadcastControl::broadcast(CellBroadcastControl::Type type, const QString &channel, const QString &text)

  Emitted whenever a new cell broadcast message of the given \a type is
  received.  \a channel indicates the channel on which the message was received
  and \a text its text.
 */

/*
    cell broadcast message has arrived.
    process the message according to the user preferences
*/
void CellBroadcastControl::cellBroadcast(const QCBSMessage &m)
{
    // check if any channel is registered
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CellBroadcast" );
    int count = cfg.value( "count",0 ).toInt();
    if ( count == 0 )
        return;

    // check if the registred channel is currently activated
    uint channel = m.channel();
    bool active = false;
    int i = 0;
    for ( ; i < count ;i++ ) {
        if ( channel == (uint)cfg.value( "num" + QString::number(i) ).toInt()
            && cfg.value( "on" + QString::number(i) ).toBool() ) {
            active = true;
            break;
        }
    }
    if ( !active )
        return;

    // check if transmitted languages matches user's preferences
    QCBSMessage::Language lang = m.language();
    QStringList lstLang = cfg.value( "languages"+QString::number(i)).toString().split(',', QString::SkipEmptyParts );
    if ( !lstLang.contains( QString::number( lang ) ) )
        return;

    // check which display mode user prefers
    int mode = cfg.value("mode"+QString::number(i)).toInt();
    QString label = cfg.value("label"+QString::number(i)).toString();
    // mode 0 = Background - Home screen
    // mode 1 = Foreground - Popup message

    QString channelText = QString("%1 %2").arg(label).arg(channel);
    emit broadcast((mode == 0)?Background:Popup, channelText, m.text());
}

void CellBroadcastControl::registrationChanged(QTelephony::RegistrationState state)
{
    switch ( state ) {
    case QTelephony::RegistrationHome:
    case QTelephony::RegistrationUnknown:
    case QTelephony::RegistrationRoaming:
        // If this is the first time we've seen registration, then
        // subscribe with the settings in Phone.conf.  Further changes
        // will be done by the "phonesettings" application.
        if ( !firstSubscribe ) {
            firstSubscribe = true;
            subscribe();
        }
        break;
    default: break;
    }
}

void CellBroadcastControl::subscribe()
{
    QList<int> list;

    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CellBroadcast" );
    int count = cfg.value( "count", 0 ).toInt();
    if ( count > 0 ) {
        for ( int i = 0; i < count; ++i ) {
            if ( cfg.value( "on" + QString::number( i ) ).toBool() ) // active
                list << cfg.value( "num" + QString::number( i ) ).toInt();
        }
    }
    if ( !list.contains( 50 ) )
        list << 50; // location channel, default
    cb->setChannels( list );
}
