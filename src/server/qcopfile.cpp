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

#include "qcopfile.h"
#include <Qtopia>
#include <QString>
#include <QFile>
#include <unistd.h>
#include <sys/file.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <QDataStream>

bool QCopFile::writeQCopMessage(const QString& app,
                                const QString& msg,
                                const QByteArray& data)
{
    QString qcopfn( app );
    // if the appname is a path with slashes, convert to underscores
    // note: this assumes that the PATH variable is not used to launch the binary
    // so that argv[0] contains the full path as well.  Otherwise the similar
    // code in QtopiaApplication will not have the app name.
    qcopfn.replace( QDir::separator(), "_" );
    qcopfn.prepend( Qtopia::tempDir() + "qcop-msg-" );
    QFile qcopfile(qcopfn);

    if(qcopfile.open(QIODevice::WriteOnly | QIODevice::Append)) {
#ifdef QTOPIA_POSIX_LOCKS
        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        fl.l_pid = getpid();
        if (fcntl(qcopfile.handle(), F_SETLKW, &fl) == -1) {
            /* some error occurred */
            qWarning(QString("Failed to obtain file lock on %1 (%2)")
                    .arg(qcopfn).arg( errno ).toAscii().constData());
        }
#else
        if(flock(qcopfile.handle(), LOCK_EX)) {
            /* some error occurred */
            qWarning(QString("Failed to obtain file lock on %1 (%2)")
                    .arg(qcopfn).arg( errno ).toAscii().constData());
        }
#endif
        {
            QDataStream ds(&qcopfile);
            ds << QString("QPE/Application/") + app << msg << data;
            qcopfile.flush();
#ifdef QTOPIA_POSIX_LOCKS
            fl.l_type = F_UNLCK;
            fcntl(qcopfile.handle(), F_SETLK, &fl);
#else
            flock(qcopfile.handle(), LOCK_UN);
#endif
            qcopfile.close();
        }

        return true;
    } else {
        qWarning(QString("Failed to open file %1")
                .arg(qcopfn).toAscii().constData());
        return false;
    }
}

