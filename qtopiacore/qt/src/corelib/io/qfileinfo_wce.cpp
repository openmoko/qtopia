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

#include "qplatformdefs.h"

#include "qlibrary.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"
#include "qapplication.h"

#include <windows.h>
#ifndef Q_OS_TEMP
#include <direct.h>
#endif
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include <ctype.h>
#include <limits.h>


void QFileInfo::slashify(QString &s)
{
    for (int i=0; i<(int)s.length(); i++) {
        if (s[i] == '\\')
            s[i] = '/';
    }
    if (s[(int)s.length() - 1] == '/' && s.length() > 3)
        s.remove((int)s.length() - 1, 1);
}


void QFileInfo::makeAbs(QString &s)
{
    if (s[0] != '/') {
        QString d = QDir::currentDirPath();
        slashify(d);
        s.prepend(d + "/");
    }
}


bool QFileInfo::isHidden() const
{
    return GetFileAttributes((TCHAR*)fn.ucs2()) & FILE_ATTRIBUTE_HIDDEN;
}


bool QFileInfo::isFile() const
{
    if (!fic || !cache)
        doStat();
    return fic ? (fic->st.st_mode & QT_STAT_MASK) == QT_STAT_REG : false;
}


bool QFileInfo::isDir() const
{
    if (!fic || !cache)
        doStat();
    return fic ? (fic->st.st_mode & QT_STAT_MASK) == QT_STAT_DIR : false;
}


bool QFileInfo::isSymLink() const
{
    return (fn.right(4) == ".lnk");
}


bool QFileInfo::permission(int p) const
{
    // just check if it's ReadOnly
    if (p & (WriteOwner | WriteUser | WriteGroup | WriteOther)) {
        DWORD attr = GetFileAttributes((TCHAR*)fn.ucs2());
        if (attr & FILE_ATTRIBUTE_READONLY)
            return false;
    }
    return true;
}


void QFileInfo::doStat() const
{
    if (fn.isEmpty())
        return;

    QFileInfo *that = ((QFileInfo*)this);        // mutable function
    if (!that->fic)
        that->fic = new QFileInfoCache;
    QT_STATBUF *b = &that->fic->st;

    int r = QT_TSTAT((TCHAR*)fn.ucs2(), (QT_STATBUF4TSTAT*)b);
    if (r!=0) {
        bool is_dir=false;
        if (fn[0] == '/' && fn[1] == '/'
          || fn[0] == '\\' && fn[1] == '\\')
        {
            // UNC - stat doesn't work for all cases (Windows bug)
            int s = fn.find(fn[0],2);
            if (s > 0) {
                // "\\server\..."
                s = fn.find(fn[0],s+1);
                if (s > 0) {
                    // "\\server\share\..."
                    if (fn[s+1]) {
                        // "\\server\share\notfound"
                    } else {
                        // "\\server\share\"
                        is_dir=true;
                    }
                } else {
                    // "\\server\share"
                    is_dir=true;
                }
            } else {
                // "\\server"
                is_dir=true;
            }
        }
        if (is_dir) {
            // looks like a UNC dir, is a dir.
            memset(b,0,sizeof(*b));
            b->st_mode = QT_STAT_DIR;
            b->st_nlink = 1;
            r = 0;
        }
    }

    if (r != 0) {
        delete that->fic;
        that->fic = 0;
    }
}


/*!
    Returns the path (which may be relative). If \a absPath is true,
    returns the absolute path, i.e., from the root.
*/
QString QFileInfo::dirPath(bool absPath) const
{
    QString s;
    if (absPath)
        s = absFilePath();
    else
        s = fn;
    int pos = s.findRev('/');
    if (pos == -1) {
        if (s[2] == '/')
            return s.left(3);
        if (s[1] == ':') {
            if (absPath)
                return s.left(2) + "/";
            return s.left(2);
        }
        return QString::fromLatin1(".");
    } else {
        if (pos == 0)
            return QString::fromLatin1("/");
        if (pos == 2 && s[1] == ':'  && s[2] == '/')
            pos++;
        return s.left(pos);
    }
}


QString QFileInfo::fileName() const
{
    int p = fn.findRev('/');
    if (p == -1) {
        int p = fn.findRev(':');
        if (p != -1)
            return fn.mid(p + 1);
        return fn;
    } else {
        return fn.mid(p + 1);
    }
}


Q_CORE_EXPORT int qt_ntfs_permission_lookup = 0;

static void resolveLibs()
{
}

QString QFileInfo::readLink() const
{
    return QString();
}

QString QFileInfo::owner() const
{
    return QString();
}

static const uint nobodyID = (uint) -2;
uint QFileInfo::ownerId() const
{
    return nobodyID;
}

QString QFileInfo::group() const
{
    return QString();
}

uint QFileInfo::groupId() const
{
    return nobodyID;
}
