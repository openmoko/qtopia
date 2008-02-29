/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qwidget.h>
#include <qdrag.h>
#include <qpixmap.h>
#include <qpoint.h>
#include "qdnd_p.h"

#ifndef QT_NO_DRAGANDDROP

/*!
    \class QDrag
    \brief The QDrag class provides support for MIME-based drag and drop data
    transfer.

    Drag and drop is an intuitive way for users to copy or move data around in an
    application, and is used in many desktop environments as a mechanism for copying
    data between applications. Drag and drop support in Qt is centered around the
    QDrag class that handles most of the details of a drag and drop operation.

    The data to be transferred by the drag and drop operation is contained in a
    QMimeData object. This is specified with the setMimeData() function in the
    following way:

    \quotefromfile snippets/dragging/mainwindow.cpp
    \skipto mousePressEvent
    \skipto QDrag
    \printuntil setMimeData

    Note that setMimeData() assigns ownership of the QMimeData object to the
    QDrag object. The QDrag must be constructed on the heap with a parent QObject
    to ensure that Qt can clean up after the drag and drop operation has been
    completed.

    A pixmap can be used to represent the data while the drag is in progress, and
    will move with the cursor to the drop target. This pixmap typically shows an
    icon that represents the MIME type of the data being transferred, but any
    pixmap can be set with setPixmap(). Care must be taken to ensure that the
    pixmap is not too large. The cursor's hot spot can be given a position relative
    to the top-left corner of the pixmap with the setHotSpot() function. The
    following code positions the pixmap so that the cursor's hot spot points to
    the center of its bottom edge:

    \quotefromfile snippets/separations/finalwidget.cpp
    \skipto setHotSpot
    \printuntil ;

    The source and target widgets can be found with source() and target().
    These functions are often used to determine whether drag and drop operations
    started and finished at the same widget, so that special behavior can be
    implemented.

    QDrag only deals with the drag and drop operation itself. It is up to the
    developer to decide when a drag operation begins, and how a QDrag object should
    be constructed and used. For a given widget, it is often necessary to
    reimplement \l{QWidget::mousePressEvent()}{mousePressEvent()} to determine
    whether the user has pressed a mouse button, and reimplement
    \l{QWidget::mouseMoveEvent()}{mouseMoveEvent()} to check whether a QDrag is
    required.

    \sa {Drag and Drop}, QClipboard, QMimeData,
        {Draggable Icons Example}, {Draggable Text Example}, {Drop Site Example},
	{Fridge Magnets Example}
*/

/*!
    Constructs a new drag object for the widget specified by \a dragSource.
*/
QDrag::QDrag(QWidget *dragSource)
    : QObject(*new QDragPrivate, dragSource)
{
    Q_D(QDrag);
    d->source = dragSource;
    d->target = 0;
    d->data = 0;
    d->hotspot = QPoint(-10, -10);
    d->possible_actions = Qt::CopyAction;
    d->executed_action = Qt::IgnoreAction;
}

/*!
    Destroys the drag object.
*/
QDrag::~QDrag()
{
    Q_D(QDrag);
    delete d->data;
    QDragManager *manager = QDragManager::self();
    if (manager && manager->object == this)
        manager->cancel(false);
}

/*!
    Sets the data to be sent to the given MIME \a data. Ownership of the data is
    transferred to the QDrag object.
*/
void QDrag::setMimeData(QMimeData *data)
{
    Q_D(QDrag);
    if (d->data != 0)
        delete d->data;
    d->data = data;
}

/*!
    Returns the MIME data that is encapsulated by the drag object.
*/
QMimeData *QDrag::mimeData() const
{
    Q_D(const QDrag);
    return d->data;
}

/*!
    Sets \a pixmap as the pixmap used to represent the data in a drag
    and drop operation.
*/
void QDrag::setPixmap(const QPixmap &pixmap)
{
    Q_D(QDrag);
    d->pixmap = pixmap;
}

/*!
    Returns the pixmap used to represent the data in a drag and drop operation.
*/
QPixmap QDrag::pixmap() const
{
    Q_D(const QDrag);
    return d->pixmap;
}

/*!
    Sets the position of the hot spot relative to the top-left corner of the
    pixmap used to the point specified by \a hotspot.
*/
void QDrag::setHotSpot(const QPoint& hotspot)
{
    Q_D(QDrag);
    d->hotspot = hotspot;
}

/*!
    Returns the position of the hot spot relative to the top-left corner of the
    cursor.
*/
QPoint QDrag::hotSpot() const
{
    Q_D(const QDrag);
    return d->hotspot;
}

/*!
    Returns the source of the drag object. This is the widget where the drag
    and drop operation originated.
*/
QWidget *QDrag::source() const
{
    Q_D(const QDrag);
    return d->source;
}

/*!
    Returns the target of the drag and drop operation. This is the widget where
    the drag object was dropped.
*/
QWidget *QDrag::target() const
{
    Q_D(const QDrag);
    return d->target;
}

/*!
    Starts the drag and drop operation and returns a value indicating the requested
    drop action when it is completed. The drop actions that the user can choose
    from are specified in \a request. Qt::CopyAction is always allowed.

    \bold{Note:} Although the drag and drop operation can take some time, this function
    does not block the event loop. Other events are still delivered to the application
    while the operation is performed.
*/
Qt::DropAction QDrag::start(Qt::DropActions request)
{
    Q_D(QDrag);
    Q_ASSERT_X(d->data, "QDrag", "No mimedata set before starting the drag");
    QDragManager *manager = QDragManager::self();
    d->possible_actions = request | Qt::CopyAction;
    if (manager)
        d->executed_action = manager->drag(this);
    return d->executed_action;
}

/*!
    Sets the drag \a cursor for the \a action. This allows you
    to override the default native cursors. To revert to using the
    native cursor for \a action pass in a null QPixmap as \a cursor.

    The \a action can only be CopyAction, MoveAction or LinkAction.
    All other values of DropAction are ignored.
*/
void QDrag::setDragCursor(const QPixmap &cursor, Qt::DropAction action)
{
    Q_D(QDrag);
    if (action != Qt::CopyAction && action != Qt::MoveAction && action != Qt::LinkAction)
        return;
    if (cursor.isNull())
        d->customCursors.remove(action);
    else
        d->customCursors[action] = cursor;
}

/*!
    \fn void QDrag::actionChanged(Qt::DropAction action)

    This signal is emitted when the \a action associated with the
    drag changes.

    \sa targetChanged()
*/

/*!
    \fn void QDrag::targetChanged(QWidget *newTarget)

    This signal is emitted when the target of the drag and drop
    operation changes, with \a newTarget the new target.

    \sa target(), actionChanged()
*/

#endif // QT_NO_DRAGANDDROP
