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

#ifndef QREADWRITELOCK_H
#define QREADWRITELOCK_H

#include <QtCore/qglobal.h>
#include <limits.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

#ifndef QT_NO_THREAD

struct QReadWriteLockPrivate;

class Q_CORE_EXPORT QReadWriteLock
{
public:
    QReadWriteLock();
    ~QReadWriteLock();

    void lockForRead();
    bool tryLockForRead();

    void lockForWrite();
    bool tryLockForWrite();

    void unlock();

private:
    Q_DISABLE_COPY(QReadWriteLock)
    QReadWriteLockPrivate *d;
};

class Q_CORE_EXPORT QReadLocker
{
public:
    inline QReadLocker(QReadWriteLock *readWriteLock);

    inline ~QReadLocker()
    { unlock(); }

    inline void unlock()
    {
        if (q_lock) {
            if ((q_val & quintptr(1u)) == quintptr(1u)) {
                q_val &= ~quintptr(1u);
                q_lock->unlock();
            }
        }
    }

    inline void relock()
    {
        if (q_lock) {
            if ((q_val & quintptr(1u)) == quintptr(0u)) {
                q_lock->lockForRead();
                q_val |= quintptr(1u);
            }
        }
    }

    inline QReadWriteLock *readWriteLock() const
    { return reinterpret_cast<QReadWriteLock *>(q_val & ~quintptr(1u)); }

private:
    Q_DISABLE_COPY(QReadLocker)
    union {
        QReadWriteLock *q_lock;
        quintptr q_val;
    };
};

inline QReadLocker::QReadLocker(QReadWriteLock *areadWriteLock)
    : q_lock(areadWriteLock)
{ relock(); }

class Q_CORE_EXPORT QWriteLocker
{
public:
    inline QWriteLocker(QReadWriteLock *readWriteLock);

    inline ~QWriteLocker()
    { unlock(); }

    inline void unlock()
    {
        if (q_lock) {
            if ((q_val & quintptr(1u)) == quintptr(1u)) {
                q_val &= ~quintptr(1u);
                q_lock->unlock();
            }
        }
    }

    inline void relock()
    {
        if (q_lock) {
            if ((q_val & quintptr(1u)) == quintptr(0u)) {
                q_lock->lockForWrite();
                q_val |= quintptr(1u);
            }
        }
    }

    inline QReadWriteLock *readWriteLock() const
    { return reinterpret_cast<QReadWriteLock *>(q_val & ~quintptr(1u)); }

private:
    Q_DISABLE_COPY(QWriteLocker)
    union{
        QReadWriteLock *q_lock;
        quintptr q_val;
    };
};

inline QWriteLocker::QWriteLocker(QReadWriteLock *areadWriteLock)
    : q_lock(areadWriteLock)
{ relock(); }

#else // QT_NO_THREAD

class Q_CORE_EXPORT QReadWriteLock
{
public:
    inline explicit QReadWriteLock() { }
    inline ~QReadWriteLock() { }

    static inline void lockForRead() { }
    static inline bool tryLockForRead() { return true; }

    static inline void lockForWrite() { }
    static inline bool tryLockForWrite() { return true; }

    static inline void unlock() { }

private:
    Q_DISABLE_COPY(QReadWriteLock)
};

class Q_CORE_EXPORT QReadLocker
{
public:
    inline QReadLocker(QReadWriteLock *) { }
    inline ~QReadLocker() { }

    static inline void unlock() { }
    static inline void relock() { }
    static inline QReadWriteLock *readWriteLock() { return 0; }

private:
    Q_DISABLE_COPY(QReadLocker)
};

class Q_CORE_EXPORT QWriteLocker
{
public:
    inline explicit QWriteLocker(QReadWriteLock *) { }
    inline ~QWriteLocker() { }

    static inline void unlock() { }
    static inline void relock() { }
    static inline QReadWriteLock *readWriteLock() { return 0; }

private:
    Q_DISABLE_COPY(QWriteLocker)
};

#endif // QT_NO_THREAD

QT_END_HEADER

#endif // QREADWRITELOCK_H
