/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef Q3CLEANUPHANDLER_H
#define Q3CLEANUPHANDLER_H

#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

template<class T>
class Q3CleanupHandler
{
    QListData p;
public:
    inline Q3CleanupHandler()
    { p.d = 0; }
    ~Q3CleanupHandler()
    {
        if (p.d) {
            for (int i = 0; i < p.size(); ++i) {
                T** t = static_cast<T**>(*p.at(i));
                delete *t;
                *t = 0;
            }
            qFree(p.d);
            p.d = 0;
        }
    }

    T* add(T **object)
    {
        if (!p.d) {
            p.d = &QListData::shared_null;
            p.d->ref.ref();
            p.detach();
        }
        *p.prepend() =  object;
        return *object;
    }
    void remove(T **object)
    {
        if (p.d)
            for (int i = 0; i < p.size(); ++i)
                if (*p.at(i) == object)
                    p.remove(i--);
    }
};

template<class T>
class Q3SingleCleanupHandler
{
    T **object;
public:
    inline Q3SingleCleanupHandler()
    : object(0) {}
    inline ~Q3SingleCleanupHandler()
    { if (object) { delete *object; *object = 0; } }
    inline T* set(T **o)
    { object = o; return *object; }
    inline void reset() { object = 0; }
};

QT_END_NAMESPACE

QT_END_HEADER

#endif //Q3CLEANUPHANDLER_H
