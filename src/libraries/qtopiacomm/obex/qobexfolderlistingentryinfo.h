/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QOBEXFOLDERLISTINGENTRYINFO_H__
#define __QOBEXFOLDERLISTINGENTRYINFO_H__

#include <qglobal.h>
#include <qobexglobal.h>
#include <QString>
#include <QMap>

class QObexFolderListingEntryInfoPrivate;
class QDateTime;

typedef QMap<QString,QString> ExtensionAttributes;

class QTOPIAOBEX_EXPORT QObexFolderListingEntryInfo
{
public:
    enum Permission {
        ReadOwner = 0400,
        WriteOwner = 0200,
        DeleteOwner = 0100,
        ReadGroup = 0040,
        WriteGroup = 0020,
        DeleteGroup = 0010,
        ReadOther = 0004,
        WriteOther = 0002,
        DeleteOther = 0001
    };

    Q_DECLARE_FLAGS(Permissions, Permission)

    QObexFolderListingEntryInfo();
    ~QObexFolderListingEntryInfo();
    QObexFolderListingEntryInfo(const QObexFolderListingEntryInfo &other);

    QObexFolderListingEntryInfo &operator=(const QObexFolderListingEntryInfo &other);

    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    qint64 size() const;
    void setSize(qint64 size);

    QDateTime lastModified() const;
    void setLastModified(const QDateTime &lastModified);

    QDateTime created() const;
    void setCreated(const QDateTime &timeCreated);

    QDateTime lastRead() const;
    void setLastRead(const QDateTime &lastRead);

    bool isFolder() const;
    void setFolder(bool b);

    bool isFile() const;
    void setFile(bool b);

    bool isParent() const;
    void setParent(bool b);

    void setPermissions(Permissions p);
    Permissions permissions() const;

    QString owner() const;
    void setOwner(const QString &owner);

    QString group() const;
    void setGroup(const QString &group);

    QString type() const;
    void setType(const QString &mimetype);

    QString description() const;
    void setDescription(const QString &description);

    QMap<QString,QString> extensionAttributes() const;
    void setExtensionAttributes(const QMap<QString,QString> &attributes);

    bool operator==(const QObexFolderListingEntryInfo &info) const;
    inline bool operator!=(const QObexFolderListingEntryInfo &info) const
    { return !operator==(info); }

    static QObexFolderListingEntryInfo
            createFolder(const QString &name, qint64 size,
                         const QDateTime &lastModified, const QDateTime &lastRead,
                         const QDateTime &timeCreated,
                         QObexFolderListingEntryInfo::Permissions permissions,
                         const QString &owner, const QString &group,
                         const QString &description = QString(),
                         const ExtensionAttributes &extensionAttributes =
                                 ExtensionAttributes());

    static QObexFolderListingEntryInfo createParent();

    static QObexFolderListingEntryInfo
            createFile(const QString &name, qint64 size,
                       const QDateTime &lastModified, const QDateTime &lastRead,
                       const QDateTime &timeCreated,
                       QObexFolderListingEntryInfo::Permissions permissions,
                       const QString &owner, const QString &group,
                       const QString &mimetype, const QString &description = QString(),
                       const ExtensionAttributes &extensionAttributes =
                               ExtensionAttributes());

private:
    QObexFolderListingEntryInfoPrivate *m_data;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QObexFolderListingEntryInfo::Permissions)

#endif
