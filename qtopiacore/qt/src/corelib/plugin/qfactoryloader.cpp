/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.1, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qfactoryloader_p.h"

#ifndef QT_NO_LIBRARY
#include "qfactoryinterface.h"
#include "qmap.h"
#include <qdir.h>
#include <qsettings.h>
#include <qdebug.h>
#include "qmutex.h"
#include "qplugin.h"
#include "qpluginloader.h"
#include "private/qobject_p.h"
#include "private/qcoreapplication_p.h"

class QFactoryLoaderPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QFactoryLoader)
public:
    QFactoryLoaderPrivate(){}
    mutable QMutex mutex;
    QByteArray iid;
    QList<QLibraryPrivate*> libraryList;
    QMap<QString,QLibraryPrivate*> keyMap;
    QStringList keyList;
};

QFactoryLoader::QFactoryLoader(const char *iid,
                               const QStringList &paths, const QString &suffix,
                               Qt::CaseSensitivity cs)
    : QObject(*new QFactoryLoaderPrivate)
{
    moveToThread(QCoreApplicationPrivate::mainThread());
    Q_D(QFactoryLoader);
    d->iid = iid;

#ifdef QT_SHARED
    QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));

    for (int i = 0; i < paths.count(); ++i) {
        QString path = paths.at(i) + suffix;
        if (!QDir(path).exists(QLatin1String(".")))
            continue;
        QStringList plugins = QDir(path).entryList(QDir::Files);
        QLibraryPrivate *library = 0;
        for (int j = 0; j < plugins.count(); ++j) {
            QString fileName = QDir::cleanPath(path + QLatin1Char('/') + plugins.at(j));
            if (qt_debug_component()) {
                qDebug() << "QFactoryLoader::QFactoryLoader() looking at" << fileName;
            }
            library = QLibraryPrivate::findOrCreate(QFileInfo(fileName).canonicalFilePath());
            if (!library->isPlugin()) {
                if (qt_debug_component()) {
                    qDebug() << library->errorString;
                    qDebug() << "         not a plugin";
                }
                library->release();
                continue;
            }
            QString regkey = QString::fromLatin1("Qt Factory Cache %1.%2/%3:/%4")
                             .arg((QT_VERSION & 0xff0000) >> 16)
                             .arg((QT_VERSION & 0xff00) >> 8)
                             .arg(QLatin1String(iid))
                             .arg(fileName);
            QStringList reg, keys;
            reg = settings.value(regkey).toStringList();
            if (reg.count() && library->lastModified == reg[0]) {
                keys = reg;
                keys.removeFirst();
            } else {
                if (!library->loadPlugin()) {
                    if (qt_debug_component()) {
                        qDebug() << library->errorString;
                        qDebug() << "           could not load";
                    }
                    library->release();
                    continue;
                }
                QObject *instance = library->instance();
                if (!instance)
                    // ignore plugins that have a valid signature but cannot be loaded.
                    continue;
                QFactoryInterface *factory = qobject_cast<QFactoryInterface*>(instance);
                if (instance && factory && instance->qt_metacast(iid))
                    keys = factory->keys();
                if (keys.isEmpty())
                    library->unload();
                reg.clear();
                reg << library->lastModified;
                reg += keys;
                settings.setValue(regkey, reg);
            }
            if (qt_debug_component()) {
                qDebug() << "keys" << keys;
            }

            if (keys.isEmpty()) {
                library->release();
                continue;
            }
            d->libraryList += library;
            for (int k = 0; k < keys.count(); ++k) {
                // first come first serve, unless the first
                // library was built with a future Qt version,
                // whereas the new one has a Qt version that fits
                // better
                QString key = keys.at(k);
                if (!cs)
                    key = key.toLower();
                QLibraryPrivate *previous = d->keyMap.value(key);
                if (!previous || (previous->qt_version > QT_VERSION && library->qt_version <= QT_VERSION)) {
                    d->keyMap[key] = library;
                    d->keyList += keys.at(k);
                }
            }
        }
    }
#else
    Q_UNUSED(paths);
    Q_UNUSED(suffix);
    Q_UNUSED(cs);
    if (qt_debug_component()) {
        qDebug() << "QFactoryLoader::QFactoryLoader() ignoring" << d->iid
                 << "since plugins are disabled in static builds";
    }
#endif
}

QFactoryLoader::~QFactoryLoader()
{
    Q_D(QFactoryLoader);
    for (int i = 0; i < d->libraryList.count(); ++i)
        d->libraryList.at(i)->release();
}

QStringList QFactoryLoader::keys() const
{
    Q_D(const QFactoryLoader);
    QMutexLocker locker(&d->mutex);
    QStringList keys = d->keyList;
    QObjectList instances = QPluginLoader::staticInstances();
    for (int i = 0; i < instances.count(); ++i)
        if (QFactoryInterface *factory = qobject_cast<QFactoryInterface*>(instances.at(i)))
            if (instances.at(i)->qt_metacast(d->iid))
                keys += factory->keys();
    return keys;
}

QObject *QFactoryLoader::instance(const QString &key) const
{
    Q_D(const QFactoryLoader);
    QMutexLocker locker(&d->mutex);
    QObjectList instances = QPluginLoader::staticInstances();
    for (int i = 0; i < instances.count(); ++i)
        if (QFactoryInterface *factory = qobject_cast<QFactoryInterface*>(instances.at(i)))
            if (instances.at(i)->qt_metacast(d->iid) && factory->keys().contains(key, Qt::CaseInsensitive))
                return instances.at(i);

    if (QLibraryPrivate* library = d->keyMap.value(key)) {
        if (library->instance || library->loadPlugin()) {
            if (QObject *obj = library->instance()) {
                if (obj && !obj->parent())
                    obj->moveToThread(QCoreApplicationPrivate::mainThread());
                return obj;
            }
        }
    }
    return 0;
}
#endif // QT_NO_LIBRARY
