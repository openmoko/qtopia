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

#include "qthreadstorage.h"

#ifndef QT_NO_THREAD
#include "qthread.h"
#include "qthread_p.h"
#include "qmutex.h"

#include <string.h>

// #define THREADSTORAGE_DEBUG
#ifdef THREADSTORAGE_DEBUG
#  define DEBUG qDebug
#else
#  define DEBUG if(false)qDebug
#endif


// 256 maximum + 1 used in QRegExp
static const int MAX_THREAD_STORAGE = 257;

Q_GLOBAL_STATIC(QMutex, mutex)

static bool thread_storage_init = false;
static struct {
    bool used;
    void (*func)(void *);
} thread_storage_usage[MAX_THREAD_STORAGE];


QThreadStorageData::QThreadStorageData(void (*func)(void *))
    : id(0)
{
    QMutexLocker locker(mutex());

    // make sure things are initialized
    if (! thread_storage_init)
        memset(thread_storage_usage, 0, sizeof(thread_storage_usage));
    thread_storage_init = true;

    for (; id < MAX_THREAD_STORAGE; ++id) {
        if (!thread_storage_usage[id].used)
            break;
    }

    Q_ASSERT(id >= 0 && id < MAX_THREAD_STORAGE);
    thread_storage_usage[id].used = true;
    thread_storage_usage[id].func = func;

    DEBUG("QThreadStorageData: Allocated id %d", id);
}

QThreadStorageData::~QThreadStorageData()
{
    QMutexLocker locker(mutex());

    // thread_storage_usage[id].used = false;
    thread_storage_usage[id].func = 0;

    DEBUG("QThreadStorageData: Released id %d", id);
}

void **QThreadStorageData::get() const
{
    QThreadData *data = QThreadData::current();
    if (!data) {
        qWarning("QThreadStorage::get: QThreadStorage can only be used with threads started with QThread");
        return 0;
    }
    return data->tls && data->tls[id] ? &data->tls[id] : 0;
}

void **QThreadStorageData::set(void *p)
{
    QThreadData *data = QThreadData::current();
    if (!data) {
        qWarning("QThreadStorage::set: QThreadStorage can only be used with threads started with QThread");
        return 0;
    }
    if (!data->tls) {
        DEBUG("QThreadStorageData: Allocating storage %d for thread %p",
              id, QThread::currentThread());

        data->tls = new void*[MAX_THREAD_STORAGE];
        memset(data->tls, 0, sizeof(void*) * MAX_THREAD_STORAGE);
    }

    // delete any previous data
    if (data->tls[id]) {
        DEBUG("QThreadStorageData: Deleting previous storage %d for thread %p",
              id, QThread::currentThread());

        void *q = data->tls[id];
        data->tls[id] = 0;
        thread_storage_usage[id].func(q);
    }

    // store new data
    data->tls[id] = p;
    DEBUG("QThreadStorageData: Set storage %d for thread %p to %p",
          id, QThread::currentThread(), p);
    return &data->tls[id];
}

void QThreadStorageData::finish(void **tls)
{
    if (!tls) return; // nothing to do

    DEBUG("QThreadStorageData: Destroying storage for thread %p",
          QThread::currentThread());

    for (int i = 0; i < MAX_THREAD_STORAGE; ++i) {
        if (!tls[i]) continue;
        if (!thread_storage_usage[i].func) {
            qWarning("QThreadStorage: Thread %p exited after QThreadStorage destroyed",
                     QThread::currentThread());
            continue;
        }

        void *q = tls[i];
        tls[i] = 0;
        thread_storage_usage[i].func(q);
    }

    delete [] tls;
}

/*!
    \class QThreadStorage
    \brief The QThreadStorage class provides per-thread data storage.

    \threadsafe

    \ingroup thread
    \ingroup environment
    \mainclass

    QThreadStorage is a template class that provides per-thread data
    storage.

    \e{Note that due to compiler limitations, QThreadStorage can only
    store pointers.}

    The setLocalData() function stores a single thread-specific value
    for the calling thread. The data can be accessed later using
    localData(). QThreadStorage takes ownership of the data (which
    must be created on the heap with \c new) and deletes it when the
    thread exits, either normally or via termination.

    The hasLocalData() function allows the programmer to determine if
    data has previously been set using the setLocalData() function.
    This is also useful for lazy initializiation.

    For example, the following code uses QThreadStorage to store a
    single cache for each thread that calls the cacheObject() and
    removeFromCache() functions. The cache is automatically
    deleted when the calling thread exits.

    \quotefromfile snippets/threads/threads.cpp
    \skipto QThreadStorage<
    \printuntil cacheObject
    \printuntil removeFromCache
    \printuntil /^\}$/

    \section1 Caveats

    \list

    \o As noted above, QThreadStorage can only store pointers due to
    compiler limitations.

    \o The QThreadStorage destructor does not delete per-thread data.
    QThreadStorage only deletes per-thread data when the thread exits
    or when setLocalData() is called multiple times.

    \o QThreadStorage can only be used with threads started with
    QThread. It cannot be used with threads started using
    platform-specific APIs.

    \o As a corollary to the above, platform-specific APIs cannot be
    used to exit or terminate a QThread using QThreadStorage. Doing so
    will cause all per-thread data to be leaked. See QThread::exit()
    and QThread::terminate().

    \o QThreadStorage can be used to store data for the \c main()
    thread after QApplication has been constructed. QThreadStorage
    deletes all data set for the \c main() thread when QApplication is
    destroyed, regardless of whether or not the \c main() thread has
    actually finished.

    \o The implementation of QThreadStorage limits the total number of
    QThreadStorage objects to 256. An unlimited number of threads
    can store per-thread data in each QThreadStorage object.

    \endlist

    \sa QThread
*/

/*!
    \fn QThreadStorage::QThreadStorage()

    Constructs a new per-thread data storage object.
*/

/*!
    \fn QThreadStorage::~QThreadStorage()

    Destroys the per-thread data storage object.

    Note: The per-thread data stored is not deleted. Any data left
    in QThreadStorage is leaked. Make sure that all threads using
    QThreadStorage have exited before deleting the QThreadStorage.

    \sa hasLocalData()
*/

/*!
    \fn bool QThreadStorage::hasLocalData() const

    Returns true if the calling thread has non-zero data available;
    otherwise returns false.

    \sa localData()
*/

/*!
    \fn T &QThreadStorage::localData()

    Returns a reference to the data that was set by the calling
    thread.

    Note: QThreadStorage can only store pointers. This function
    returns a reference to the pointer that was set by the calling
    thread. The value of this reference is 0 if no data was set by
    the calling thread,

    \sa hasLocalData()
*/

/*!
    \fn const T QThreadStorage::localData() const
    \overload

    Returns a copy of the data that was set by the calling thread.

    Note: QThreadStorage can only store pointers. This function
    returns a pointer to the data that was set by the calling thread.
    If no data was set by the calling thread, this function returns 0.

    \sa hasLocalData()
*/

/*!
    \fn void QThreadStorage::setLocalData(T data)

    Sets the local data for the calling thread to \a data. It can be
    accessed later using the localData() functions.

    If \a data is 0, this function deletes the previous data (if
    any) and returns immediately.

    If \a data is non-zero, QThreadStorage takes ownership of the \a
    data and deletes it automatically either when the thread exits
    (either normally or via termination) or when setLocalData() is
    called again.

    Note: QThreadStorage can only store pointers. The \a data
    argument must be either a pointer to an object created on the heap
    (i.e. using \c new) or 0. You should not delete \a data
    yourself; QThreadStorage takes ownership and will delete the \a
    data itself.

    \sa localData(), hasLocalData()
*/

#endif // QT_NO_THREAD
