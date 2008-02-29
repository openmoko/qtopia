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

#include "qplatformdefs.h"
#include "qabstractfileengine.h"
#include "private/qfsfileengine_p.h"
#ifndef QT_NO_REGEXP
# include "qregexp.h"
#endif
#include "qfile.h"
#include "qdir.h"
#include "qdatetime.h"
#include "qdebug.h"
#include "qvarlengtharray.h"

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#if !defined(QWS) && defined(Q_OS_MAC)
# include <private/qcore_mac_p.h>
#endif

void QFSFileEnginePrivate::init()
{
}

int QFSFileEnginePrivate::sysOpen(const QString &fileName, int flags)
{
    return QT_OPEN(QFile::encodeName(fileName), flags, 0666);
}

bool QFSFileEngine::remove()
{
    Q_D(QFSFileEngine);
    return unlink(QFile::encodeName(d->file)) == 0;
}

/*!
  \internal
  Implement unix byte copy.  Attempt to copy file to the same file-system
  as the \a dst, as a temporary file.  If this fails error out.

  Once temporary file has been created, a rename of the temporary file to
  the \a dst is attempted - since the file is on the same file-system this
  is an atomic operation and there is no point at which "neither or both"
  exists.
*/
bool QFSFileEngine::copy(const QString &dst)
{
    Q_D(QFSFileEngine);
    char block[1024];
    bool error = false;
    qint64 in = 0;
    if ( open( QIODevice::ReadOnly ))
    {
        QByteArray tempfile = dst.toLocal8Bit();
        tempfile.append( ".temp_XXXXXX" );
        int fd = ::mkstemp( tempfile.data() );
        if ( fd != -1 ) {
            while (( in = read( block, 1024 ))) {
                if ( in == -1 || in != ::write( fd, block, in )) {
                    error = true;
                    break;
                }
            }
            if ( ::close( fd ) == 0 ) {
                if ( ::rename( tempfile, qPrintable( dst )) == -1 ) {
                    qWarning( strerror( errno ));
                    error = true;
                    ::unlink( tempfile );
                }
            } else {
                qWarning( strerror( errno ));
                error = true;
            }
        }
    } else {
        qWarning( strerror( errno ));
        error = true;
    }
    if ( error )
        qWarning( "Could not copy %s to %s", qPrintable( d->file ), qPrintable( dst ));
    return true;
}

bool QFSFileEngine::rename(const QString &newName)
{
    Q_D(QFSFileEngine);
    return ::rename(QFile::encodeName(d->file), QFile::encodeName(newName)) == 0;
}

bool QFSFileEngine::link(const QString &newName)
{
    Q_D(QFSFileEngine);
    return ::symlink(QFile::encodeName(d->file), QFile::encodeName(newName)) == 0;
}

qint64 QFSFileEngine::size() const
{
    Q_D(const QFSFileEngine);
    QT_STATBUF st;
    int ret = 0;
    const_cast<QFSFileEngine *>(this)->flush();
    if(d->fd != -1)
        ret = QT_FSTAT(d->fd, &st);
    else
        ret = QT_STAT(QFile::encodeName(d->file), &st);
    if(ret == -1)
        return 0;
    return st.st_size;
}

bool QFSFileEngine::mkdir(const QString &name, bool createParentDirectories) const
{
    QString dirName = name;
    if(createParentDirectories) {
        dirName = QDir::cleanPath(dirName);
        for(int oldslash = -1, slash=0; slash != -1; oldslash = slash) {
            slash = dirName.indexOf(QDir::separator(), oldslash+1);
            if(slash == -1) {
                if(oldslash == dirName.length())
                    break;
                slash = dirName.length();
            }
            if(slash) {
                QByteArray chunk = QFile::encodeName(dirName.left(slash));
                QT_STATBUF st;
                if(QT_STAT(chunk, &st) != -1) {
                    if((st.st_mode & S_IFMT) != S_IFDIR)
                        return false;
                } else if(::mkdir(chunk, 0777) != 0) {
                        return false;
                }
            }
        }
        return true;
    }
#if defined(Q_OS_DARWIN)  // Mac X doesn't support trailing /'s
    if(dirName[dirName.length() - 1] == '/')
        dirName = dirName.left(dirName.length() - 1);
#endif
    return (::mkdir(QFile::encodeName(dirName), 0777) == 0);
}

bool QFSFileEngine::rmdir(const QString &name, bool recurseParentDirectories) const
{
    QString dirName = name;
    if(recurseParentDirectories) {
        dirName = QDir::cleanPath(dirName);
        for(int oldslash = 0, slash=dirName.length(); slash > 0; oldslash = slash) {
            QByteArray chunk = QFile::encodeName(dirName.left(slash));
            QT_STATBUF st;
            if(QT_STAT(chunk, &st) != -1) {
                if((st.st_mode & S_IFMT) != S_IFDIR)
                    return false;
                if(::rmdir(chunk) != 0)
                    return oldslash != 0;
            } else {
                return false;
            }
            slash = dirName.lastIndexOf(QDir::separator(), oldslash-1);
        }
        return true;
    }
    return ::rmdir(QFile::encodeName(dirName)) == 0;
}

QStringList QFSFileEngine::entryList(QDir::Filters filters, const QStringList &filterNames) const
{
    Q_D(const QFSFileEngine);
    QStringList ret;
    DIR *dir = opendir(QFile::encodeName(d->file));
    if(!dir)
        return ret; // cannot read the directory

    const bool filterPermissions = ((filters & QDir::PermissionMask) && (filters & QDir::PermissionMask) != QDir::PermissionMask);
    const bool skipDirs     = !(filters & (QDir::Dirs | QDir::AllDirs));
    const bool skipFiles    = !(filters & QDir::Files);
    const bool skipSymlinks = (filters & QDir::NoSymLinks);
    const bool doReadable   = !filterPermissions || (filters & QDir::Readable);
    const bool doWritable   = !filterPermissions || (filters & QDir::Writable);
    const bool doExecutable = !filterPermissions || (filters & QDir::Executable);
    const bool includeHidden = (filters & QDir::Hidden);
    const bool includeSystem = (filters & QDir::System);

#ifndef QT_NO_REGEXP
    // Prepare name filters
    QList<QRegExp> regexps;
    for (int i = 0; i < filterNames.size(); ++i) {
        regexps << QRegExp(filterNames.at(i),
                           (filters & QDir::CaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive,
                           QRegExp::Wildcard);
    }
#endif

    QFileInfo fi;
    dirent   *file;
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_CYGWIN)
    union {
        struct dirent mt_file;
        char b[sizeof(struct dirent) + MAXNAMLEN + 1];
    } u;
    while (readdir_r(dir, &u.mt_file, &file) == 0 && file)
#else
    while ((file = readdir(dir)))
#endif // _POSIX_THREAD_SAFE_FUNCTIONS
    {
        QString fn = QFile::decodeName(QByteArray(file->d_name));
        if (fn.isEmpty()) {
            // unreadable entry
            continue;
        }

        const QString filePath = d->file + QLatin1Char('/') + fn;
        fi.setFile(filePath);

#ifndef QT_NO_REGEXP
        if(!((filters & QDir::AllDirs) && fi.isDir())) {
            bool matched = false;
            for (int i = 0; i < regexps.size(); ++i) {
                if (regexps.at(i).exactMatch(fn)) {
                    matched = true;
                    break;
                }
            }
            if (!matched)
                continue;
        }
#else
        Q_UNUSED(filterNames);
#endif
        if ((filters & QDir::NoDotAndDotDot) && ((fn == QLatin1String(".") || fn == QLatin1String(".."))))
            continue;
        bool isHidden = (fn.at(0) == QLatin1Char('.') && fn.length() > 1 && fn != QLatin1String("..")
#if defined(Q_WS_MAC)
            || d->isMacHidden(filePath)
#endif
        );

        if (!includeHidden && isHidden)
            continue;

        bool alwaysShow = (filters & QDir::TypeMask) == 0
                          && ((isHidden && includeHidden)
                              || (includeSystem && ((fi.exists() && !fi.isFile() && !fi.isDir() && !fi.isSymLink())
                                                    || (!fi.exists() && fi.isSymLink()))));

        // Skip files and directories
        if ((filters & QDir::AllDirs) == 0 && skipDirs && fi.isDir()) {
            if (!alwaysShow)
                continue;
        }
        if ((skipFiles && (fi.isFile() || !fi.exists()))
            || (skipSymlinks && fi.isSymLink())) {
            if (!alwaysShow)
                continue;
        }
        if (filterPermissions
            && !((doReadable && fi.isReadable())
                 || (doWritable && fi.isWritable())
                 || (doExecutable && fi.isExecutable()))) {
            continue;
        }
        if (!includeSystem && ((fi.exists() && !fi.isFile() && !fi.isDir() && !fi.isSymLink())
                               || (!fi.exists() && fi.isSymLink()))) {
            continue;
        }

        ret.append(fn);
    }
    if (closedir(dir) != 0) {
        qWarning("QDir::readDirEntries: Cannot close directory: %s", d->file.toLocal8Bit().data());
    }
    return ret;
}

bool QFSFileEngine::caseSensitive() const
{
    return true;
}

bool QFSFileEngine::setCurrentPath(const QString &path)
{
    int r;
    r = ::chdir(QFile::encodeName(path));
    return r >= 0;
}

QString QFSFileEngine::currentPath(const QString &)
{
    QString result;
    QT_STATBUF st;
    if(QT_STAT(".", &st) == 0) {
        char currentName[PATH_MAX+1];
        if(::getcwd(currentName, PATH_MAX))
            result = QFile::decodeName(QByteArray(currentName));
#if defined(QT_DEBUG)
        if(result.isNull())
            qWarning("QDir::currentPath: getcwd() failed");
#endif
    } else {
#if defined(QT_DEBUG)
        qWarning("QDir::currentPath: stat(\".\") failed");
#endif
    }
    return result;
}

QString QFSFileEngine::homePath()
{
    QString home = QFile::decodeName(qgetenv("HOME"));
    if(home.isNull())
        home = rootPath();
    return home;
}

QString QFSFileEngine::rootPath()
{
    return QString::fromLatin1("/");
}

QString QFSFileEngine::tempPath()
{
    QString temp = QFile::decodeName(qgetenv("TMPDIR"));
    if(temp.isEmpty())
        temp = QString::fromLatin1("/tmp/");
    return temp;
}

QFileInfoList QFSFileEngine::drives()
{
    QFileInfoList ret;
    ret.append(rootPath());
    return ret;
}

bool QFSFileEnginePrivate::doStat() const
{
    if (tried_stat == 0) {
        QFSFileEnginePrivate *that = const_cast<QFSFileEnginePrivate*>(this);
        if (fd != -1) {
            that->could_stat = (QT_FSTAT(fd, &st) == 0);
        } else {
            that->could_stat = (QT_STAT(QFile::encodeName(file), &st) == 0);
        }
	that->tried_stat = 1;
    }
    return could_stat;
}

bool QFSFileEnginePrivate::isSymlink() const
{
    if (need_lstat) {
        QFSFileEnginePrivate *that = const_cast<QFSFileEnginePrivate *>(this);
        that->need_lstat = false;
        QT_STATBUF st;          // don't clobber our main one
        that->is_link = (QT_LSTAT(QFile::encodeName(file), &st) == 0) ? S_ISLNK(st.st_mode) : false;
    }
    return is_link;
}

#if defined (Q_WS_MAC)
bool QFSFileEnginePrivate::isMacHidden(const QString &path) const
{
    OSErr err = noErr;
    FSRef fsRef;
    err = FSPathMakeRef((const UInt8 *)QFile::encodeName(QDir::cleanPath(path)).data(), &fsRef, 0);
    if (err != noErr)
        return false;

    FSCatalogInfo catInfo;
    err = FSGetCatalogInfo(&fsRef, kFSCatInfoFinderInfo, &catInfo, NULL, NULL, NULL);
    if (err != noErr)
        return false;

    FileInfo * const fileInfo = reinterpret_cast<FileInfo*>(&catInfo.finderInfo);
    bool result = (fileInfo->finderFlags & kIsInvisible);
    return result;
}
#endif

/*!
    \reimp
*/
QAbstractFileEngine::FileFlags QFSFileEngine::fileFlags(FileFlags type) const
{
    Q_D(const QFSFileEngine);
    // Force a stat, so that we're guaranteed to get up-to-date results
    if (type & QAbstractFileEngine::FileFlag(0x1000000)) { // QDir::Refresh
        d->tried_stat = 0;
        d->need_lstat = 1;
    }

    QAbstractFileEngine::FileFlags ret = 0;
    bool exists = d->doStat();
    if (!exists && !d->isSymlink())
        return ret;

    if (exists && (type & PermsMask)) {
        if(d->st.st_mode & S_IRUSR)
            ret |= ReadOwnerPerm;
        if(d->st.st_mode & S_IWUSR)
            ret |= WriteOwnerPerm;
        if(d->st.st_mode & S_IXUSR)
            ret |= ExeOwnerPerm;
        if(d->st.st_mode & S_IRUSR)
            ret |= ReadUserPerm;
        if(d->st.st_mode & S_IWUSR)
            ret |= WriteUserPerm;
        if(d->st.st_mode & S_IXUSR)
            ret |= ExeUserPerm;
        if(d->st.st_mode & S_IRGRP)
            ret |= ReadGroupPerm;
        if(d->st.st_mode & S_IWGRP)
            ret |= WriteGroupPerm;
        if(d->st.st_mode & S_IXGRP)
            ret |= ExeGroupPerm;
        if(d->st.st_mode & S_IROTH)
            ret |= ReadOtherPerm;
        if(d->st.st_mode & S_IWOTH)
            ret |= WriteOtherPerm;
        if(d->st.st_mode & S_IXOTH)
            ret |= ExeOtherPerm;
    }
    if(type & TypesMask) {
#if !defined(QWS) && defined(Q_OS_MAC)
        bool foundAlias = false;
        {
            FSRef fref;
            if(FSPathMakeRef((const UInt8 *)QFile::encodeName(QDir::cleanPath(d->file)).data(),
                             &fref, NULL) == noErr) {
                Boolean isAlias, isFolder;
                if(FSIsAliasFile(&fref, &isAlias, &isFolder) == noErr && isAlias) {
                    foundAlias = true;
                    ret |= LinkType;
                }
            }
        }
        if(!foundAlias)
#endif
        {
            if ((type & LinkType) && d->isSymlink())
                ret |= LinkType;
            if (exists && (d->st.st_mode & S_IFMT) == S_IFREG)
                ret |= FileType;
            else if (exists && (d->st.st_mode & S_IFMT) == S_IFDIR)
                ret |= DirectoryType;
        }
    }
    if(type & FlagsMask) {
        ret |= LocalDiskFlag;
        if (exists)
            ret |= ExistsFlag;
        if(fileName(BaseName)[0] == QLatin1Char('.')
#if defined(Q_WS_MAC)
            || d->isMacHidden(d->file)
#endif
        )
            ret |= HiddenFlag;
        if(d->file == QLatin1String("/"))
            ret |= RootFlag;
    }
    return ret;
}

QString QFSFileEngine::fileName(FileName file) const
{
    Q_D(const QFSFileEngine);
    if(file == BaseName) {
        int slash = d->file.lastIndexOf(QLatin1Char('/'));
        if(slash != -1)
            return d->file.mid(slash + 1);
    } else if(file == PathName) {
        int slash = d->file.lastIndexOf(QLatin1Char('/'));
        if(slash == -1)
            return QLatin1String(".");
        else if(!slash)
            return QLatin1String("/");
        return d->file.left(slash);
    } else if(file == AbsoluteName || file == AbsolutePathName) {
        QString ret;
        if(d->file.isEmpty() || !d->file.startsWith(QLatin1Char('/')))
            ret = QDir::currentPath();
        if(!d->file.isEmpty() && d->file != QLatin1String(".")) {
            if(!ret.isEmpty() && !ret.endsWith(QLatin1Char('/')))
                ret += QLatin1Char('/');
            ret += d->file;
        }
        if (ret == QLatin1String("/"))
            return ret;
        bool isDir = ret.endsWith(QLatin1Char('/'));
        ret = QDir::cleanPath(ret);
        if (isDir)
            ret += QLatin1String("/");
        if(file == AbsolutePathName) {
            int slash = ret.lastIndexOf(QLatin1Char('/'));
            if(slash == -1)
                return QDir::currentPath();
            else if(!slash)
                return QLatin1String("/");
            return ret.left(slash);
        }
        return ret;
    } else if(file == CanonicalName || file == CanonicalPathName) {
        char cur[PATH_MAX+1];
        if(::getcwd(cur, PATH_MAX)) {
            QString ret;
            char real[PATH_MAX+1];
            // need the cast for old solaris versions of realpath that doesn't take
            // a const char*.
            if(::realpath(QFile::encodeName(d->file).data(), real))
                ret = QFile::decodeName(QByteArray(real));
            ::chdir(cur); // always make sure we go back to the current dir
            //check it
            QT_STATBUF st;
            if(QT_STAT(QFile::encodeName(ret), &st) != 0)
                ret = QString();
            if(!ret.isEmpty() && file == CanonicalPathName) {
                int slash = ret.lastIndexOf(QLatin1Char('/'));
                if(slash == -1)
                    return QDir::currentPath();
                else if(!slash)
                    return QLatin1String("/");
                return ret.left(slash);
            }
            return ret;
        }
        if(file == CanonicalPathName)
            return fileName(AbsolutePathName);
        return fileName(AbsoluteName);
    } else if(file == LinkName) {
        if (d->isSymlink()) {
            char s[PATH_MAX+1];
            int len = readlink(QFile::encodeName(d->file), s, PATH_MAX);
            if(len > 0) {
                QString ret;
                if (S_ISDIR(d->st.st_mode) && s[0] != '/') {
                    QDir parent(d->file);
                    parent.cdUp();
                    ret = parent.path();
                    if (!ret.isEmpty() && !ret.endsWith(QLatin1Char('/')))
                        ret += QLatin1Char('/');
                }
                s[len] = '\0';
                ret += QFile::decodeName(QByteArray(s));

                if (!ret.startsWith(QLatin1Char('/'))) {
                    if (d->file.startsWith(QLatin1Char('/'))) {
                        ret.prepend(d->file.left(d->file.lastIndexOf(QLatin1Char('/')))
                                    + QLatin1Char('/'));
                    } else {
                        ret.prepend(QDir::currentPath() + QLatin1Char('/'));
                    }
                }
                ret = QDir::cleanPath(ret);
                if (ret.size() > 1 && ret.endsWith(QLatin1Char('/')))
                    ret.chop(1);
                return ret;
            }
        }
#if !defined(QWS) && defined(Q_OS_MAC)
        {
            FSRef fref;
            if(FSPathMakeRef((const UInt8 *)QFile::encodeName(QDir::cleanPath(d->file)).data(), &fref, 0) == noErr) {
                Boolean isAlias, isFolder;
                if(FSResolveAliasFile(&fref, true, &isFolder, &isAlias) == noErr && isAlias) {
                    AliasHandle alias;
                    if(FSNewAlias(0, &fref, &alias) == noErr && alias) {
                        CFStringRef cfstr;
                        if(FSCopyAliasInfo(alias, 0, 0, &cfstr, 0, 0) == noErr)
                            return QCFString::toQString(cfstr);
                    }
                }
            }
        }
#endif
        return QString();
    }
    return d->file;
}

bool QFSFileEngine::isRelativePath() const
{
    Q_D(const QFSFileEngine);
    int len = d->file.length();
    if(len == 0)
        return true;
    return d->file[0] != QLatin1Char('/');
}

uint QFSFileEngine::ownerId(FileOwner own) const
{
    Q_D(const QFSFileEngine);
    static const uint nobodyID = (uint) -2;
    if(d->doStat()) {
        if(own == OwnerUser)
            return d->st.st_uid;
        else
            return d->st.st_gid;
    }
    return nobodyID;
}

QString QFSFileEngine::owner(FileOwner own) const
{
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    QVarLengthArray<char, 1024> buf(sysconf(_SC_GETPW_R_SIZE_MAX));
#endif

    if(own == OwnerUser) {
        struct passwd *pw = 0;
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
        struct passwd entry;
        getpwuid_r(ownerId(own), &entry, buf.data(), buf.size(), &pw);
#else
        pw = getpwuid(ownerId(own));
#endif
        if(pw)
            return QFile::decodeName(QByteArray(pw->pw_name));
    } else if(own == OwnerGroup) {
        struct group *gr = 0;
#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
        buf.resize(sysconf(_SC_GETGR_R_SIZE_MAX));
        struct group entry;
        getgrgid_r(ownerId(own), &entry, buf.data(), buf.size(), &gr);
#else
        gr = getgrgid(ownerId(own));
#endif
        if(gr)
            return QFile::decodeName(QByteArray(gr->gr_name));
    }
    return QString();
}

bool QFSFileEngine::setPermissions(uint perms)
{
    Q_D(QFSFileEngine);
    mode_t mode = 0;
    if(perms & ReadOwnerPerm)
        mode |= S_IRUSR;
    if(perms & WriteOwnerPerm)
        mode |= S_IWUSR;
    if(perms & ExeOwnerPerm)
        mode |= S_IXUSR;
    if(perms & ReadUserPerm)
        mode |= S_IRUSR;
    if(perms & WriteUserPerm)
        mode |= S_IWUSR;
    if(perms & ExeUserPerm)
        mode |= S_IXUSR;
    if(perms & ReadGroupPerm)
        mode |= S_IRGRP;
    if(perms & WriteGroupPerm)
        mode |= S_IWGRP;
    if(perms & ExeGroupPerm)
        mode |= S_IXGRP;
    if(perms & ReadOtherPerm)
        mode |= S_IROTH;
    if(perms & WriteOtherPerm)
        mode |= S_IWOTH;
    if(perms & ExeOtherPerm)
        mode |= S_IXOTH;
    if(d->fd != -1)
        return !fchmod(d->fd, mode);
    const QByteArray file = QFile::encodeName(d->file);
    return !::chmod(file.data(), mode);
}

bool QFSFileEngine::setSize(qint64 size)
{
    Q_D(QFSFileEngine);
    if(d->fd != -1)
        return !QT_FTRUNCATE(d->fd, size);
    const QByteArray file = QFile::encodeName(d->file);
    return !QT_TRUNCATE(file.data(), size);
}

QDateTime QFSFileEngine::fileTime(FileTime time) const
{
    Q_D(const QFSFileEngine);
    QDateTime ret;
    if(d->doStat()) {
        if(time == CreationTime)
            ret.setTime_t(d->st.st_ctime ? d->st.st_ctime : d->st.st_mtime);
        else if(time == ModificationTime)
            ret.setTime_t(d->st.st_mtime);
        else if(time == AccessTime)
            ret.setTime_t(d->st.st_atime);
    }
    return ret;
}
