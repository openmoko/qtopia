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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QLIBRARY_H
#define QLIBRARY_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

#ifndef QT_NO_LIBRARY

class QLibraryPrivate;

class Q_CORE_EXPORT QLibrary : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
    Q_PROPERTY(LoadHints loadHints READ loadHints WRITE setLoadHints)
    Q_FLAGS(LoadHint LoadHints)
public:
    enum LoadHint {
        ResolveAllSymbolsHint = 0x01,
        ExportExternalSymbolsHint = 0x02,
        LoadArchiveMemberHint = 0x04
    };
    Q_DECLARE_FLAGS(LoadHints, LoadHint)

    explicit QLibrary(QObject *parent = 0);
    explicit QLibrary(const QString& fileName, QObject *parent = 0);
    explicit QLibrary(const QString& fileName, int verNum, QObject *parent = 0);
    ~QLibrary();

    void *resolve(const char *symbol);
    static void *resolve(const QString &fileName, const char *symbol);
    static void *resolve(const QString &fileName, int verNum, const char *symbol);

    bool load();
    bool unload();
    bool isLoaded() const;

    static bool isLibrary(const QString &fileName);

    void setFileName(const QString &fileName);
    QString fileName() const;

    void setFileNameAndVersion(const QString &fileName, int verNum);
    QString errorString() const;

    void setLoadHints(LoadHints hints);
    LoadHints loadHints() const;
#ifdef QT3_SUPPORT
    inline QT3_SUPPORT QString library() const { return fileName(); }
    inline QT3_SUPPORT void setAutoUnload( bool ) {}
#endif
private:
    QLibraryPrivate *d;
    bool did_load;
    Q_DISABLE_COPY(QLibrary)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QLibrary::LoadHints)

#endif //QT_NO_LIBRARY

QT_END_HEADER

#endif //QLIBRARY_H
