/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
