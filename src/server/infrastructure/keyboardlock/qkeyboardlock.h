/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef _QKEYBOARDLOCK_H_
#define _QKEYBOARDLOCK_H_

#include <QObject>

class QKeyboardLockPrivate;
class QKeyboardLock : public QObject
{
Q_OBJECT
public:
    QKeyboardLock(QObject *parent = 0);
    virtual ~QKeyboardLock();

    bool isLocked() const;

    void lock( bool lock = true );
    void unlock();

    QList<Qt::Key> exemptions() const;
    void setExemptions(const QList<Qt::Key> &);

signals:
    void lockedKeyEvent(uint, ushort, bool);

private:
    friend class QKeyboardLockPrivate;
    QKeyboardLockPrivate *d;
};

#endif // _QKEYBOARDLOCK_H_
