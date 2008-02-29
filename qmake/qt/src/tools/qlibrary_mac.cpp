/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "private/qlibrary_p.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

/*
  The platform dependent implementations of
  - loadLibrary
  - freeLibrary
  - resolveSymbol

  It's not too hard to guess what the functions do.
*/
#if defined(Q_OS_MACX) || defined(Q_OS_DARWIN)

#if defined(Q_OS_MACX)
#  include "qt_mac.h"
#endif
# define ENUM_DYLD_BOOL
 enum DYLD_BOOL { DYLD_FALSE, DYLD_TRUE };
 extern "C" {
# include "mach-o/dyld.h"
 }
#include "qdict.h"
#include "qdir.h"
#include "qstringlist.h"

struct glibs_ref {
    QString name;
    int count;
    void *handle;
};
static QDict<glibs_ref> *glibs_loaded = 0;

#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
static NSModule qt_mac_library_multiple(NSSymbol sym, NSModule o, NSModule)
{
    qDebug("multiple definition %s", NSNameOfSymbol(sym));
    return o;
}

static void qt_mac_library_undefined(const char *symbolname)
{
    qDebug("qlibrary_mac.cpp: undefined symbol (%s)", symbolname);
    exit(666);
}

static void qt_mac_library_error(NSLinkEditErrors err, int line, const char *fileName,
				 const char *error)
{
    qDebug("qlibrary_mac.cpp: %d: %d: %s (%s)", err, line, fileName, error);
}
#endif

bool QLibraryPrivate::loadLibrary()
{
    if(pHnd)
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect glibs_loaded creation/access
    QMutexLocker locker(qt_global_mutexpool ? qt_global_mutexpool->get(&glibs_loaded) : 0);
#endif // QT_THREAD_SUPPORT

#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    static bool first = TRUE;
    if(first) { //deal with errors
	first = FALSE;
	NSLinkEditErrorHandlers hdl;
	memset(&hdl, 0, sizeof(hdl));
	hdl.undefined = qt_mac_library_undefined;
	hdl.multiple = qt_mac_library_multiple;
	hdl.linkEdit = qt_mac_library_error;
	NSInstallLinkEditErrorHandlers(&hdl);
    }
#endif

    /* Not pretty, but it looks to me like this has to be done
       if I want to look in the regular loading places (like dyld isn't
       used to find the plugin). So instead I'll just hardcode it so
       it matches dyld(1) paths */
    QStringList places(""); //just look for the filename first..
    if(const char *fallback = getenv("DYLD_FALLBACK_LIBRARY_PATH")) {
	QStringList lst = QStringList::split(':', fallback, TRUE);
	for(QStringList::Iterator it = lst.begin(); it != lst.end(); it++) {
	    QString d = (*it);
	    if(d.isEmpty())
		d = QDir::currentDirPath();
	    if(!d.endsWith(QString(QChar(QDir::separator()))))
		d += QDir::separator();
	    places << d << "qt_plugins/" << d;
	}
    } else {
	places << QDir::homeDirPath() << "/lib/qt_plugins/" << QDir::homeDirPath() << "/lib/"
	       << "/usr/local/lib/qt_plugins/" << "/usr/local/lib/"
	       << "/lib/qt_plugins/" << "/lib/"
	       << "/usr/lib/qt_plugins/" << "/usr/lib/";
    }
    if(const char *dyld_path = getenv("DYLD_LIBRARY_PATH")) {
	QStringList lst = QStringList::split(':', dyld_path, TRUE);
	for(QStringList::Iterator it = lst.begin(); it != lst.end(); it++) {
	    QString d = (*it);
	    if(d.isEmpty())
		d = QDir::currentDirPath();
	    if(!d.endsWith(QString(QChar(QDir::separator()))))
		d += QDir::separator();
	    places << d;
	}
    }

    QString filename;
    NSObjectFileImage img;
    for(QStringList::Iterator it = places.begin(); it != places.end(); it++) {
	QString tmp = (*it);
	if(!tmp.isEmpty() && !tmp.endsWith(QString(QChar(QDir::separator()))))
		tmp += QDir::separator();
	tmp += library->library();
	if(!glibs_loaded) {
	    glibs_loaded = new QDict<glibs_ref>();
	} else if(glibs_ref *i = glibs_loaded->find(tmp)) {
	    i->count++;
	    pHnd = i->handle;
	    return TRUE;
	}
	if(NSCreateObjectFileImageFromFile(tmp, &img) == NSObjectFileImageSuccess) {
	    filename = tmp;
	    break;
	}
    }
    if(filename.isEmpty()) {
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
	qDebug("Could not find %s in '%s'", library->library().latin1(), places.join("::").latin1());
#endif
	return FALSE;
    }

    if((pHnd = (void *)NSLinkModule(img, filename,
				    NSLINKMODULE_OPTION_BINDNOW|NSLINKMODULE_OPTION_PRIVATE|
				    NSLINKMODULE_OPTION_RETURN_ON_ERROR))) {
	glibs_ref *i = new glibs_ref;
	i->handle = pHnd;
	i->count = 1;
	i->name = filename;
	glibs_loaded->insert(filename, i); //insert it in the loaded hash
	return TRUE;
    }
#if defined(QT_DEBUG) || defined(QT_DEBUG_COMPONENT)
    NSLinkEditErrors lee;
    int err;
    const char *f, *msg;
    NSLinkEditError(&lee, &err, &f, &msg);
    qDebug("Failed to load library '%s' (%d:%d):\n  %s",  filename.latin1(), lee, err, msg);
#endif
    pHnd = NULL;
    return FALSE;
}

bool QLibraryPrivate::freeLibrary()
{
    if(!pHnd)
	return TRUE;

#ifdef QT_THREAD_SUPPORT
    // protect glibs_loaded access
    QMutexLocker locker(qt_global_mutexpool ? qt_global_mutexpool->get(&glibs_loaded) : 0);
#endif // QT_THREAD_SUPPORT

    if(glibs_loaded) {
	for(QDictIterator<glibs_ref> it(*glibs_loaded); it.current(); ++it) {
	    if(it.current()->handle == pHnd && !(--it.current()->count)) {
		glibs_loaded->remove(it.currentKey());

		NSUnLinkModule(pHnd,
			       NSUNLINKMODULE_OPTION_KEEP_MEMORY_MAPPED|
			       NSUNLINKMODULE_OPTION_RESET_LAZY_REFERENCES);

		break;
	    }
	}
    }
    pHnd = 0;
    return TRUE;
}

void* QLibraryPrivate::resolveSymbol(const char *symbol)
{
    if(!pHnd)
	return 0;
    void *ret = NULL;
    QCString symn2;
    symn2.sprintf("_%s", symbol);
    ret = NSAddressOfSymbol(NSLookupSymbolInModule(pHnd, symn2));
#if defined(QT_DEBUG_COMPONENT)
    if(!ret)
	qDebug("Couldn't resolve symbol \"%s\"", symbol);
#endif
    return ret;
}

#else

bool QLibraryPrivate::loadLibrary()
{
    return FALSE;
}

bool QLibraryPrivate::freeLibrary()
{
    return FALSE;
}

void* QLibraryPrivate::resolveSymbol(const char *symbol)
{
    return 0;
}

#endif // Q_OS_MAC9
