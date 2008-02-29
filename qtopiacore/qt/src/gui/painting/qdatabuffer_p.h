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

#ifndef QDATABUFFER_P_H
#define QDATABUFFER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qbytearray.h"

template <typename Type> class QDataBuffer
{
public:
    QDataBuffer(int res = 64)
    {
        capacity = res;
        buffer = (Type*) qMalloc(capacity * sizeof(Type));
        siz = 0;
    }

    ~QDataBuffer()
    {
        qFree(buffer);
    }

    inline void reset() { siz = 0; }

    inline bool isEmpty() const { return siz==0; }

    inline int size() const { return siz; }
    inline Type *data() const { return buffer; }

    inline const Type &at(int i) const { Q_ASSERT(i >= 0 && i < siz); return buffer[i]; }
    inline const Type &last() const { Q_ASSERT(!isEmpty()); return buffer[siz-1]; }
    inline const Type &first() const { Q_ASSERT(!isEmpty()); return buffer[0]; }

    inline void add(const Type &t) {
        if (siz >= capacity) {
            capacity *= 2;
            buffer = (Type*) qRealloc(buffer, capacity * sizeof(Type));
        }
        buffer[siz] = t;
        ++siz;
    }

    inline QDataBuffer &operator<<(const Type &t) { add(t); return *this; }

private:
    int capacity;
    int siz;
    Type *buffer;
};

#endif // QDATABUFFER_P_H
