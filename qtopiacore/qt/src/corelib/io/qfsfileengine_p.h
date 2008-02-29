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

#ifndef QFSFILEENGINE_P_H
#define QFSFILEENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qplatformdefs.h"
#include "QtCore/qfsfileengine.h"
#include "private/qabstractfileengine_p.h"

class QFSFileEnginePrivate : public QAbstractFileEnginePrivate
{
    Q_DECLARE_PUBLIC(QFSFileEngine)

public:
#ifdef Q_WS_WIN
    static QString fixToQtSlashes(const QString &path);
    static QByteArray win95Name(const QString &path);
    static QString longFileName(const QString &path);
#else
    static inline QString fixToQtSlashes(const QString &path) { return path; }
#endif

    QString file;

    int fd;
    mutable uint sequential : 1;
    QByteArray ungetchBuffer;

    mutable uint could_stat : 1;
    mutable uint tried_stat : 1;
#ifdef Q_OS_UNIX
    mutable uint need_lstat : 1;
    mutable uint is_link : 1;
#endif
    mutable uint is_readonly : 1;
#ifdef Q_WS_WIN
    mutable DWORD fileAttrib;
#else
    mutable QT_STATBUF st;
#endif
    bool doStat() const;
    bool isSymlink() const;
#if defined (Q_WS_MAC)
    bool isMacHidden(const QString &path) const;
#endif

    int sysOpen(const QString &, int flags);

    FILE *fh;
    bool closeFileHandle;
    enum LastIOCommand
    {
        IOFlushCommand,
        IOReadCommand,
        IOWriteCommand
    };
    LastIOCommand  lastIOCommand;
    bool lastFlushFailed;

protected:
    QFSFileEnginePrivate();

    void init();

#if defined(Q_OS_WIN32)
    QAbstractFileEngine::FileFlags getPermissions() const;
    QString getLink() const;
#endif
};

#endif // QFSFILEENGINE_P_H
