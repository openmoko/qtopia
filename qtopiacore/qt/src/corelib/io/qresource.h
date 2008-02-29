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

#ifndef QRESOURCE_H
#define QRESOURCE_H

#include <QtCore/qstring.h>
#include <QtCore/qlocale.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

class QResourcePrivate;

class Q_CORE_EXPORT QResource
{
public:
    QResource(const QString &file=QString(), const QLocale &locale=QLocale());
    ~QResource();

    void setFileName(const QString &file);
    QString fileName() const;
    QString absoluteFilePath() const;

    void setLocale(const QLocale &locale);
    QLocale locale() const;

    bool isValid() const;

    bool isCompressed() const;
    qint64 size() const;
    const uchar *data() const;

    static void addSearchPath(const QString &path);
    static QStringList searchPaths();

    static bool registerResource(const QString &rccFilename, const QString &resourceRoot=QString());
    static bool unregisterResource(const QString &rccFilename, const QString &resourceRoot=QString());

protected:
    friend class QResourceFileEngine;
    bool isDir() const;
    inline bool isFile() const { return !isDir(); }
    QStringList children() const;

protected:
    QResourcePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QResource)
};

QT_END_HEADER

#endif // QRESOURCE_H
