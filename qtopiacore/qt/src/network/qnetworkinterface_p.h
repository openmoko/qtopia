/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtNetwork module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QNETWORKINTERFACEPRIVATE_H
#define QNETWORKINTERFACEPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the QLibrary class.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qatomic.h>
#include <QtCore/qlist.h>
#include <QtCore/qreadwritelock.h>
#include <QtCore/qstring.h>
#include <QtNetwork/qhostaddress.h>

class QNetworkAddressEntryPrivate
{
public:
    QHostAddress address;
    QHostAddress netmask;
    QHostAddress broadcast;
};

class QNetworkInterfacePrivate: public QSharedData
{
public:
    QNetworkInterfacePrivate() : index(0), flags(0)
    { }
    ~QNetworkInterfacePrivate()
    { }

    int index;                  // interface index, if know
    QNetworkInterface::InterfaceFlags flags;

    QString name;
    QString hardwareAddress;

    QList<QNetworkAddressEntry> addressEntries;

    static QString makeHwAddress(int len, uchar *data);

private:
    // disallow copying -- avoid detaching
    QNetworkInterfacePrivate &operator=(const QNetworkInterfacePrivate &other);
    QNetworkInterfacePrivate(const QNetworkInterfacePrivate &other);
};

class QNetworkInterfaceManager
{
public:
    QNetworkInterfaceManager();
    ~QNetworkInterfaceManager();

    QSharedDataPointer<QNetworkInterfacePrivate> interfaceFromName(const QString &name);
    QSharedDataPointer<QNetworkInterfacePrivate> interfaceFromIndex(int index);
    QList<QSharedDataPointer<QNetworkInterfacePrivate> > allInterfaces();

    // convenience:
    QSharedDataPointer<QNetworkInterfacePrivate> empty;

private:
    QList<QNetworkInterfacePrivate *> scan();
};


#endif
