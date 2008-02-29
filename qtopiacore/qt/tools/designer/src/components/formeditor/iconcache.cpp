/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "iconcache.h"

#include <resourcefile_p.h>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>

using namespace qdesigner_internal;

IconCache::IconCache(QObject *parent)
    : QDesignerIconCacheInterface(parent)
{
}

QIcon IconCache::nameToIcon(const QString &path, const QString &resourcePath)
{ return m_icon_cache.keyToItem(path, resourcePath); }

QString IconCache::iconToFilePath(const QIcon &pm) const
{ return m_icon_cache.itemToFilePath(pm); }

QString IconCache::iconToQrcPath(const QIcon &pm) const
{ return m_icon_cache.itemToQrcPath(pm); }

QPixmap IconCache::nameToPixmap(const QString &path, const QString &resourcePath)
{ return m_pixmap_cache.keyToItem(path, resourcePath); }

QString IconCache::pixmapToFilePath(const QPixmap &pm) const
{ return m_pixmap_cache.itemToFilePath(pm); }

QString IconCache::pixmapToQrcPath(const QPixmap &pm) const
{ return m_pixmap_cache.itemToQrcPath(pm); }

QList<QPixmap> IconCache::pixmapList() const
{ return m_pixmap_cache.itemList(); }

QList<QIcon> IconCache::iconList() const
{ return m_icon_cache.itemList(); }

QString IconCache::resolveQrcPath(const QString &filePath, const QString &qrcPath, const QString &wd) const
{
    QString workingDirectory = wd;
    if (workingDirectory.isEmpty()) {
        workingDirectory = QDir::currentPath();
    }

    QString icon_path = filePath;
    QString qrc_path = qrcPath;

    if (!qrc_path.isEmpty()) {
        qrc_path = QFileInfo(QDir(workingDirectory), qrcPath).absoluteFilePath();
        ResourceFile rf(qrc_path);
        if (rf.load())
            return rf.resolvePath(filePath);
    } else {
        return QFileInfo(QDir(workingDirectory), filePath).absoluteFilePath();
    }

    return QString();
}
