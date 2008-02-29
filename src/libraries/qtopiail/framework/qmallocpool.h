/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef _QMALLOCPOOL_H_
#define _QMALLOCPOOL_H_

#include <cstdlib>
#include <QString>

#include "qtopiailglobal.h"

class QMallocPoolPrivate;

class QTOPIAIL_EXPORT QMallocPool
{
public:
    enum PoolType { Owned, NewShared, Shared };
    QMallocPool();
    QMallocPool(void * poolBase, unsigned int poolLength,
                PoolType type = Owned, const QString& name = QString());
    ~QMallocPool();

    size_t size_of(void *);
    void *calloc(size_t nmemb, size_t size);
    void *malloc(size_t size);
    void free(void *ptr);
    void *realloc(void *ptr, size_t size);

    bool isValid() const;

    struct MemoryStats {
        unsigned long poolSize;
        unsigned long maxSystemBytes;
        unsigned long systemBytes;
        unsigned long inuseBytes;
        unsigned long keepCost;
    };
    MemoryStats memoryStatistics() const;
    void dumpStats() const;

private:
    Q_DISABLE_COPY(QMallocPool)
    QMallocPoolPrivate * d;
};

#endif // _QMALLOCPOOL_H_
