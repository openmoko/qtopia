/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef __DBUSIPCCOMMON_P_H__
#define __DBUSIPCCOMMON_P_H__

#include <QString>

static const QString dbusPathBase("/com/trolltech/qtopia/");
static const QString dbusInterface("com.trolltech.qtopia");

void convert_dbus_path_to_qcop_channel(const QString &path,
                                       QString &channel);

void convert_qcop_channel_to_dbus_path(const QString &channel,
                                       QString &dbusPath);

void convert_dbus_to_qcop_message_name(const QString &dbusMsg,
                                       QString &msg);

void convert_qcop_message_name_to_dbus(const QString &msg,
                                       QString &dbusMsg);

#endif
