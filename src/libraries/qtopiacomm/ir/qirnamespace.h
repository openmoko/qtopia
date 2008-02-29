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

#ifndef __QIRNAMESPACE_H__
#define __QIRNAMESPACE_H__

#include <QFlags>

#ifndef Q_QDOC
namespace QIr
{
#else
class QIr
{
public:
#endif
    enum DeviceClass {
        PlugNPlay = 0x0001,
        PDA = 0x0002,
        Computer = 0x0004,
        Printer = 0x0008,
        Modem = 0x0010,
        Fax = 0x0020,
        LAN = 0x0040,
        Telephony = 0x0080,
        FileServer = 0x0100,
        Communications = 0x0200,
        Message = 0x0400,
        HTTP = 0x0800,
        OBEX = 0x1000,
        All = 0xffff
    };

    Q_DECLARE_FLAGS(DeviceClasses, DeviceClass)

    QByteArray convert_charset_to_string(int charset);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QIr::DeviceClasses)

#endif
