/****************************************************************************
** $Id: qt/src/kernel/qmemorymanager_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of QMemoryManager class
**
** Created : 000411
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QMEMORYMANAGER_H
#define QMEMORYMANAGER_H

#ifndef QT_H
#include <qfontmanager_qws.h>
#include <qstring.h>
#include <qmap.h>
#endif // QT_H

//#define DEBUG_SHARED_MEMORY_CACHE

#if !defined(_WS_QWS_) || defined(QT_NO_QWS_MULTIPROCESS)
#  define QT_NO_QWS_SHARED_MEMORY_CACHE
#endif

//#define QT_NO_QWS_SHARED_MEMORY_CACHE	//XXX just for now.

#ifdef QT_NO_QWS_SHARED_MEMORY_CACHE
#define QT_NO_SHARED_FONT_CACHE
#define QT_NO_SHARED_PIXMAP_CACHE
#endif

// We can't use the shared font cache if saving qpf
// fonts because we need to build the glyph tree.
#ifndef QT_NO_QWS_SAVEFONTS
#define QT_NO_SHARED_FONT_CACHE
#endif

#ifndef QT_NO_QWS_SHARED_MEMORY_CACHE

extern char *qt_sharedMemoryData;

/*
  Process independant pointer to data in shared memory
  (on some systems a piece of shared memory could be mapped
  to different addresses in each process, therefore references
  to shared memory stored in shared memory need to be
  relative to where the shared memory is mapped)
*/
class QSMemPtr {
public:
    QSMemPtr(char *mem) { index = (mem) ? int(mem - qt_sharedMemoryData) : -1; }
    QSMemPtr() { index = -1; }
    QSMemPtr &operator =(int i) { index = i; return *this; }
    operator char*() const { return index != -1 ? &qt_sharedMemoryData[index] : 0; }
    operator uchar*() const { return index != -1 ? (uchar*)&qt_sharedMemoryData[index] : 0; }
    operator short*() const { return index != -1 ? (short*)&qt_sharedMemoryData[index] : 0; }
    operator ushort*() const { return index != -1 ? (ushort*)&qt_sharedMemoryData[index] : 0; }
    operator int*() const { return index != -1 ? (int*)&qt_sharedMemoryData[index] : 0; }
    operator uint*() const { return index != -1 ? (uint*)&qt_sharedMemoryData[index] : 0; }
    operator bool() const { return index != -1; }
    operator int() const { return index; }
private:
    int index; // offset from start of shared memory
};

/*
  A cached item in shared memory, it has a key which is
  in shared memory and a ref count which will be in shared memory
  also if this object is allocated in shared memory, usually
  it is allocated with data following the reference to the key
*/
class QSMCacheItem : public QShared {
public:
    QSMemPtr key;
    enum QSMCacheItemType {
        Global = 0,
        Pixmap = 1,
        GlyphRowIndex = 2,
        GlyphRow = 3
    };
    /*
        Would be nice to have virtual destructors for when
        deref'd and then cleaning up from the shared memory different cached items.
        But this is not really possible in a generic way because it would depend
        on the process which created the item to still be in memory to provide
        the destructor and for the cleanup to get that process to do the work.
        Obviously not very sensible, the alternative is to only have a fixed set
        of types of items with the cleanup for those all provided in the library.
    */
    QSMCacheItemType type;
};

class QSMCacheItemPtr {
public:
    // default
    QSMCacheItemPtr() : d(0) {}
    // construct from pointer to item
    QSMCacheItemPtr(QSMCacheItem *ptr) : d(ptr) {}
    // constructs from an index to the start of the item structure
    QSMCacheItemPtr(QSMemPtr ptr) {
	d = (QSMCacheItem*)(char*)ptr;
    }
    // constructs from a memory ptr to where the data after the item starts
    QSMCacheItemPtr(void *data) {
	char *ptr = (char*)data;
	if ( (int)ptr != (((int)ptr+3)&~3) )
	    qDebug("err, passed a non-aligned data ptr %x", (int)ptr);
	d = (QSMCacheItem*)(ptr - sizeof(QSMCacheItem));
    }
    // returns a pointer to the data after the item
    operator char*() const {
	return d ? (char*)d + sizeof(QSMCacheItem) : 0;
    }
    // pointer to the item itself
    operator QSMCacheItem*() const { return d; }
    void free();
    void ref();
    void deref();
    char *key() { return (char*)d->key; }
    int count() { return d->count; }
    void setCount(int c) { d->count = c; }
private:
    QSMCacheItem *d;
};

class QSharedMemoryCache;
class QSharedMemoryData;
class QLock;

class QSharedMemoryManager {
public:
    QSharedMemoryManager();
    ~QSharedMemoryManager();

    // finds a pixmap from the shared memory cache
    QPixmap *findPixmap(const QString &key, bool ref) const;
    // inserts a pixmap to the shared memory cache
    bool insertPixmap(const QString &key, const QPixmap *pm);

    // creates a new item in the cache
    static QSMCacheItemPtr newItem(const char *key, int size, int type = QSMCacheItem::Global);
    // finds an item from the cache
    static QSMCacheItemPtr findItem(const char *key, bool ref, int type = QSMCacheItem::Global);
    // removes an item from the cache, items are ref counted, on last
    // deref this will get called, normally just use item->deref().
    static void freeItem(QSMCacheItem *item);

    QLock *lock() { return l; }
    QSharedMemoryCache *pixmapCache() { return cache; }
    QSharedMemoryData *data() { return shm; }

    QSMemPtr alloc(int size, bool lock=true);
    void free(QSMemPtr ptr, bool lock=true);

#ifdef DEBUG_SHARED_MEMORY_CACHE
    bool checkConsistency(const char *msg, const char *file, int line);
    void rebuildFreeList();
#endif

private:
    QSMemPtr internal_alloc(int size);
    void internal_free(QSMemPtr ptr);

    bool isGlobalPixmap(const QString &) const { return true; }

    int shmId;
    QLock *l;
    QSharedMemoryCache *cache;
    QSharedMemoryData *shm;
};

//extern QSharedMemoryManager *qt_getSMManager();
extern void derefSharedMemoryPixmap(uchar *data);

#endif // !QT_NO_QWS_SHARED_MEMORY_CACHE

class QFontDef;

class QMemoryManager {
public:
    QMemoryManager();

    // Pixmaps
    typedef int PixmapID;
    enum PixmapType { Normal, VRAM, Invalid, Shared };

    PixmapID newPixmap(int w, int h, int d, int optim);
    void deletePixmap(PixmapID);

    uchar *pixmapData(PixmapID id) { return *(pixmap_map.find(id)); }

    static int pixmapLinestep(PixmapID id, int width, int depth);
    static PixmapType pixmapType(PixmapID id) { return (PixmapType)(id & 3); }

    PixmapID mapPixmapData(uchar *data, PixmapType type = Shared) {
	PixmapID id = next_pixmap_id + (int)type;
	next_pixmap_id += 4;
	pixmap_map[id] = data;
	return id;
    }

    // Fonts
    typedef void* FontID;
    FontID refFont(const QFontDef&);
    void derefFont(FontID);
    QRenderedFont* fontRenderer(FontID); // XXX JUST FOR METRICS
    bool inFont(FontID, const QChar&) const;
    QGlyph lockGlyph(FontID, const QChar&);
    QGlyphMetrics* lockGlyphMetrics(FontID, const QChar&);
    void unlockGlyph(FontID, const QChar&);
#ifndef QT_NO_QWS_SAVEFONTS
    void savePrerenderedFont(const QFontDef&, bool all=TRUE);
    void savePrerenderedFont(FontID id, bool all=TRUE);
#endif
    bool fontSmooth(FontID id) const;
    int fontAscent(FontID id) const;
    int fontDescent(FontID id) const;
    int fontMinLeftBearing(FontID id) const;
    int fontMinRightBearing(FontID id) const;
    int fontLeading(FontID id) const;
    int fontMaxWidth(FontID id) const;
    int fontUnderlinePos(FontID id) const;
    int fontLineWidth(FontID id) const;
    int fontLineSpacing(FontID id) const;

private:
    QMap<PixmapID,uchar*> pixmap_map;
    int next_pixmap_id;
    QMap<QString,FontID> font_map;
};

extern QMemoryManager* memorymanager;

#endif
