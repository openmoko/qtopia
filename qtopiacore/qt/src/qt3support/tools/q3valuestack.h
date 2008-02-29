/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3VALUESTACK_H
#define Q3VALUESTACK_H

#include <Qt3Support/q3valuelist.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

template<class T>
class Q3ValueStack : public Q3ValueList<T>
{
public:
    Q3ValueStack() {}
   ~Q3ValueStack() {}
    void  push(const T& val) { this->append(val); }
    T pop()
    {
        T elem(this->last());
        if (!this->isEmpty())
            this->remove(this->fromLast());
        return elem;
    }
    T& top() { return this->last(); }
    const T& top() const { return this->last(); }
};

QT_END_HEADER

#endif // Q3VALUESTACK_H
