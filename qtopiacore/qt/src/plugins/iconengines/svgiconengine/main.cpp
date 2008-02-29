/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qiconengineplugin.h>
#include <qstringlist.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#include "qsvgiconengine.h"

#include <qiodevice.h>
#include <qbytearray.h>
#include <qdebug.h>

class QSvgIconPlugin : public QIconEnginePlugin
{
public:
    QStringList keys() const;
    QIconEngine *create(const QString &filename);
};

QStringList QSvgIconPlugin::keys() const
{
    return QStringList() << "svg";
}

QIconEngine *QSvgIconPlugin::create(const QString &file)
{
    QSvgIconEngine *engine = new QSvgIconEngine();
    engine->addFile(file, QSize(), QIcon::Normal, QIcon::On);
    return engine;
}

Q_EXPORT_STATIC_PLUGIN(QSvgIconPlugin)
Q_EXPORT_PLUGIN2(qsvg, QSvgIconPlugin)

#endif // !QT_NO_IMAGEFORMATPLUGIN
