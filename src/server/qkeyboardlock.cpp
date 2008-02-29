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

#include "qkeyboardlock.h"
#include "qtopiaserverapplication.h"
#include <qwindowsystem_qws.h>

// declare QKeyboardLockPrivate
class QKeyboardLockPrivate : public QtopiaServerApplication::QWSEventFilter
{
public:
    QKeyboardLockPrivate(QKeyboardLock *p);
    ~QKeyboardLockPrivate();

    bool locked;

    bool standardExemptions;
    QList<Qt::Key> exemptions;
    QKeyboardLock *parent;

protected:
    bool qwsEventFilter(QWSEvent *);
};

// define QKeyboardLockPrivate
QKeyboardLockPrivate::QKeyboardLockPrivate(QKeyboardLock *p)
: locked(false), standardExemptions(true), parent(p)
{
    QtopiaServerApplication::instance()->installQWSEventFilter(this);
}

QKeyboardLockPrivate::~QKeyboardLockPrivate()
{
    QtopiaServerApplication::instance()->removeQWSEventFilter(this);
}

bool QKeyboardLockPrivate::qwsEventFilter(QWSEvent *e)
{
    if(locked) {
        if (e->type == QWSEvent::Key) {
            QWSKeyEvent *ke = (QWSKeyEvent *)e;

            bool exempt = false;
            if(standardExemptions &&
               (ke->simpleData.keycode == Qt::Key_Back ||
                ke->simpleData.keycode == Qt::Key_No)) {
                exempt = true;
            } else {
                for(int ii = 0; !exempt && ii < exemptions.count(); ++ii)
                    if(exemptions.at(ii) == (int)ke->simpleData.keycode)
                        exempt = true;
            }

            if(!exempt) {
                emit parent->lockedKeyEvent(ke->simpleData.keycode, ke->simpleData.unicode, ke->simpleData.is_press);
                return true;
            }
        }

#ifdef QTOPIA4_TODO
        else if (Qtopia::mousePreferred() && e->type == QWSEvent::IMEvent)  {
            QWSIMEvent *ie = (QWSIMEvent *)e;
            // simpleData doesn't have a type value... do we need to fix Qt?
            if (ie->simpleData.type == QWSServer::IMEnd && ie->simpleData.textLen == 1) {
                emit preProcessKeyLockEvent(*(ie->text), ie->text->unicode(), true);
                return true;
            }
        }
#endif

    }

    return false;
}

// define QKeyboardLock
QKeyboardLock::QKeyboardLock(QObject *parent)
: QObject(parent), d(new QKeyboardLockPrivate(this))
{
}

QKeyboardLock::~QKeyboardLock()
{
    delete d;
    d = 0;
}

/*!
  Returns true if the keyboard is locked, false otherwise.
  */
bool QKeyboardLock::isLocked() const
{
    return d->locked;
}

/*!
  Lock the keyboard.
 */
void QKeyboardLock::lock(bool l)
{
    d->locked = l;
}

/*!
  Unlock the keyboard.
 */
void QKeyboardLock::unlock()
{
    d->locked = false;
}

/*!
  Returns the keys that are exempt to lock filtering.  By default,
  Qt::Key_Back and Qt::Key_No are exempt to allow visible dialogs to be
  dismissed.
 */
QList<Qt::Key> QKeyboardLock::exemptions() const
{
    if(d->standardExemptions) {
        Q_ASSERT(d->exemptions.isEmpty());
        d->exemptions.append(Qt::Key_Back);
        d->exemptions.append(Qt::Key_No);
        d->standardExemptions = false;
    }

    return d->exemptions;
}

/*!
  Set the keys exempt to lock filtering to \a exempt.
 */
void QKeyboardLock::setExemptions(const QList<Qt::Key> &exempt)
{
    d->exemptions = exempt;
    d->standardExemptions = false;
}

