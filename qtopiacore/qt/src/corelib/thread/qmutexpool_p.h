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

#ifndef QMUTEXPOOL_P_H
#define QMUTEXPOOL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QSettings. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qmutex.h"

#ifndef QT_NO_THREAD

class Q_CORE_EXPORT QMutexPool
{
public:
    explicit QMutexPool(bool recursive = false, int size = 128);
    ~QMutexPool();

    QMutex *get(const void *address);

private:
    QMutex mutex;
    QMutex **mutexes;
    int count;
    bool recurs;
};

extern Q_CORE_EXPORT QMutexPool *qt_global_mutexpool;

#endif // QT_NO_THREAD

#endif // QMUTEXPOOL_P_H
