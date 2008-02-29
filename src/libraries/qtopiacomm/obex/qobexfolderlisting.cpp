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

#include <QString>
#include <QDebug>
#include <QXmlAttributes>

#include <QDateTime>

#include <private/qobexfolderlisting_p.h>

#undef QOBEXFOLDERLISTING_DEBUG

static const QString LOCAL_TIME_FORMAT = "yyyyMMddThhmmss";
static const QString UTC_TIME_FORMAT = LOCAL_TIME_FORMAT + 'Z';

#ifdef QOBEXFOLDERLISTING_DEBUG
QDebug& operator<<(QDebug debug, const QXmlAttributes &attributes)
{
    for (int i = 0; i < attributes.count(); i++) {
        qDebug() << attributes.value(i);
    }

    return debug;
}

QDebug& operator<<(QDebug debug, const QObexFolderListingEntryInfo &info)
{
    debug << "FolderListing(" << info.name() << info.size() << info.lastModified() <<
            info.timeCreated() << info.lastAccessed() << info.isFolder() <<
            info.isFile() << info.isParent() << info.permissions() <<
            info.owner() << info.group() << info.mimetype() << info.description() <<
            info.extensionAttributes();

    return debug;
}
#endif

QObexFolderListingHandler::QObexFolderListingHandler()
{
    m_valid_elem = false;
}

inline QDateTime parseTime(const QString &time)
{
    if (time.isEmpty())
        return QDateTime();

    return QDateTime::fromString(time,
                                 ((time[time.size()-1] == 'Z') ?
                                         UTC_TIME_FORMAT : LOCAL_TIME_FORMAT));
}

inline QObexFolderListingEntryInfo::Permissions parsePermissions(const QString &perm,
        QObexFolderListingEntryInfo::Permission read,
        QObexFolderListingEntryInfo::Permission write,
        QObexFolderListingEntryInfo::Permission remove)
{
    QObexFolderListingEntryInfo::Permissions permissions;

    for (int i = 0; i < perm.length(); i++) {
        if ((perm[i] == 'R') || (perm[i] == 'r'))
            permissions |= read;
        if ((perm[i] == 'W') || (perm[i] == 'w'))
            permissions |= write;
        if ((perm[i] == 'D') || (perm[i] == 'd'))
            permissions |= remove;
    }

    return permissions;
}

bool QObexFolderListingHandler::startElement(const QString &,
                                             const QString &,
                                             const QString &qName,
                                             const QXmlAttributes &attributes)
{
    if ((qName == "folder") || (qName == "file")) {
        QString name;
        qint64 size = 0;
        QDateTime modified;
        QDateTime accessed;
        QDateTime created;
        QString owner;
        QString group;
        QObexFolderListingEntryInfo::Permissions permissions = 0;
        QString mimetype;
        QMap<QString,QString> extended;

        for (int i = 0; i < attributes.count(); i++) {
            if (attributes.qName(i) == QLatin1String("name")) {
                name = attributes.value(i);
            }
            else if (attributes.qName(i) == QLatin1String("size")) {
                bool ok = false;
                size = attributes.value(i).toLongLong(&ok, 10);
                if (!ok) {
                    size = 0;
                }
            }
            else if (attributes.qName(i) == QLatin1String("modified")) {
                modified = parseTime(attributes.value(i));
            }
            else if (attributes.qName(i) == QLatin1String("created")) {
                created = parseTime(attributes.value(i));
            }
            else if (attributes.qName(i) == QLatin1String("accessed")) {
                accessed = parseTime(attributes.value(i));
            }
            else if (attributes.qName(i) == QLatin1String("user-perm")) {
                permissions |= parsePermissions(attributes.value(i),
                        QObexFolderListingEntryInfo::ReadOwner,
                        QObexFolderListingEntryInfo::WriteOwner,
                        QObexFolderListingEntryInfo::DeleteOwner);
            }
            else if (attributes.qName(i) == QLatin1String("group-perm")) {
                permissions |= parsePermissions(attributes.value(i),
                        QObexFolderListingEntryInfo::ReadGroup,
                        QObexFolderListingEntryInfo::WriteGroup,
                        QObexFolderListingEntryInfo::DeleteGroup);
            }
            else if (attributes.qName(i) == QLatin1String("other-perm")) {
                permissions |= parsePermissions(attributes.value(i),
                        QObexFolderListingEntryInfo::ReadOther,
                        QObexFolderListingEntryInfo::WriteOther,
                        QObexFolderListingEntryInfo::DeleteOther);
            }
            else if (attributes.qName(i) == QLatin1String("group")) {
                group = attributes.value(i);
            }
            else if (attributes.qName(i) == QLatin1String("owner")) {
                owner = attributes.value(i);
            }
            else if (attributes.qName(i) == QLatin1String("type")) {
                mimetype = attributes.value(i);
            }
            else {
                extended.insert(attributes.qName(i), attributes.value(i));
            }
        }

        if (qName == "folder")
            m_info = QObexFolderListingEntryInfo::createFolder(name, size, modified,
                    accessed, created, permissions,
                    owner, group, QString(), extended);
        else
            m_info = QObexFolderListingEntryInfo::createFile(name, size, modified,
                    accessed, created, permissions,
                    owner, group, mimetype, QString(), extended);

        m_valid_elem = true;

        return true;
    }

    if (qName == "parent-folder") {
        m_info = QObexFolderListingEntryInfo::createParent();
        return true;
    }

    if (qName == "folder-listing") {
        return true;
    }

    // Just ignore unknown elements

    return true;
}

bool QObexFolderListingHandler::endElement(const QString &,
                                           const QString &,
                                           const QString &qName)
{
    if (qName == "file" || qName == "folder" || qName == "parent-folder") {
#ifdef QOBEXFOLDERLISTING_DEBUG
        qDebug() << "InfoIs:" << m_info;
#endif
        m_valid_elem = false;
        emit info(m_info);
        return true;
    }

    if (qName == "folder-listing")
        return true;

    // Just ignore unknown elements

    return true;
}

bool QObexFolderListingHandler::characters(const QString &ch)
{
    if (m_valid_elem)
        m_info.setDescription(ch.trimmed());

    return true;
}

bool QObexFolderListingHandler::fatalError(const QXmlParseException &/*err*/)
{
#ifdef QOBEXFOLDERLISTING_DEBUG
    qDebug() << "Line:" << err.lineNumber() << "Column: " << err.columnNumber() <<
            err.message();
#endif

    return false;
}

bool QObexFolderListingHandler::error(const QXmlParseException &/*err*/)
{
#ifdef QOBEXFOLDERLISTING_DEBUG
    qDebug() << "Line:" << err.lineNumber() << "Column: " << err.columnNumber() <<
            err.message();
#endif

    return false;
}

bool QObexFolderListingHandler::warning(const QXmlParseException &/*err*/)
{
#ifdef QOBEXFOLDERLISTING_DEBUG
    qDebug() << "Line:" << err.lineNumber() << "Column: " << err.columnNumber() <<
            err.message();
#endif

    return false;
}

