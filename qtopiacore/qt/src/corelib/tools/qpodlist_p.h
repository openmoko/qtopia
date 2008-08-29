/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPODLIST_P_H
#define QPODLIST_P_H

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

#include <QtCore/qcontainerfwd.h>
#include <QtCore/qglobal.h>
#include <new>

QT_BEGIN_HEADER

QT_MODULE(Core)

template<class T, int Prealloc>
class QPodList
{
public:
    inline explicit QPodList(int size = 0);

    inline QPodList(const QPodList<T, Prealloc> &other)
        : a(Prealloc), s(0), ptr(reinterpret_cast<T *>(array))
    {
        append(other.constData(), other.size());
    }

    inline ~QPodList() {
        if (ptr != reinterpret_cast<T *>(array))
            qFree(ptr);
    }
    inline QPodList<T, Prealloc> &operator=(const QPodList<T, Prealloc> &other)
    {
        if (this != &other) {
            clear();
            append(other.constData(), other.size());
        }
        return *this;
    }

    inline int size() const { return s; }
    inline int count() const { return s; }
    inline bool isEmpty() const { return (s == 0); }
    inline void resize(int size);
    inline void clear() { resize(0); }

    inline int capacity() const { return a; }
    inline void reserve(int size);

    inline T &operator[](int idx) {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }
    inline const T &operator[](int idx) const {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }

    inline const T &at(int idx) const {
        Q_ASSERT(idx >= 0 && idx < s);
        return ptr[idx];
    }

    inline const T &first() const {
        return at(0);
    }

    inline T& append() {
        const int idx = s++;
        if (s == a)
            realloc(s, s<<1);
        return ptr[idx];
    }
    inline void append(const T &t) {
        append() = t;
    }

    inline T& insert(int idx) {
        Q_ASSERT(idx >= 0 && idx <= s);
        const int sz = s++;
        if (s == a)
            realloc(s, s<<1);
        ::memmove(ptr + idx + 1, ptr + idx, (sz - idx) * sizeof(T));
        return ptr[idx];
    }
    inline void insert(int idx, const T &t) {
        insert(idx) = t;
    }

    inline void removeAt(int idx) {
        Q_ASSERT(idx >= 0 && idx < s);
        ::memmove(ptr + idx, ptr + idx + 1, (s - idx - 1) * sizeof(T));
        --s;
    }

    inline void removeAll(const T &t) {
        int i = 0;
        for (int j = 0; j < s; ++j) {
            if (ptr[j] != t)
                ptr[i++] = ptr[j];
        }
        s = i;
    }

    inline int indexOf(const T &t, int from = 0) const {
        if (from < 0)
            from = qMax(from + s, 0);
        if (from < s) {
            const T *n = ptr + from - 1;
            const T *e = ptr + s;
            while (++n != e)
                if (*n == t)
                    return n - ptr;
        }
        return -1;
    }

    inline bool contains(const T &t) const {
        return indexOf(t) >= 0;
    }

    inline T takeFirst() {
        Q_ASSERT(s > 0);
        T tmp = ptr[0];
        removeAt(0);
        return tmp;
    }

    inline T *data() { return ptr; }
    inline const T *data() const { return ptr; }
    inline const T * constData() const { return ptr; }

private:
    void append(const T *buf, int size);
    void realloc(int size, int alloc);

    int a;
    int s;
    T *ptr;
    union {
        // ### Qt 5: Use 'Prealloc * sizeof(T)' as array size
        char array[sizeof(qint64) * (((Prealloc * sizeof(T)) / sizeof(qint64)) + 1)];
        qint64 q_for_alignment_1;
        double q_for_alignment_2;
    };
};

template <class T, int Prealloc>
Q_INLINE_TEMPLATE QPodList<T, Prealloc>::QPodList(int asize)
    : s(asize) {
    if (s > Prealloc) {
        ptr = reinterpret_cast<T *>(qMalloc(s * sizeof(T)));
        a = s;
    } else {
        ptr = reinterpret_cast<T *>(array);
        a = Prealloc;
    }
}

template <class T, int Prealloc>
Q_INLINE_TEMPLATE void QPodList<T, Prealloc>::resize(int asize)
{ realloc(asize, qMax(asize, a)); }

template <class T, int Prealloc>
Q_INLINE_TEMPLATE void QPodList<T, Prealloc>::reserve(int asize)
{ if (asize > a) realloc(s, asize); }

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE void QPodList<T, Prealloc>::append(const T *abuf, int asize)
{
    Q_ASSERT(abuf);
    if (asize <= 0)
        return;

    const int idx = s;
    const int news = s + asize;
    if (news >= a)
        realloc(news, news<<1);
    else
        s = news;

    qMemCopy(&ptr[idx], abuf, asize * sizeof(T));
}

template <class T, int Prealloc>
Q_OUTOFLINE_TEMPLATE void QPodList<T, Prealloc>::realloc(int asize, int aalloc)
{
    Q_ASSERT(aalloc >= asize);
    T *oldPtr = ptr;
    int osize = s;
    s = asize;

    if (aalloc != a) {
        ptr = reinterpret_cast<T *>(qMalloc(aalloc * sizeof(T)));
        if (ptr) {
            a = aalloc;
            qMemCopy(ptr, oldPtr, osize * sizeof(T));
        } else {
            ptr = oldPtr;
            s = 0;
            asize = 0;
        }
    }

    if (oldPtr != reinterpret_cast<T *>(array) && oldPtr != ptr)
        qFree(oldPtr);
}

QT_END_HEADER

#endif // QPODLIST_P_H
