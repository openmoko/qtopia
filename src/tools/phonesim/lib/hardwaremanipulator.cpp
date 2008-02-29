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

#include "hardwaremanipulator.h"
#include <Qt>
#include <qdebug.h>
#include <qbuffer.h>
#include <qtimer.h>
#include "../../../libraries/qtopiaphone/qsmsmessage.h"
#include "../../../libraries/qtopiaphone/qcbsmessage.h"
#include "../../../libraries/qtopiacomm/serial/qgsmcodec.h"
#include "../../../libraries/qtopiaphone/wap/qwsppdu.h"

#define NIBBLE_MAX 15
#define TWO_BYTE_MAX 65535
#define THREE_BYTE_MAX 16777215
#define PORT_MAX 999999;//arbitrary value
#define FOUR_CHAR 4
#define SIX_CHAR 6
#define ONE_CHAR 1
#define HEX_BASE 16

HardwareManipulator::HardwareManipulator(QObject *parent)
        : QObject(parent)
{
}


QSMSMessageList & HardwareManipulator::getSMSList()
{
    return SMSList;
}

void HardwareManipulator::warning( const QString &title, const QString &message)
{
    qWarning() << title << ":" << message;
}

void HardwareManipulator::setPhoneNumber( const QString& )
{
}

QString PS_toHex( const QByteArray& binary );

QString HardwareManipulator::constructCBMessage(const QString &messageCode, int geographicalScope, const QString &updateNumber,
    const QString &channel, const QString &/*scheme*/, int language, const QString &numPages, const QString &page, const QString &content)
{

    bool ok;
    uint mc = convertString(messageCode,1023,3,HEX_BASE, &ok);
    if ( !ok ) {
        warning(tr("Invalid Message Code"),
                tr("Message code 3 hex digits long and no larger than 3FF"));
        return "";
    }


    QCBSMessage::GeographicalScope gs = (QCBSMessage::GeographicalScope)geographicalScope;

    uint un = convertString(updateNumber,NIBBLE_MAX,ONE_CHAR,HEX_BASE,&ok);
    if ( !ok ) {
        warning(tr("Invalid Update Number"),
                tr("Update number must be 1 hex digit long"
                   "and no larger than F"));
        return "";
    }


    uint ch = convertString(channel, TWO_BYTE_MAX,FOUR_CHAR,HEX_BASE,&ok);
    if ( !ok ) {
        warning(tr("Invalid Channel,"),
                tr("Channel  must be 4 hex digits long "
                   "and no larger than FFFF"));
        return "";
    }

    //scheme is currently hardcoded to QSMS8_BitCodingScheme
    //uint sch = convertString(scheme, NIBBLE_MAX, ONE_CHAR,HEX_BASE,&ok);
    //if ( !ok )
    //    return "";

    QCBSMessage::Language lang = (QCBSMessage::Language)language;

    uint npag = convertString(numPages, NIBBLE_MAX,ONE_CHAR,HEX_BASE,&ok);
    if ( !ok ) {
        warning(tr("Invalid number of pages,"),
                tr("Number of pages  must be 1 hex digit long "
                   "and no larger than F"));
        return "";
    }

    uint pag = convertString(page, NIBBLE_MAX,ONE_CHAR,HEX_BASE,&ok);
    if ( !ok ) {
        warning(tr("Invalid page number,"),
                tr("Page number  must be 1 hex digit long "
                   "and no larger than F"));
        return "";
    }

    QCBSMessage m;
    m.setMessageCode(mc);
    m.setScope(gs);
    m.setUpdateNumber(un);
    m.setChannel(ch);
    m.setLanguage(lang);
    m.setNumPages(npag);
    m.setPage(pag);
    m.setText(content);

    return PS_toHex( m.toPdu() );
}

void HardwareManipulator::constructSMSMessage( const QString &sender, const QString &serviceCenter, const QString &text )
{
    QSMSMessage m;
    if ( sender.contains(QRegExp("\\D")) ) {
        warning(tr("Invalid Sender"),
                tr("Sender must not be empty and contain "
                   "only digits"));
        return;
    }
    m.setSender(sender);

    if ( serviceCenter.contains(QRegExp("\\D")) ) {
        warning(tr("Invalid Service Center"),
                tr("Service Center must not be empty and contain "
                   "only digits"));
        return;
    }
    m.setServiceCenter(serviceCenter);

    m.setText(text);
    m.setTimestamp(QDateTime::currentDateTime());

    SMSList.appendSMS( m.toPdu() );
}

void HardwareManipulator::constructSMSDatagram(int port, const QString &sender, const QByteArray &data,
                                               const QByteArray &contentType)
{
    QWspPush pdu;
    pdu.setIdentifier(0);
    pdu.setPduType(6);

    pdu.setData(data.data(),data.length());

    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QWspPduEncoder encoder(&buffer);

    if ( contentType.length() != 0 ) {
        pdu.addHeader("Content-Type", contentType);
        encoder.encodePush(pdu);
    } else {
        pdu.writeData(&buffer);
    }
    QByteArray appData = buffer.buffer();
    buffer.close();

    QSMSMessage m;
    m.setDestinationPort(port);
    m.setSender(sender);
    m.setApplicationData(appData);

    if( m.shouldSplit() ) {
        QList<QSMSMessage> list = m.split();

        for( int i =0; i < list.count(); i++ ) {
           SMSList.appendSMS( list[i].toPdu() );
        }
    } else {
        SMSList.appendSMS( m.toPdu() );
    }
}

int HardwareManipulator::convertString(const QString &number, int maxValue, int numChar, int base, bool *ok)
{
    bool b;
    int num = number.toInt(&b, base);

    *ok = true;
    if ( !b || num < 0 || num > maxValue || number.size() != numChar ) {
        *ok = false;
    }
    return num;
}

void HardwareManipulator::handleFromData( const QString& /*cmd*/ )
{
}

void HardwareManipulator::handleToData( const QString& /*cmd*/ )
{
}
