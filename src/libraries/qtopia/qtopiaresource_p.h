/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef QTOPIA_RESOURCE_H
#define QTOPIA_RESOURCE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qabstractfileengine.h>
#include <qtopiaglobal.h>
#include <QMap>

class QFileResourceFileEngineHandler : QAbstractFileEngineHandler
{
public:
    QFileResourceFileEngineHandler();
    virtual ~QFileResourceFileEngineHandler();

    void setIconPath(const QStringList& p);
    virtual QAbstractFileEngine *create(const QString &path) const;

    static int fontHeightToIconSize( const int fontHeight );

private:
    void appendSearchDirs(QList<QByteArray> &dirs, const QString& dir, const char *subdir) const;

#if QT_VERSION >= 0x040300
    QAbstractFileEngine *findArchivedResourceFile(const QString &path) const;
    QAbstractFileEngine *findArchivedImage(const QString &path) const;
    QAbstractFileEngine *findArchivedIcon(const QString &path) const;
    QString loadArchive(const QString &) const;
    mutable QMap<QString, QString> m_registeredArchives;
#endif

    QString findDiskResourceFile(const QString &path) const;
    QString findDiskSound(const QString &path) const;
    QString findDiskImage(const QString &path, const QString& _subdir) const;

    mutable QList<QByteArray> imagedirs;
    mutable QList<QByteArray> sounddirs;
    mutable QList<QByteArray> iconpath;
};

#endif
