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

#ifndef __QTOPIASENDVIA_H__
#define __QTOPIASENDVIA_H__

class QByteArray;
class QString;
class QWidget;
class QContent;

#include <qtopiaglobal.h>

class QTOPIA_EXPORT QtopiaSendVia
{
public:
    static bool isDataSupported(const QString &metatype);
    static bool isFileSupported();

    static bool sendData(QWidget *parent, const QByteArray &data, const QString &mimetype);
    static bool sendFile(QWidget *parent, const QString &filename, const QString &mimetype,
                         const QString &description = QString(), bool autodelete = false);
    static bool sendFile(QWidget *parent, const QContent &content, bool autodelete = false);
};

#endif
