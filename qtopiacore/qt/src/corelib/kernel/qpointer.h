/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPOINTER_H
#define QPOINTER_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

template <class T>
class QPointer
{
    QObject *o;
public:
    inline QPointer() : o(0) {}
    inline QPointer(T *p) : o(p)
        { QMetaObject::addGuard(&o); }
    inline QPointer(const QPointer<T> &p) : o(p.o)
        { QMetaObject::addGuard(&o); }
    inline ~QPointer()
        { QMetaObject::removeGuard(&o); }
    inline QPointer<T> &operator=(const QPointer<T> &p)
        { if (this != &p) QMetaObject::changeGuard(&o, p.o); return *this; }
    inline QPointer<T> &operator=(T* p)
        { if (o != p) QMetaObject::changeGuard(&o, p); return *this; }

    inline bool isNull() const
        { return !o; }

    inline T* operator->() const
        { return static_cast<T*>(const_cast<QObject*>(o)); }
    inline T& operator*() const
        { return *static_cast<T*>(const_cast<QObject*>(o)); }
    inline operator T*() const
        { return static_cast<T*>(const_cast<QObject*>(o)); }
};


template <class T>
inline bool operator==(const T *o, const QPointer<T> &p)
{ return o == p.operator->(); }

#ifndef Q_CC_SUN // ambiguity between const T * and T *
template<class T>
inline bool operator==(const QPointer<T> &p, const T *o)
{ return p.operator->() == o; }
#endif

template <class T>
inline bool operator==(T *o, const QPointer<T> &p)
{ return o == p.operator->(); }

template<class T>
inline bool operator==(const QPointer<T> &p, T *o)
{ return p.operator->() == o; }

template<class T>
inline bool operator==(const QPointer<T> &p1, const QPointer<T> &p2)
{ return p1.operator->() == p2.operator->(); }


template <class T>
inline bool operator!=(const T *o, const QPointer<T> &p)
{ return o != p.operator->(); }

#ifndef Q_CC_SUN // ambiguity between const T * and T *
template<class T>
inline bool operator!= (const QPointer<T> &p, const T *o)
{ return p.operator->() != o; }
#endif

template <class T>
inline bool operator!=(T *o, const QPointer<T> &p)
{ return o != p.operator->(); }

template<class T>
inline bool operator!= (const QPointer<T> &p, T *o)
{ return p.operator->() != o; }

template<class T>
inline bool operator!= (const QPointer<T> &p1, const QPointer<T> &p2)
{ return p1.operator->() != p2.operator->() ; }

// Make MSVC < 1400 (2005) handle "if (NULL == p)" syntax
#if defined(Q_CC_MSVC) && (_MSC_VER < 1400)
template<class T>
inline bool operator== (int i, const QPointer<T> &p)
{ Q_ASSERT(i == 0); return !i && p.isNull(); }

template<class T>
inline bool operator!= (int i, const QPointer<T> &p)
{ Q_ASSERT(i == 0); return !i && !p.isNull(); }
#endif

QT_END_HEADER

#endif // QPOINTER_H
