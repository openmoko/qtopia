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

#ifndef QWSLOCK_P_H
#define QWSLOCK_P_H

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

class QWSLock
{
public:
    enum LockType { BackingStore, Communication };

    QWSLock();
    QWSLock(int lockId);
    ~QWSLock();

    bool lock(LockType type, int timeout = -1);
    void unlock(LockType type);
    bool wait(LockType type, int timeout = -1);
    int id() const { return semId; }

private:
    int semId;
    int lockCount[2];

    bool hasLock(LockType type);
};

#endif // QWSLOCK_P_H
