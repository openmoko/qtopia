/****************************************************************************
** $Id: qt/src/kernel/qsnoopdata_qws.cpp   2.3.12   edited 2005-10-27 $
**
** QSNOOP data class
**
** Created : 20050808
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsnoopdata_qws_p.h"

#ifndef QT_NO_SNOOP
#include "qdatastream.h"

class QSnoopDataPrivate {
public:
    QString m_senderId;
    QString m_receiverId;
    QString m_channelName;
    QString m_messageType;
    QByteArray m_messageData;
};

QSnoopData::QSnoopData()
{
    d = new QSnoopDataPrivate;
}

QSnoopData::~QSnoopData()
{
    delete d;
}


void QSnoopData::setSenderId(const QString& id)
{
    d->m_senderId=id;
}

void QSnoopData::setReceiverId(const QString& id)
{
    d->m_receiverId=id;
}

void QSnoopData::setChannelName(const QString& name)
{
    d->m_channelName=name;
}

void QSnoopData::setMessageType(const QString& type)
{
    d->m_messageType=type;
}

void QSnoopData::setMessageData(const QByteArray& data)
{
    d->m_messageData=data;
}

QString QSnoopData::senderId() const 
{
    return d->m_senderId; 
}

QString QSnoopData::receiverId() const 
{
    return d->m_receiverId; 
}

QString QSnoopData::channelName() const 
{
    return d->m_channelName; 
}

QString QSnoopData::messageType() const 
{
    return d->m_messageType; 
}

QByteArray QSnoopData::messageData() const 
{ 
    return d->m_messageData; 
}

#ifndef QT_NO_DATASTREAM
QDataStream& operator<<( QDataStream& s, const QSnoopData& data)
{
    s << data.senderId();
    s << data.receiverId();
    s << data.channelName();
    s << data.messageType();
    s << data.messageData();
    return s;
}

QDataStream& operator>>( QDataStream &s, QSnoopData &data )
{
    QString senderId;
    QString receiverId;
    QString channelName;
    QString messageType;
    QByteArray messageData;

    s >> senderId;
    s >> receiverId;
    s >> channelName;
    s >> messageType;
    s >> messageData;

    data.setSenderId(senderId);
    data.setReceiverId(receiverId);
    data.setChannelName(channelName);
    data.setMessageType(messageType);
    data.setMessageData(messageData);

    return s;
}
#endif

#endif
