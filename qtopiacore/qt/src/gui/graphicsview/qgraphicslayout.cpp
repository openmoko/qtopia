/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qapplication.h"

#ifndef QT_NO_GRAPHICSVIEW
#include "qgraphicslayout.h"
#include "qgraphicslayout_p.h"
#include "qgraphicslayoutitem.h"
#include "qgraphicswidget.h"
#include "qgraphicswidget_p.h"
#include "qgraphicsscene.h"

QT_BEGIN_NAMESPACE

/*!
    \class QGraphicsLayout
    \brief The QGraphicsLayout class provides the base class for all layouts
    in Graphics View.
    \since 4.4
    \ingroup multimedia
    \ingroup graphicsview-api

    QGraphicsLayout is an abstract class that defines a virtual API for
    arranging QGraphicsWidget children and other QGraphicsLayoutItem objects
    for a QGraphicsWidget. QGraphicsWidget assigns responsibility to a
    QGraphicsLayout through QGraphicsWidget::setLayout(). As the widget
    is resized, the layout will automatically arrange the widget's children.
    QGraphicsLayout inherits QGraphicsLayoutItem, so, it can be managed by
    any layout, including its own subclasses.

    \section1 Writing a Custom Layout

    You can use QGraphicsLayout as a base to write your own custom layout
    (e.g., a flowlayout), but it is more common to use one of its subclasses
    instead - QGraphicsLinearLayout or QGraphicsGridLayout. When creating
    a custom layout, the following functions must be reimplemented as a bare
    minimum:

    \table
    \header \o Function                     \o Description
    \row     \o QGraphicsLayoutItem::setGeometry()
               \o Notifies you when the geometry of the layout is set. You can
                   store the geometry in your own layout class in a reimplementation
                   of this function.
    \row    \o QGraphicsLayoutItem::sizeHint()
               \o Returns the layout's size hints.
    \row    \o QGraphicsLayout::count()
              \o Returns the number of items in your layout.
    \row    \o QGraphicsLayout::itemAt()
              \o Returns a pointer to an item in your layout.
    \row    \o QGraphicsLayout::removeAt()
              \o Removes an item from your layout without destroying it.
    \endtable

    For more details on how to implement each function, refer to the individual
    function documentation.

    Each layout defines its own API for arranging widgets and layout items.
    For example, with a grid layout, you require a row and a
    column index with optional row and column spans, alignment, spacing, and more.
    A linear layout, however, requires a single row or column index to position its
    items. For a grid layout, the order of insertion does not affect the layout in
    any way, but for a linear layout, the order is essential. When writing your own
    layout subclass, you are free to choose the API that best suits your layout.

    \section1 Activating the Layout

    When the layout's geometry changes, QGraphicsLayout immediately rearranges
    all of its managed items by calling setGeometry() on each item. This
    rearrangement is called \e activating the layout.

    QGraphicsLayout updates its own geometry to match the contentsRect() of the
    QGraphicsLayoutItem it is managing. Thus, it will automatically rearrange all
    its items when the widget is resized. QGraphicsLayout caches the sizes of all
    its managed items to avoid calling setGeometry() too often.

    \note A QGraphicsLayout will have the same geometry as the contentsRect()
    of the widget (not the layout) it is assigned to.

    \section2 Activating the Layout Implicitly

    The layout can be activated implicitly using one of two ways: by calling
    activate() or by calling invalidate(). Calling activate() activates the layout
    immediately. In contrast, calling invalidate() is delayed, as it posts a
    \l{QEvent::LayoutRequest}{LayoutRequest} event to the managed widget. Due
    to event compression, the activate() will only be called once after control has
    returned to the event loop. This is referred to as \e invalidating the layout.
    Invalidating the layout also invalidates any cached information. Also, the
    invalidate() function is a virtual function. So, you can invalidate your own
    cache in a subclass of QGraphicsLayout by reimplementing this function.

    \section1 Event Handling

    QGraphicsLayout listens to events for the widget it manages through the
    virtual widgetEvent() event handler. When the layout is assigned to a
    widget, all events delivered to the widget are first processed by
    widgetEvent(). This allows the layout to be aware of any relevant state
    changes on the widget such as visibility changes or layout direction changes.

    \section1 Margin Handling

    The margins of a QGraphicsLayout can be modified by reimplementing
    setContentsMargins() and getContentsMargins().

*/

/*!
    Contructs a QGraphicsLayout object.
    
    \a parent is passed to QGraphicsLayoutItem's constructor and the
    QGraphicsLayoutItem's isLayout argument is set to \e true.
*/
QGraphicsLayout::QGraphicsLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLayoutItem(*(new QGraphicsLayoutPrivate()))
{
    setParentLayoutItem(parent);
    if (parent && !parent->isLayout()) {
        //### Ugly, but keeps old Qt-style
        static_cast<QGraphicsWidget *>(parent)->d_func()->setLayout_helper(this);
    }
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::DefaultType);
}

/*!
    \internal
*/
QGraphicsLayout::QGraphicsLayout(QGraphicsLayoutPrivate &dd, QGraphicsLayoutItem *parent)
    : QGraphicsLayoutItem(dd)
{
    setParentLayoutItem(parent);
    if (parent && !parent->isLayout())
        static_cast<QGraphicsWidget*>(parent)->d_func()->setLayout_helper(this);     //### Ugly, but keeps old Qt-style
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, QSizePolicy::DefaultType);
}

/*!
    Destroys the QGraphicsLayout object.
*/
QGraphicsLayout::~QGraphicsLayout()
{
}

/*!
    Sets the contents margins to \a left, \a top, \a right and \a bottom. The
    default contents margins for toplevel layouts are style dependent
    (by querying the pixelMetric for QStyle::PM_LayoutLeftMargin,
    QStyle::PM_LayoutTopMargin, QStyle::PM_LayoutRightMargin and
    QStyle::PM_LayoutBottomMargin).

    For sublayouts the default margins are 0.

    Changing the contents margins automatically invalidates the layout.

    \sa invalidate()
*/
void QGraphicsLayout::setContentsMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    Q_D(QGraphicsLayout);
    if (d->left == left && d->top == top && d->right == right && d->bottom == bottom)
        return;
    d->left = left;
    d->right = right;
    d->top = top;
    d->bottom = bottom;
    invalidate();
}

/*!
    \reimp
*/
void QGraphicsLayout::getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const
{
    Q_D(const QGraphicsLayout);
    d->getMargin(left, d->left, QStyle::PM_LayoutLeftMargin);
    d->getMargin(top, d->top, QStyle::PM_LayoutTopMargin);
    d->getMargin(right, d->right, QStyle::PM_LayoutRightMargin);
    d->getMargin(bottom, d->bottom, QStyle::PM_LayoutBottomMargin);
}

/*!
    Activates the layout, causing all items in the layout to be immediately
    rearranged. This function is based on calling count() and itemAt(), and
    then calling setGeometry() on all items sequentially. When activated,
    the layout will adjust its geometry to its parent's contentsRect().
    The parent will then invalidate any layout of its own.

    If called in sequence or recursively, e.g., by one of the arranged items
    in response to being resized, this function will do nothing.

    Note that the layout is free to use geometry caching to optimize this
    process.  To forcefully invalidate any such cache, you can call
    invalidate() before calling activate().

    \sa invalidate()
*/
void QGraphicsLayout::activate()
{
    Q_D(QGraphicsLayout);
    if (d->activated)
        return;

    // we don't call activate on a sublayout, but somebody might.
    // Therefore, we walk to the parentitem of the toplevel layout.
    QGraphicsLayoutItem *parentItem = this;
    while (parentItem && parentItem->isLayout())
        parentItem = parentItem->parentLayoutItem();
    if (!parentItem)
        return;
    Q_ASSERT(!parentItem->isLayout());

    d->activated = true;                        // yes, we are now starting a relayout of our children
    setGeometry(parentItem->contentsRect());    // relayout children
    d->activated = false;
    parentLayoutItem()->updateGeometry();            // bubble up; will set activated to false
    // ### too many resizes? maybe we should walk up the chain to the
    // ### top-level layouted layoutItem and call activate there.
}

/*!
    Returns true if the layout is currently being activated; otherwise,
    returns false. If the layout is being activated, this means that it is
    currently in the process of rearranging its items (i.e., the activate()
    function has been called, and has not yet returned).

    \sa activate(), invalidate()
*/
bool QGraphicsLayout::isActivated() const
{
    Q_D(const QGraphicsLayout);
    return d->activated;
}

/*!
    Clears any cached geometry and size hint information in the layout, and
    posts a \l{QEvent::LayoutRequest}{LayoutRequest} event to the managed
    parent QGraphicsLayoutItem.

    \sa activate(), setGeometry()
*/
void QGraphicsLayout::invalidate()
{
    QGraphicsLayoutItem *layoutItem = this;
    while (layoutItem && layoutItem->isLayout()) {
        static_cast<QGraphicsLayout*>(layoutItem)->d_func()->activated = false;
        layoutItem = layoutItem->parentLayoutItem();
    }
    if (layoutItem && !layoutItem->isLayout())
        QApplication::postEvent(static_cast<QGraphicsWidget *>(layoutItem), new QEvent(QEvent::LayoutRequest));
}

/*!
    This virtual event handler receives all events for the managed
    widget. QGraphicsLayout uses this event handler to listen for layout
    related events such as geometry changes, layout changes or layout
    direction changes.
    
    \a e is a pointer to the event.

    You can reimplement this event handler to track similar events for your
    own custom layout.

    \sa QGraphicsWidget::event(), QGraphicsItem::sceneEvent()
*/
void QGraphicsLayout::widgetEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::GraphicsSceneResize:
        if (isActivated()) {
            setGeometry(parentLayoutItem()->contentsRect());
        } else {
            activate(); // relies on that activate() will call updateGeometry()
        }
        break;
    case QEvent::LayoutRequest:
        activate();
        break;
    case QEvent::LayoutDirectionChange:
        invalidate();
        break;
    default:
        break;
    }
}

/*!
    \fn virtual int QGraphicsLayout::count() const = 0

    This pure virtual function must be reimplemented in a subclass of
    QGraphicsLayout to return the number of items in the layout.

    The subclass is free to decide how to store the items.

    \sa itemAt(), removeAt()
*/

/*!
    \fn virtual QGraphicsLayoutItem *QGraphicsLayout::itemAt(int i) const = 0

    This pure virtual function must be reimplemented in a subclass of
    QGraphicsLayout to return a pointer to the item at index \a i. The
    reimplementation can assume that \a i is valid (i.e., it respects the
    value of count()).

    The subclass is free to decide how to store the items.

    \sa count(), removeAt()
*/

/*!
    \fn virtual void QGraphicsLayout::removeAt(int index) = 0

    This pure virtual function must be reimplemented in a subclass of
    QGraphicsLayout to remove the item at index, \a index, without destroying
    it. The reimplementation can assume that \a index is valid (i.e., it respects
    the value of count()).

    The subclass is free to decide how to store the items.

    \sa itemAt(), count()
*/

QT_END_NAMESPACE

#endif //QT_NO_GRAPHICSVIEW
