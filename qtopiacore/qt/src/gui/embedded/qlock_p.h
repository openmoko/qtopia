/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QLOCK_P_H
#define QLOCK_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//

#include "QtCore/qstring.h"

class QLockData;

class Q_GUI_EXPORT QLock
{
public:
    QLock(const QString &filename, char id, bool create = false);
    ~QLock();

    enum Type { Read, Write };

    bool isValid() const;
    void lock(Type type);
    void unlock();
    bool locked() const;

private:
    Type type;
    QLockData *data;
};


// Nice class for ensuring the lock is released.
// Just create one on the stack and the lock is automatically released
// when QLockHandle is destructed.
class Q_GUI_EXPORT QLockHandle
{
public:
    QLockHandle(QLock *l, QLock::Type type) : qlock(l) { qlock->lock(type); }
    ~QLockHandle() { if (locked()) qlock->unlock(); }

    void lock(QLock::Type type) { qlock->lock(type); }
    void unlock() { qlock->unlock(); }
    bool locked() const { return qlock->locked(); }

private:
    QLock *qlock;
};

#endif // QLOCK_P_H
