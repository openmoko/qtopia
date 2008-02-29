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

#include "qplatformdefs.h"

#include "qdir.h"
#include "qdir_p.h"
#include "qnamespace.h"
#include "qfileinfo.h"
#include "qregexp.h"
#include "qstringlist.h"
#include "qdeepcopy.h"
#include "qlibrary.h"

#  include <private/qmutexpool_p.h>

#include <windows.h>
#include <shlobj.h>
#include <limits.h>

static QString *theCWD = 0;

void QDir::slashify(QString& n)
{
    if (n.isNull())
        return;
    for (int i=0; i<(int)n.length(); i++) {
        if (n[i] ==  '\\')
            n[i] = '/';
    }
}

/*!
    Returns the user's home directory or an empty string if it cannot
    be determined.

    \sa currentDirPath() rootDirPath()
*/
QString QDir::homeDirPath()
{
    typedef HRESULT (WINAPI *PtrSHGetSpecialFolderLocation)(HWND,int,LPITEMIDLIST*);
    typedef BOOL    (WINAPI *PtrSHGetPathFromIDList)(LPCITEMIDLIST,LPSTR);
    typedef HRESULT (WINAPI *PtrSHGetMalloc)(LPMALLOC*);

    static PtrSHGetSpecialFolderLocation ptrSHGetSpecialFolderLocation= 0;
    static PtrSHGetPathFromIDList ptrSHGetPathFromIDList = 0;
    static PtrSHGetMalloc ptrSHGetMalloc = 0;

    static bool shGSFLLookup = false;
    if (!shGSFLLookup) {
        shGSFLLookup = true;
        ptrSHGetSpecialFolderLocation = (PtrSHGetSpecialFolderLocation)QLibrary::resolve("ceshell", "SHGetSpecialFolderLocation");
        ptrSHGetPathFromIDList = (PtrSHGetPathFromIDList)QLibrary::resolve("ceshell", "SHGetPathFromIDList");
        ptrSHGetMalloc = (PtrSHGetMalloc)QLibrary::resolve("ceshell", "SHGetMalloc");
    }

    if (ptrSHGetSpecialFolderLocation) {
        LPMALLOC pIMalloc = 0;
        HRESULT res = ptrSHGetMalloc(&pIMalloc);
        Q_ASSERT(pIMalloc);

        LPITEMIDLIST il;
        if (NOERROR != ptrSHGetSpecialFolderLocation(0, CSIDL_PERSONAL, &il))
            return QString();
        char Path[MAX_PATH];
        ptrSHGetPathFromIDList(il, Path);
        QString d = QString::fromLocal8Bit(Path);
        pIMalloc->Free(il);
        slashify(d);
        return d;
    }
    return QString();
}


QString QDir::canonicalPath() const
{
    QString aPath = dPath.copy();
    if (aPath.length() && aPath[0] == '.')
        aPath.prepend(QDir::currentDirPath());
    slashify(aPath);

    QStringList aList = QStringList::split('/', aPath);
    QStringList::Iterator it = aList.begin();
    uint elm = aList.size(), len = elm;

    // Create new canonical path at the end
    // of the current string list, by pushing
    // and popping to the back
    while (len--) {
        if ((*it) == ".") {
            ++it;
            continue;
        } else if ((*it) == ".." &&
            aList.size() > elm) {
            aList.pop_back();
        } else {
            aList.push_back((*it));
        }
        ++it;
    }

    // Remove all the original elements, so
    // we are left with only the canonical.
    len = elm;
    while (len--)
        aList.pop_front();

    aPath = aList.join("/");
    return aPath.prepend('/');
}


bool QDir::mkdir(const QString &dirName, bool acceptAbsPath) const
{
    return ::_wmkdir((TCHAR*)filePath(dirName,acceptAbsPath).ucs2()) == 0;
}

bool QDir::rmdir(const QString &dirName, bool acceptAbsPath) const
{
    return ::_wrmdir((TCHAR*)filePath(dirName,acceptAbsPath).ucs2()) == 0;
}


bool QDir::isReadable() const
{
    return ::_waccess((TCHAR*)dPath.ucs2(), R_OK) == 0;
}


bool QDir::isRoot() const
{
    return dPath == "/" || dPath == "//" ||
        (dPath[0].isLetter() && dPath.mid(1,dPath.length()) == ":/");
}


bool QDir::rename(const QString &oldName, const QString &newName,
                   bool acceptAbsPaths       )
{
    if (oldName.isEmpty() || newName.isEmpty()) {
        qWarning("QDir::rename: Empty or null file name");
        return false;
    }
    QString fn1 = filePath(oldName, acceptAbsPaths);
    QString fn2 = filePath(newName, acceptAbsPaths);
    return ::_wrename((TCHAR*)fn1.ucs2(), (TCHAR*)fn2.ucs2()) == 0;
}


bool QDir::setCurrent(const QString &path)
{
#ifndef QT_NO_THREAD
    QMutexLocker locker(qt_global_mutexpool ?
                         qt_global_mutexpool->get(&theCWD) : 0);
#endif

    DWORD res = GetFileAttributes((TCHAR*)path.ucs2());
    if (0xFFFFFFFF == res)
        return false;

    if (! theCWD)
        theCWD = new QString(QDeepCopy<QString>(path));
    else
        *theCWD = QDeepCopy<QString>(path);

    slashify(*theCWD);
    return true;
}


/*!
    Returns the current working directory or "/" if the current
    directory cannot be determined.

    \sa homeDirPath() rootDirPath()
*/
QString QDir::currentDirPath()
{
#ifndef QT_NO_THREAD
    QMutexLocker locker(qt_global_mutexpool ?
                         qt_global_mutexpool->get(&theCWD) : 0);
#endif

    if (! theCWD)
        setCurrent("/");
    Q_ASSERT(theCWD != 0);
    return *theCWD;
}


/*!
    Returns "/".

    \sa homeDirPath() currentDirPath()
*/
QString QDir::rootDirPath()
{

    return QString("/");
}


bool QDir::isRelativePath(const QString &path)
{
    if (path.isEmpty())
        return true;

    return path[0] != '/' && path[0] != '\\';
}


bool QDir::readDirEntries(const QStringList &nameFilters,
                           int filterSpec, int sortSpec)
{
    int i;
    fList.clear();
    fiList.clear();

    bool doDirs     = (filterSpec & Dirs)       != 0;
    bool doFiles    = (filterSpec & Files)      != 0;
    bool noSymLinks = (filterSpec & NoSymLinks) != 0;
    bool doReadable = (filterSpec & Readable)   != 0;
    bool doWritable = (filterSpec & Writable)   != 0;
    bool doExecable = (filterSpec & Executable) != 0;
    bool doHidden   = (filterSpec & Hidden)     != 0;

    // show hidden files if the user asks explicitly for e.g. .*
    if (!doHidden && !nameFilters.size()) {
        QStringList::ConstIterator sit = nameFilters.begin();
        while (sit != nameFilters.end()) {
            if ((*sit)[0] == '.') {
                doHidden = true;
                break;
            }
            ++sit;
        }
    }
    bool doModified = (filterSpec & Modified)   != 0;
    bool doSystem   = (filterSpec & System)     != 0;

    bool first = true;
    QString p = dPath;
    int plen = p.length();
    HANDLE ff;
    WIN32_FIND_DATA finfo;
    QFileInfo fi;

#undef IS_SUBDIR
#undef IS_RDONLY
#undef IS_ARCH
#undef IS_HIDDEN
#undef IS_SYSTEM
#undef FF_ERROR

#define IS_SUBDIR   FILE_ATTRIBUTE_DIRECTORY
#define IS_RDONLY   FILE_ATTRIBUTE_READONLY
#define IS_ARCH     FILE_ATTRIBUTE_ARCHIVE
#define IS_HIDDEN   FILE_ATTRIBUTE_HIDDEN
#define IS_SYSTEM   FILE_ATTRIBUTE_SYSTEM
#define FF_ERROR    INVALID_HANDLE_VALUE

    if (plen == 0) {
        qWarning("QDir::readDirEntries: No directory name specified");
        return;
    }
    if (p.at(plen-1) != '/' && p.at(plen-1) != '\\')
        p += '/';
    p += QLatin1String("*.*");

    ff = FindFirstFile((TCHAR*)p.utf16(), &finfo);

    if (ff == FF_ERROR)
        return; // cannot read the directory

    for (;;) {
        if (first)
            first = false;
        else {
            if (!FindNextFile(ff,&finfo))
                break;
        }
        int  attrib = finfo.dwFileAttributes;
        bool isDir        = (attrib & IS_SUBDIR) != 0;
        bool isFile        = !isDir;
        bool isSymLink        = false;
        bool isReadable = true;
        bool isWritable = (attrib & IS_RDONLY) == 0;
        bool isExecable = false;
        bool isModified = (attrib & IS_ARCH)   != 0;
        bool isHidden        = (attrib & IS_HIDDEN) != 0;
        bool isSystem        = (attrib & IS_SYSTEM) != 0;

        QString fname = QString::fromUtf16((unsigned short *)finfo.cFileName);

#ifndef QT_NO_REGEXP
        if (!match(nameFilters, fname) && !(allDirs && isDir))
            continue;
#endif
        if  ((doDirs && isDir) || (doFiles && isFile)) {
            QString name = fname;
            slashify(name);
            if (doExecable) {
                QString ext = name.right(4).toLower();
                if (ext == ".exe" || ext == ".com" || ext == ".bat" ||
                     ext == ".pif" || ext == ".cmd")
                    isExecable = true;
            }

            if (noSymLinks && isSymLink)
                continue;
            if ((filterSpec & RWEMask) != 0)
                if ((doReadable && !isReadable) ||
                     (doWritable && !isWritable) ||
                     (doExecable && !isExecable))
                    continue;
            if (doModified && !isModified)
                continue;
            if (!doHidden && isHidden)
                continue;
            if (!doSystem && isSystem)
                continue;
            fi.setFile(*this, name);
            fiList.append(fi);
        }
    }
    FindClose(ff);

#undef        IS_SUBDIR
#undef        IS_RDONLY
#undef        IS_ARCH
#undef        IS_HIDDEN
#undef        IS_SYSTEM
#undef        FF_ERROR

    // Sort...
    QDirSortItem* si= new QDirSortItem[fiList.count()];
    for (i = 0; i < fiList.size(); ++i)
        si[i].item = fiList.at(i);
    qt_cmp_si_sortSpec = sortSpec;
    qsort(si, i, sizeof(si[0]), qt_cmp_si);
    fiList.clear();
    int j;
    for (j=0; j<i; j++) {
        fiList.append(si[j].item);
        fList.append(si[j].item.fileName());
    }
    delete [] si;

    if (filterSpec == (FilterSpec)filtS && sortSpec == (SortSpec)sortS &&
         nameFilters == nameFilts)
        dirty = false;
    else
        dirty = true;
    return;
}


QFileInfoList QDir::drives()
{
    QFileInfoList drives;
    drives.append(rootDirPath());
    return drives;
}
