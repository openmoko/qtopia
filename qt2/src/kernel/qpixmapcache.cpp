/****************************************************************************
** $Id: qt/src/kernel/qpixmapcache.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QPixmapCache class
**
** Created : 950504
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qpixmapcache.h"
#include "qcache.h"
#include "qobject.h"


// REVISED: paul
/*!
  \class QPixmapCache qpixmapcache.h

  \brief The QPixmapCache class provides an application-global cache for
  pixmaps.

  \ingroup environment
  \ingroup drawing

  This class is a tool for optimized drawing with QPixmap.  You can
  use it to store temporary pixmaps that are expensive to generate,
  without using more storage space than cacheLimit(). Use insert() to
  insert pixmaps, find() to find them and clear() to empty the cache.

  Here follows an example. QRadioButton has a non-trivial visual
  representation. In the function QRadioButton::drawButton(), we do
  not draw the radio button directly. Instead, we first check the
  global pixmap cache for a pixmap with the key "$qt_radio_nnn_",
  where \c nnn is a numerical value that specifies the the radio
  button state.  If a pixmap is found, we bitBlt() it onto the widget
  and return. Otherwise, we create a new pixmap, draw the radio button
  in the pixmap and finally insert the pixmap in the global pixmap
  cache, using the key above.  The bitBlt() is 10 times faster than
  drawing the radio button.  All radio buttons in the program share
  the cached pixmap since QPixmapCache is application-global.

  QPixmapCache contains no member data, only static functions to access
  the global pixmap cache.  It creates an internal QCache for caching the
  pixmaps.

  The cache associates a pixmap with a normal string (key).  If two
  pixmaps are inserted into the cache using equal keys, then the last
  pixmap will hide the first pixmap. The QDict and QCache classes do
  exactly the same.

  The cache becomes full when the total size of all pixmaps in the
  cache exceeds cacheLimit().  The initial cache limit is 1024 KByte
  (1 MByte); it is changed with setCacheLimit().  A pixmap takes
  roughly width*height*depth/8 bytes of memory.

  See the \l QCache documentation for more details about the cache mechanism.
*/

#include "qmemorymanager_qws.h"
#include <qwindowdefs.h>
#include <qbitmap.h>


static const int cache_size = 149;		// size of internal hash array
static int cache_limit	  = 1024;		// 1024 KB cache limit

void cleanup_pixmap_cache();


#ifndef QT_NO_QWS_SHARED_MEMORY_CACHE

#ifdef DEBUG_SHARED_MEMORY_CACHE
# define TEST_SHM_AT_RANDOM_LOCATION
#endif

#define THROW_AWAY_UNUSED_PAGES

// Only define this if there really isn't going to be enough room for
// a full cache, in which case it really means the cache is set too large
// for the system, instead consider making the cache smalled.
#undef WHEN_APP_CLOSES_DESTROY_ANY_PIXMAPS_WITH_ZERO_REFERENCES

#include "qapplication.h"
#include "qgfx_qws.h"
#include "qlock_qws.h"
#include "qwsdisplay_qws.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>

#ifdef THROW_AWAY_UNUSED_PAGES
# include <sys/mman.h> // madvise
# include <asm/page.h> // PAGE_SIZE,PAGE_MASK,PAGE_ALIGN
# ifndef PAGE_ALIGN
# define PAGE_ALIGN(addr)	(((addr)+PAGE_SIZE-1)&PAGE_MASK)
# endif // PAGE_ALIGN
#endif // THROW_AWAY_UNUSED_PAGES 


char *qt_sharedMemoryData = 0;
QSharedMemoryManager *qt_getSMManager();


/*
  Structure used to link together the chunks of shared memory allocated or freed
  This is used to manage the shared memory
*/
class QSMemNode {
public:
    QSMemNode(QSMemPtr prev, QSMemPtr next) :
	signature(0xF00C0DE), freeFlag(1), prevNode(prev), nextNode(next) {}
    bool isValid() { return signature == 0xF00C0DE; }
    void invalidate() { signature = 0; }
    QSMemNode *prev() { return (QSMemNode*)(char*)prevNode; }
    QSMemNode *next() { return (QSMemNode*)(char*)nextNode; }
    QSMemNode *nextFree() { return (QSMemNode*)(char*)nextFreeNode; }
    QSMemPtr prevPtr() { return prevNode; }
    QSMemPtr nextPtr() { return nextNode; }
    QSMemPtr nextFreePtr() { return nextFreeNode; }
    void setPrev(QSMemNode *prev) { prevNode = QSMemPtr((char*)prev); }
    void setNext(QSMemNode *next) { nextNode = QSMemPtr((char*)next); }
    void setNextFree(QSMemNode *nextFree) { nextFreeNode = QSMemPtr((char*)nextFree); }
    void setPrev(QSMemPtr prev) { prevNode = prev; }
    void setNext(QSMemPtr next) { nextNode = next; }
    void setNextFree(QSMemPtr nextFree) { nextFreeNode = nextFree; }
    int size() { return (int)nextNode - (int)QSMemPtr((char*)this) - sizeof(QSMemNode); }
    void setFree(bool free) { freeFlag = (free ? 1 : 0); }
    bool isFree() { return (freeFlag == 1); }
private:
    uint	signature : 31;
    uint	freeFlag : 1;
    QSMemPtr	prevNode;
    QSMemPtr	nextNode;
    QSMemPtr	nextFreeNode;
};


// Double hashing algorithm is employed by the shared memory pixmap cache
#define SHM_ITEMS		1537	    // 1537 entries
#define MAX_SHM_ITEMS		1200	    // only add 1200 entries,
					    // don't fill so hash has spaces so it works
#define MAX_SHM_FREE_ITEMS	100	    // 100 entries
#define MAX_SHM_DATA		1000000	    // 1 Mb

#define MAGIC_HASH_DELETED_VAL	-2

#ifdef DEBUG_SHARED_MEMORY_CACHE
# define CHECK_CONSISTENCY_A(msg) \
    ASSERT( checkConsistency(msg, __FILE__, __LINE__) )
# define CHECK_CONSISTENCY(msg) \
    ASSERT( qt_getSMManager()->checkConsistency(msg, __FILE__, __LINE__) )
# define CHECK_CACHE_CONSISTENCY() \
    ASSERT( checkCacheConsistency() );
#else
# define CHECK_CONSISTENCY_A(msg)
# define CHECK_CONSISTENCY(msg)
# define CHECK_CACHE_CONSISTENCY()
#endif

/*
class QSharedMemoryHashTable {
public:
    QSMCacheItemPtr newItem(const char *key, int size);
    QSMCacheItemPtr findItem(const char *key, bool ref);
};
*/

class QSharedMemoryCacheData {
public:
    // Cache version
    int version;

    // Offsets and sizes for arrays
    int dataOffset;
    int dataSize;
    int offsetsOffset;
    int offsetsSize;	// Size of the hash table
    int freeItemsOffset;
    int freeItemsSize;

    // Counts
    int maxItems;	// Max items allowed in the hash table
    int items;		// Current number of items in the hash table
    int freeItemCount;	// Current number of items marked for deletion

    // Profiling data
    int cacheFindHits;
    int cacheFindMisses;
    int cacheFindStrcmps;
    int cacheInsertHits;
    int cacheInsertMisses;
    int cacheInsertStrcmps;

    QSMemPtr offsets[SHM_ITEMS];		// Hash table
    QSMemPtr freeItems[MAX_SHM_FREE_ITEMS+1];	// List of items marked for deletion
};


class QSharedMemoryCache {
public:
    QSharedMemoryCache(QSharedMemoryCacheData *data) : d(data) { }
    ~QSharedMemoryCache() { }

    void init() {
	d->version = 100;
	d->dataOffset = sizeof(QSharedMemoryCacheData) + 2*sizeof(QSMemNode);
	d->dataSize = MAX_SHM_DATA;
	d->offsetsOffset = (int)((char*)&d->offsets[0] - (char*)d);
	d->offsetsSize = SHM_ITEMS;
	d->freeItemsOffset = (int)((char*)&d->freeItems[0] - (char*)d);
	d->freeItemsSize = MAX_SHM_FREE_ITEMS;
	d->maxItems = MAX_SHM_ITEMS;
	d->items = 0;
	d->freeItemCount = 0;
	d->cacheFindHits = 0;
	d->cacheFindMisses = 0;
	d->cacheFindStrcmps = 0;
	d->cacheInsertHits = 0;
	d->cacheInsertMisses = 0;
	d->cacheInsertStrcmps = 0;
	for (int i = 0; i < SHM_ITEMS; i++)
	    d->offsets[i] = -1;
	//memset(offsets,-1,SHM_ITEMS*sizeof(QSMemPtr));
    }

    QSMCacheItemPtr newItem(const char *key, int size, int type);
    QSMCacheItemPtr findItem(const char *key, bool ref, int type);
    void freeItem(QSMCacheItem *item) {
	CHECK_CONSISTENCY("Pre free item");
	CHECK_CACHE_CONSISTENCY();
	{
	    QLockHolder lh(qt_getSMManager()->lock(),QLock::Write);
	    if ( d->freeItemCount >= d->freeItemsSize )
		if ( !cleanUp(false) )
		    // all the items have been referenced again, clear the list
		    d->freeItemCount = 0;
#ifdef DEBUG_SHARED_MEMORY_CACHE
	    for (int i = 0; i < d->freeItemCount; i++) {
		QSMemPtr ptr = d->freeItems[i];
		QSMCacheItemPtr ptr2 = (QSMCacheItemPtr)ptr;
		if ( ptr2.count() )
		    qFatal("found item with references in free list: %s", ptr2.key() );
		if ((char*)ptr == (char*)item)
		    qFatal("found item already in free list: %s", ptr2.key() );
	    }
#endif // DEBUG_SHARED_MEMORY_CACHE
	    d->freeItems[d->freeItemCount] = QSMemPtr((char*)item);
	    d->freeItemCount++;
	}
	CHECK_CACHE_CONSISTENCY();
	CHECK_CONSISTENCY("Post free item");
    }

    bool cleanUp(bool needLock=true);

#ifdef DEBUG_SHARED_MEMORY_CACHE
    bool checkCacheConsistency();
#endif

private:
    void hash(const char *key, int &hash, int &inc);
    bool cleanUp_internal();

    QSharedMemoryCacheData *d;
};


bool QSharedMemoryCache::cleanUp(bool needLock)
{
    CHECK_CONSISTENCY("Pre CleanUp");
    CHECK_CACHE_CONSISTENCY();
    bool ret = false;
    if ( needLock ) {
	QLockHolder lh(qt_getSMManager()->lock(),QLock::Write);
	ret = cleanUp_internal();
    } else {
	ret = cleanUp_internal();
    }
    CHECK_CACHE_CONSISTENCY();
    CHECK_CONSISTENCY("Post CleanUp");
    return ret;
}


bool QSharedMemoryCache::cleanUp_internal()
{
    if ( !d->freeItemCount )
       return false;

    int releasedItems = 0;
    for ( int i = 0; i < d->freeItemCount; i++ ) {
       QSMCacheItemPtr item(d->freeItems[i]);
        if (!item.count()) {
           bool found = false;
           for ( int j = 0; j < SHM_ITEMS; j++ ) {
		if ( (char*)d->freeItems[i] == (char*)d->offsets[j] ) {
#ifdef DEBUG_SHARED_MEMORY_CACHE
		    if ( found )
			qDebug("same pointer twice in cache hash table");
#endif // DEBUG_SHARED_MEMORY_CACHE

		    // See if next in hash table is null
		    int hashIndex, hashInc;
		    QSMemPtr memPtr = d->offsets[j];
		    QSMCacheItemPtr delItem(memPtr);
		    hash(delItem.key(), hashIndex, hashInc);
		    hashIndex = (j + hashInc) % SHM_ITEMS;
		    memPtr = d->offsets[hashIndex];

		    if ( memPtr )
			d->offsets[j] = MAGIC_HASH_DELETED_VAL;
		    else
			d->offsets[j] = -1;

		    d->items--;
		    found = true;
		}
           }
           item.free();
           memmove(&d->freeItems[i],&d->freeItems[i+1],(d->freeItemCount-i)*sizeof(QSMemPtr));
           d->freeItemCount--;
           releasedItems++;
	   if ( releasedItems >= 3 )
	       break;
        }
     }

    return (releasedItems >= 1);
}


class QSharedMemoryData {
public:
    QSharedMemoryCacheData cacheData;

    QSMemNode head;
    QSMemNode first;
    char data[MAX_SHM_DATA];
    QSMemNode tail;
};


QSharedMemoryManager *qt_getSMManager()
{
    static QSharedMemoryManager *qt_SMManager = 0;
    if ( !qt_SMManager ) {
	qt_SMManager = new QSharedMemoryManager;
	CHECK_PTR(qt_SMManager);
    }
    return qt_SMManager;
}


QSMCacheItemPtr QSharedMemoryManager::newItem(const char *key, int size, int type)
{
    return qt_getSMManager()->cache->newItem(key, size, type);
}


QSMCacheItemPtr QSharedMemoryManager::findItem(const char *key, bool ref, int type)
{
    return qt_getSMManager()->cache->findItem(key, ref, type);
}


void QSharedMemoryManager::freeItem(QSMCacheItem *item)
{
    return qt_getSMManager()->cache->freeItem(item);
}


void QSMCacheItemPtr::free()
{
    if (d) {
	if ( d->count ) {
	    qWarning("Can't free null item or item with references");
	    return;
	}
	qt_getSMManager()->free(d->key, false);
	qt_getSMManager()->free(QSMemPtr((char*)d), false);
	d = 0;
    } else {
	qWarning("QSMCacheItemPtr::free(): QSMCacheItem has been freed");
    }
}


void QSMCacheItemPtr::ref()
{
    if (!d) {
	qWarning("QSMCacheItemPtr::ref(): QSMCacheItem has been freed");
    } else {
	d->ref();
    }
}


void QSMCacheItemPtr::deref()
{
    if (!d) {
	qWarning("QSMCacheItemPtr::deref(): QSMCacheItem has been freed");
	return;
    }
    if ( !d->count ) {
	qWarning("QSMCacheItem ref count already zero (%s)", (char*)d->key);
	return;
    }
    d->deref();
#ifdef DEBUG_SHARED_MEMORY_CACHE
    qDebug("releasing %s (count %i)", (char*)d->key, d->count);
#endif
    if ( !d->count ) {
#ifdef DEBUG_SHARED_MEMORY_CACHE
	qDebug("add to list of items to clean up (%s)", (char*)d->key);
#endif
	QSharedMemoryManager::freeItem(d);
    }
}


#ifdef USE_EVIL_SIGNAL_HANDLERS
int signalList[14] = {
    SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGABRT, SIGFPE, SIGKILL,
    SIGSEGV, SIGPIPE, SIGALRM, SIGTERM, SIGUSR1, SIGUSR2, SIGBUS
};


typedef void (*sighandler_t)(int);
sighandler_t oldHandlers[sizeof(signalList)/sizeof(int)];


void cleanUpCacheSignalHandler(int sig)
{
    qDebug("got signal %i", sig);
    for ( uint i = 0; i < sizeof(signalList)/sizeof(int); i++ )
	if ( signalList[i] == sig ) {
	    qDebug("found old sig handler, chaining %i -> %x", sig, (void*)oldHandlers[i]);
	    if ( oldHandlers[i] )
		oldHandlers[i](sig);
	    else
		abort();
	}
}
#endif // USE_EVIL_SIGNAL_HANDLERS


// Attach/create shared memory cache
QSharedMemoryManager::QSharedMemoryManager()
{
    shm = 0;
    shmId = -1;

    bool server = qApp->type() == QApplication::GuiServer;

    extern QString qws_qtePipeFilename();
    key_t key = ftok(qws_qtePipeFilename(), 0xEEDDCCC2);

    l = new QLock(qws_qtePipeFilename(), 't', server);

    if ( server ) {
	shmId = shmget(key, 0, 0);
	if ( shmId != -1 ) 
	    shmctl(shmId, IPC_RMID, 0); // Delete if exists already
    }

    shmId = shmget(key, 0, 0);

    if ( shmId == -1 ) { // It needs to be created
	shmId = shmget(key, sizeof(QSharedMemoryData), IPC_CREAT|0600);
	if ( shmId != -1 ) {
	    struct shmid_ds shmInfo;
	    shmctl(shmId, IPC_STAT, &shmInfo);
	    shm = (QSharedMemoryData*)shmat( shmId, 0, 0 );
#ifdef DEBUG_SHARED_MEMORY_CACHE
	    qDebug("Created shm at %p of size %i bytes", shm, shmInfo.shm_segsz);
#endif // DEBUG_SHARED_MEMORY_CACHE
	}
    } else {
#ifdef TEST_SHM_AT_RANDOM_LOCATION
	srand((int)clock());
	shm = (QSharedMemoryData*)shmat(shmId, (void*)rand(), SHM_RND);
#else
	shm = (QSharedMemoryData*)shmat(shmId, 0, 0);
#endif
#ifdef DEBUG_SHARED_MEMORY_CACHE
	qDebug("Connected to shm at %p", shm);
#endif // DEBUG_SHARED_MEMORY_CACHE
    }

    if ( shmId == -1 || (int)shm == -1 )
	qFatal("Cannot attach to shared memory");

    qt_sharedMemoryData = shm->data;
    cache = new QSharedMemoryCache(&shm->cacheData);

    if ( server ) {
 	shm->head = QSMemNode(QSMemPtr(),QSMemPtr((char*)&shm->first));
	shm->head.setFree(true);
 	shm->head.setNextFree((char*)&shm->first);
 	shm->first = QSMemNode(QSMemPtr((char*)&shm->head),QSMemPtr((char*)&shm->tail));
	shm->head.setFree(true);
	shm->first.setNextFree(QSMemPtr());
 	shm->tail = QSMemNode(QSMemPtr((char*)&shm->first),QSMemPtr());
	shm->tail.setFree(false);
	shm->tail.setNextFree(QSMemPtr());
#ifdef THROW_AWAY_UNUSED_PAGES
	int freePageStart = PAGE_ALIGN((int)&shm->first + sizeof(QSMemNode));
	int freePagesLength = PAGE_ALIGN((int)&shm->tail) - freePageStart;
	if ( freePagesLength ) {
# ifdef DEBUG_SHARED_MEMORY_CACHE
	    qDebug("Initially marking free pages as not needed");
# endif // DEBUG_SHARED_MEMORY_CACHE
	    if ( madvise((void*)freePageStart, freePagesLength, MADV_DONTNEED) ) {
		perror("madvise of shared memory");
		qWarning("madvise error with marking shared memory pages as not needed");
	    }
	}
#endif // THROW_AWAY_UNUSED_PAGES
	cache->init();
    }

    CHECK_CONSISTENCY_A("Initial");

#ifdef USE_EVIL_SIGNAL_HANDLERS
    for ( uint i = 0; i < sizeof(signalList)/sizeof(int); i++ ) {
	struct sigaction act;
	struct sigaction oldact;
	act.sa_handler = cleanUpCacheSignalHandler;
	act.sa_flags = SA_ONESHOT | SA_ONSTACK;
	if ( sigaction(i, &act, &oldact) == 0 )
	    oldHandlers[i] = oldact.sa_handler;
	else
	    oldHandlers[i] = 0;
    }
#endif // USE_EVIL_SIGNAL_HANDLERS
}


QSharedMemoryManager::~QSharedMemoryManager()
{
    // Detach from shared memory 
    if ( qApp->type() == QApplication::GuiServer )
	shmctl(shmId, IPC_RMID, 0); // Mark for deletion
    shmdt((char*)shm);
#ifdef DEBUG_SHARED_MEMORY_CACHE
    qDebug("Disconnected from shm at: %p", shm);
#endif // DEBUG_SHARED_MEMORY_CACHE 
    shm = 0;
}


#ifdef DEBUG_SHARED_MEMORY_CACHE

#define SHARED_MEMORY_CHECK( condition ) \
    if ( !(condition) ) {\
	qDebug("%s Consistency Check (line %i) fails test: %s (line %i)", msg, line, #condition, __LINE__ );\
	return false;\
    }

bool QSharedMemoryManager::checkConsistency(const char *msg, const char * /*file*/, int line)
{
    SHARED_MEMORY_CHECK( shm->head.isValid() );
    SHARED_MEMORY_CHECK( shm->first.isValid() );
    SHARED_MEMORY_CHECK( shm->tail.isValid() );

    SHARED_MEMORY_CHECK( !shm->head.prev() );
    SHARED_MEMORY_CHECK( shm->head.isFree() );
    SHARED_MEMORY_CHECK( shm->head.next() == &shm->first );
    SHARED_MEMORY_CHECK( !shm->tail.isFree() );
    SHARED_MEMORY_CHECK( !shm->tail.nextFree() );
    SHARED_MEMORY_CHECK( !shm->tail.next() );

    QSMemNode *prev = &shm->head;
    QSMemNode *node = &shm->first;
    QSMemNode *nextFree = prev->nextFree();
    while (node) {
	SHARED_MEMORY_CHECK( (bool)prev );
	SHARED_MEMORY_CHECK( (bool)node );
	SHARED_MEMORY_CHECK( prev->isValid() );
	SHARED_MEMORY_CHECK( node->isValid() );
	SHARED_MEMORY_CHECK( prev->next() == node );
	if ( node->isFree() ) {
	    SHARED_MEMORY_CHECK( node == nextFree );
	    nextFree = node->nextFree();
	} else {
	    SHARED_MEMORY_CHECK( node != nextFree );
	}
	prev = node;
	node = node->next();
    }
    SHARED_MEMORY_CHECK( !(bool)nextFree );
    SHARED_MEMORY_CHECK( prev == &shm->tail );
    return true;
}


void QSharedMemoryManager::rebuildFreeList()
{
    // rebuild free list
    QSMemNode *node = &shm->tail;
    node = node->prev();
    QSMemNode *lastFree = 0;
    while ( node && node != &shm->head ) {
	ASSERT(node->isValid());
	if ( node->isFree() ) {
	    node->setNextFree(lastFree);
	    lastFree = node;
	}
	node = node->prev();
    }
    ASSERT( node == &shm->head );
    node->setNextFree(lastFree);
    return;
}
#endif // DEBUG_SHARED_MEMORY_CACHE
 

QSMemPtr QSharedMemoryManager::alloc(int size, bool needLock)
{
    CHECK_CONSISTENCY_A("Pre Alloc");
    QSMemPtr ret;
    if ( needLock ) {
	QLockHolder lh(lock(),QLock::Write);
	ret = internal_alloc(size);
    } else {
	ret = internal_alloc(size);
    }
    CHECK_CONSISTENCY_A("Post Alloc");
    return ret;
}


QSMemPtr QSharedMemoryManager::internal_alloc(int size)
{
    if ( !size )
	return QSMemPtr();

    size = (size+sizeof(QSMemNode)+3)&~0x3;
    QSMemNode *node = &shm->head;
    QSMemNode *prevFree = node;
    while ( node ) {
	if ( !node->isValid() || !node->isFree() ) {
	    qDebug("QSharedMemoryManager::alloc inconsistency error, possible memory corruption");
	} else {
	    if ( node->size() == size ) {
		prevFree->setNextFree(node->nextFreePtr());
		node->setFree(false);
		return QSMemPtr((char*)node + sizeof(QSMemNode));
	    } else if ( node->size() >= size + (int)sizeof(QSMemNode) ) {

		QSMemNode *newNode = (QSMemNode*)((char*)node + size);
		*newNode = QSMemNode(QSMemPtr((char*)node),node->nextPtr());
		if ( node->next() ) 
		    node->next()->setPrev(QSMemPtr((char*)newNode));
		node->setNext(QSMemPtr((char*)newNode));

		newNode->setNextFree(node->nextFreePtr());
		prevFree->setNextFree(newNode);

		node->setFree(false);
		return QSMemPtr((char*)node + sizeof(QSMemNode));
	    }
	}
	prevFree = node;
	node = node->nextFree();
    }

//    qDebug("no free holes in shared memory");
    return QSMemPtr();
}


void QSharedMemoryManager::free(QSMemPtr ptr, bool needLock)
{
    CHECK_CONSISTENCY_A("Pre Free");
    if ( needLock ) {
	QLockHolder lh(lock(),QLock::Write);
	internal_free(ptr);
    } else {
	internal_free(ptr);
    }
    CHECK_CONSISTENCY_A("Post Free");
}


void QSharedMemoryManager::internal_free(QSMemPtr ptr)
{
//qDebug("free");
    QSMemNode *node = (QSMemNode *)((char*)ptr - sizeof(QSMemNode));
    if ( !node->isValid() ) {
	qDebug("free of non QSharedMemoryManager::alloc'd block");
	return;
    }

    node->setFree(true);

    // Consolidate to the left
    while ( node != &shm->first && node->isFree() && node->prev() && node->prev()->isFree() ) {
	QSMemNode *prev = node->prev();

	QSMemNode *next = node->next();
	if ( !node->isValid() )
	    qDebug("QSharedMemoryManager::free consistency check failed A");
	if ( next && !next->isValid() )
	    qDebug("QSharedMemoryManager::free consistency check failed B");
	if ( !prev->isValid() )
	    qDebug("QSharedMemoryManager::free consistency check failed C");
	if ( next )
	    next->setPrev(prev);
	prev->setNext(next);
	node->invalidate();

	node = prev;
    }

    QSMemNode *newFreeNode = node;

    // Consolidate to the right
    while ( node->isFree() && node->next() && node->next()->isFree() ) {
	QSMemNode *prev = node;
	node = node->next();

	QSMemNode *next = node->next();
	if ( !node->isValid() )
	    qWarning("QSharedMemoryManager::free consistency check failed D");
	if ( next && !next->isValid() )
	    qWarning("QSharedMemoryManager::free consistency check failed E");
	if ( !prev->isValid() )
	    qWarning("QSharedMemoryManager::free consistency check failed F");
	if ( next )
	    next->setPrev(prev);
	prev->setNext(next);
	node->invalidate();
    }

    // fix up free list
    // set the new free node to point to the next one
    node = newFreeNode->next();

#ifdef THROW_AWAY_UNUSED_PAGES
    int freePageStart = PAGE_ALIGN((int)newFreeNode+sizeof(QSMemNode));
    int freePagesLength = PAGE_ALIGN((int)node) - freePageStart;
    if ( freePagesLength ) {
#ifdef DEBUG_SHARED_MEMORY_CACHE
	qDebug("Marking pages not needed");
#endif // DEBUG_SHARED_MEMORY_CACHE
	if ( madvise((void*)freePageStart, freePagesLength, MADV_DONTNEED) ) {
	    perror("madvise of shared memory");
	    qWarning("madvise error with marking shared memory pages as not needed");
	}
    }
#endif // THROW_AWAY_UNUSED_PAGES

    newFreeNode->setNextFree(QSMemPtr());
    while ( node ) {
	if ( !node->isValid() )
	    qWarning("QSharedMemoryManager::free consistency check failed G");
	if ( node->isFree() ) {
	    newFreeNode->setNextFree(node);
	    break;
	}
	node = node->next();
    }

    // set the first free node before the new one to point to the new one
    node = newFreeNode->prev();
    while ( node ) {
	if ( !node->isValid() )
	    qWarning("QSharedMemoryManager::free consistency check failed H");

	if ( node->isFree() ) {
	    node->setNextFree(newFreeNode);
	    return;
	}
	node = node->prev();
    }

    // We should have eventually found the head node which should be free
    qWarning("QSharedMemoryManager::free consistency check failed I");
}

 
// Double Hashing is what is implemented, we use increment as our probe variable
void QSharedMemoryCache::hash(const char *key, int &hash, int &increment)
{
    int i = 0;
    hash = 1;
    while (key[i]) { 
	hash += i * 131 + key[i];
	i++;
    }
    increment = hash + 31;
    if (i)
	increment -= key[i-1];
    hash %= SHM_ITEMS;
    increment %= SHM_ITEMS - 1;
    if ( increment == 0 )
	increment = 31 % (SHM_ITEMS - 1);
}


#ifdef DEBUG_SHARED_MEMORY_CACHE
bool QSharedMemoryCache::checkCacheConsistency()
{
    for ( int i = 0; i < SHM_ITEMS; i++ ) {
	QSMemPtr memPtr = d->offsets[i];
	if ( (int)memPtr == MAGIC_HASH_DELETED_VAL )
	    continue;
	if ( !memPtr )
	    continue;
	QSMemNode *node = (QSMemNode *)((char *)memPtr - sizeof(QSMemNode));
	if ( !node->isValid() ) {
	    qWarning( "cache inconsistent");
	    return false;
	}
    }
    return true;

}
#endif // DEBUG_SHARED_MEMORY_CACHE


QSMCacheItemPtr QSharedMemoryCache::newItem(const char *key, int size, int type)
{
    CHECK_CACHE_CONSISTENCY();

    // qDebug("allocate for %s", key);
    bool retry = true;
    while ( retry ) {
	if ( d->items + 1 < d->maxItems ) {
	    QSMemPtr memPtr = qt_getSMManager()->alloc(size + sizeof(QSMCacheItem));
	    if ( memPtr ) {
		int strLen = strlen(key);
		QSMCacheItem *item = (QSMCacheItem*)(char*)memPtr;
		item->count = 1;
		item->type = (QSMCacheItem::QSMCacheItemType)type;
		item->key = qt_getSMManager()->alloc(strLen+1);
		if (!item->key) {
		    cleanUp();
		    item->key = qt_getSMManager()->alloc(strLen+1);
		    if (!item->key) {
			qt_getSMManager()->free(memPtr); 
#ifdef PROFILE_SHARED_MEMORY_CACHE
			d->cacheInsertMisses++;
#endif
			return QSMCacheItemPtr();
		    }
		}
		d->items++;
		memcpy((char*)item->key, key, strLen+1);
		QLockHolder lh(qt_getSMManager()->lock(),QLock::Write);
		int hashIndex, hashInc;
		hash(key, hashIndex, hashInc);
		// qDebug("hash for %s is %i", key, hashIndex);
		QSMemPtr memPtr2 = d->offsets[hashIndex];
		// qDebug("new: starting item %i has value %i", hashIndex, (int)memPtr2);
		int firstReplaceHashIndex = -1;
		while (memPtr2 && (int)memPtr2 != MAGIC_HASH_DELETED_VAL) {
#ifdef PROFILE_SHARED_MEMORY_CACHE
		    if ( firstReplaceHashIndex == -1 )
			d->cacheInsertStrcmps++;
#endif
		    QSMCacheItemPtr item(memPtr2);
		    if ( firstReplaceHashIndex == -1 && !qstrcmp(key, item.key()) ) {
#ifdef DEBUG_SHARED_MEMORY_CACHE
			qDebug("found a match already in the pixmap cache, replacing case");
#endif
			// Well actually, can't just remove it because other processess may
			// have references to it, they perhaps have active pixmap data connected
			// to this shared memory, but it will go away when they release it and
			// it's refcount reaches 0. What we can do instead is make sure when
			// we look up in the cache we hit this new one first instead of the
			// old one, so we insert it earlier, ie swap the positions around in the
			// hash table to where we would put it otherwise.
			firstReplaceHashIndex = hashIndex;
		    }

		    hashIndex = (hashIndex + hashInc) % SHM_ITEMS;
		    memPtr2 = d->offsets[hashIndex];
		    // qDebug("new: next item %i has value %i", hashIndex, (int)memPtr2);
		}
		// qDebug("new: using item %i with value %i", hashIndex, (int)memPtr2);
		if ( firstReplaceHashIndex != -1 ) {
		    d->offsets[hashIndex] = d->offsets[firstReplaceHashIndex];
		    d->offsets[firstReplaceHashIndex] = memPtr;
		} else {
		    d->offsets[hashIndex] = memPtr;
		}

		CHECK_CACHE_CONSISTENCY();
#ifdef PROFILE_SHARED_MEMORY_CACHE
	        d->cacheInsertHits++;
#endif
		// qDebug("allocated %s to index %i", key, hashIndex);
		return QSMCacheItemPtr(item);
	    }
	}
#ifdef DEBUG_SHARED_MEMORY_CACHE
	//	qDebug("no more objects allowed in shared memory cache");
	qDebug("error allocing %i bytes", size);
#endif
	retry = cleanUp();
    }

    CHECK_CACHE_CONSISTENCY();
#ifdef PROFILE_SHARED_MEMORY_CACHE
    d->cacheInsertMisses++;
#endif
    return QSMCacheItemPtr();
}


// XXX "type" is currently ignored, but could be used to find the hash table to look in
QSMCacheItemPtr QSharedMemoryCache::findItem(const char *keyStr, bool ref, int /*type*/)
{
    QLockHolder lh(qt_getSMManager()->lock(),QLock::Read);

    CHECK_CACHE_CONSISTENCY();

//    qDebug("seaching for %s", keyStr);
    // search the shared memory
    int hashIndex, hashInc;
    hash(keyStr, hashIndex, hashInc);
    QSMemPtr memPtr = d->offsets[hashIndex];
//    qDebug("find: starting item %i has value %i", hashIndex, (int)memPtr);
    while (memPtr) {
	if ( (int)memPtr != MAGIC_HASH_DELETED_VAL ) {
	    QSMCacheItemPtr item(memPtr);
//	    qDebug("key %s index: %i item %i", keyStr, hashIndex, (int)memPtr);
//	    qDebug("comparing %s with %s (hash: %i)", keyStr, item.key(), hashIndex);
#ifdef PROFILE_SHARED_MEMORY_CACHE
	    d->cacheFindStrcmps++;
#endif
	    if ( !qstrcmp(keyStr, item.key()) ) {
		// Item was deref'd and put in free list, remove from free list
		if ( item.count() == 0 ) {
		    for ( int i = 0; i < d->freeItemCount; i++ ) {
			if ((int)d->freeItems[i] == (int)memPtr) {
#ifdef DEBUG_SHARED_MEMORY_CACHE
			    qDebug("found %s in the free list, fixing up the free list", item.key());
#endif
			    memmove(&d->freeItems[i],&d->freeItems[i+1],(d->freeItemCount-i)*sizeof(QSMemPtr));
			    d->freeItemCount--;
			    break;
			}
		    }
		}
		if ( ref )
		    item.ref();
//		qDebug("using: using item %i with value %i", hashIndex, (int)memPtr);
//		qDebug("found %s (hash: %i refcount: %i)", item.key(), hashIndex, item.count());
		CHECK_CACHE_CONSISTENCY();
#ifdef PROFILE_SHARED_MEMORY_CACHE
		d->cacheFindHits++;
#endif
		return item;
	    }
	}
	hashIndex = (hashIndex + hashInc) % SHM_ITEMS;
	memPtr = d->offsets[hashIndex];
//	qDebug("find: next item %i has value %i", hashIndex, (int)memPtr);
    }

    CHECK_CACHE_CONSISTENCY();
#ifdef PROFILE_SHARED_MEMORY_CACHE
    d->cacheFindMisses++;
#endif
//    qDebug("didn't find %s (hash: %i)", keyStr, hashIndex);
    return QSMCacheItemPtr();
}


class PixmapShmItem {
public:
    uint    w : 12;
    uint    h : 12;
    uint    rw : 12;
    uint    rh : 12;
    uint    d : 6;
    uint    bitmap : 1;
    uint    selfMask : 1;
    uint    hasMask : 1;
    uint    hasAlpha : 1;
    //uint    mostRecentId; // relative age of the pixmap
};


void derefSharedMemoryPixmap(uchar *data)
{
    QSMCacheItemPtr item((char*)data - sizeof(PixmapShmItem));
    item.deref();
}


const QString mangleRot(const QString &key)
{
    return QString("%1_$R%2").arg( key ).arg( qt_screen->transformOrientation() );
}


QPixmap *QSharedMemoryManager::findPixmap(const QString &key, bool ref) const
{
    const QString k = mangleRot(key);
    //qDebug("search for %s", k.latin1());

    if ( isGlobalPixmap(k) ) {
	// search the shared memory
	PixmapShmItem *item = (PixmapShmItem*)(char*)cache->findItem(k.latin1(), ref, QSMCacheItem::Pixmap);

	if ( item ) {
	    QPixmap *pm = new QPixmap;
	    QSize mappedSize = qt_screen->mapToDevice(QSize(item->w, item->h));
	    pm->data->count    = 0; // local reference count is 0 until it is assigned locally.
				    // the shared global pixmap cache has been ref'ed in the findItem
				    // call. When the local refcount reachs 0, the global refcount
				    // is decremented.
	    pm->data->uninit   = false;
	    pm->data->w	       = item->w;
	    pm->data->h	       = item->h;
	    pm->data->d	       = item->d;
	    pm->data->bitmap   = item->bitmap;
	    pm->data->hasAlpha = item->hasAlpha; 
	    pm->data->selfmask = item->selfMask;
	    pm->data->optim    = QPixmap::defOptim;
	    pm->data->rw       = mappedSize.width();
	    pm->data->rh       = mappedSize.height();
	    pm->data->id       = memorymanager->mapPixmapData((uchar*)item + sizeof(PixmapShmItem));
	    if ( pm->data->d <= 8 ) {
		pm->data->numcols = qt_screen->numCols();
		pm->data->clut = new QRgb[pm->data->numcols]; // Might be able to connect in shm
		memcpy(pm->data->clut, qt_screen->clut(), pm->data->numcols*sizeof(QRgb));
	    } else {
		pm->data->numcols = 0;
		pm->data->clut = 0;
	    }
	    pm->data->mask = 0;
	    if ( item->hasMask ) {
		QPixmap *bm = findPixmap(key + "_$M", ref);
		if ( bm ) {
		    pm->data->mask = new QBitmap;
		    *pm->data->mask = *bm;
		}
	    }
	    if ( !pm->data->mask && item->hasMask ) {
		item->hasMask = 0;
		qWarning("Mask for %s expected but not found in cache", k.latin1());
	    }
	    return pm;
	}
    }
    return 0;
}


bool QSharedMemoryManager::insertPixmap(const QString& key, const QPixmap *pm)
{
    const QString k = mangleRot(key);

    if ( isGlobalPixmap(k) ) {
	// insert mask if it has one
	if (pm->mask()) 
	    if (!insertPixmap(key + "_$M", (QPixmap*)pm->mask()))
		return false;

	int align = qt_screen->pixmapLinestepAlignment();
	int size = (((pm->data->rw*pm->data->d+align-1)/align)*align/8)*pm->data->rh + sizeof(PixmapShmItem);

	QSMCacheItemPtr cacheItem = cache->newItem(k.latin1(), size, QSMCacheItem::Pixmap);
	PixmapShmItem *item = (PixmapShmItem*)(char*)cacheItem;
 
 	if ( item ) {
	    cacheItem.setCount(1); // one reference, the app inserting this pixmap holds a local
			           // refcount, when the local refcount is 0, it will cause the
				   // shared global refcount to be deref'ed, when that is 0, it is
				   // marked for freeing from shared memory
	    item->w        = pm->data->w;
	    item->h        = pm->data->h;
	    item->d        = pm->data->d;
	    item->bitmap   = pm->data->bitmap;
	    item->hasAlpha = pm->data->hasAlpha;
	    item->selfMask = pm->data->selfmask;
	    item->hasMask  = pm->data->mask ? 1 : 0;

	    QMemoryManager::PixmapID id = memorymanager->mapPixmapData((uchar*)item + sizeof(PixmapShmItem));
            pm->data->copyData(id,pm->data->rw,pm->data->d);
	    memorymanager->deletePixmap(pm->data->id);
	    pm->data->id = id;

	    return true;
	}
    }
    return false;
}

#endif // QT_NO_QWS_SHARED_MEMORY_CACHE





class QPMCache : public QObject, public QCache<QPixmap>
{
public:
    QPMCache():
	QObject( 0, "local pixmap cache" ),
	QCache<QPixmap>( cache_limit * 1024, cache_size ),
	id( 0 ), ps( 0 ), t( false )
	{
	    qAddPostRoutine( cleanup_pixmap_cache );
	    setAutoDelete( TRUE );
	}
    ~QPMCache() {
    }

    void timerEvent(QTimerEvent *);
    bool insert(const QString& k, const QPixmap *d, bool canDelete);
    QPixmap *find(const QString &k, bool ref=TRUE) const;
private:
    int id;
    int ps;
    bool t;
};


static QPMCache *pm_cache = 0;			// global pixmap cache


QPixmap *QPMCache::find(const QString &k, bool ref) const
{
#ifndef QT_NO_QWS_SHARED_MEMORY_CACHE
    if ( k[0] == '_' || (k[0] == '$' && k.left(3) == "$qt")) {
	QPixmap *shmPix = qt_getSMManager()->findPixmap(k, ref);
	if ( shmPix )
	    return shmPix;
    }
#endif // QT_NO_QWS_SHARED_MEMORY_CACHE
    return QCache<QPixmap>::find(k,ref);
}


bool QPMCache::insert(const QString& k, const QPixmap *d, bool canDelete)
{
#ifndef QT_NO_QWS_SHARED_MEMORY_CACHE
    if ( k[0] == '_' || (k[0] == '$' && k.left(3) == "$qt")) {
	if ( canDelete && qt_getSMManager()->insertPixmap(k, d) ) {
	    delete d;
	    return true;
	}
    }
#endif // QT_NO_QWS_SHARED_MEMORY_CACHE
    int cost = d->width() * d->height() * d->depth() / 8;
    bool r = QCache<QPixmap>::insert(k, d, cost, 0);
    if ( r && !id ) {
	id = startTimer(30000);
	t = false;
    }
    if ( !r && canDelete )
	delete d;
    return r;
}


/*
  This is supposed to cut the cache size down by about 80-90% in a
  minute once the application becomes idle, to let any inserted pixmap
  remain in the cache for some time before it becomes a candidate for
  cleaning-up, and to not cut down the size of the cache while the
  cache is in active use.

  When the last pixmap has been deleted from the cache, kill the
  timer so Qt won't keep the CPU from going into sleep mode.
*/

void QPMCache::timerEvent( QTimerEvent * )
{
    int mc = maxCost();
    bool nt = totalCost() == ps;
    setMaxCost(nt ? totalCost() * 3 / 4 : totalCost() -1);
    setMaxCost(mc);
    ps = totalCost();

    if ( !count() ) {
	killTimer(id);
	id = 0;
    } else if ( nt != t ) {
	killTimer(id);
	id = startTimer(nt ? 10000 : 30000);
	t = nt;
    }
}


void cleanup_pixmap_cache()
{
    delete pm_cache;
    pm_cache = 0;
#ifndef QT_NO_QWS_SHARED_MEMORY_CACHE
# ifdef THROW_AWAY_UNUSED_PAGES
#  ifdef DEBUG_SHARED_MEMORY_CACHE
    qDebug("Cleaning up cache");
#  endif // DEBUG_SHARED_MEMORY_CACHE
#ifdef WHEN_APP_CLOSES_DESTROY_ANY_PIXMAPS_WITH_ZERO_REFERENCES
    while ( qt_getSMManager()->pixmapCache()->cleanUp() )
	/*do nothing*/;
#endif
# endif // THROW_AWAY_UNUSED_PAGES
#endif // QT_NO_QWS_SHARED_MEMORY_CACHE
}


static inline QPMCache *get_pm_cache()
{
    if ( !pm_cache ) {				// create pixmap cache
	pm_cache = new QPMCache;
	CHECK_PTR(pm_cache);
    }
    return pm_cache;
}


/*!
  Returns the pixmap associated with \a key in the cache, or null if there
  is no such pixmap.

  <strong>
    NOTE: if valid, you should copy the pixmap immediately (this is quick
    since QPixmaps are \link shclass.html implicitly shared\endlink), because
    subsequent insertions into the cache could cause the pointer to become
    invalid.  For this reason, we recommend you use
    find(const QString&, QPixmap&) instead.
  </strong>

  Example:
  \code
    QPixmap* pp;
    QPixmap p;
    if ( (pp=QPixmapCache::find("my_previous_copy", pm)) ) {
	p = *pp;
    } else {
	p.load("bigimage.png");
	QPixmapCache::insert("my_previous_copy", new QPixmap(p));
    }
    painter->drawPixmap(0, 0, p);
  \endcode
*/

QPixmap *QPixmapCache::find( const QString &key )
{
    return get_pm_cache()->find(key);
}


/*!
  Looks for a cached pixmap associated with \a key in the cache.  If a
  pixmap is found, the function sets \a pm to that pixmap and returns
  TRUE.  Otherwise, the function returns false and does not change \a
  pm.

  Example:
  \code
    QPixmap p;
    if ( !QPixmapCache::find("my_previous_copy", pm) ) {
	pm.load("bigimage.png");
	QPixmapCache::insert("my_previous_copy", pm);
    }
    painter->drawPixmap(0, 0, p);
  \endcode
*/

bool QPixmapCache::find( const QString &key, QPixmap& pm )
{
    QPixmap* p = get_pm_cache()->find(key);
    if ( p ) pm = *p;
    return !!p;
}


/*!
  \obsolete
  Inserts the pixmap \a pm associated with \a key into the cache.
  Returns TRUE if successful, or false if the pixmap is too big for the cache.

  <strong>
    NOTE: \a pm must be allocated on the heap (using \c new).

    If this function returns false, you must delete \a pm yourself.

    If this function returns TRUE, do not use \a pm afterwards or
    keep references to it, as any other insertions into the cache,
    from anywhere in the application, or within Qt itself, could cause
    the pixmap to be discarded from the cache, and the pointer to
    become invalid.

    Due to these dangers, we strongly recommend that you use
    insert(const QString&, const QPixmap&) instead.
  </strong>
*/

bool QPixmapCache::insert( const QString &key, QPixmap *pm )
{
    return get_pm_cache()->insert( key, pm, false );
}


/*!
  Inserts a copy of the pixmap \a pm associated with \a key into the cache.

  All pixmaps inserted by the Qt library have a key starting with "$qt..".
  Use something else for your own pixmaps.

  When a pixmap is inserted and the cache is about to exceed its limit, it
  removes pixmaps until there is enough room for the pixmap to be inserted.

  The oldest pixmaps (least recently accessed in the cache) are deleted
  when more space is needed.

  \sa setCacheLimit().
*/

void QPixmapCache::insert( const QString &key, const QPixmap& pm )
{
    get_pm_cache()->insert( key, new QPixmap(pm), true );
}


/*!
  Returns the cache limit (in kilobytes).

  The default setting is 1024 kilobytes.

  \sa setCacheLimit().
*/

int QPixmapCache::cacheLimit()
{
    return cache_limit;
}


/*!
  Sets the cache limit to \a n kilobytes.

  The default setting is 1024 kilobytes.

  \sa cacheLimit()
*/

void QPixmapCache::setCacheLimit( int n )
{
    cache_limit = n;
    get_pm_cache()->setMaxCost( 1024*cache_limit );
}


/*!
  Removes all pixmaps from the cache.
*/

void QPixmapCache::clear()
{
    get_pm_cache()->clear();
}

