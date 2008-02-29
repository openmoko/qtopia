/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
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
                library->release();
                if (qt_debug_component()) {
                    qDebug() << "         not a plugin";
                }
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
