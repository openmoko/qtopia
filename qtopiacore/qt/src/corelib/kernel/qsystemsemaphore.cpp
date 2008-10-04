/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qsystemsemaphore.h"
#include "qsystemsemaphore_p.h"
#include <qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SYSTEMSEMAPHORE

/*!
  \class QSystemSemaphore
  \since 4.4

  \brief The QSystemSemaphore class provides a general counting system semaphore.

  A semaphore is a generalization of a mutex. While a mutex can be
  locked only once, a semaphore can be acquired multiple times.
  Typically, a semaphore is used to protect a certain number of
  identical resources.

  Like its lighter counterpart QSemaphore, a QSystemSemaphore can be
  accessed from multiple \l {QThread} {threads}. Unlike QSemaphore, a
  QSystemSemaphore can also be accessed from multiple \l {QProcess}
  {processes}. This means QSystemSemaphore is a much heavier class, so
  if your application doesn't need to access your semaphores across
  multiple processes, you will probably want to use QSemaphore.

  When using this class, be aware of the following platform
  differences:

  \list

  \o Windows: QSystemSemaphore does not own its underlying system
  semaphore. Windows owns it. This means that when all instances of
  QSystemSemaphore for a particular key have been destroyed, either by
  having their destructors called, or because one or more processes
  crash, Windows removes the underlying system semaphore.

  \o Unix: QSystemSemaphore owns the underlying system semaphore in
  Unix systems. This means that the last process having an instance of
  QSystemSemaphore for a particular key must remove the underlying
  system semaphore in its destructor. If the last process crashes
  without running the QSystemSemaphore destructor, Unix does not
  automatically remove the underlying system semaphore, and the
  semaphore survives the crash. A subsequent process that constructs a
  QSystemSemaphore with the same key will then be given the existing
  system semaphore. In that case, if the QSystemSemaphore constructor
  has specified its \l {QSystemSemaphore::AccessMode} {access mode} as
  \l {QSystemSemaphore::} {Open}, its initial resource count will not
  be reset to the one provided but remain set to the value it received
  in the crashed process. To protect against this, the first process
  to create a semaphore for a particular key (usually a server), must
  pass its \l {QSystemSemaphore::AccessMode} {access mode} as \l
  {QSystemSemaphore::} {Create}, which will force Unix to reset the
  resource count in the underlying system semaphore.

  \o Unix: When a process using QSystemSemaphore terminates for any
  reason, Unix automatically reverses the effect of all acquire
  operations that were not released. Thus if the process acquires a
  resource and then exits without releasing it, Unix will release that
  resource.

  \endlist

  Semaphores support two fundamental operations, acquire() and release():

  acquire() tries to acquire one resource. If there isn't a resource
  available, the call blocks until a resource becomes available. Then
  the resource is acquired and the call returns.

  release() releases one resource so it can be acquired by another
  process. The function can also be called with a parameter n > 1,
  which releases n resources.

  A system semaphore is created with a string key that other processes
  can use to use the same semaphore.

  Example: Create a system semaphore
  \snippet doc/src/snippets/code/src_corelib_kernel_qsystemsemaphore.cpp 0

  A typical application of system semaphores is for controlling access
  to a circular buffer shared by a producer process and a consumer
  processes.

  See also QSharedMemory, QSemaphore
 */

/*!
  Requests a system semaphore for the specified \a key. The parameters
  \a initialValue and \a mode are used according to the following
  rules, which are system dependent.

  In Unix, if the \a mode is \l {QSystemSemaphore::} {Open} and the
  system already has a semaphore identified by \a key, that semaphore
  is used, and the semaphore's resource count is not changed, i.e., \a
  initialValue is ignored. But if the system does not already have a
  semaphore identified by \a key, it creates a new semaphore for that
  key and sets its resource count to \a initialValue.

  In Unix, if the \a mode is \l {QSystemSemaphore::} {Create} and the
  system already has a semaphore identified by \a key, that semaphore
  is used, and its resource count is set to \a initialValue. If the
  system does not already have a semaphore identified by \a key, it
  creates a new semaphore for that key and sets its resource count to
  \a initialValue.

  In Windows, \a mode is ignored, and the system always tries to
  create a semaphore for the specified \a key. If the system does not
  already have a semaphore identified as \a key, it creates the
  semaphore and sets its resource count to \a initialValue. But if the
  system already has a semaphore identified as \a key it uses that
  semaphore and ignores \a initialValue.

  The \l {QSystemSemaphore::AccessMode} {mode} parameter is only used
  in Unix systems to handle the case where a semaphore survives a
  process crash. In that case, the next process to allocate a
  semaphore with the same \a key will get the semaphore that survived
  the crash, and unless \a mode is \l {QSystemSemaphore::} {Create},
  the resource count will not be reset to \a initialValue but will
  retain the initial value it had been given by the crashed process.

  \sa acquire(), key()
 */
QSystemSemaphore::QSystemSemaphore(const QString &key, int initialValue, AccessMode mode)
{
    d = new QSystemSemaphorePrivate;
    setKey(key, initialValue, mode);
}

/*!
  The destructor destroys the QSystemSemaphore object, but the
  underlying system semaphore is not removed from the system unless
  this instance of QSystemSemaphore is the last one existing for that
  system semaphore.

  Two important side effects of the destructor depend on the system.
  In Windows, if acquire() has been called for this semaphore but not
  release(), release() will not be called by the destructor, nor will
  the resource be released when the process exits normally. This would
  be a program bug which could be the cause of a deadlock in another
  process trying to acquire the same resource. In Unix, acquired
  resources that are not released before the destructor is called are
  automatically released when the process exits.
*/
QSystemSemaphore::~QSystemSemaphore()
{
    d->cleanHandle();
    delete d;
}

/*!
  \enum QSystemSemaphore::AccessMode

  This enum is used by the constructor and setKey(). Its purpose is to
  enable handling the problem in Unix implementations of semaphores
  that survive a crash. In Unix, when a semaphore survives a crash, we
  need a way to force it to reset its resource count, when the system
  reuses the semaphore. In Windows, where semaphores can't survive a
  crash, this enum has no effect.

  \value Open If the semaphore already exists, its initial resource
  count is not reset. If the semaphore does not already exist, it is
  created and its initial resource count set.

  \value Create QSystemSemaphore takes ownership of the semaphore and
  sets its resource count to the requested value, regardless of
  whether the semaphore already exists by having survived a crash.
  This value should be passed to the constructor, when the first
  semaphore for a particular key is constructed and you know that if
  the semaphore already exists it could only be because of a crash. In
  Windows, where a semaphore can't survive a crash, Create and Open
  have the same behavior.
*/

/*!
  This function works the same as the constructor. It reconstructs
  this QSystemSemaphore object. If the new \a key is different from
  the old key, calling this function is like calling the destructor of
  the semaphore with the old key, then calling the constructor to
  create a new semaphore with the new \a key. The \a initialValue and
  \a mode parameters are as defined for the constructor.

  \sa QSystemSemaphore(), key()
 */
void QSystemSemaphore::setKey(const QString &key, int initialValue, AccessMode mode)
{
    if (key == d->key && mode == Open)
        return;
    d->error = NoError;
    d->errorString = QString();
#ifndef Q_OS_WIN
    // optimization to not destroy/create the file & semaphore
    if (key == d->key && mode == Create && d->createdSemaphore && d->createdFile) {
        d->initialValue = initialValue;
        d->unix_key = -1;
        d->handle(mode);
        return;
    }
#endif
    d->cleanHandle();
    d->key = key;
    d->initialValue = initialValue;
    // cache the file name so it doesn't have to be generated all the time.
    d->fileName = d->makeKeyFileName();
    d->handle(mode);
}

/*!
  Returns the key assigned to this system semaphore. The key is the
  name by which the semaphore can be accessed from other processes.

  \sa setKey()
 */
QString QSystemSemaphore::key() const
{
    return d->key;
}

/*!
  Acquires one of the resources guarded by this semaphore, if there is
  one available, and returns true. If all the resources guarded by this
  semaphore have already been acquired, the call blocks until one of
  them is released by another process or thread having a semaphore
  with the same key.

  If false is returned, a system error has occurred.

  \sa release()
 */
bool QSystemSemaphore::acquire()
{
    return d->modifySemaphore(-1);
}

/*!
  Releases \a n resources guarded by the semaphore. Returns true
  unless there is a system error.

  Example: Create a system semaphore having five resources; acquire
  them all and then release them all.

  \snippet doc/src/snippets/code/src_corelib_kernel_qsystemsemaphore.cpp 1

  This function can also "create" resources. For example, immediately
  following the sequence of statements above, suppose we add the
  statement:

  \snippet doc/src/snippets/code/src_corelib_kernel_qsystemsemaphore.cpp 2

  Ten new resources are now guarded by the semaphore, in addition to
  the five that already existed. You would not normally use this
  function to create more resources.

  \sa acquire()
 */
bool QSystemSemaphore::release(int n)
{
    if (n == 0)
        return true;
    if (n < 0) {
        qWarning("QSystemSemaphore::release: n is negative.");
        return false;
    }
    return d->modifySemaphore(n);
}

/*!
  Returns a value indicating whether an error occurred, and, if so,
  which error it was.

  \sa errorString()
 */
QSystemSemaphore::SystemSemaphoreError QSystemSemaphore::error() const
{
    return d->error;
}

/*!
  \enum QSystemSemaphore::SystemSemaphoreError

  \value NoError No error occurred.

  \value PermissionDenied The operation failed because the caller
  didn't have the required permissions.

  \value KeyError The operation failed because of an invalid key.

  \value AlreadyExists The operation failed because a system
  semaphore with the specified key already existed.

  \value NotFound The operation failed because a system semaphore
  with the specified key could not be found.

  \value OutOfResources The operation failed because there was
  not enough memory available to fill the request.

  \value UnknownError Something else happened and it was bad.
*/

/*!
  Returns a text description of the last error that occurred. If
  error() returns an \l {QSystemSemaphore::SystemSemaphoreError} {error
  value}, call this function to get a text string that describes the
  error.

  \sa error()
 */
QString QSystemSemaphore::errorString() const
{
    return d->errorString;
}

#endif // QT_NO_SYSTEMSEMAPHORE

QT_END_NAMESPACE
