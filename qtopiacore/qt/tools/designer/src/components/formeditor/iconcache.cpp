/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
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
** Trolltech ASA (c) 2007
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
