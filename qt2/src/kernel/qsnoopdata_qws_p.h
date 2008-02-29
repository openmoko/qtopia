/****************************************************************************
** $Id: qt/src/kernel/qsnoopdata_qws_p.h   2.3.12   edited 2005-10-27 $
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

#ifndef QSNOOP_H
#define QSNOOP_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H


#ifndef QT_NO_SNOOP

class QDataStream;
class QSnoopDataPrivate;

class Q_EXPORT QSnoopData 
{
public:
    QSnoopData();
    ~QSnoopData();

    void setSenderId(const QString&);
    void setReceiverId(const QString&);
    void setChannelName(const QString&);
    void setMessageType(const QString&);
    void setMessageData(const QByteArray&);

    QString senderId() const; 
    QString receiverId() const; 
    QString channelName() const; 
    QString messageType() const; 
    QByteArray messageData() const; 

private:
    QSnoopDataPrivate* d;

};

QDataStream& operator<<( QDataStream&, const QSnoopData& );
QDataStream& operator>>( QDataStream&, QSnoopData& );

#endif

#endif
