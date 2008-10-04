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

#include "qobjectcleanuphandler.h"

QT_BEGIN_NAMESPACE

/*!
    \class QObjectCleanupHandler
    \brief The QObjectCleanupHandler class watches the lifetime of multiple QObjects.

    \ingroup objectmodel

    A QObjectCleanupHandler is useful whenever you need to know when a
    number of \l{QObject}s that are owned by someone else have been
    deleted. This is important, for example, when referencing memory
    in an application that has been allocated in a shared library.

    To keep track of some \l{QObject}s, create a
    QObjectCleanupHandler, and add() the objects you are interested
    in. If you are no longer interested in tracking a particular
    object, use remove() to remove it from the cleanup handler. If an
    object being tracked by the cleanup handler gets deleted by
    someone else it will automatically be removed from the cleanup
    handler. You can delete all the objects in the cleanup handler
    with clear(), or by destroying the cleanup handler. isEmpty()
    returns true if the QObjectCleanupHandler has no objects to keep
    track of.

    \sa QPointer
*/

/*!
    Constructs an empty QObjectCleanupHandler.
*/
QObjectCleanupHandler::QObjectCleanupHandler()
{
}

/*!
    Destroys the cleanup handler. All objects in this cleanup handler
    will be deleted.

    \sa clear()
*/
QObjectCleanupHandler::~QObjectCleanupHandler()
{
    clear();
}

/*!
    Adds \a object to this cleanup handler and returns the pointer to
    the object.

    \sa remove()
*/
QObject *QObjectCleanupHandler::add(QObject* object)
{
    if (!object)
        return 0;

    connect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
    cleanupObjects.insert(0, object);
    return object;
}

/*!
    Removes the \a object from this cleanup handler. The object will
    not be destroyed.

    \sa add()
*/
void QObjectCleanupHandler::remove(QObject *object)
{
    int index;
    if ((index = cleanupObjects.indexOf(object)) != -1) {
        cleanupObjects.removeAt(index);
        disconnect(object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
    }
}

/*!
    Returns true if this cleanup handler is empty or if all objects in
    this cleanup handler have been destroyed; otherwise return false.

    \sa add() remove() clear()
*/
bool QObjectCleanupHandler::isEmpty() const
{
    return cleanupObjects.isEmpty();
}

/*!
    Deletes all objects in this cleanup handler. The cleanup handler
    becomes empty.

    \sa isEmpty()
*/
void QObjectCleanupHandler::clear()
{
    while (!cleanupObjects.isEmpty())
        delete cleanupObjects.takeFirst();
}

void QObjectCleanupHandler::objectDestroyed(QObject *object)
{
    remove(object);
}

QT_END_NAMESPACE
