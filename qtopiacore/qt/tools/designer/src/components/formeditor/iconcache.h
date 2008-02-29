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

#ifndef ICONCACHE_H
#define ICONCACHE_H

#include "formeditor_global.h"

#include <QtDesigner/QtDesigner>

#include <resourcefile_p.h>

#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtCore/QPair>

#include <QtGui/QPixmap>
#include <QtGui/QIcon>

namespace qdesigner_internal {

/*
    We need two caches - one for icons and one for pixmaps - which are in all
    other respects identical.
*/
template <typename Item>
class ResourceCache
{
public:
    Item keyToItem(const QString &path, const QString &resourcePath = QString());
    QString itemToFilePath(const Item &item) const;
    QString itemToQrcPath(const Item &item) const;

    QList<Item> itemList() const;

private:
    typedef QPair<QString, QString> Key;
    typedef QMap<Key, Item> KeyToItemMap;
    typedef QMap<int, Key> SerialToKeyMap;

    KeyToItemMap m_key_to_item;
    SerialToKeyMap m_serial_to_key;
};

template <typename Item>
Item ResourceCache<Item>::keyToItem(const QString &filePath, const QString &qrcPath)
{
    Key key = qMakePair(filePath, qrcPath);
    typename KeyToItemMap::const_iterator it = m_key_to_item.constFind(key);
    if (it != m_key_to_item.constEnd())
        return *it;

    QString real_path;
    if (!qrcPath.isEmpty()) {
        ResourceFile rf(qrcPath);
        if (rf.load()) {
            real_path = rf.resolvePath(filePath);
        } else {
            qWarning("IconCache::nameToIcon(): failed to open \"%s\": %s",
                        qrcPath.toUtf8().constData(),
                        rf.errorMessage().toUtf8().constData());
        }
    } else {
       real_path = filePath;
    }

    if (real_path.isEmpty())
        return Item();

    Item item(real_path);
    if (item.isNull())
        return Item();
    m_key_to_item.insert(key, item);
    m_serial_to_key.insert(item.serialNumber(), key);

    return item;
}

template <typename Item>
QString ResourceCache<Item>::itemToFilePath(const Item &item) const
{
    typename SerialToKeyMap::const_iterator it = m_serial_to_key.find(item.serialNumber());
    if (it != m_serial_to_key.end())
        return (*it).first;
    return QString();
}

template <typename Item>
QString ResourceCache<Item>::itemToQrcPath(const Item &item) const
{
    typename SerialToKeyMap::const_iterator it = m_serial_to_key.find(item.serialNumber());
    if (it != m_serial_to_key.end())
        return (*it).second;
    return QString();
}

template <typename Item>
QList<Item> ResourceCache<Item>::itemList() const
{
    return m_key_to_item.values();
}

class QT_FORMEDITOR_EXPORT IconCache : public QDesignerIconCacheInterface
{
    Q_OBJECT
public:
    IconCache(QObject *parent);

    virtual QIcon nameToIcon(const QString &path, const QString &resourcePath = QString());
    virtual QString iconToFilePath(const QIcon &pm) const;
    virtual QString iconToQrcPath(const QIcon &pm) const;
    virtual QPixmap nameToPixmap(const QString &path, const QString &resourcePath = QString());
    virtual QString pixmapToFilePath(const QPixmap &pm) const;
    virtual QString pixmapToQrcPath(const QPixmap &pm) const;

    virtual QList<QPixmap> pixmapList() const;
    virtual QList<QIcon> iconList() const;

    virtual QString resolveQrcPath(const QString &filePath, const QString &qrcPath, const QString &workingDirectory = QString()) const;

private:
    ResourceCache<QIcon> m_icon_cache;
    ResourceCache<QPixmap> m_pixmap_cache;
};

}  // namespace qdesigner_internal

#endif // ICONCACHE_H
