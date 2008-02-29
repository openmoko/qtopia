/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the plugins of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qimageiohandler.h>
#include <qstringlist.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEFORMAT_MNG
#undef QT_NO_IMAGEFORMAT_MNG
#endif
#include "qmnghandler.h"

#include <qiodevice.h>
#include <qbytearray.h>

class QMngPlugin : public QImageIOPlugin
{
    public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList QMngPlugin::keys() const
{
    return QStringList() << "mng";
}

QImageIOPlugin::Capabilities QMngPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "mng")
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && QMngHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *QMngPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QMngHandler *hand = new QMngHandler();
    hand->setDevice(device);
    hand->setFormat(format);
    return hand;
}

Q_EXPORT_STATIC_PLUGIN(QMngPlugin)
Q_EXPORT_PLUGIN2(qmng, QMngPlugin)

#endif // !QT_NO_IMAGEFORMATPLUGIN
