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

#ifndef Q3DEEPCOPY_H
#define Q3DEEPCOPY_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

template <class T>
class Q3DeepCopy
{
public:
    inline Q3DeepCopy()
    {
    }

    inline Q3DeepCopy(const T &t)
	: deepcopy(t)
    {
	deepcopy.detach();
    }

    inline Q3DeepCopy<T> &operator=(const T &t)
    {
	deepcopy = t;
	deepcopy.detach();
	return *this;
    }

    inline operator T ()
    {
	T tmp = deepcopy;
	tmp.detach();
	return tmp;
    }

private:
    T deepcopy;
};

QT_END_HEADER

#endif // Q3DEEPCOPY_H
