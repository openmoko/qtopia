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

/*!
    \class QGraphicsItem
    \brief The QGraphicsItem class is the base class for all graphical
    items in a QGraphicsScene.
    \since 4.2
    \ingroup multimedia

    It provides a light-weight foundation for writing your own custom items.
    This includes defining the item's geometry, collision detection, its
    painting implementation and item interaction through its event handlers.
    QGraphicsItem is part of \l{The Graphics View Framework}

    \img graphicsview-items.png

    For convenience, Qt provides a set of standard graphics items for the most
    common shapes. These are:

    \list
    \o QGraphicsEllipseItem provides an ellipse item
    \o QGraphicsLineItem provides a line item
    \o QGraphicsPathItem provides an arbitrary path item
    \o QGraphicsPixmapItem provides a pixmap item
    \o QGraphicsPolygonItem provides a polygon item
    \o QGraphicsRectItem provides a rectangular item
    \o QGraphicsTextItem provides a text item
    \endlist

    All of an item's geometric information is based on its local coordinate
    system. The item's position, pos(), is the only function that does not
    operate in local coordinates, as it returns a position in parent
    coordinates. {The Graphics View Coordinate System} describes the coordinate
    system in detail.

    You can set whether an item should be visible (i.e., drawn, and accepting
    events), by calling setVisible(). Hiding an item will also hide its
    children. Similarly, you can enable or disable an item by calling
    setEnabled(). If you disable an item, all its children will also be
    disabled. By default, items are both visible and enabled. To toggle
    whether an item is selected or not, first enable selection by setting
    the ItemIsSelectable flag, and then call setSelected(). Normally,
    selection is toggled by the scene, as a result of user interaction.

    To write your own graphics item, you first create a subclass of
    QGraphicsItem, and then start by implementing its two pure virtual public
    functions: boundingRect(), which returns an estimate of the area painted
    by the item, and paint(), which implements the actual painting. For
    example:

    \code
        class SimpleItem : public QGraphicsItem
        {
        public:
            QRectF boundingRect() const
            {
                qreal penWidth = 1;
                return QRectF(-10 - penWidth / 2, -10 - penWidth / 2,
                              20 + penWidth / 2, 20 + penWidth / 2);
            }

            void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
            {
                painter->drawRoundRect(-10, -10, 20, 20);
            }
        };
    \endcode

    The boundingRect() function has many different purposes. QGraphicsScene
    bases its item index on boundingRect(), and QGraphicsView uses it both for
    culling invisible items, and for determining the area that needs to be
    recomposed when drawing overlapping items. In addition, QGraphicsItem's
    collision detection mechanisms use boundingRect() to provide an efficient
    cut-off. The fine grained collision algorithm in collidesWithItem() is based
    on calling shape(), which returns an accurate outline of the item's shape
    as a QPainterPath.

    Collision detection can be done in two ways:

    \list 1

    \o Reimplement shape() to return an accurate shape for your item, and rely
    on the default implementation of collidesWithItem() to do shape-shape
    intersection. This can be rather expensive if the shapes are complex.

    \o Reimplement collidesWithItem() to provide your own custom item and shape
    collision algorithm.

    \endlist

    The contains() function can be called to determine whether the item \e
    contains a point or not. This function can also be reimplemented by the
    item. The default behavior of contains() is based on calling shape().

    Items can contain other items, and also be contained by other items. All
    items can have a parent item and a list of children. Unless the item has
    no parent, its position is in \e parent coordinates (i.e., the parent's
    local coordinates). Parent items propagate both their position and their
    transformation to all children.

    \img graphicsview-parentchild.png

    QGraphicsItem supports affine transformations in addition to its base
    position, pos(). To change the item's transformation, you can either pass
    a transformation matrix to setMatrix(), or call one of the convenience
    functions rotate(), scale(), translate(), or shear(). Item transformations
    accumulate from parent to child, so if both a parent and child item are
    rotated 90 degrees, the child's total transformation will be 180 degrees.
    Similarly, if the item's parent is scaled to 2x its original size, its
    children will also be twice as large. An item's transformation does not
    affect its own local geometry; all geometry functions (e.g., contains(),
    update(), and all the mapping functions) still operate in local
    coordinates. For convenience, QGraphicsItem provides the functions
    sceneMatrix(), which returns the item's total transformation matrix
    (including its position and all parents' positions and transformations),
    and scenePos(), which returns its position in scene coordinates. To reset
    an item's matrix, call resetMatrix().

    The paint() function is called by QGraphicsView to paint the item's
    contents. The item has no background or default fill of its own; whatever
    is behind the item will shine through all areas that are not explicitly
    painted in this function.  You can call update() to schedule a repaint,
    optionally passing the rectangle that needs a repaint. Depending on
    whether or not the item is visible in a view, the item may or may not be
    repainted; there is no equivalent to QWidget::repaint() in QGraphicsItem.

    Items are painted by the view, starting with the parent items and then
    drawing children, in ascending stacking order. You can set an item's
    stacking order by calling setZValue(), and test it by calling
    zValue(), where items with low z-values are painted before items with
    high z-values. Stacking order applies to sibling items; parents are always
    drawn before their children.

    QGraphicsItem receives events from QGraphicsScene through the virtual
    function sceneEvent(). This function distributes the most common events
    to a set of convenience event handlers:

    \list
    \o contextMenuEvent() handles context menu events
    \o focusInEvent() and focusOutEvent() handle focus in and out events
    \o hoverEnterEvent(), hoverMoveEvent(), and hoverLeaveEvent() handles
    hover enter, move and leave events
    \o inputMethodEvent() handles input events, for accessibility support
    \o keyPressEvent() and keyReleaseEvent handle key press and release events
    \o mousePressEvent(), mouseMoveEvent(), mouseReleaseEvent(), and
    mouseDoubleClickEvent() handles mouse press, move, release, click and
    doubleclick events
    \endlist

    You can filter events for any other item by installing event
    filters. This functionaly is separate from from Qt's regular
    event filters (see QObject::installEventFilter()), which only
    work on subclasses of QObject. After installing your item as an
    event filter for another item by calling
    installSceneEventFilter(), the filtered events will be received
    by the virtual function sceneEventFilter(). You can remove item
    event filters by calling removeSceneEventFilter().

    Sometimes it's useful to register custom data with an item, be it a custom
    item, or a standard item. You can call setData() on any item to store data
    in it using a key-value pair (the key being an integer, and the value is a
    QVariant). To get custom data from an item, call data(). This
    functionality is completely untouched by Qt itself; it is provided for the
    user's convenience.

    \sa QGraphicsScene, QGraphicsView, {The Graphics View Framework}
*/

/*!
    \variable QGraphicsItem::UserType

    The lowest permitted type value for custom items (subclasses
    of QGraphicsItem or any of the standard items). This value is
    used in conjunction with a reimplementation of QGraphicsItem::type()
    and declaring a Type enum value. Example:

    \code
        class CustomItem : public QGraphicsItem
        {
           ...
           enum { Type = UserType + 1 }

           int type() const
           {
               // Enable the use of qgraphicsitem_cast with this item.
               return Type;
           }
           ...
        };
    \endcode
*/

/*!
    \enum QGraphicsItem::GraphicsItemFlag

    This enum describes different flags that you can set on an item to
    toggle different features in the item's behavior.

    All flags are disabled by default.

    \value ItemIsMovable The item supports interactive movement using
    the mouse. By clicking on the item and then dragging, the item
    will move together with the mouse cursor. If the item has
    children, all children are also moved. If the item is part of a
    selection, all selected items are also moved. This feature is
    provided as a convenience through the base implementation of
    QGraphicsItem's mouse event handlers.

    \value ItemIsSelectable The item supports selection. Enabling this
    feature will enable setSelected() to toggle selection for the
    item. It will also let the item be selected automatically as a
    result of calling QGraphicsScene::setSelectionArea(), by clicking
    on an item, or by using rubber band selection in QGraphicsView.

    \value ItemIsFocusable The item supports keyboard input focus (i.e., it is
    an input item). Enabling this flag will allow the item to accept focus,
    which again allows the delivery of key events to
    QGraphicsItem::keyPressEvent() and QGraphicsItem::keyReleaseEvent().
*/

/*!
    \enum QGraphicsItem::GraphicsItemChange

    This enum describes the state changes that are notified by
    QGraphicsItem::itemChange(). The notifications are sent as the state
    changes, and in some cases, adjustments can be made (see the documentation
    for each change for details).

    Note: Be careful with calling functions on the QGraphicsItem itself inside
    itemChange(), as certain function calls can lead to unwanted
    recursion. For example, you cannot call setPos() in itemChange() on an
    ItemPositionChange notification, as the setPos() function will again call
    itemChange(ItemPositionChange). Instead, you can return the new, adjusted
    position from itemChange().

    \value ItemEnabledChange The item's enabled state changes. If the item is
    presently enabled, it will become disabled, and vice verca. The value
    argument is the new enabled state (i.e., true or false). Do not call
    setEnabled() in itemChange() as this notification is delivered. Instead,
    you can return the new state from itemChange().

    \value ItemMatrixChange The item's matrix changes. This notification is
    only sent when the item's local matrix changes (i.e., as a result of
    calling setMatrix(), or one of the convenience transformation functions,
    such as rotate()). The value argument is the new matrix (i.e., a QMatrix);
    to get the old matrix, call matrix(). Do not call setMatrix() or any of
    the transformation convenience functions in itemChange() as this
    notification is delivered; instead, you can return the new matrix from
    itemChange().

    \value ItemPositionChange The item's position changes. This notification
    is only sent when the item's local position changes, relative to its
    parent, has changed (i.e., as a result of calling setPos() or
    moveBy()). The value argument is the new position (i.e., a QPointF).
    You can call pos() to get the original position. Do not call setPos() or
    moveBy() in itemChange() as this notification is delivered; instead, you
    can return the new, adjusted position from itemChange().

    \value ItemSelectedChange The item's selected state changes. If the item
    is presently selected, it will become unselected, and vice verca. The
    value argument is the new selected state (i.e., true or false). Do not
    call setSelected() in itemChange() as this notification is delivered();
    instead, you can return the new selected state from itemChange().

    \value ItemVisibleChange The item's visible state changes. If the item is
    presently visible, it will become invisible, and vice verca. The value
    argument is the new visible state (i.e., true or false). Do not call
    setVisible() in itemChange() as this notification is delivered; instead,
    you can return the new visible state from itemChange().

    \value ItemParentChange The item's parent changes. The value argument is
    the new parent item (i.e., a QGraphicsItem pointer).  Do not call
    setParentItem() in itemChange() as this notification is delivered;
    instead, you can return the new parent from itemChange().

    \value ItemChildAddedChange A child is added to this item. The value
    argument is the new child item (i.e., a QGraphicsItem pointer). Do not
    pass this item to any item's setParentItem() function as this notification
    is delivered. The return value is unused; you cannot adjust anything in
    this notification. Note that the new child might not be fully constructed
    when this notification is sent; calling pure virtual functions on
    the child can lead to a crash.

    \value ItemChildRemovedChange A child is removed from this item. The value
    argument is the child item that is about to be removed (i.e., a
    QGraphicsItem pointer). The return value is unused; you cannot adjust
    anything in this notification.
*/

/*!
    \enum QGraphicsItem::Extension
    \internal

    Note: This is provided as a hook to avoid future problems related
    to adding virtual functions. See also extension(),
    supportsExtension() and setExtension().
*/

#include "qgraphicsitem.h"

#ifndef QT_NO_GRAPHICSVIEW

#include "qgraphicsscene.h"
#include "qgraphicsscene_p.h"
#include "qgraphicssceneevent.h"
#include "qgraphicsview.h"
#include <QtCore/qbitarray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qpoint.h>
#include <QtCore/qtimer.h>
#include <QtCore/qvariant.h>
#include <QtGui/qapplication.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qstyleoption.h>
#include <QtGui/qevent.h>

#include <private/qgraphicsitem_p.h>
#include <private/qtextcontrol_p.h>
#include <private/qtextengine_p.h>

#include <math.h>

/*
    ### Move this into QGraphicsItemPrivate
 */
class QGraphicsItemCustomDataStore
{
public:
    QMap<const QGraphicsItem *, QMap<int, QVariant> > data;
};
Q_GLOBAL_STATIC(QGraphicsItemCustomDataStore, qt_dataStore);

/*!
    \internal

    Propagates updates to \a item and all its children.
*/
static void qt_graphicsItem_fullUpdate(QGraphicsItem *item)
{
    item->update();
    foreach (QGraphicsItem *child, item->children())
        qt_graphicsItem_fullUpdate(child);
}

/*!
    \internal

    Returns a QPainterPath of \a path when stroked with the \a pen.
    Ignoring dash pattern.
*/
static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal penWidthZero = qreal(0.00000001);

    if (path == QPainterPath())
        return path;
    QPainterPathStroker ps;
    ps.setCapStyle(pen.capStyle());
    ps.setWidth(pen.widthF() <= 0.0 ? penWidthZero : pen.widthF());
    ps.setJoinStyle(pen.joinStyle());
    ps.setMiterLimit(pen.miterLimit());
    QPainterPath p = ps.createStroke(path);
    p.addPath(path);
    return p;
}

/*!
    \internal

    Propagates child event handling for this item and all its children.
*/
void QGraphicsItemPrivate::setAncestorHandlesChildEvents(bool enabled)
{
    if (!handlesChildEvents) {
        ancestorHandlesChildEvents = enabled;
        foreach (QGraphicsItem *child, children)
            child->d_func()->setAncestorHandlesChildEvents(enabled);
    }
}

/*!
    \internal

    Propagates item group membership.
*/
void QGraphicsItemPrivate::setIsMemberOfGroup(bool enabled)
{
    Q_Q(QGraphicsItem);
    isMemberOfGroup = enabled;
    if (!qgraphicsitem_cast<QGraphicsItemGroup *>(q)) {
        foreach (QGraphicsItem *child, children)
            child->d_func()->setIsMemberOfGroup(enabled);
    }
}

/*!
    \internal

    Maps any item pos properties of \a event to \a item's coordinate system.
*/
void QGraphicsItemPrivate::remapItemPos(QEvent *event, QGraphicsItem *item)
{
    Q_Q(QGraphicsItem);
    switch (event->type()) {
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick: {
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        mouseEvent->setPos(item->mapFromItem(q, mouseEvent->pos()));
        mouseEvent->setLastPos(item->mapFromItem(q, mouseEvent->pos()));
        for (int i = 0x1; i <= 0x10; i <<= 1) {
            if (mouseEvent->buttons() & i) {
                Qt::MouseButton button = Qt::MouseButton(i);
                mouseEvent->setButtonDownPos(button, item->mapFromItem(q, mouseEvent->buttonDownPos(button)));
            }
        }
        break;
    }
    case QEvent::GraphicsSceneWheel: {
        QGraphicsSceneWheelEvent *wheelEvent = static_cast<QGraphicsSceneWheelEvent *>(event);
        wheelEvent->setPos(item->mapFromItem(q, wheelEvent->pos()));
        break;
    }
    case QEvent::GraphicsSceneContextMenu: {
        QGraphicsSceneContextMenuEvent *contextEvent = static_cast<QGraphicsSceneContextMenuEvent *>(event);
        contextEvent->setPos(item->mapFromItem(q, contextEvent->pos()));
        break;
    }
    case QEvent::GraphicsSceneHoverMove: {
        QGraphicsSceneHoverEvent *hoverEvent = static_cast<QGraphicsSceneHoverEvent *>(event);
        hoverEvent->setPos(item->mapFromItem(q, hoverEvent->pos()));
        break;
    }
    default:
        break;
    }
}

/*!
    Constructs a QGraphicsItem with the parent \a parent on \a scene. If \a
    parent is 0, the item will be a top-level. If \a scene is 0, the item
    will not be associated with a scene.

    \sa QGraphicsScene::addItem()
*/
QGraphicsItem::QGraphicsItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : d_ptr(new QGraphicsItemPrivate)
{
    d_ptr->q_ptr = this;
    setParentItem(parent);
    if (scene && d_ptr->scene != scene)
        scene->addItem(this);
}

/*!
    \internal
*/
QGraphicsItem::QGraphicsItem(QGraphicsItemPrivate &dd, QGraphicsItem *parent,
                             QGraphicsScene *scene)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    setParentItem(parent);
    if (scene && d_ptr->scene != scene)
        scene->addItem(this);
}

/*!
    Destroys the QGraphicsItem and all its children. If this item is currently
    associated with a scene, the item will be removed from the scene before it
    is deleted.
*/
QGraphicsItem::~QGraphicsItem()
{
    QVariant variant;
    foreach (QGraphicsItem *child, d_ptr->children) {
        if (QGraphicsItem *parent = child->parentItem()) {
            qVariantSetValue<QGraphicsItem *>(variant, child);
            parent->itemChange(ItemChildRemovedChange, variant);
        }
        delete child;
    }
    d_ptr->children.clear();

    if (QGraphicsItem *parent = parentItem()) {
        qVariantSetValue<QGraphicsItem *>(variant, this);
        parent->itemChange(ItemChildRemovedChange, variant);
        parent->d_func()->children.removeAll(this);
    }
    if (d_ptr->scene)
        d_ptr->scene->d_func()->_q_removeItemLater(this);

    delete d_ptr;

    qt_dataStore()->data.remove(this);
}

/*!
    Returns the current scene for the item, or 0 if the item is not stored in
    a scene.

    Too add or move an item to a scene, call QGraphicsScene::addItem().
*/
QGraphicsScene *QGraphicsItem::scene() const
{
    return d_ptr->scene;
}

/*!
    Returns a pointer to this item's item group, or 0 if this item is not
    member of a group.

    \sa QGraphicsItemGroup, QGraphicsScene::createItemGroup()
*/
QGraphicsItemGroup *QGraphicsItem::group() const
{
    if (!d_ptr->isMemberOfGroup)
        return 0;
    QGraphicsItem *parent = const_cast<QGraphicsItem *>(this);
    while ((parent = parent->parentItem())) {
        if (QGraphicsItemGroup *group = qgraphicsitem_cast<QGraphicsItemGroup *>(parent))
            return group;
    }
    // Unreachable; if d_ptr->isMemberOfGroup is != 0, then one parent of this
    // item is a group item.
    return 0;
}

/*!
    Adds this item to the item group \a group. If \a group is 0, this item is
    removed from any current group and added as a child of the previous
    group's parent.

    \sa group(), QGraphicsScene::createItemGroup()
*/
void QGraphicsItem::setGroup(QGraphicsItemGroup *group)
{
    if (!group) {
        if (QGraphicsItemGroup *group = this->group())
            group->removeFromGroup(this);
    } else {
        group->addToGroup(this);
    }
}

/*!
    Returns a pointer to this item's parent item. If this item does not have a
    parent, 0 is returned.

    \sa setParentItem(), children()
*/
QGraphicsItem *QGraphicsItem::parentItem() const
{
    return d_ptr->parent;
}

/*!
    Returns this item's top-level item. The top-level item is the item's
    topmost ancestor item whose parent is 0. If an item has no parent, its own
    pointer is returned (i.e., a top-level item is its own top-level item).

    \sa parentItem()
*/
QGraphicsItem *QGraphicsItem::topLevelItem() const
{
    QGraphicsItem *parent = const_cast<QGraphicsItem *>(this);
    while (QGraphicsItem *grandPa = parent->parentItem())
        parent = grandPa;
    return parent;
}

/*!
    Sets this item's parent item to \a parent. If this item already has a
    parent, it is first removed from the previous parent. If \a parent is 0,
    this item will become a top-level item.

    \sa parentItem(), children()
*/
void QGraphicsItem::setParentItem(QGraphicsItem *parent)
{
    if (parent == this) {
        qWarning("QGraphicsItem::setParentItem: cannot assign %p as a parent of itself", this);
        return;
    }
    if (parent == d_ptr->parent)
        return;
    QVariant variant;
    qVariantSetValue<QGraphicsItem *>(variant, parent);
    parent = qVariantValue<QGraphicsItem *>(itemChange(ItemParentChange, variant));
    if (parent == d_ptr->parent)
        return;

    if (d_ptr->parent) {
        // Remove from current parent
        removeFromIndex();
        d_ptr->parent->d_func()->children.removeAll(this);
        qVariantSetValue<QGraphicsItem *>(variant, this);
        d_ptr->parent->itemChange(ItemChildRemovedChange, variant);
    }

    if ((d_ptr->parent = parent)) {
        d_ptr->parent->d_func()->children << this;
        qVariantSetValue<QGraphicsItem *>(variant, this);
        d_ptr->parent->itemChange(ItemChildAddedChange, variant);
        addToIndex();

        // Optionally inherit ancestor event handling from the new parent
        if (!d_ptr->handlesChildEvents) {
            d_ptr->setAncestorHandlesChildEvents(parent->handlesChildEvents()
                                             || parent->d_func()->ancestorHandlesChildEvents);
        }
    } else {
        // Item is a top-level; clear the ancestor event handling flag
        d_ptr->setAncestorHandlesChildEvents(false);
        addToIndex();
    }
}

/*!
    Returns a list of this item's children. The items are returned in no
    particular order.

    \sa setParentItem()
*/
QList<QGraphicsItem *> QGraphicsItem::children() const
{
    return d_ptr->children;
}

/*!
    Returns this item's flags. The flags describe what configurable features
    of the item are enabled and not. For example, if the flags include
    ItemIsFocusable, the item can accept input focus.

    By default, no flags are enabled.

    \sa setFlags(), setFlag()
*/
QGraphicsItem::GraphicsItemFlags QGraphicsItem::flags() const
{
    return GraphicsItemFlags(d_ptr->flags);
}

/*!
    If \a enabled is true, the item flag \a flag is enabled; otherwise, it is
    disabled.

    \sa flags(), setFlags()
*/
void QGraphicsItem::setFlag(GraphicsItemFlag flag, bool enabled)
{
    if (enabled)
        setFlags(flags() | flag);
    else
        setFlags(flags() & ~flag);
}

/*!
    Sets the item flags to \a flags. All flags in \a flags are enabled; all
    flags not in \a flags are disabled.

    If the item had focus and \a flags does not enable ItemIsFocusable, the
    item loses focus as a result of calling this function. Similarly, if the
    item was selected, and \a flags does not enabled ItemIsSelectable, the
    item is automatically unselected.

    By default, no flags are enabled.

    \sa flags(), setFlag()
*/
void QGraphicsItem::setFlags(GraphicsItemFlags flags)
{
    if (GraphicsItemFlags(d_ptr->flags) != flags) {
        d_ptr->flags = flags;

        if (!(d_ptr->flags & ItemIsFocusable) && hasFocus())
            clearFocus();
        if (!(d_ptr->flags & ItemIsSelectable) && isSelected())
            setSelected(false);

        update();
    }
}

#ifndef QT_NO_TOOLTIP
/*!
    Returns the item's tool tip, or an empty QString if no tool tip has been
    set.

    \sa setToolTip(), QToolTip
*/
QString QGraphicsItem::toolTip() const
{
    return d_ptr->extra(QGraphicsItemPrivate::ExtraToolTip).toString();
}

/*!
    Sets the item's tool tip to \a toolTip. If \a toolTip is empty, the item's
    tool tip is cleared.

    \sa toolTip(), QToolTip
*/
void QGraphicsItem::setToolTip(const QString &toolTip)
{
    d_ptr->setExtra(QGraphicsItemPrivate::ExtraToolTip, toolTip);
}
#endif // QT_NO_TOOLTIP

#ifndef QT_NO_CURSOR
/*!
    Returns the current cursor shape for the item. The mouse cursor
    will assume this shape when it's over this item. See the \link
    Qt::CursorShape list of predefined cursor objects\endlink for a
    range of useful shapes.

    An editor item might want to use an I-beam cursor:

    \code
        item->setCursor(Qt::IBeamCursor);
    \endcode

    If no cursor has been set, the parent's cursor is used.

    \sa setCursor(), hasCursor(), unsetCursor(), QWidget::cursor,
    QApplication::overrideCursor()
*/
QCursor QGraphicsItem::cursor() const
{
    return qVariantValue<QCursor>(d_ptr->extra(QGraphicsItemPrivate::ExtraCursor));
}

/*!
    Sets the current cursor shape for the item to \a cursor. The mouse cursor
    will assume this shape when it's over this item. See the \link
    Qt::CursorShape list of predefined cursor objects\endlink for a range of
    useful shapes.

    An editor item might want to use an I-beam cursor:

    \code
        item->setCursor(Qt::IBeamCursor);
    \endcode

    If no cursor has been set, the parent's cursor is used.

    \sa cursor(), hasCursor(), unsetCursor(), QWidget::cursor,
    QApplication::overrideCursor()
*/
void QGraphicsItem::setCursor(const QCursor &cursor)
{
    d_ptr->setExtra(QGraphicsItemPrivate::ExtraCursor, cursor);
    d_ptr->hasCursor = 1;
    if (d_ptr->scene) {
        foreach (QGraphicsView *view, d_ptr->scene->views()) {
            if (view->underMouse() && view->itemAt(view->mapFromGlobal(QCursor::pos())) == this) {
                view->viewport()->setCursor(cursor);
                break;
            }
        }
    }
}

/*!
    Returns true if this item has a cursor set; otherwise, false is returned.

    By default, items don't have any cursor set. cursor() will return a
    standard pointing arrow cursor.

    \sa unsetCursor()
*/
bool QGraphicsItem::hasCursor() const
{
    return d_ptr->hasCursor;
}

/*!
    Clears the cursor from this item.

    \sa hasCursor(), setCursor()
*/
void QGraphicsItem::unsetCursor()
{
    d_ptr->unsetExtra(QGraphicsItemPrivate::ExtraCursor);
    d_ptr->hasCursor = 0;
}

#endif // QT_NO_CURSOR

/*!
   Returns true if the item is visible; otherwise, false is returned.

   Note that the item's general visibility is unrelated to whether or not it
   is actually being visualized by a QGraphicsView.

   \sa setVisible()
*/
bool QGraphicsItem::isVisible() const
{
    return d_ptr->visible;
}

/*!
    If \a visible is true, the item is made visible. Otherwise, the item is
    made invisible. Invisible items are not painted, nor do they receive any
    events. In particular, mouse events pass right through invisible items,
    and are delivered to any item that may be behind. Invisible items are also
    unselectable, they cannot take input focus, and are not detected by
    QGraphicsScene's item location functions.

    If an item becomes invisible while grabbing the mouse, (i.e., while it is
    receiving mouse events,) it will automatically lose the mouse grab, and
    the grab is not regained by making the item visible again; it must receive
    a new mouse press to regain the mouse grab.

    Similarly, an invisible item cannot have focus, so if the item has focus
    when it becomes invisible, it will lose focus, and the focus is not
    regained by simply making the item visible again.

    Items are visible by default; it is unnecessary to call
    setVisible() on a new item.

    \sa isVisible(), show(), hide()
*/
void QGraphicsItem::setVisible(bool visible)
{
    if (d_ptr->visible == quint32(visible))
        return;
    if (!visible) {
        if (d_ptr->scene && d_ptr->scene->mouseGrabberItem() == this)
            d_ptr->scene->d_func()->mouseGrabberItem = 0;
        if (hasFocus())
            clearFocus();
        if (isSelected())
            setSelected(false);
    }

    if (!visible)
        update();
    d_ptr->visible = itemChange(ItemVisibleChange, quint32(visible)).toBool();
    if (visible)
        update();

    foreach (QGraphicsItem *child, children())
        child->setVisible(visible);
}

/*!
    \fn void QGraphicsItem::hide()

    Hides the item. (Items are visible by default.)

    This convenience function is equivalent to calling \c setVisible(false).

    \sa show(), setVisible()
*/

/*!
    \fn void QGraphicsItem::show()

    Shows the item. (Items are visible by default.)

    This convenience function is equivalent to calling \c setVisible(true).

    \sa hide(), setVisible()
*/

/*!
    Returns true if the item is enabled; otherwise, false is returned.

    \sa setEnabled()
*/
bool QGraphicsItem::isEnabled() const
{
    return d_ptr->enabled;
}

/*!
    If \a enabled is true, the item is enabled; otherwise, it is disabled.

    Disabled items are visible, but they do not receive any events, and cannot
    take focus nor be selected. Mouse events are discarded; they are not
    propagated unless the item is also invisible, or if it does not accept
    mouse events (see acceptedMouseButtons()). A disabled item cannot become the
    mouse grabber, and as a result of this, an item loses the grab if it
    becomes disabled when grabbing the mouse, just like it loses focus if it
    had focus when it was disabled.

    Disabled items are traditionally drawn using grayed-out colors (see \l
    QPalette::Disabled).

    Items are enabled by default.

    \sa isEnabled()
*/
void QGraphicsItem::setEnabled(bool enabled)
{
    if (d_ptr->enabled == quint32(enabled))
        return;
    if (!enabled) {
        if (d_ptr->scene && d_ptr->scene->mouseGrabberItem() == this)
            d_ptr->scene->d_func()->mouseGrabberItem = 0;
        if (hasFocus())
            clearFocus();
        if (isSelected())
            setSelected(false);
    }
    d_ptr->enabled = itemChange(ItemEnabledChange, quint32(enabled)).toBool();
    update();

    foreach (QGraphicsItem *child, children())
        child->setEnabled(enabled);
}

/*!
    Returns true if this item is selected; otherwise, false is returned.

    Items that are in a group inherit the group's selected state.

    Items are not selected by default.

    \sa setSelected(), QGraphicsScene::setSelectionArea()
*/
bool QGraphicsItem::isSelected() const
{
    if (QGraphicsItemGroup *group = this->group())
        return group->isSelected();
    return d_ptr->selected;
}

/*!
    If \a selected is true and this item is selectable, this item is selected;
    otherwise, it is unselected.

    If the item is in a group, the whole group's selected state is toggled by
    this function. If the group is selected, all items in the group are also
    selected, and if the group is not selected, no item in the group is
    selected.

    Only visible, enabled, selectable items can be selected.  If \a selected
    is true and this item is either invisible or disabled or unselectable,
    this function does nothing.

    By default, items cannot be selected. To enable selection, set the
    ItemIsSelectable flag.

    This function is provided for convenience, allowing individual toggling of
    the selected state of an item. However, a more common way of selecting
    items is to call QGraphicsScene::setSelectionArea(), which will call this
    function for all visible, enabled, and selectable items within a specified
    area on the scene.

    \sa isSelected(), QGraphicsScene::selectedItems()
*/
void QGraphicsItem::setSelected(bool selected)
{
    if (QGraphicsItemGroup *group = this->group()) {
        group->setSelected(selected);
        return;
    }

    if (!(d_ptr->flags & ItemIsSelectable) || !d_ptr->enabled || !d_ptr->visible)
        selected = false;
    if (d_ptr->selected == selected)
        return;

    d_ptr->selected = itemChange(ItemSelectedChange, quint32(selected)).toBool();

    update();
    if (selected && d_ptr->scene) {
        d_ptr->scene->d_func()->selectedItems << this;
    } else {
        // QGraphicsScene::selectedItems() lazily pulls out all items that are
        // no longer selected.
    }
}

/*!
   Returns true if this item can accept drag and drop events; otherwise,
   returns false. By default, items do not accept drag and drop events; items
   are transparent to drag and drop.

   \sa setAcceptDrops()
*/
bool QGraphicsItem::acceptDrops() const
{
    return d_ptr->acceptDrops;
}

/*!
    If \a on is true, this item will accept drag and drop events; otherwise,
    it is transparent for drag and drop events. By default, items do not
    accept drag and drop events.

    \sa acceptDrops()
*/
void QGraphicsItem::setAcceptDrops(bool on)
{
    d_ptr->acceptDrops = on;
}

/*!
    Returns the mouse buttons that this item accepts mouse events for.  By
    default, all mouse buttons are accepted.

    If an item accepts a mouse button, it will become the mouse
    grabber item when a mouse press event is delivered for that mouse
    button. However, if the item does not accept the button,
    QGraphicsScene will forward the mouse events to the first item
    beneath it that does.

    \sa setAcceptedMouseButtons(), mousePressEvent()
*/
Qt::MouseButtons QGraphicsItem::acceptedMouseButtons() const
{
    return Qt::MouseButtons(d_ptr->acceptedMouseButtons);
}

/*!
    Sets the mouse \a buttons that this item accepts mouse events for.

    By default, all mouse buttons are accepted. If an item accepts a
    mouse button, it will become the mouse grabber item when a mouse
    press event is delivered for that button. However, if the item
    does not accept the mouse button, QGraphicsScene will forward the
    mouse events to the first item beneath it that does.

    To disable mouse events for an item (i.e., make it transparent for mouse
    events), call setAcceptedMouseButtons(0).

    \sa acceptedMouseButtons(), mousePressEvent()
*/
void QGraphicsItem::setAcceptedMouseButtons(Qt::MouseButtons buttons)
{
    if (Qt::MouseButtons(d_ptr->acceptedMouseButtons) != buttons) {
        if (buttons == 0 && d_ptr->scene && d_ptr->scene->mouseGrabberItem() == this)
            d_ptr->scene->d_func()->mouseGrabberItem = 0;
        d_ptr->acceptedMouseButtons = quint32(buttons);
    }
}

/*!
    Returns true if an item accepts hover events
    (QGraphicsSceneHoverEvent); otherwise, returns false. By default,
    items do not accept hover events.

    \sa setAcceptedMouseButtons()
*/
bool QGraphicsItem::acceptsHoverEvents() const
{
    return d_ptr->acceptsHover;
}

/*!
    If \a enabled is true, this item will accept hover events;
    otherwise, it will ignore them. By default, items do not accept
    hover events.

    Hover events are delivered when there is no current mouse grabber
    item.  They are sent when the mouse cursor enters an item, when it
    moves around inside the item, and when the cursor leaves an
    item. Hover events are commonly used to highlight an item when
    it's entered, and for tracking the mouse cursor as it hovers over
    the item (equivalent to QWidget::mouseTracking).

    Parent items receive hover enter events before their children, and
    leave events after their children. The parent does not receive a
    hover leave event if the cursor enters a child, though; the parent
    stays "hovered" until the cursor leaves its area, including its
    children's areas.

    If a parent item handles child events (setHandlesChildEvents()), it will
    receive hover move, drag move, and drop events as the cursor passes
    through its children, but it does not receive hover enter and hover leave,
    nor drag enter and drag leave events on behalf of its children.

    \sa acceptsHoverEvents(), hoverEnterEvent(), hoverMoveEvent(),
    hoverLeaveEvent()
*/
void QGraphicsItem::setAcceptsHoverEvents(bool enabled)
{
    d_ptr->acceptsHover = quint32(enabled);
}

/*!
    Returns true if this item handles child events (i.e., all events
    intended for any of its children are instead sent to this item);
    otherwise, false is returned.

    This property is useful for item groups; it allows one item to
    handle events on behalf of its children, as opposed to its
    children handling their events individually.

    The default is to return false; children handle their own events.
    The exception for this is if the item is a QGraphicsItemGroup, then 
    it defaults to return true.

    \sa setHandlesChildEvents()
*/
bool QGraphicsItem::handlesChildEvents() const
{
    return d_ptr->handlesChildEvents;
}

/*!
    If \a enabled is true, this item is set to handle all events for
    all its children (i.e., all events intented for any of its
    children are instead sent to this item); otherwise, if \a enabled
    is false, this item will only handle its own events. The default
    value is false.

    This property is useful for item groups; it allows one item to
    handle events on behalf of its children, as opposed to its
    children handling their events individually.

    If a child item accepts hover events, its parent will receive
    hover move events as the cursor passes through the child, but it
    does not receive hover enter and hover leave events on behalf of
    its child.

    \sa handlesChildEvents()
*/
void QGraphicsItem::setHandlesChildEvents(bool enabled)
{
    if (d_ptr->handlesChildEvents == enabled)
        return;

    d_ptr->handlesChildEvents = enabled;
    foreach (QGraphicsItem *item, d_ptr->children)
        item->d_func()->setAncestorHandlesChildEvents(enabled);
}

/*!
    Returns true if this item has focus (i.e., can accept key events);
    otherwise, returns false.

    \sa setFocus(), QGraphicsScene::setFocusItem()
*/
bool QGraphicsItem::hasFocus() const
{
    return d_ptr->scene && d_ptr->scene->focusItem() == this;
}

/*!
    Gives keyboard input focus to this item. The \a focusReason argument will
    be passed into any focus event generated by this function; it is used to
    give an explanation of what caused the item to get focus.

    Only items that set the ItemIsFocusable flag can accept keyboard focus.

    If this item is not visible (i.e., isVisible() returns false), not
    enabled, not associated with a scene, or if it already has input focus,
    this function will do nothing.

    As a result of calling this function, this item will receive a focus in
    event with \a focusReason. If another item already has focus, that item
    will first receive a focus out event indicating that it has lost input
    focus.

    \sa clearFocus(), hasFocus()
*/
void QGraphicsItem::setFocus(Qt::FocusReason focusReason)
{
    if (!d_ptr->scene || !isVisible() || !isEnabled() || hasFocus())
        return;
    d_ptr->scene->setFocusItem(this, focusReason);
}

/*!
    Takes keyboard input focus from the item.

    If it has focus, a focus out event is sent to this item to tell it that it
    is about to lose the focus.

    Only items that set the ItemIsFocusable flag can accept keyboard focus.

    \sa setFocus()
*/
void QGraphicsItem::clearFocus()
{
    if (!d_ptr->scene || !hasFocus())
        return;
    d_ptr->scene->setFocusItem(0);
}

/*!
    Returns the position of the item in parent coordinates. If the item has no
    parent, its position is given in scene coordinates.

    The position of the item describes its origin (local coordinate
    (0, 0)) in parent coordinates; this function returns the same as
    mapToParent(0, 0).

    For convenience, you can also call scenePos() to determine the
    item's position in scene coordinates, regardless of its parent.

    \sa x(), y(), setPos(), matrix(), {The Graphics View Coordinate System}
*/
QPointF QGraphicsItem::pos() const
{
    return d_ptr->pos;
}

/*!
    \fn QGraphicsItem::x() const

    This convenience function is equivalent to calling pos().x().

    \sa y()
*/

/*!
    \fn QGraphicsItem::y() const

    This convenience function is equivalent to calling pos().y().

    \sa x()
*/

/*!
    Returns the item's position in scene coordinates. This is
    equivalent to calling \c mapToScene(0, 0).

    \sa pos(), sceneMatrix(), {The Graphics View Coordinate System}
*/
QPointF QGraphicsItem::scenePos() const
{
    return mapToScene(0, 0);
}

/*!
    Sets the position of the item to \a pos, which is in parent
    coordinates.  For items with no parent, \a pos is in scene
    coordinates.

    The position of the item describes its origin (local coordinate
    (0, 0)) in parent coordinates.

    \sa pos(), scenePos(), {The Graphics View Coordinate System}
*/
void QGraphicsItem::setPos(const QPointF &pos)
{
    if(d_ptr->pos == pos)
        return;
    if (d_ptr->scene) {
        qt_graphicsItem_fullUpdate(this);
        removeFromIndex();
    }
    d_ptr->pos = itemChange(ItemPositionChange, pos).toPointF();
    if (d_ptr->scene) {
        qt_graphicsItem_fullUpdate(this);
        addToIndex();
    }
}

/*!
    \fn void QGraphicsItem::setPos(qreal x, qreal y)
    \overload

    This convenience function is equivalent to calling setPos(QPointF(\a x, \a
    y)).
*/

/*!
    \fn void QGraphicsItem::moveBy(qreal dx, qreal dy)

    Moves the item by \a dx points horizontally, and \a dy point
    vertically. This function is equivalent to calling setPos(pos() +
    QPointF(\a dx, \a dy)).
*/

/*!
    If this item is part of a scene that is viewed by a QGraphicsView, this
    convenience function will attempt to scroll the view to ensure that \a
    rect is visible inside the view's viewport. If \a rect is a null rect (the
    default), QGraphicsItem will default to the item's bounding rect. \a xmargin
    and \a ymargin are the number of pixels the view should use for margins.

    If the specified rect cannot be reached, the contents are scrolled to the
    nearest valid position.

    If this item is not viewed by a QGraphicsView, this function does nothing.

    \sa QGraphicsView::ensureVisible()
*/
void QGraphicsItem::ensureVisible(const QRectF &rect, int xmargin, int ymargin)
{
    if (d_ptr->scene) {
        QRectF sceneRect = !rect.isNull() ? sceneMatrix().mapRect(rect)
                           : sceneBoundingRect();
        foreach (QGraphicsView *view, d_ptr->scene->d_func()->views)
            view->ensureVisible(sceneRect, xmargin, ymargin);
    }
}

/*!
    \fn void QGraphicsItem::ensureVisible(qreal x, qreal y, qreal w, qreal h,
    int xmargin = 50, int ymargin = 50)

    This convenience function is equivalent to calling
    ensureVisible(QRectF(\a x, \a y, \a w, \a h), \a xmargin, \a ymargin):
*/

/*!
    Returns this item's transformation matrix. If no matrix has been set, the
    identity matrix is returned.

    \sa setMatrix(), sceneMatrix()
*/
QMatrix QGraphicsItem::matrix() const
{
    if (!d_ptr->hasMatrix)
        return QMatrix();
    return qVariantValue<QMatrix>(d_ptr->extra(QGraphicsItemPrivate::ExtraMatrix));
}

/*!
    Returns this item's scene transformation matrix. This matrix can be used
    to map coordinates and geometrical shapes from this item's local
    coordinate system to the scene's coordinate system. To map coordinates
    from the scene, you must first invert the returned matrix.

    Example:

    \code
        QGraphicsRectItem rect;
        rect.setPos(100, 100);

        rect.sceneMatrix().map(QPointF(0, 0));
        // returns QPointF(100, 100);

        rect.sceneMatrix().inverted().map(QPointF(100, 100));
        // returns QPointF(0, 0);
    \endcode

    Unlike matrix(), which returns only an item's local transformation, this
    function includes the item's (and any parents') position.

    \sa matrix(), setMatrix(), scenePos(), {The Graphics View Coordinate System}
*/
QMatrix QGraphicsItem::sceneMatrix() const
{
    QMatrix m = matrix() * QMatrix().translate(d_ptr->pos.x(), d_ptr->pos.y());
    return d_ptr->parent ? m * d_ptr->parent->sceneMatrix() : m;
}

/*!
    Sets the item's current transformation matrix to \a matrix.

    If \a combine is true, then \a matrix is combined with the current matrix;
    otherwise, \a matrix \e replaces the current matrix. \a combine is false
    by default.

    To simplify interaction with items using a transformed view, QGraphicsItem
    provides mapTo... and mapFrom... functions that can translate between
    items' and the scene's coordinates. For example, you can call mapToScene()
    to map an item coordiate to a scene coordinate, or mapFromScene() to map
    from scene coordinates to item coordinates.

    \sa matrix(), rotate(), scale(), shear(), translate(), {The Graphics View Coordinate System}
*/
void QGraphicsItem::setMatrix(const QMatrix &matrix, bool combine)
{
    QMatrix oldMatrix = this->matrix();
    QMatrix newMatrix;
    if (!combine)
        newMatrix = matrix;
    else
        newMatrix = matrix * oldMatrix;
    if (oldMatrix == newMatrix)
        return;

    qt_graphicsItem_fullUpdate(this);
    removeFromIndex();
    d_ptr->hasMatrix = !newMatrix.isIdentity();

    QVariant variant;
    qVariantSetValue<QMatrix>(variant, newMatrix);
    d_ptr->setExtra(QGraphicsItemPrivate::ExtraMatrix,
                    itemChange(ItemMatrixChange, variant));
    addToIndex();
    qt_graphicsItem_fullUpdate(this);
}

/*!
    Resets this item's tranformation matrix to the identity matrix. This is
    equivalent to calling \c setMatrix(QMatrix()).

    \sa setMatrix(), matrix()
*/
void QGraphicsItem::resetMatrix()
{
    setMatrix(QMatrix(), false);
}

/*!
    Rotates the current item transformation \a angle degrees clockwise.

    \sa setMatrix(), matrix(), scale(), shear(), translate()
*/
void QGraphicsItem::rotate(qreal angle)
{
    setMatrix(QMatrix().rotate(angle), true);
}

/*!
    Scales the current item transformation by (\a sx, \a sy).

    \sa setMatrix(), matrix(), rotate(), shear(), translate()
*/
void QGraphicsItem::scale(qreal sx, qreal sy)
{
    setMatrix(QMatrix().scale(sx, sy), true);
}

/*!
    Shears the current item transformation by (\a sh, \a sv).

    \sa setMatrix(), matrix(), rotate(), scale(), translate()
*/
void QGraphicsItem::shear(qreal sh, qreal sv)
{
    setMatrix(QMatrix().shear(sh, sv), true);
}

/*!
    Translates the current item transformation by (\a dx, \a dy).

    If all you want is to move an item, you should call moveBy() or
    setPos() instead; this function changes the item's translation,
    which is conceptually separate from its position.

    \sa setMatrix(), matrix(), rotate(), scale(), shear()
*/
void QGraphicsItem::translate(qreal dx, qreal dy)
{
    setMatrix(QMatrix().translate(dx, dy), true);
}

/*!
    This virtual function is called twice for all items by the
    QGraphicsScene::advance() slot. In the first phase, all items are called
    with \a phase == 0, indicating that items on the scene are about to
    advance, and then all items are called with \a phase == 1. Reimplement
    this function to update your item if you need simple scene-controlled
    animation.

    The default implementation does nothing.

    For individual item animation, an alternative to this function is to
    either use QGraphicsItemAnimation, or to multiple-inherit from QObject and
    QGraphicsItem, and animate your item using QObject::startTimer() and
    QObject::timerEvent().

    \sa QGraphicsItemAnimation, QTimeLine
*/
void QGraphicsItem::advance(int phase)
{
    Q_UNUSED(phase);
}

/*!
    Returns the Z-value, or the elevation, of the item. The Z-value decides
    the stacking order of sibling (neighboring) items.

    The default Z-value is 0.

    \sa setZValue()
*/
qreal QGraphicsItem::zValue() const
{
    return d_ptr->z;
}

/*!
    Sets the Z-value, or the elevation, of the item, to \a z. The elevation
    decides the stacking order of sibling (neighboring) items. An item of high
    Z-value will be drawn on top of an item with a lower Z-value if they
    share the same parent item. In addition, children of an item will always be drawn
    on top of the parent, regardless of the child's Z-value. Sibling items
    that share the same Z-value will be drawn in an undefined order, although
    the order will stay the same for as long as the items live.

    Children of different parents are stacked according to the Z-value of
    each item's ancestor item which is an immediate child of the two
    items' closest common ancestor. For example, a robot item might
    define a torso item as the parent of a head item, two arm items,
    and two upper-leg items. The upper-leg items would each be parents
    of one lower-leg item, and each lower-leg item would be parents of
    one foot item.  The stacking order of the feet is the same as the
    stacking order of each foot's ancestor that is an immediate child
    of the two feet's common ancestor (i.e., the torso item); so the
    feet are stacked in the same order as the upper-leg items,
    regardless of each foot's Z-value.

    The Z-value does not affect the item's size in any way.

    The default Z-value is 0.

    \sa zValue()
*/
void QGraphicsItem::setZValue(qreal z)
{
    if (z != d_ptr->z) {
        d_ptr->z = z;
        qt_graphicsItem_fullUpdate(this);
    }
}

/*!
    Returns the bounding rect of this item's descendents (i.e., its children,
    their children, etc.) in local coordinates. If the item has no children,
    this function returns an empty QRectF.

    This does not include this item's own bounding rect; it only returns
    its descendents' accumulated bounding rect. If you need to include this
    item's bounding rect, you can add boundingRect() to childrenBoundingRect()
    using QRectF::operator|().

    This function is linear in complexity; it determines the size of the
    returned bounding rect by iterating through all descendents.

    \sa boundingRect(), sceneBoundingRect()
*/
QRectF QGraphicsItem::childrenBoundingRect() const
{
    QRectF childRect;
    foreach (QGraphicsItem *child, children()) {
        QPointF childPos = child->pos();
        QMatrix matrix = child->matrix() * QMatrix().translate(childPos.x(), childPos.y());
        childRect |= matrix.mapRect(child->boundingRect() | child->childrenBoundingRect());
    }
    return childRect;
}

/*!
    \fn virtual QRectF QGraphicsItem::boundingRect() const = 0

    This pure virtual function defines the outer bounds of the item as
    a rectangle; all painting must be restricted to inside an item's
    bounding rect. QGraphicsView uses this to determine whether the
    item requires redrawing.

    Although the item's shape can be arbitrary, the bounding rect is
    always rectangular, and it is unaffected by the items'
    transformation (scale(), rotate(), etc.).

    Reimplement this function to let QGraphicsView determine what
    parts of the widget, if any, need to be redrawn.

    Note: For shapes that paint an outline / stroke, it is important
    to include half the pen width in the bounding rect. It is not
    necessary to compensate for antialiasing, though.

    Example:

    \code
    QRectF CircleItem::boundingRect() const
    {
        qreal penWidth = 1;
        return QRectF(-radius - penWidth / 2, -radius - penWidth / 2,
                      diameter + penWidth, diameter + penWidth);
    }
    \endcode

    \sa shape(), contains(), {The Graphics View Coordinate System}
*/

/*!
    Returns the bounding rect of this item in scene coordinates, by combining
    sceneMatrix() with boundingRect().

    \sa boundingRect(), {The Graphics View Coordinate System}
*/
QRectF QGraphicsItem::sceneBoundingRect() const
{
    if (d_ptr->parent || d_ptr->hasMatrix)
        return sceneMatrix().mapRect(boundingRect());
    return boundingRect().translated(d_ptr->pos);
}

/*!
    Returns the shape of this item as a QPainterPath in local
    coordinates. The shape is used for many things, including collision
    detection, hit tests, and for the QGraphicsScene::items() functions.

    The default implementation calls boundingRect() to return a simple
    rectangular shape, but subclasses can reimplement this function to return
    a more accurate shape for non-rectangular items. For example, a round item
    may choose to return an elliptic shape for better collision detection. For
    example:

    \code
        QPainterPath RoundItem::shape() const
        {
            QPainterPath path;
            path.addEllipse(boundingRect());
            return path;
        }
    \endcode

    This function is called by the default implementations of contains() and
    collidesWithPath().

    \sa boundingRect(), contains()
*/
QPainterPath QGraphicsItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

/*!
    Returns true if this item contains \a point, which is in local
    coordinates; otherwise, false is returned. It is most often called from
    QGraphicsView to determine what item is under the cursor, and for that
    reason, the implementation of this function should be as light-weight as
    possible.

    By default, this function calls shape(), but you can reimplement it in a
    subclass to provide a (perhaps more efficient) implementation.

    \sa shape(), boundingRect(), collidesWithPath()
*/
bool QGraphicsItem::contains(const QPointF &point) const
{
    return shape().contains(point);
}

/*!
    Returns true if this item collides with \a other; otherwise returns false.
    The ways items collide is determined by \a mode. The default value for \a
    mode is Qt::IntersectsItemShape; \a other collides with this item if it
    either intersect or are contained by this item's shape.

    The default implementation is based on shape intersection, and it calls
    shape() on both items. Because the complexity of arbitrary shape-shape
    intersection grows with an order of magnitude when the shapes are complex,
    this operation can be noticably time consuming. You have the option of
    reimplementing this function in a subclass of QGraphicsItem to provide a
    custom algorithm. This allows you to make use of natural constraints in
    the shapes of your own items, in order to improve the performance of the
    collision detection. For instance, two untransformed perfectly circular
    items' collision can be determined very efficiently by comparing their
    positions and radii.

    Keep in mind that when reimplementing this function and calling shape() or
    boundingRect() on \a other, the returned coordinates must be mapped to
    this item's coordinate system before any intersection can take place.

    \sa contains(), shape()
*/
bool QGraphicsItem::collidesWithItem(const QGraphicsItem *other, Qt::ItemSelectionMode mode) const
{
    if (other == this)
        return true;
    if (!other)
        return false;
    return collidesWithPath(mapFromItem(other, other->shape()), mode);
}

/*!
    Returns true if this item collides with \a path.

    The collision is determined by \a mode. The default value for \a mode is
    Qt::IntersectsItemShape; \a path collides with this item if it either
    intersects or is contained by this item's shape.

    \sa collidesWithItem(), contains(), shape()
*/
bool QGraphicsItem::collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode mode) const
{
    if (path.isEmpty()) {
        // No collision with empty paths.
        return false;
    }

    QRectF rectA = boundingRect();
    QRectF rectB = path.controlPointRect();
    if (!rectA.intersects(rectB)) {
        // This we can determine efficiently. If the two rects neither
        // intersect nor contain eachother, then the two items do not collide.
        return false;
    }

    // For further testing, we need this item's shape or bounding rect.
    bool checkShape = (mode == Qt::IntersectsItemShape || mode == Qt::ContainsItemShape);
    QPainterPath thisShape;
    if (checkShape)
        thisShape = shape();
    else
        thisShape.addRect(boundingRect());
    if (thisShape.isEmpty()) {
        // Empty shape? No collision.
        return false;
    }
    if (thisShape == path) {
        // When precisely on top of eachother, they collide regardless.
        return true;
    }

    // Check for intersections or whether one contains the other.
    bool checkIntersections = (mode == Qt::IntersectsItemShape || mode == Qt::IntersectsItemBoundingRect);
    bool pathIntersectsItem = path.contains(thisShape.elementAt(0));
    if (checkIntersections && pathIntersectsItem)
        return true;
    bool itemIntersectsPath = thisShape.contains(path.elementAt(0));
    if (checkIntersections && itemIntersectsPath)
        return true;

    // No complete intersections; we need to intersect the path outlines.
    QPolygonF polyA = thisShape.toFillPolygon();
    QPolygonF polyB = path.toFillPolygon();

    // Check if any lines intersect, O(N^2).
    bool outlineIntersects = false;
    for (int a = 1; !outlineIntersects && a < polyA.size(); ++a) {
        QLineF lineA(polyA.at(a - 1), polyA.at(a));
        for (int b = 1; b < polyB.size(); ++b) {
            QLineF lineB(polyB.at(b - 1), polyB.at(b));
            if (lineA.intersect(lineB, 0) == QLineF::BoundedIntersection) {
                outlineIntersects = true;
                break;
            }
        }
    }

    // Only containment is OK.
    if (mode == Qt::ContainsItemShape || mode == Qt::ContainsItemBoundingRect)
        return !outlineIntersects && path.contains(polyA.first());

    // Any intersection or containment is OK.
    return outlineIntersects || path.contains(polyA.first()) || thisShape.contains(polyB.first());
}

/*!
    Returns a list of all items that collide with this item.

    The way collisions are detected is determined by \a mode. The default
    value for \a mode is Qt::IntersectsItemShape; All items whose shape
    intersects or is contained by this item's shape are returned.

    \sa QGraphicsScene::collidingItems(), collidesWithItem()
*/
QList<QGraphicsItem *> QGraphicsItem::collidingItems(Qt::ItemSelectionMode mode) const
{
    return d_ptr->scene ? d_ptr->scene->collidingItems(this, mode) : QList<QGraphicsItem *>();
}

/*!
    Returns true if this item's bounding rect is completely obscured by the
    opaque shape of any of its colliding items. This function returns false if
    all the colliding items are at the same zValue() as this item.

  \sa opaqueArea()
*/
bool QGraphicsItem::isObscured() const
{
    foreach (QGraphicsItem *item, collidingItems()) {
        if (item->zValue() > zValue() && isObscuredBy(item))
            return true;
    }
    return false;
}

/*!
    Returns true if this item's bounding rect is completely obscured by the
    opaque shape of \a item.

    \sa opaqueArea(), isObscured()
*/
bool QGraphicsItem::isObscuredBy(const QGraphicsItem *item) const
{
    return item->mapToItem(this, item->opaqueArea()).contains(boundingRect());
}

/*!
    This virtual function returns a shape representing the area where this
    item is opaque. An area is opaque if it is filled using an opaque brush or
    color (i.e., not transparent).

    This function is used by isObscuredBy(), which is called by underlying
    items to determine if they are obscured by this item.

    The default implementation returns an empty QPainterPath, indicating that
    this item is completely transparent and does not obscure any other items.

    \sa isObscuredBy(), isObscured(), shape()
*/
QPainterPath QGraphicsItem::opaqueArea() const
{
    return QPainterPath();
}

/*!
    \fn virtual void QGraphicsItem::paint(QPainter *painter, const
    QStyleOptionGraphicsItem *option, QWidget *widget = 0) = 0

    This function, which is usually called by QGraphicsView, paints the
    contents of an item in local coordinates.

    Reimplement this function in a QGraphicsItem subclass to provide the
    item's painting implementation, using \a painter. The \a option parameter
    provides style options for the item, such as its state, exposed area and
    its level-of-detail hints. The \a widget argument is optional. If
    provided, it points to the widget that is being painted on; otherwise, it
    is 0.

    \code
        void RoundRectItem::paint(QPainter *painter,
                                  const QStyleOptionGraphicsItem *option,
                                  QWidget *widget)
        {
            painter->drawRoundRect(-10, -10, 20, 20);
        }
    \endcode

    The painter's pen is 0-width by default, and its pen is initialized to the
    QPalette::Text brush from the paint device's palette. The brush is
    initialized to QPalette::Window.

    All painting is done in local coordinates.
*/

/*!
    Schedules a redraw of the area covered by \a rect in this item. You can
    call this function whenever your item needs to be redrawn, such as if it
    changes appearance or size.

    This function does not cause an immediate paint; instead it schedules a
    paint request that is processed by QGraphicsView after control reaches the
    event loop. The item will only be redrawn if it is visible in any
    associated view.

    As a side effect of the item being repainted, other items that overlap the
    area \a rect may also be repainted.

    If the item is invisible (i.e., isVisible() returns false), this function
    does nothing.

    \sa paint(), boundingRect()
*/
void QGraphicsItem::update(const QRectF &rect)
{
    if (d_ptr->scene && isVisible())
        d_ptr->scene->itemUpdated(this, rect);
}

/*!
    \fn void QGraphicsItem::update(qreal x, qreal y, qreal width, qreal height)
    \overload

    This convenience function is equivalent to calling update(QRectF(\a x, \a
    y, \a width, \a height)).
*/

/*!
    Maps the point \a point, which is in this item's coordinate system, to \a
    item's coordinate system, and returns the mapped coordinate.

    If \a item is 0, this function returns the same as mapToScene().

    \sa mapToParent(), mapToScene(), matrix(), mapFromItem(), {The Graphics
    View Coordinate System}
*/
QPointF QGraphicsItem::mapToItem(const QGraphicsItem *item, const QPointF &point) const
{
    return item ? item->mapFromScene(mapToScene(point)) : mapToScene(point);
}

/*!
    \fn QPointF QGraphicsItem::mapToItem(const QGraphicsItem *item, qreal x, qreal y) const
    \overload

    This convenience function is equivalent to calling mapToItem(\a item,
    QPointF(\a x, \a y)).
*/

/*!
    Maps the point \a point, which is in this item's coordinate system, to its
    parent's coordinate system, and returns the mapped coordinate. If the item
    has no parent, \a point will be mapped to the scene's coordinate system.

    \sa mapToItem(), mapToScene(), matrix(), mapFromParent(), {The Graphics
    View Coordinate System}
*/
QPointF QGraphicsItem::mapToParent(const QPointF &point) const
{
    return matrix().map(point) + d_ptr->pos;
}

/*!
    \fn QPointF QGraphicsItem::mapToParent(qreal x, qreal y) const
    \overload

    This convenience function is equivalent to calling mapToParent(QPointF(\a
    x, \a y)).
*/

/*!
    Maps the point \a point, which is in this item's coordinate system, to the
    scene's coordinate system, and returns the mapped coordinate.

    \sa mapToItem(), mapToParent(), matrix(), mapFromScene(), {The Graphics
    View Coordinate System}
*/
QPointF QGraphicsItem::mapToScene(const QPointF &point) const
{
    return d_ptr->parent ? d_ptr->parent->mapToScene(mapToParent(point)) : mapToParent(point);
}

/*!
    \fn QPointF QGraphicsItem::mapToScene(qreal x, qreal y) const
    \overload

    This convenience function is equivalent to calling mapToScene(QPointF(\a
    x, \a y)).
*/

/*!
    Maps the rectangle \a rect, which is in this item's coordinate system, to
    \a item's coordinate system, and returns the mapped rectangle as a polygon.

    If \a item is 0, this function returns the same as mapToScene().

    \sa mapToParent(), mapToScene(), mapFromItem(), {The Graphics View
    Coordinate System}
*/
QPolygonF QGraphicsItem::mapToItem(const QGraphicsItem *item, const QRectF &rect) const
{
    return item ? item->mapFromScene(mapToScene(rect)) : mapToScene(rect);
}

/*!
    Maps the rectangle \a rect, which is in this item's coordinate system, to
    its parent's coordinate system, and returns the mapped rectangle as a
    polygon. If the item has no parent, \a rect will be mapped to the scene's
    coordinate system.

    \sa mapToScene(), mapToItem(), mapFromParent(), {The Graphics View
    Coordinate System}
*/
QPolygonF QGraphicsItem::mapToParent(const QRectF &rect) const
{
    return mapToItem(parentItem(), rect);
}

/*!
    Maps the rectangle \a rect, which is in this item's coordinate system, to
    the scene's coordinate system, and returns the mapped rectangle as a polygon.

    \sa mapToParent(), mapToItem(), mapFromScene(), {The Graphics View
    Coordinate System}
*/
QPolygonF QGraphicsItem::mapToScene(const QRectF &rect) const
{
    return sceneMatrix().map(rect);
}

/*!
    Maps the polygon \a polygon, which is in this item's coordinate system, to
    \a item's coordinate system, and returns the mapped polygon.

    If \a item is 0, this function returns the same as mapToScene().

    \sa mapToParent(), mapToScene(), mapFromItem(), {The Graphics View
    Coordinate System}
*/
QPolygonF QGraphicsItem::mapToItem(const QGraphicsItem *item, const QPolygonF &polygon) const
{
    return item ? item->mapFromScene(mapToScene(polygon)) : mapToScene(polygon);
}

/*!
    Maps the polygon \a polygon, which is in this item's coordinate system, to
    its parent's coordinate system, and returns the mapped polygon. If the
    item has no parent, \a polygon will be mapped to the scene's coordinate
    system.

    \sa mapToScene(), mapToItem(), mapFromParent(), {The Graphics View
    Coordinate System}
*/
QPolygonF QGraphicsItem::mapToParent(const QPolygonF &polygon) const
{
    return mapToItem(parentItem(), polygon);
}

/*!
    Maps the polygon \a polygon, which is in this item's coordinate system, to
    the scene's coordinate system, and returns the mapped polygon.

    \sa mapToParent(), mapToItem(), mapFromScene(), {The Graphics View
    Coordinate System}
*/
QPolygonF QGraphicsItem::mapToScene(const QPolygonF &polygon) const
{
    return sceneMatrix().map(polygon);
}

/*!
    Maps the path \a path, which is in this item's coordinate system, to
    \a item's coordinate system, and returns the mapped path.

    If \a item is 0, this function returns the same as mapToScene().

    \sa mapToParent(), mapToScene(), mapFromItem(), {The Graphics View
    Coordinate System}
*/
QPainterPath QGraphicsItem::mapToItem(const QGraphicsItem *item, const QPainterPath &path) const
{
    return item ? item->mapFromScene(mapToScene(path)) : mapToScene(path);
}

/*!
    Maps the path \a path, which is in this item's coordinate system, to
    its parent's coordinate system, and returns the mapped path. If the
    item has no parent, \a path will be mapped to the scene's coordinate
    system.

    \sa mapToScene(), mapToItem(), mapFromParent(), {The Graphics View
    Coordinate System}
*/
QPainterPath QGraphicsItem::mapToParent(const QPainterPath &path) const
{
    return mapToItem(parentItem(), path);
}

/*!
    Maps the path \a path, which is in this item's coordinate system, to
    the scene's coordinate system, and returns the mapped path.

    \sa mapToParent(), mapToItem(), mapFromScene(), {The Graphics View
    Coordinate System}
*/
QPainterPath QGraphicsItem::mapToScene(const QPainterPath &path) const
{
    return sceneMatrix().map(path);
}

/*!
    Maps the point \a point, which is in \a item's coordinate system, to this
    item's coordinate system, and returns the mapped coordinate.

    If \a item is 0, this function returns the same as mapFromScene().

    \sa mapFromParent(), mapFromScene(), matrix(), mapToItem(), {The Graphics
    View Coordinate System}
*/
QPointF QGraphicsItem::mapFromItem(const QGraphicsItem *item, const QPointF &point) const
{
    return item ? mapFromScene(item->mapToScene(point)) : mapFromScene(point);
}

/*!
    \fn QPointF QGraphicsItem::mapFromItem(const QGraphicsItem *item, qreal x, qreal y) const
    \overload

    This convenience function is equivalent to calling mapFromItem(\a item,
    QPointF(\a x, \a y)).
*/

/*!
    Maps the point \a point, which is in this item's parent's coordinate
    system, to this item's coordinate system, and returns the mapped
    coordinate.

    \sa mapFromItem(), mapFromScene(), matrix(), mapToParent(), {The Graphics
    View Coordinate System}
*/
QPointF QGraphicsItem::mapFromParent(const QPointF &point) const
{
    return matrix().inverted().map(point - d_ptr->pos);
}

/*!
    \fn QPointF QGraphicsItem::mapFromParent(qreal x, qreal y) const
    \overload

    This convenience function is equivalent to calling
    mapFromParent(QPointF(\a x, \a y)).
*/

/*!
    Maps the point \a point, which is in this item's scene's coordinate
    system, to this item's coordinate system, and returns the mapped
    coordinate.

    \sa mapFromItem(), mapFromParent(), matrix(), mapToScene(), {The Graphics
    View Coordinate System}
*/
QPointF QGraphicsItem::mapFromScene(const QPointF &point) const
{
    return d_ptr->parent ? mapFromParent(d_ptr->parent->mapFromScene(point)) : mapFromParent(point);
}

/*!
    \fn QPointF QGraphicsItem::mapFromScene(qreal x, qreal y) const
    \overload

    This convenience function is equivalent to calling mapFromScene(QPointF(\a
    x, \a y)).
*/

/*!
    Maps the rectangle \a rect, which is in \a item's coordinate system, to
    this item's coordinate system, and returns the mapped rectangle as a
    polygon.

    If \a item is 0, this function returns the same as mapFromScene()

    \sa mapToItem(), mapFromParent(), matrix(), {The Graphics View Coordinate
    System}
*/
QPolygonF QGraphicsItem::mapFromItem(const QGraphicsItem *item, const QRectF &rect) const
{
    return mapFromScene(item ? item->mapToScene(rect) : rect);
}

/*!
    Maps the rectangle \a rect, which is in this item's parent's coordinate
    system, to this item's coordinate system, and returns the mapped rectangle
    as a polygon.

    \sa mapToParent(), mapFromItem(), matrix(), {The Graphics View Coordinate
    System}
*/
QPolygonF QGraphicsItem::mapFromParent(const QRectF &rect) const
{
    return mapFromItem(parentItem(), rect);
}

/*!
    Maps the rectangle \a rect, which is in this item's scene's coordinate
    system, to this item's coordinate system, and returns the mapped rectangle
    as a polygon.

    \sa mapToScene(), mapFromItem(), matrix(), {The Graphics View Coordinate
    System}
*/
QPolygonF QGraphicsItem::mapFromScene(const QRectF &rect) const
{
    return sceneMatrix().inverted().map(rect);
}

/*!
    Maps the polygon \a polygon, which is in \a item's coordinate system, to
    this item's coordinate system, and returns the mapped polygon.

    If \a item is 0, this function returns the same as mapFromScene().

    \sa mapToItem(), mapFromParent(), matrix(), {The Graphics View Coordinate
    System}
*/
QPolygonF QGraphicsItem::mapFromItem(const QGraphicsItem *item, const QPolygonF &polygon) const
{
    return mapFromScene(item ? item->mapToScene(polygon) : polygon);
}

/*!
    Maps the polygon \a polygon, which is in this item's parent's coordinate
    system, to this item's coordinate system, and returns the mapped polygon.

    \sa mapToParent(), mapToItem(), matrix(), {The Graphics View Coordinate
    System}
*/
QPolygonF QGraphicsItem::mapFromParent(const QPolygonF &polygon) const
{
    return mapFromItem(parentItem(), polygon);
}

/*!
    Maps the polygon \a polygon, which is in this item's scene's coordinate
    system, to this item's coordinate system, and returns the mapped polygon.

    \sa mapToScene(), mapFromParent(), matrix(), {The Graphics View Coordinate
    System}
*/
QPolygonF QGraphicsItem::mapFromScene(const QPolygonF &polygon) const
{
    return sceneMatrix().inverted().map(polygon);
}

/*!
    Maps the path \a path, which is in \a item's coordinate system, to
    this item's coordinate system, and returns the mapped path.

    If \a item is 0, this function returns the same as mapFromScene().

    \sa mapFromParent(), mapFromScene(), mapToItem(), {The Graphics View
    Coordinate System}
*/
QPainterPath QGraphicsItem::mapFromItem(const QGraphicsItem *item, const QPainterPath &path) const
{
    return mapFromScene(item ? item->mapToScene(path) : path);
}

/*!
    Maps the path \a path, which is in this item's parent's coordinate
    system, to this item's coordinate system, and returns the mapped path.

    \sa mapFromScene(), mapFromItem(), mapToParent(), {The Graphics View
    Coordinate System}
*/
QPainterPath QGraphicsItem::mapFromParent(const QPainterPath &path) const
{
    return mapFromItem(parentItem(), path);
}

/*!
    Maps the path \a path, which is in this item's scene's coordinate
    system, to this item's coordinate system, and returns the mapped path.

    \sa mapFromParent(), mapFromItem(), mapToScene(), {The Graphics View
    Coordinate System}
*/
QPainterPath QGraphicsItem::mapFromScene(const QPainterPath &path) const
{
    return sceneMatrix().inverted().map(path);
}

/*!
    Returns true if this item is an ancestor of \a child (i.e., if this item
    is \a child's parent, or one of \a child's parent's ancestors).

    \sa parentItem()
*/
bool QGraphicsItem::isAncestorOf(const QGraphicsItem *child) const
{
    if (!child || child == this)
        return false;
    for (int i = 0; i < d_ptr->children.size(); ++i)
        return d_ptr->children.at(i) == child || d_ptr->children.at(i)->isAncestorOf(child);
    return false;
}

/*!
    Returns this item's custom data for the key \a key as a QVariant.

    Custom item data is useful for storing arbitrary properties in any
    item. Example:

    \code
        static const int ObjectName = 0;

        QGraphicsItem *item = scene.itemAt(100, 50);
        if (item->data(ObjectName).toString().isEmpty()) {
            if (qgraphicsitem_cast<ButtonItem *>(item))
                item->setData(ObjectName, "Button");
        }
    \endcode

    Qt does not use this feature for storing data; it is provided solely
    for the convenience of the user.

    \sa setData()
*/
QVariant QGraphicsItem::data(int key) const
{
    QGraphicsItemCustomDataStore *store = qt_dataStore();
    if (!store->data.contains(this))
        return QVariant();
    return store->data.value(this).value(key);
}

/*!
    Sets this item's custom data for the key \a key to \a value.

    Custom item data is useful for storing arbitrary properties for any
    item. Qt does not use this feature for storing data; it is provided solely
    for the convenience of the user.

    \sa data()
*/
void QGraphicsItem::setData(int key, const QVariant &value)
{
    qt_dataStore()->data[this][key] = value;
}

/*!
    \fn T qgraphicsitem_cast(QGraphicsItem *item)
    \relates QGraphicsItem
    \since 4.2

    Returns the given \a item cast to type T if \a item is of type T;
    otherwise, 0 is returned.
*/

/*!
    Returns the type of an item as an int. All standard graphicsitem classes
    are associated with a unique value; see QGraphicsItem::Type. This type
    information is used by qgraphicsitem_cast() to distinguish between types.

    Reimplementing this function and declaring a Type enum value equal to your
    custom item's type will enable use of qgraphicsitem_cast() with
    the item. Custom items must return a value larger than or equal to UserType
    (65536).

    The default implementation (in QGraphicsItem) returns UserType.
*/
int QGraphicsItem::type() const
{
    return (int)UserType;
}

/*!
    Installs an event filter for this item on \a filterItem, causing
    all events for this item to first pass through \a filterItem's
    sceneEventFilter() function.

    To filter another item's events, install this item as an event filter
    for the other item. Example:

    \code
        QGraphicsScene scene;
        QGraphicsEllipseItem *ellipse = scene.addEllipse(QRectF(-10, -10, 20, 20));
        QGraphicsLineItem *line = scene.addLine(QLineF(-10, -10, 20, 20));

        line->installSceneEventFilter(ellipse);
        // line's events are filtered by ellipse's sceneEventFilter() function.

        ellipse->installSceneEventFilter(line);
        // ellipse's events are filtered by line's sceneEventFilter() function.
    \endcode

    An item can only filter events for other items in the same
    scene. Also, an item cannot filter its own events; instead, you
    can reimplement sceneEvent() directly.

    \sa removeSceneEventFilter(), sceneEventFilter(), sceneEvent()
*/
void QGraphicsItem::installSceneEventFilter(QGraphicsItem *filterItem)
{
    if (!d_ptr->scene) {
        qWarning("QGraphicsItem::installSceneEventFilter: event filters can only be installed"
                 " on items in a scene.");
        return;
    }
    if (d_ptr->scene != filterItem->scene()) {
        qWarning("QGraphicsItem::installSceneEventFilter: event filters can only be installed"
                 " on items in the same scene.");
        return;
    }
    d_ptr->scene->d_func()->installSceneEventFilter(this, filterItem);
}

/*!
    Removes an event filter on this item from \a filterItem.

    \sa installSceneEventFilter()
*/
void QGraphicsItem::removeSceneEventFilter(QGraphicsItem *filterItem)
{
    if (!d_ptr->scene || d_ptr->scene != filterItem->scene())
        return;
    d_ptr->scene->d_func()->removeSceneEventFilter(this, filterItem);
}

/*!
    Filters events for the item \a watched. \a event is the filtered
    event.

    Reimplementing this function in a subclass makes it possible
    for the item to be used as an event filter for other items,
    intercepting all the events send to those items before they are
    able to respond.

    Reimplementations must return true to prevent further processing of
    a given event, ensuring that it will not be delivered to the watched
    item, or return false to indicate that the event should be propagated
    further by the event system.

    \sa installSceneEventFilter()
*/
bool QGraphicsItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Q_UNUSED(watched);
    Q_UNUSED(event);
    return false;
}

/*!
    This virtual function receives events to this item. Reimplement
    this function to intercept events before they are dispatched to
    the specialized event handlers contextMenuEvent(), focusInEvent(),
    focusOutEvent(), hoverEnterEvent(), hoverMoveEvent(),
    hoverLeaveEvent(), keyPressEvent(), keyReleaseEvent(),
    mousePressEvent(), mouseReleaseEvent(), mouseMoveEvent(), and
    mouseDoubleClickEvent().

    Returns true if the event was recognized and handled; otherwise, (e.g., if
    the event type was not recognized,) false is returned.

    \a event is the intercepted event.
*/
bool QGraphicsItem::sceneEvent(QEvent *event)
{
    if (d_ptr->ancestorHandlesChildEvents) {
        if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverLeave
            || event->type() == QEvent::DragEnter || event->type() == QEvent::DragLeave) {
            // Hover enter and hover leave events for children are ignored;
            // hover move events are forwarded.
            return true;
        }

        QGraphicsItem *handler = this;
        while (!handler->handlesChildEvents())
            handler = handler->parentItem();
        if (handler) {
            // Forward the event to the closest parent that handles child
            // events, mapping existing item-local coordinates to its
            // coordinate system.
            d_ptr->remapItemPos(event, handler);
            handler->sceneEvent(event);
        }
        return true;
    }

    if (!d_ptr->enabled || !d_ptr->visible) {
        // Eaten
        return true;
    }

    switch (event->type()) {
    case QEvent::FocusIn:
        focusInEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::FocusOut:
        focusOutEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::GraphicsSceneContextMenu:
        contextMenuEvent(static_cast<QGraphicsSceneContextMenuEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragEnter:
        dragEnterEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragMove:
        dragMoveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragLeave:
        dragLeaveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDrop:
        dropEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverEnter:
        hoverEnterEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverMove:
        hoverMoveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverLeave:
        hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseMove:
        mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMousePress:
        mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
        mouseDoubleClickEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneWheel:
        wheelEvent(static_cast<QGraphicsSceneWheelEvent *>(event));
        break;
    case QEvent::KeyPress:
        keyPressEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::KeyRelease:
        keyReleaseEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::InputMethod:
        inputMethodEvent(static_cast<QInputMethodEvent *>(event));
        break;
    default:
        return false;
    }

    return true;
}

/*!
    This event handler, for event \a event, can be reimplemented to
    receive context menu events for this item.

    If you ignore the event, (i.e., by calling QEvent::ignore(),) \a event
    will propagate to any item beneath this item. If no items accept the
    event, it will be ignored by the scene, and propagate to the view.

    The default implementation does nothing.

    \sa sceneEvent()
*/
void QGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    drag enter events for this item. Drag enter events are generated as the
    cursor enters the item's area.

    By accepting the event, (i.e., by calling QEvent::accept(),) the item will
    accept drop events, in addition to receiving drag move and drag
    leave. Otherwise, the event will be ignored and propagate to the item
    beneath. If the event is accepted, the item will receive a drag move event
    before control goes back to the event loop.

    A common implementation of dragEnterEvent accepts or ignores \a event
    depending on the associated mime data in \a event. Example:

    \code
        CustomItem::CustomItem()
        {
            setAcceptDrops(true);
            ...
        }

        void CustomItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
        {
            event->setAccepted(event->mimeData()->hasFormat("text/plain"));
        }
    \endcode

    Items do not receive drag and drop events by default; to enable this
    feature, call \c setAcceptDrops(true).

    The default implementation does nothing.

    \sa dropEvent(), dragMoveEvent(), dragLeaveEvent()
*/
void QGraphicsItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    drag leave events for this item. Drag leave events are generated as the
    cursor leaves the item's area. Most often you will not need to reimplement
    this function, but it can be useful for resetting state in your item
    (e.g., highlighting).

    Calling QEvent::ignore() or QEvent::accept() on \a event has no effect.

    Items do not receive drag and drop events by default; to enable this
    feature, call \c setAcceptDrops(true).

    The default implementation does nothing.

    \sa dragEnterEvent(), dropEvent(), dragMoveEvent()
*/
void QGraphicsItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    drag move events for this item. Drag move events are generated as the
    cursor moves around inside the item's area. Most often you will not need
    to reimplement this function; it is used to indicate that only parts of
    the item can accept drops.

    Calling QEvent::ignore() or QEvent::accept() on \a event toggles whether
    or not the item will accept drops at the position from the event. By
    default, \a event is accepted, indicating that the item allows drops at
    the specified position.

    Items do not receive drag and drop events by default; to enable this
    feature, call \c setAcceptDrops(true).

    The default implementation does nothing.

    \sa dropEvent(), dragEnterEvent(), dragLeaveEvent()
*/
void QGraphicsItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    drop events for this item. Items can only receive drop events if the last
    drag move event was accepted.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no effect.

    Items do not receive drag and drop events by default; to enable this
    feature, call \c setAcceptDrops(true).

    The default implementation does nothing.

    \sa dragEnterEvent(), dragMoveEvent(), dragLeaveEvent()
*/
void QGraphicsItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    focus in events for this item. The default implementation does nothing.

    \sa focusOutEvent(), sceneEvent()
*/
void QGraphicsItem::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    focus out events for this item. The default implementation does nothing.

    \sa focusInEvent(), sceneEvent()
*/
void QGraphicsItem::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    hover enter events for this item. The default implementation calls
    update(); otherwise it does nothing.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no effect.

    \sa hoverMoveEvent(), hoverLeaveEvent(), sceneEvent()
*/
void QGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    update();
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    hover move events for this item. The default implementation does nothing.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no effect.

    \sa hoverEnterEvent(), hoverLeaveEvent(), sceneEvent()
*/
void QGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    hover leave events for this item. The default implementation calls
    update(); otherwise it does nothing.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no effect.

    \sa hoverEnterEvent(), hoverMoveEvent(), sceneEvent()
*/
void QGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    update();
}

/*!
    This event handler, for event \a event, can be reimplemented to
    receive key press events for this item. The default implementation
    ignores the event. If you reimplement this handler, the event will by
    default be accepted.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no effect.

    Note that key events are only received for items that set the
    ItemIsFocusable flag, and that have keyboard input focus.

    \sa keyReleaseEvent(), setFocus(), QGraphicsScene::setFocusItem(),
    sceneEvent()
*/
void QGraphicsItem::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    key release events for this item. The default implementation
    ignores the event. If you reimplement this handler, the event will by
    default be accepted.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no effect.

    Note that key events are only received for items that set the
    ItemIsFocusable flag, and that have keyboard input focus.

    \sa keyPressEvent(), setFocus(), QGraphicsScene::setFocusItem(),
    sceneEvent()
*/
void QGraphicsItem::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}

/*!
    This event handler, for event \a event, can be reimplemented to
    receive mouse press events for this item. Mouse press events are
    only delivered to items that accept the mouse button that is
    pressed. By default, an item accepts all mouse buttons, but you
    can change this by calling setAcceptedMouseButtons().

    The mouse press event decides which item should become the mouse
    grabber (see QGraphicsScene::mouseGrabberItem()). If you do not
    reimplement this function, the press event will propagate to any
    topmost item beneath this item, and no other mouse events will be
    delivered to this item.

    If you do reimplement this function, \a event will by default be
    accepted (see QEvent::accept()), and this item is then the mouse
    grabber. This allows the item to receive future move, release and
    doubleclick events. If you call QEvent::ignore() on \a event, this
    item will lose the mouse grab, and \a event will propagate to any
    topmost item beneath. No further mouse events will be delivered to
    this item unless a new mouse press event is received.

    The default implementation handles basic item interaction, such as
    selection and moving. If you want to keep the base implementation
    when reimplementing this function, call
    QGraphicsItem::mousePressEvent() in your reimplementation.

    \sa mouseMoveEvent(), mouseReleaseEvent(),
    mouseDoubleClickEvent(), sceneEvent()
*/
void QGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && (flags() & ItemIsSelectable)) {
        bool multiSelect = (event->modifiers() & Qt::ControlModifier) != 0;
        if (!multiSelect) {
            if (!d_ptr->selected) {
                if (d_ptr->scene)
                    d_ptr->scene->clearSelection();
                setSelected(true);
            }
        }
    } else if (!(flags() & ItemIsMovable)) {
        event->ignore();
    }
}

/*!
    This event handler, for event \a event, can be reimplemented to
    receive mouse move events for this item. If you do receive this
    event, you can be certain that this item also received a mouse
    press event, and that this item is the current mouse grabber.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no
    effect.

    The default implementation handles basic item interaction, such as
    selection and moving. If you want to keep the base implementation
    when reimplementing this function, call
    QGraphicsItem::mouseMoveEvent() in your reimplementation.

    \sa mousePressEvent(), mouseReleaseEvent(),
    mouseDoubleClickEvent(), sceneEvent()
*/
void QGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        // Handle ItemIsMovable.
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QPointF diff = newPos - pos();

        // Determine the list of selected items
        QList<QGraphicsItem *> selectedItems;
        if (d_ptr->scene) {
            selectedItems = d_ptr->scene->selectedItems();
        } else {
            QGraphicsItem *parent = this;
            while ((parent = parent->parentItem()))
                if (parent->isSelected())
                    selectedItems << parent;
        }
        selectedItems << this;

        // Move all selected items
        foreach (QGraphicsItem *item, selectedItems) {
            if ((item->flags() & ItemIsMovable) && (!item->parentItem() || !item->parentItem()->isSelected())) {
                item->setPos(item == this ? newPos : item->pos() + diff);
                if (item->flags() & ItemIsSelectable)
                    item->setSelected(true);
            }
        }

    } else {
        event->ignore();
    }
}

/*!
    This event handler, for event \a event, can be reimplemented to
    receive mouse release events for this item.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no
    effect.

    The default implementation handles basic item interaction, such as
    selection and moving. If you want to keep the base implementation
    when reimplementing this function, call
    QGraphicsItem::mouseReleaseEvent() in your reimplementation.

    \sa mousePressEvent(), mouseMoveEvent(), mouseDoubleClickEvent(),
    sceneEvent()
*/
void QGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & ItemIsSelectable) {
        bool multiSelect = (event->modifiers() & Qt::ControlModifier) != 0;
        if (event->scenePos() == event->buttonDownScenePos(Qt::LeftButton)) {
            // The item didn't move
            if (multiSelect) {
                setSelected(!isSelected());
            } else {
                if (d_ptr->scene)
                    d_ptr->scene->clearSelection();
                setSelected(true);
            }
        }
    }
}

/*!
    This event handler, for event \a event, can be reimplemented to
    receive mouse doubleclick events for this item.

    When doubleclicking an item, the item will first receive a mouse
    press event, followed by a release event (i.e., a click), then a
    doubleclick event, and finally a release event.

    Calling QEvent::ignore() or QEvent::accept() on \a event has no
    effect.

    The default implementation calls mousePressEvent(). If you want to
    keep the base implementation when reimplementing this function,
    call QGraphicsItem::mouseDoubleClickEvent() in your
    reimplementation.

    \sa mousePressEvent(), mouseMoveEvent(), mouseReleaseEvent(), sceneEvent()
*/
void QGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    mousePressEvent(event);
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    wheel events for this item. If you reimplement this function, \a event
    will be accepted by default.

    If you ignore the event, (i.e., by calling QEvent::ignore(),) it will
    propagate to any item beneath this item. If no items accept the event, it
    will be ignored by the scene, and propagate to the view (e.g., the view's
    vertical scroll bar).

    The default implementation ignores the event.

    \sa sceneEvent()
*/
void QGraphicsItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    event->ignore();
}

/*!
    This event handler, for event \a event, can be reimplemented to receive
    input method events for this item. The default implementation ignores the
    event.

    \sa inputMethodQuery(), sceneEvent()
*/
void QGraphicsItem::inputMethodEvent(QInputMethodEvent *event)
{
    event->ignore();
}

/*!
    This method is only relevant for input items. It is used by the
    input method to query a set of properties of the item to be able
    to support complex input method operations, such as support for
    surrounding text and reconversions. \a query specifies which
    property is queried.

    \sa inputMethodEvent()
*/
QVariant QGraphicsItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    Q_UNUSED(query);
    return QVariant();
}

/*!
    This virtual function is called by QGraphicsItem to notify custom items
    that some part of the item's state changes. By reimplementing this
    function, your can react to a change, and in some cases, (depending on \a
    change,) adjustments can be made.

    \a change is the parameter of the item that is changing. \a value is the
    new value; the type of the value depends on \a change.

    Example:

    \code
        QVariant Component::itemChange(GraphicsItemChange change, const QVariant &value)
        {
            if (change == ItemPositionChange && scene()) {
                // value is the new position.
                QPointF newPos = value.toPointF();
                QRectF rect = scene()->sceneRect();
                if (!rect.contains(newPos)) {
                    // Keep the item inside the scene rect.
                    newPos.setX(qMin(rect.right(), qMax(newPos.x(), rect.left())));
                    newPos.setY(qMin(rect.bottom(), qMax(newPos.y(), rect.top())));
                    return newPos;
                }
            }
            return QGraphicsItem::itemChange(change, value);
        }
    \endcode

    The default implementation does nothing, and returns \a value.

    Note: Certain QGraphicsItem functions cannot be called in a
    reimplementation of this function; see the GraphicsItemChange
    documentation for details.

    \sa GraphicsItemChange
*/
QVariant QGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    Q_UNUSED(change);
    return value;
}

/*!
    \internal

    Note: This is provided as a hook to avoid future problems related
    to adding virtual functions.
*/
bool QGraphicsItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal

    Note: This is provided as a hook to avoid future problems related
    to adding virtual functions.
*/
void QGraphicsItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal

    Note: This is provided as a hook to avoid future problems related
    to adding virtual functions.
*/
QVariant QGraphicsItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \internal

    Adds this item to the scene's index. Called in conjunction with
    removeFromIndex() to ensure the index bookkeeping is correct when
    the item's position, transformation or shape changes.
*/
void QGraphicsItem::addToIndex()
{
    if (d_ptr->scene)
        d_ptr->scene->d_func()->addToIndex(this);
    update();
}

/*!
    \internal

    Removes this item from the scene's index. Called in conjunction
    with addToIndex() to ensure the index bookkeeping is correct when
    the item's position, transformation or shape changes.
*/
void QGraphicsItem::removeFromIndex()
{
    update();
    if (d_ptr->scene)
        d_ptr->scene->d_func()->removeFromIndex(this);
}

/*!
    Prepares the item for a geometry change. Call this function before
    changing the bounding rect of an item to keep QGraphicsScene's index up to
    date.

    prepareGeometryChange() will call update() if this is necessary.

    Example:

    \code
        void CircleItem::setRadius(qreal newRadius)
        {
            if (radius != newRadius) {
                prepareGeometryChange();
                radius = newRadius;
            }
        }
    \endcode

    \sa boundingRect()
*/
void QGraphicsItem::prepareGeometryChange()
{
    if (d_ptr->scene) {
        update();

        QGraphicsScenePrivate *scenePrivate = d_ptr->scene->d_func();
        scenePrivate->removeFromIndex(this);

        if (!scenePrivate->calledEmitUpdated) {
            scenePrivate->calledEmitUpdated = true;
            QTimer::singleShot(0, d_ptr->scene, SLOT(_q_emitUpdated()));
        }
    }
}

/*!
    \class QAbstractGraphicsShapeItem
    \brief The QAbstractGraphicsShapeItem class provides a common base for
    all path items.
    \since 4.2
    \ingroup multimedia

    This class does not fully implement an item by itself; in particular, it
    does not implement boundingRect() and paint(), which are inherited by
    QGraphicsItem.

    You can subclass this item to provide a simple base implementation of
    accessors for the item's pen and brush.

    \sa QGraphicsRectItem, QGraphicsEllipseItem, QGraphicsPathItem,
    QGraphicsPolygonItem, QGraphicsTextItem, QGraphicsLineItem,
    QGraphicsPixmapItem, {The Graphics View Framework}
*/

class QAbstractGraphicsShapeItemPrivate : public QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QAbstractGraphicsShapeItem)
public:

    QBrush brush;
    QPen pen;
};

/*!
    Constructs a QAbstractGraphicsShapeItem. \a parent and \a scene are
    passed to QGraphicsItem's constructor.
*/
QAbstractGraphicsShapeItem::QAbstractGraphicsShapeItem(QGraphicsItem *parent,
                                                     QGraphicsScene *scene)
    : QGraphicsItem(*new QAbstractGraphicsShapeItemPrivate, parent, scene)
{
}

/*!
    \internal
*/
QAbstractGraphicsShapeItem::QAbstractGraphicsShapeItem(QAbstractGraphicsShapeItemPrivate &dd,
                                                     QGraphicsItem *parent,
                                                     QGraphicsScene *scene)
    : QGraphicsItem(dd, parent, scene)
{
}

/*!
    Destroys a QAbstractGraphicsShapeItem.
*/
QAbstractGraphicsShapeItem::~QAbstractGraphicsShapeItem()
{
}

/*!
    Returns the item's pen. If no pen has been set, this function returns
    QPen(), a default black solid line pen with 0 width.
*/
QPen QAbstractGraphicsShapeItem::pen() const
{
    Q_D(const QAbstractGraphicsShapeItem);
    return d->pen;
}

/*!
    Sets the pen for this item to \a pen.

    The pen is used to draw the item's outline.

    \sa pen()
*/
void QAbstractGraphicsShapeItem::setPen(const QPen &pen)
{
    Q_D(QAbstractGraphicsShapeItem);
    bool updateGeometry = (pen.widthF() != d->pen.widthF());
    if (updateGeometry)
        removeFromIndex();
    d->pen = pen;
    if (updateGeometry)
        addToIndex();
    else
        update();
}

/*!
    Returns the item's brush, or an empty brush if no brush has been set.

    \sa setBrush()
*/
QBrush QAbstractGraphicsShapeItem::brush() const
{
    Q_D(const QAbstractGraphicsShapeItem);
    return d->brush;
}

/*!
    Sets the item's brush to \a brush.

    The item's brush is used to fill the item.

    \sa brush()
*/
void QAbstractGraphicsShapeItem::setBrush(const QBrush &brush)
{
    Q_D(QAbstractGraphicsShapeItem);
    d->brush = brush;
    update();
}

/*!
    \reimp
*/
bool QAbstractGraphicsShapeItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QGraphicsItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QAbstractGraphicsShapeItem::opaqueArea() const
{
    Q_D(const QAbstractGraphicsShapeItem);
    if (d->brush.isOpaque())
        return shape();
    return QGraphicsItem::opaqueArea();
}

/*!
    \class QGraphicsPathItem
    \brief The QGraphicsPathItem class provides a path item that you
    can add to a QGraphicsScene.
    \since 4.2
    \ingroup multimedia

    To set the item's path, pass a QPainterPath to QGraphicsPathItem's
    constructor, or call the setPath() function. The path() function
    returns the current path.

    \image graphicsview-pathitem.png

    QGraphicsPathItem uses the path to provide a reasonable
    implementation of boundingRect(), shape(), and contains(). The
    paint() function draws the path using the item's associated pen
    and brush, which you can set by calling the setPen() and
    setBrush() functions.

    \sa QGraphicsRectItem, QGraphicsEllipseItem, QGraphicsPolygonItem,
    QGraphicsTextItem, QGraphicsLineItem, QGraphicsPixmapItem, {The Graphics
    View Framework}
*/

class QGraphicsPathItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsPathItem)
public:
    QPainterPath path;
    QRectF boundingRect;
};

/*!
    Constructs a QGraphicsPath item using \a path as the default
    path. The \a parent and \a scene parameters are passed to
    QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsPathItem::QGraphicsPathItem(const QPainterPath &path,
                                     QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsPathItemPrivate, parent, scene)
{
    if (!path.isEmpty())
        setPath(path);
}

/*!
    Constructs a QGraphicsPath. The \a parent and \a scene parameters
    are passed to QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsPathItem::QGraphicsPathItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsPathItemPrivate, parent, scene)
{
}

/*!
    Destroys the QGraphicsPathItem.
*/
QGraphicsPathItem::~QGraphicsPathItem()
{
}

/*!
    Returns the item's path as a QPainterPath. If no item has been set, an
    empty QPainterPath is returned.

    \sa setPath()
*/
QPainterPath QGraphicsPathItem::path() const
{
    Q_D(const QGraphicsPathItem);
    return d->path;
}

/*!
    Sets the item's path to be the given \a path.

    \sa path()
*/
void QGraphicsPathItem::setPath(const QPainterPath &path)
{
    Q_D(QGraphicsPathItem);
    removeFromIndex();
    d->path = path;
    d->boundingRect = path.controlPointRect();
    addToIndex();
}

/*!
    \reimp
*/
QRectF QGraphicsPathItem::boundingRect() const
{
    Q_D(const QGraphicsPathItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    return d->boundingRect.adjusted(-penWidth, -penWidth,
                                    penWidth, penWidth);
}

/*!
    \reimp
*/
QPainterPath QGraphicsPathItem::shape() const
{
    Q_D(const QGraphicsPathItem);
    return qt_graphicsItem_shapeFromPath(d->path, d->pen);
}

/*!
    \reimp
*/
bool QGraphicsPathItem::contains(const QPointF &point) const
{
    return QAbstractGraphicsShapeItem::contains(point);
}

/*!
    \reimp
*/
void QGraphicsPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    Q_D(QGraphicsPathItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->setBrush(d->brush);
    painter->drawPath(d->path);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
}

/*!
    \reimp
*/
bool QGraphicsPathItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsPathItem::opaqueArea() const
{
    return QAbstractGraphicsShapeItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsPathItem::type() const
{
    return Type;
}

/*!
    \internal
*/
bool QGraphicsPathItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsPathItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsPathItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \class QGraphicsRectItem
    \brief The QGraphicsRectItem class provides a rectangle item that you
    can add to a QGraphicsScene.
    \since 4.2
    \ingroup multimedia

    To set the item's rectangle, pass a QRectF to QGraphicsRectItem's
    constructor, or call the setRect() function. The rect() function
    returns the current rectangle.

    \image graphicsview-rectitem.png

    QGraphicsRectItem uses the rectangle and the pen width to provide
    a reasonable implementation of boundingRect(), shape(), and
    contains(). The paint() function draws the rectangle using the
    item's associated pen and brush, which you can set by calling the
    setPen() and setBrush() functions.

    \sa QGraphicsPathItem, QGraphicsEllipseItem, QGraphicsPolygonItem,
    QGraphicsTextItem, QGraphicsLineItem, QGraphicsPixmapItem, {The Graphics
    View Framework}
*/

class QGraphicsRectItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsRectItem)
public:
    QRectF rect;
};

/*!
    Constructs a QGraphicsRectItem, using \a rect as the default
    rectangle.  The \a parent and \a scene parameters are passed to
    QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsRectItem::QGraphicsRectItem(const QRectF &rect, QGraphicsItem *parent,
                                     QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsRectItemPrivate, parent, scene)
{
    setRect(rect);
}

/*!
    \fn QGraphicsRectItem::QGraphicsRectItem(qreal x, qreal y, qreal width, qreal height,
                                     QGraphicsItem *parent, QGraphicsScene *scene)

    Constructs a QGraphicsRectItem with a default rectangle defined
    by (\a x, \a y) and the given \a width and \a height.

    The \a parent parameter is passed to QAbstractGraphicsShapeItem's
    constructor.
*/
QGraphicsRectItem::QGraphicsRectItem(qreal x, qreal y, qreal w, qreal h,
                                     QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsRectItemPrivate, parent, scene)
{
    setRect(QRectF(x, y, w, h));
}

/*!
    Constructs a QGraphicsRectItem. The \a parent and \a scene
    parameters are passed to QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsRectItem::QGraphicsRectItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsRectItemPrivate, parent, scene)
{
}

/*!
    Destroys the QGraphicsRectItem.
*/
QGraphicsRectItem::~QGraphicsRectItem()
{
}

/*!
    Returns the item's rectangle.

    \sa setRect()
*/
QRectF QGraphicsRectItem::rect() const
{
    Q_D(const QGraphicsRectItem);
    return d->rect;
}

/*!
    \fn void QGraphicsRectItem::setRect(const QRectF &rectangle)

    Sets the item's rectangle to be the given \a rectangle.

    \sa rect()
*/
void QGraphicsRectItem::setRect(const QRectF &rect)
{
    Q_D(QGraphicsRectItem);
    removeFromIndex();
    d->rect = rect;
    addToIndex();
}

/*!
    \fn void QGraphicsRectItem::setRect(qreal x, qreal y, qreal width, qreal height)
    \fn void QGraphicsEllipseItem::setRect(qreal x, qreal y, qreal width, qreal height)

    Sets the item's rectangle to the rectangle defined by (\a x, \a y)
    and the given \a width and \a height.

    This convenience function is equivalent to calling \c
    {setRect(QRectF(x, y, width, height))}

    \sa rect()
*/

/*!
    \reimp
*/
QRectF QGraphicsRectItem::boundingRect() const
{
    Q_D(const QGraphicsRectItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    return d->rect.adjusted(-penWidth, -penWidth,
                            penWidth, penWidth);
}

/*!
    \reimp
*/
QPainterPath QGraphicsRectItem::shape() const
{
    Q_D(const QGraphicsRectItem);
    QPainterPath path;
    path.addRect(d->rect);
    return qt_graphicsItem_shapeFromPath(path, d->pen);
}

/*!
    \reimp
*/
bool QGraphicsRectItem::contains(const QPointF &point) const
{
    return QAbstractGraphicsShapeItem::contains(point);
}

/*!
    \reimp
*/
void QGraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    Q_D(QGraphicsRectItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->setBrush(d->brush);
    painter->drawRect(d->rect);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
}

/*!
    \reimp
*/
bool QGraphicsRectItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsRectItem::opaqueArea() const
{
    return QAbstractGraphicsShapeItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsRectItem::type() const
{
    return Type;
}

/*!
    \internal
*/
bool QGraphicsRectItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsRectItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsRectItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \class QGraphicsEllipseItem
    \brief The QGraphicsEllipseItem class provides an ellipse item that you
    can add to a QGraphicsScene.
    \since 4.2
    \ingroup multimedia

    QGraphicsEllipseItem respresents an ellipse with a fill and an outline,
    and you can also use it for ellipse segments (see startAngle(),
    spanAngle()).

    \table
        \row
            \o \inlineimage graphicsview-ellipseitem.png
            \o \inlineimage graphicsview-ellipseitem-pie.png
    \endtable

    To set the item's ellipse, pass a QRectF to QGraphicsEllipseItem's
    constructor, or call setRect(). The rect() function returns the
    current ellipse geometry.

    QGraphicsEllipseItem uses the rect and the pen width to provide a
    reasonable implementation of boundingRect(), shape(), and contains(). The
    paint() function draws the ellipse using the item's associated pen and
    brush, which you can set by calling setPen() and setBrush().

    \sa QGraphicsPathItem, QGraphicsRectItem, QGraphicsPolygonItem,
    QGraphicsTextItem, QGraphicsLineItem, QGraphicsPixmapItem, {The Graphics
    View Framework}
*/

class QGraphicsEllipseItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsEllipseItem)
public:
    inline QGraphicsEllipseItemPrivate()
        : startAngle(0), spanAngle(360 * 16)
    { }

    QRectF rect;
    int startAngle;
    int spanAngle;
};

/*!
    Constructs a QGraphicsEllipseItem using \a rect as the default
    rectangle.  The \a parent and \a scene parameters are passed to
    QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsEllipseItem::QGraphicsEllipseItem(const QRectF &rect, QGraphicsItem *parent,
                                           QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsEllipseItemPrivate, parent, scene)
{
    setRect(rect);
}

/*!
    \fn QGraphicsEllipseItem::QGraphicsEllipseItem(qreal x, qreal y, qreal width, qreal height, QGraphicsItem *parent, QGraphicsScene *scene)

    Constructs a QGraphicsEllipseItem using the rectangle defined by
    (\a x, \a y) and the given \a width and \a height, as the default
    rectangle. The \a parent and \a scene parameters are passed to
    QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsEllipseItem::QGraphicsEllipseItem(qreal x, qreal y, qreal w, qreal h,
                                           QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsEllipseItemPrivate, parent, scene)
{
    setRect(x,y,w,h);
}



/*!
    Constructs a QGraphicsEllipseItem. The \a parent and \a scene
    parameters are passed to QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsEllipseItem::QGraphicsEllipseItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsEllipseItemPrivate, parent, scene)
{
}

/*!
    Destroys the QGraphicsEllipseItem.
*/
QGraphicsEllipseItem::~QGraphicsEllipseItem()
{
}

/*!
    Returns the item's ellipse geometry as a QRectF.

    \sa setRect(), QPainter::drawEllipse()
*/
QRectF QGraphicsEllipseItem::rect() const
{
    Q_D(const QGraphicsEllipseItem);
    return d->rect;
}

/*!
    Sets the item's ellipse geometry to \a rect. The rectangle's left edge
    defines the left edge of the ellipse, and the rectangle's top edge
    describes the top of the ellipse. The height and width of the rectangle
    describe the height and width of the ellipse.

    \sa rect(), QPainter::drawEllipse()
*/
void QGraphicsEllipseItem::setRect(const QRectF &rect)
{
    Q_D(QGraphicsEllipseItem);
    removeFromIndex();
    d->rect = rect;
    addToIndex();
}

/*!
    Returns the start angle for an ellipse segment in 16ths of a degree. This
    angle is used together with spanAngle() for representing an ellipse
    segment (a pie). By default, the start angle is 0.

    \sa setStartAngle(), spanAngle()
*/
int QGraphicsEllipseItem::startAngle() const
{
    Q_D(const QGraphicsEllipseItem);
    return d->startAngle;
}

/*!
    Sets the start angle for an ellipse segment to \a angle, which is in 16ths
    of a degree. This angle is used together with spanAngle() for representing
    an ellipse segment (a pie). By default, the start angle is 0.

    \sa startAngle(), setSpanAngle(), QPainter::drawPie()
*/
void QGraphicsEllipseItem::setStartAngle(int angle)
{
    Q_D(QGraphicsEllipseItem);
    if (angle != d->startAngle) {
        d->startAngle = angle;
        update();
    }
}

/*!
    Returns the span angle of an ellipse segment in 16ths of a degree. This
    angle is used together with startAngle() for representing an ellipse
    segment (a pie). By default, this function returns 5760 (360 * 16, a full
    ellipse).

    \sa setSpanAngle(), startAngle()
*/
int QGraphicsEllipseItem::spanAngle() const
{
    Q_D(const QGraphicsEllipseItem);
    return d->spanAngle;
}

/*!
    Sets the span angle for an ellipse segment to \a angle, which is in 16ths
    of a degree. This angle is used together with startAngle() to represent an
    ellipse segment (a pie). By default, the span angle is 5760 (360 * 16, a
    full ellipse).

    \sa spanAngle(), setStartAngle(), QPainter::drawPie()
*/
void QGraphicsEllipseItem::setSpanAngle(int angle)
{
    Q_D(QGraphicsEllipseItem);
    if (angle != d->spanAngle) {
        d->spanAngle = angle;
        update();
    }
}

/*!
    \reimp
*/
QRectF QGraphicsEllipseItem::boundingRect() const
{
    Q_D(const QGraphicsEllipseItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    return d->rect.adjusted(-penWidth, -penWidth,
                            penWidth, penWidth);
}

/*!
    \reimp
*/
QPainterPath QGraphicsEllipseItem::shape() const
{
    Q_D(const QGraphicsEllipseItem);
    QPainterPath path;
    if (d->rect.isNull())
        return path;
    if (d->spanAngle != 360 * 16) {
        path.moveTo(d->rect.center());
        path.arcTo(d->rect, d->startAngle / 16.0, d->spanAngle / 16.0);
    } else {
        path.addEllipse(d->rect);
    }

    return qt_graphicsItem_shapeFromPath(path, d->pen);
}

/*!
    \reimp
*/
bool QGraphicsEllipseItem::contains(const QPointF &point) const
{
    return QAbstractGraphicsShapeItem::contains(point);
}

/*!
    \reimp
*/
void QGraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                 QWidget *widget)
{
    Q_D(QGraphicsEllipseItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->setBrush(d->brush);
    if ((d->spanAngle != 0) && (qAbs(d->spanAngle) % (360 * 16) == 0))
        painter->drawEllipse(d->rect);
    else
        painter->drawPie(d->rect, d->startAngle, d->spanAngle);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
}

/*!
    \reimp
*/
bool QGraphicsEllipseItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsEllipseItem::opaqueArea() const
{
    return QAbstractGraphicsShapeItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsEllipseItem::type() const
{
    return Type;
}


/*!
    \internal
*/
bool QGraphicsEllipseItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsEllipseItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsEllipseItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \class QGraphicsPolygonItem
    \brief The QGraphicsPolygonItem class provides a polygon item that you
    can add to a QGraphicsScene.
    \since 4.2
    \ingroup multimedia

    To set the item's polygon, pass a QPolygonF to
    QGraphicsPolygonItem's constructor, or call the setPolygon()
    function. The polygon() function returns the current polygon.

    \image graphicsview-polygonitem.png

    QGraphicsPolygonItem uses the polygon and the pen width to provide
    a reasonable implementation of boundingRect(), shape(), and
    contains(). The paint() function draws the polygon using the
    item's associated pen and brush, which you can set by calling the
    setPen() and setBrush() functions.

    \sa QGraphicsPathItem, QGraphicsRectItem, QGraphicsEllipseItem,
    QGraphicsTextItem, QGraphicsLineItem, QGraphicsPixmapItem, {The Graphics
    View Framework}
*/

class QGraphicsPolygonItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsPolygonItem)
public:
    inline QGraphicsPolygonItemPrivate()
        : fillRule(Qt::OddEvenFill)
    { }

    QPolygonF polygon;
    Qt::FillRule fillRule;
};

/*!
    Constructs a QGraphicsPolygonItem with \a polygon as the default
    polygon. The \a parent and \a scene parameters are passed to
    QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsPolygonItem::QGraphicsPolygonItem(const QPolygonF &polygon,
                                           QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsPolygonItemPrivate, parent, scene)
{
    setPolygon(polygon);
}

/*!
    Constructs a QGraphicsPolygonItem. The \a parent and \a scene
    parameters are passed to QAbstractGraphicsShapeItem's constructor.
*/
QGraphicsPolygonItem::QGraphicsPolygonItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsPolygonItemPrivate, parent, scene)
{
}

/*!
    Destroys the QGraphicsPolygonItem.
*/
QGraphicsPolygonItem::~QGraphicsPolygonItem()
{
}

/*!
    Returns the item's polygon, or an empty polygon if no polygon
    has been set.

    \sa setPolygon()
*/
QPolygonF QGraphicsPolygonItem::polygon() const
{
    Q_D(const QGraphicsPolygonItem);
    return d->polygon;
}

/*!
    Sets the item's polygon to be the given \a polygon.

    \sa polygon()
*/
void QGraphicsPolygonItem::setPolygon(const QPolygonF &polygon)
{
    Q_D(QGraphicsPolygonItem);
    removeFromIndex();
    d->polygon = polygon;
    addToIndex();
}

/*!
     Returns the fill rule of the polygon. The default fill rule is
     Qt::OddEvenFill.

     \sa setFillRule(), QPainterPath::fillRule(), QPainter::drawPolygon()
*/
Qt::FillRule QGraphicsPolygonItem::fillRule() const
{
     Q_D(const QGraphicsPolygonItem);
     return d->fillRule;
}

/*!
     Sets the fill rule of the polygon to \a rule. The default fill rule is
     Qt::OddEvenFill.

     \sa fillRule(), QPainterPath::fillRule(), QPainter::drawPolygon()
*/
void QGraphicsPolygonItem::setFillRule(Qt::FillRule rule)
{
     Q_D(QGraphicsPolygonItem);
     if (rule != d->fillRule) {
         d->fillRule = rule;
         update();
     }
}

/*!
    \reimp
*/
QRectF QGraphicsPolygonItem::boundingRect() const
{
    Q_D(const QGraphicsPolygonItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    return d->polygon.boundingRect().adjusted(-penWidth, -penWidth,
                                penWidth, penWidth);
}

/*!
    \reimp
*/
QPainterPath QGraphicsPolygonItem::shape() const
{
    Q_D(const QGraphicsPolygonItem);
    QPainterPath path;
    path.addPolygon(d->polygon);
    return qt_graphicsItem_shapeFromPath(path, d->pen);
}

/*!
    \reimp
*/
bool QGraphicsPolygonItem::contains(const QPointF &point) const
{
    return QAbstractGraphicsShapeItem::contains(point);
}

/*!
    \reimp
*/
void QGraphicsPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(QGraphicsPolygonItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->setBrush(d->brush);
    painter->drawPolygon(d->polygon, d->fillRule);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
}

/*!
    \reimp
*/
bool QGraphicsPolygonItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsPolygonItem::opaqueArea() const
{
    return QAbstractGraphicsShapeItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsPolygonItem::type() const
{
    return Type;
}

/*!
    \internal
*/
bool QGraphicsPolygonItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsPolygonItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsPolygonItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \class QGraphicsLineItem
    \brief The QGraphicsLineItem class provides a line item that you can add to a
    QGraphicsScene.
    \since 4.2
    \ingroup multimedia

    To set the item's line, pass a QLineF to QGraphicsLineItem's
    constructor, or call the setLine() function. The line() function
    returns the current line. By default the line is black with a
    width of 0, but you can change this by calling setPen().

    \img graphicsview-lineitem.png

    QGraphicsLineItem uses the line and the pen width to provide a reasonable
    implementation of boundingRect(), shape(), and contains(). The paint()
    function draws the line using the item's associated pen.

    \sa QGraphicsPathItem, QGraphicsRectItem, QGraphicsEllipseItem,
    QGraphicsTextItem, QGraphicsPolygonItem, QGraphicsPixmapItem, {The
    Graphics View Framework}
*/

class QGraphicsLineItemPrivate : public QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsLineItem)
public:
    QLineF line;
    QPen pen;
};

/*!
    Constructs a QGraphicsLineItem, using \a line as the default
    line. The \a parent and \a scene parameters are passed to
    QGraphicsItem's constructor.
*/
QGraphicsLineItem::QGraphicsLineItem(const QLineF &line, QGraphicsItem *parent,
                                     QGraphicsScene *scene)
    : QGraphicsItem(*new QGraphicsLineItemPrivate, parent, scene)
{
    setLine(line);
}

/*!
    Constructs a QGraphicsLineItem, using the line between (\a x1, \a
    y1) and (\a x2, \a y2) as the default line.  The \a parent and \a
    scene parameters are passed to QGraphicsItem's constructor.
*/
QGraphicsLineItem::QGraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent,
                                     QGraphicsScene *scene)
    : QGraphicsItem(*new QGraphicsLineItemPrivate, parent, scene)
{
    setLine(x1, y1, x2, y2);
}



/*!
    Constructs a QGraphicsLineItem. The \a parent and \a scene
    parameters are passed to QGraphicsItem's constructor.
*/
QGraphicsLineItem::QGraphicsLineItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsItem(*new QGraphicsLineItemPrivate, parent, scene)
{
}

/*!
    Destroys the QGraphicsLineItem.
*/
QGraphicsLineItem::~QGraphicsLineItem()
{
}

/*!
    Returns the item's pen, or a black solid 0-width pen if no pen has
    been set.

    \sa setPen()
*/
QPen QGraphicsLineItem::pen() const
{
    Q_D(const QGraphicsLineItem);
    return d->pen;
}

/*!
    Sets the item's pen to \a pen. If no pen is set, the line will be painted
    using a black solid 0-width pen.

    \sa pen()
*/
void QGraphicsLineItem::setPen(const QPen &pen)
{
    Q_D(QGraphicsLineItem);
    removeFromIndex();
    d->pen = pen;
    addToIndex();
}

/*!
    Returns the item's line, or a null line if no line has been set.

    \sa setLine()
*/
QLineF QGraphicsLineItem::line() const
{
    Q_D(const QGraphicsLineItem);
    return d->line;
}

/*!
    Sets the item's line to be the given \a line.

    \sa line()
*/
void QGraphicsLineItem::setLine(const QLineF &line)
{
    Q_D(QGraphicsLineItem);
    removeFromIndex();
    d->line = line;
    addToIndex();
}

/*!
    \fn void QGraphicsLineItem::setLine(qreal x1, qreal y1, qreal x2, qreal y2)
    \overload

    Sets the item's line to be the line between (\a x1, \a y1) and (\a
    x2, \a y2).

    This is the same as calling \c {setLine(QLineF(x1, y1, x2, y2))}.
*/

/*!
    \reimp
*/
QRectF QGraphicsLineItem::boundingRect() const
{
    Q_D(const QGraphicsLineItem);
    qreal penWidth = d->pen.widthF() / 2.0;

    QPointF p1 = d->line.p1();
    QPointF p2 = d->line.p2();

    return QRectF(p1, QSizeF(p2.x() - p1.x(), p2.y() - p1.y())).normalized()
        .adjusted(-penWidth, -penWidth, penWidth, penWidth);
}

/*!
    \reimp
*/
QPainterPath QGraphicsLineItem::shape() const
{
    Q_D(const QGraphicsLineItem);
    QPainterPath path;
    if (d->line.isNull())
        return path;

    path.moveTo(d->line.p1());
    path.lineTo(d->line.p2());
    return qt_graphicsItem_shapeFromPath(path, d->pen);
}

/*!
    \reimp
*/
bool QGraphicsLineItem::contains(const QPointF &point) const
{
    return QGraphicsItem::contains(point);
}

/*!
    \reimp
*/
void QGraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(QGraphicsLineItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->drawLine(d->line);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);

        QPointF p1 = d->line.p1();
        QPointF p2 = d->line.p2();
        qreal leftMostX;
        qreal topMostY;
        qreal width;
        qreal height;
        if (p1.x() < p2.x())
            leftMostX = p1.x();
        else
            leftMostX = p2.x();
        if (p1.y() < p2.y())
            topMostY = p1.y();
        else
            topMostY = p2.y();
        width = p1.x() - p2.x();
        if (width < 0)
            width = -width;
        height = p1.y() - p2.y();
        if (height < 0)
            height = -height;

        painter->drawRect(QRectF(leftMostX, topMostY, width, height));
    }
}

/*!
    \reimp
*/
bool QGraphicsLineItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QGraphicsItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsLineItem::opaqueArea() const
{
    return QGraphicsItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsLineItem::type() const
{
    return Type;
}

/*!
    \internal
*/
bool QGraphicsLineItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsLineItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsLineItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \class QGraphicsPixmapItem
    \brief The QGraphicsPixmapItem class provides a pixmap item that you can add to
    a QGraphicsScene.
    \since 4.2
    \ingroup multimedia

    To set the item's pixmap, pass a QPixmap to QGraphicsPixmapItem's
    constructor, or call the setPixmap() function. The pixmap()
    function returns the current pixmap.

    QGraphicsPixmapItem uses pixmap's optional alpha mask to provide a
    reasonable implementation of boundingRect(), shape(), and contains().

    \image graphicsview-pixmapitem.png

    The pixmap is drawn at the item's (0, 0) coordinate, as returned by
    offset(). You can change the drawing offset by calling setOffset().

    You can set the pixmap's transformation mode by calling
    setTransformationMode(). By default, Qt::FastTransformation is
    used, which provides fast, non-smooth scaling. Call
    transformationMode() to get the current transformation mode for
    the item.

    \sa QGraphicsPathItem, QGraphicsRectItem, QGraphicsEllipseItem,
    QGraphicsTextItem, QGraphicsPolygonItem, QGraphicsLineItem, {The
    Graphics View Framework}
*/

/*!
    \enum QGraphicsPixmapItem::ShapeMode

    This enum describes how QGraphicsPixmapItem calculates its shape.

    The default value is MaskShape.

    \value MaskShape The shape is determined by calling QPixmap::mask().
    This shape includes only the opaque pixels of the pixmap.
    Because the shape is more complex, however, it can be slower than the other modes,
    and uses more memory.

    \value BoundingRectShape The shape is determined by tracing the outline of
    the pixmap. This is the fastest shape mode, but it does not take into account
    any transparent areas on the pixmap.

    \value HeuristicMaskShape The shape is determine by calling
    QPixmap::createHeuristicMask().  The performance and memory consumption
    is similar to MaskShape.
*/

class QGraphicsPixmapItemPrivate : public QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsPixmapItem)
public:
    QGraphicsPixmapItemPrivate()
        : transformationMode(Qt::FastTransformation),
          shapeMode(QGraphicsPixmapItem::MaskShape)
    {}

    QPixmap pixmap;
    Qt::TransformationMode transformationMode;
    QPointF offset;
    QGraphicsPixmapItem::ShapeMode shapeMode;
    QPainterPath shape;

    void updateShape()
    {
        shape = QPainterPath();
        switch (shapeMode) {
        case QGraphicsPixmapItem::MaskShape: {
            QBitmap mask = pixmap.mask();
            if (!mask.isNull()) {
                shape.addRegion(QRegion(pixmap.mask()).translated(offset.toPoint()));
                break;
            }
            // FALL THROUGH
        }
        case QGraphicsPixmapItem::BoundingRectShape:
            shape.addRect(QRectF(offset.x(), offset.y(), pixmap.width(), pixmap.height()));
            break;
        case QGraphicsPixmapItem::HeuristicMaskShape:
#ifndef QT_NO_IMAGE_HEURISTIC_MASK
            shape.addRegion(QRegion(pixmap.createHeuristicMask()).translated(offset.toPoint()));
#else
            shape.addRect(QRectF(offset.x(), offset.y(), pixmap.width(), pixmap.height()));
#endif
            break;
        }
    }
};

/*!
    Constructs a QGraphicsPixmapItem, using \a pixmap as the default
    pixmap.  The \a parent and \a scene parameteres are passed to
    QGraphicsItem's constructor.
*/
QGraphicsPixmapItem::QGraphicsPixmapItem(const QPixmap &pixmap,
                                         QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsItem(*new QGraphicsPixmapItemPrivate, parent, scene)
{
    setPixmap(pixmap);
}

/*!
    Constructs a QGraphicsPixmapItem. The \a parent and \a scene
    parameteres are passed to QGraphicsItem's constructor.
*/
QGraphicsPixmapItem::QGraphicsPixmapItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsItem(*new QGraphicsPixmapItemPrivate, parent, scene)
{
}

/*!
    Destroys the QGraphicsPixmapItem.
*/
QGraphicsPixmapItem::~QGraphicsPixmapItem()
{
}

/*!
    Sets the item's pixmap to \a pixmap.

    \sa pixmap()
*/
void QGraphicsPixmapItem::setPixmap(const QPixmap &pixmap)
{
    Q_D(QGraphicsPixmapItem);
    removeFromIndex();
    d->pixmap = pixmap;
    d->updateShape();
    addToIndex();
}

/*!
    Returns the item's pixmap, or an invalid QPixmap if no pixmap has been
    set.

    \sa setPixmap()
*/
QPixmap QGraphicsPixmapItem::pixmap() const
{
    Q_D(const QGraphicsPixmapItem);
    return d->pixmap;
}

/*!
    Returns the transformation mode of the pixmap. The default mode is
    Qt::FastTransformation, which provides quick transformation with no
    smoothing.

    \sa setTransformationMode()
*/
Qt::TransformationMode QGraphicsPixmapItem::transformationMode() const
{
    Q_D(const QGraphicsPixmapItem);
    return d->transformationMode;
}

/*!
    Sets the pixmap item's transformation mode to \a mode, and toggles an
    update of the item. The default mode is Qt::FastTransformation, which
    provides quick transformation with no smoothing.

    \sa transformationMode()
*/
void QGraphicsPixmapItem::setTransformationMode(Qt::TransformationMode mode)
{
    Q_D(QGraphicsPixmapItem);
    if (mode != d->transformationMode) {
        update();
        d->transformationMode = mode;
        update();
    }
}

/*!
    Returns the pixmap item's \e offset, which defines the point of the
    top-left corner of the pixmap, in local coordinates.

    \sa setOffset()
*/
QPointF QGraphicsPixmapItem::offset() const
{
    Q_D(const QGraphicsPixmapItem);
    return d->offset;
}

/*!
    Sets the pixmap item's offset to \a offset. QGraphicsPixmapItem will draw
    its pixmap using \a offset for its top-left corner.

    \sa offset()
*/
void QGraphicsPixmapItem::setOffset(const QPointF &offset)
{
    Q_D(QGraphicsPixmapItem);
    if (offset != d->offset) {
        removeFromIndex();
        d->offset = offset;
        addToIndex();
    }
}

/*!
    \reimp
*/
QRectF QGraphicsPixmapItem::boundingRect() const
{
    Q_D(const QGraphicsPixmapItem);
    qreal halfPw = 0.5;
    return d->pixmap.isNull() ? QRectF() : QRectF(d->offset, d->pixmap.size())
        .adjusted(-halfPw, -halfPw, halfPw, halfPw);
}

/*!
    \reimp
*/
QPainterPath QGraphicsPixmapItem::shape() const
{
    return d_func()->shape;
}

/*!
    \reimp
*/
bool QGraphicsPixmapItem::contains(const QPointF &point) const
{
    return QGraphicsItem::contains(point);
}

/*!
    \reimp
*/
void QGraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                QWidget *widget)
{
    Q_D(QGraphicsPixmapItem);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::SmoothPixmapTransform,
                           (d->transformationMode == Qt::SmoothTransformation));

    QRectF exposed = option->exposedRect.adjusted(-1, -1, 1, 1);
    exposed &= QRectF(d->offset.x(), d->offset.y(), d->pixmap.width(), d->pixmap.height());
    exposed.translate(d->offset);
    painter->drawPixmap(exposed, d->pixmap, exposed);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(QRectF(QPointF(d->offset.x(), d->offset.y()),
                                 QSizeF(d->pixmap.width(), d->pixmap.height())));
    }
}

/*!
    \reimp
*/
bool QGraphicsPixmapItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QGraphicsItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsPixmapItem::opaqueArea() const
{
    return QGraphicsItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsPixmapItem::type() const
{
    return Type;
}

/*!
    Returns the item's shape mode. The shape mode describes how
    QGraphicsPixmapItem calculates its shape. The default mode is MaskShape.

    \sa setShapeMode(), ShapeMode
*/
QGraphicsPixmapItem::ShapeMode QGraphicsPixmapItem::shapeMode() const
{
    return d_func()->shapeMode;
}

/*!
    Sets the item's shape mode to \a mode. The shape mode describes how
    QGraphicsPixmapItem calculates its shape. The default mode is MaskShape.

    \sa shapeMode(), ShapeMode
*/
void QGraphicsPixmapItem::setShapeMode(ShapeMode mode)
{
    Q_D(QGraphicsPixmapItem);
    if (d->shapeMode == mode)
        return;
    d->shapeMode = mode;
    d->updateShape();
}

/*!
    \internal
*/
bool QGraphicsPixmapItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsPixmapItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsPixmapItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \class QGraphicsTextItem
    \brief The QGraphicsTextItem class provides a text item that you can add to
    a QGraphicsScene to display formatted text.
    \since 4.2
    \ingroup multimedia

    To set the item's text, pass a QString to QGraphicsTextItem's
    constructor, or call setHtml()/setPlainText().

    QGraphicsTextItem uses the text's formatted size and the associated font
    to provide a reasonable implementation of boundingRect(), shape(),
    and contains(). You can set the font by calling setFont().

    It is possible to make the item editable by setting the Qt::TextEditable flag
    using setTextInteractionFlags().

    \sa QGraphicsSimpleTextItem, QGraphicsPathItem, QGraphicsRectItem,
        QGraphicsEllipseItem, QGraphicsPixmapItem, QGraphicsPolygonItem,
        QGraphicsLineItem, {The Graphics View Framework}
*/

class QGraphicsTextItemPrivate
{
public:
    QGraphicsTextItemPrivate()
        : control(0), pageNumber(0)
    { }

    mutable QTextControl *control;
    QTextControl *textControl() const;

    inline QPointF controlOffset() const
    { return QPointF(0., pageNumber * control->document()->pageSize().height()); }
    inline void sendControlEvent(QEvent *e)
    { if (control) control->processEvent(e, controlOffset()); }

    void _q_updateBoundingRect(const QSizeF &);
    void _q_update(QRectF);
    void _q_ensureVisible(QRectF);

    QRectF boundingRect;
    int pageNumber;

    QGraphicsTextItem *qq;
};

/*!
    Constructs a QGraphicsTextItem, using \a text as the default plain text. \a
    parent and \a scene are passed to QGraphicsItem's constructor.
*/
QGraphicsTextItem::QGraphicsTextItem(const QString &text, QGraphicsItem *parent,
                                     QGraphicsScene *scene)
    : QGraphicsItem(parent, scene), dd(new QGraphicsTextItemPrivate)
{
    dd->qq = this;
    if (!text.isEmpty())
        setPlainText(text);
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
}

/*!
    Constructs a QGraphicsTextItem. \a parent and \a scene are passed to
    QGraphicsItem's constructor.
*/
QGraphicsTextItem::QGraphicsTextItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsItem(parent, scene), dd(new QGraphicsTextItemPrivate)
{
    dd->qq = this;
    setAcceptDrops(true);
    setAcceptsHoverEvents(true);
}

/*!
    Destroys the QGraphicsTextItem.
*/
QGraphicsTextItem::~QGraphicsTextItem()
{
    delete dd;
}

/*!
    Returns the item's text converted to HTML, or an empty QString if no text has been set.

    \sa setHtml()
*/
QString QGraphicsTextItem::toHtml() const
{
    return dd->control
        ? dd->control->toHtml()
        : QString();
}

/*!
    Sets the item's text to \a text, assuming that text is HTML formatted.

    \sa toHtml()
*/
void QGraphicsTextItem::setHtml(const QString &text)
{
    dd->textControl()->setHtml(text);
}

/*!
    Returns the item's text converted to plain text, or an empty QString if no text has been set.

    \sa setPlainText()
*/
QString QGraphicsTextItem::toPlainText() const
{
    return dd->control
        ? dd->control->toPlainText()
        : QString();
}

/*!
    Sets the item's text to \a text.

    \sa toHtml()
*/
void QGraphicsTextItem::setPlainText(const QString &text)
{
    dd->textControl()->setPlainText(text);
}

/*!
    Returns the item's font, which is used to render the text.

    \sa setFont()
*/
QFont QGraphicsTextItem::font() const
{
    if (!dd->control)
        return QFont();
    return dd->control->document()->defaultFont();
}

/*!
    Sets the font used to render the text item to \a font.

    \sa font()
*/
void QGraphicsTextItem::setFont(const QFont &font)
{
    dd->textControl()->document()->setDefaultFont(font);
}

/*!
    Sets the color for unformatted text to \a col.
*/
void QGraphicsTextItem::setDefaultTextColor(const QColor &col)
{
    QTextControl *c = dd->textControl();
    QPalette pal = c->palette();
    pal.setColor(QPalette::Text, col);
    c->setPalette(pal);
}

/*!
    Returns the default text color that is used to for unformatted text.
*/
QColor QGraphicsTextItem::defaultTextColor() const
{
    return dd->textControl()->palette().color(QPalette::Text);
}

/*!
    \reimp
*/
QRectF QGraphicsTextItem::boundingRect() const
{
    return dd->boundingRect;
}

/*!
    \reimp
*/
QPainterPath QGraphicsTextItem::shape() const
{
    if (!dd->control)
        return QPainterPath();
    QPainterPath path;
    path.addRect(dd->boundingRect);
    return path;
}

/*!
    \reimp
*/
bool QGraphicsTextItem::contains(const QPointF &point) const
{
    return dd->boundingRect.contains(point);
}

/*!
    \reimp
*/
void QGraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget)
{
    Q_UNUSED(widget);
    if (dd->control) {
        painter->save();
        QRectF r = option->exposedRect;
        painter->translate(-dd->controlOffset());
        r.translate(dd->controlOffset());
        dd->control->drawContents(painter, r);
        painter->restore();
    }

    if (option->state & (QStyle::State_Selected | QStyle::State_HasFocus)) {
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(dd->boundingRect);
    }
}

/*!
    \reimp
*/
bool QGraphicsTextItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QGraphicsItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsTextItem::opaqueArea() const
{
    return QGraphicsItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsTextItem::type() const
{
    return Type;
}

/*!
    Sets the preferred width for the item's text. If the actual text
    is wider than the specified width then it will be broken into
    multiple lines.

    If \a width is set to -1 then the text will not be broken into multiple
    lines unless it is enforced through an explicit line break or a new paragraph.

    The default value is -1.
*/
void QGraphicsTextItem::setTextWidth(qreal width)
{
    dd->textControl()->setTextWidth(width);
}

/*!
    Returns the text width.

    \sa setTextWidth()
*/
qreal QGraphicsTextItem::textWidth() const
{
    if (!dd->control)
        return -1;
    return dd->control->textWidth();
}

/*!
    Adjusts the text item to a reasonable size.
*/
void QGraphicsTextItem::adjustSize()
{
    if (dd->control)
        dd->control->adjustSize();
}

/*!
    Sets the text document \a document on the item.
*/
void QGraphicsTextItem::setDocument(QTextDocument *document)
{
    dd->textControl()->setDocument(document);
    dd->_q_updateBoundingRect(dd->control->size());
}

/*!
    Returns the item's text document.
*/
QTextDocument *QGraphicsTextItem::document() const
{
    return dd->textControl()->document();
}

/*!
    \reimp
*/
bool QGraphicsTextItem::sceneEvent(QEvent *event)
{
    return QGraphicsItem::sceneEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!hasFocus()) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!hasFocus()) {
        QGraphicsItem::mouseMoveEvent(event);
        return;
    }

    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!hasFocus()) {
        QGraphicsItem::mouseReleaseEvent(event);
        return;
    }

    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!hasFocus()) {
        QGraphicsItem::mouseDoubleClickEvent(event);
        return;
    }

    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::keyPressEvent(QKeyEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::keyReleaseEvent(QKeyEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::focusInEvent(QFocusEvent *event)
{
    dd->sendControlEvent(event);
    update();
}

/*!
    \reimp
*/
void QGraphicsTextItem::focusOutEvent(QFocusEvent *event)
{
    dd->sendControlEvent(event);
    update();
}

/*!
    \reimp
*/
void QGraphicsTextItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::inputMethodEvent(QInputMethodEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
void QGraphicsTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    dd->sendControlEvent(event);
}

/*!
    \reimp
*/
QVariant QGraphicsTextItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    QVariant v;
    if (dd->control)
        v = dd->control->inputMethodQuery(query);
    if (v.type() == QVariant::RectF)
        v = v.toRectF().translated(-dd->controlOffset());
    else if (v.type() == QVariant::PointF)
        v = v.toPointF() - dd->controlOffset();
    else if (v.type() == QVariant::Rect)
        v = v.toRect().translated(-dd->controlOffset().toPoint());
    else if (v.type() == QVariant::Point)
        v = v.toPoint() - dd->controlOffset().toPoint();
    return v;
}

/*!
    \internal
*/
bool QGraphicsTextItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsTextItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsTextItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \internal
*/
void QGraphicsTextItemPrivate::_q_update(QRectF rect)
{
    if (rect.isValid()) {
        rect.translate(-controlOffset());
    } else {
        rect = boundingRect;
    }
    if (rect.intersects(boundingRect))
        qq->update(rect);
}

/*!
    \internal
*/
void QGraphicsTextItemPrivate::_q_updateBoundingRect(const QSizeF &size)
{
    if (!control) return; // can't happen
    const QSizeF pageSize = control->document()->pageSize();
    // paged items have a constant (page) size
    if (size == boundingRect.size() || pageSize.height() != -1)
        return;
    qq->removeFromIndex();
    boundingRect.setSize(size);
    qq->addToIndex();
}

/*!
    \internal
*/
void QGraphicsTextItemPrivate::_q_ensureVisible(QRectF rect)
{
    rect.translate(-controlOffset());
    qq->ensureVisible(rect, /*xmargin=*/0, /*ymargin=*/0);
}

QTextControl *QGraphicsTextItemPrivate::textControl() const
{
    if (!control) {
        QGraphicsTextItem *that = const_cast<QGraphicsTextItem *>(qq);
        control = new QTextControl(that);
        control->setTextInteractionFlags(Qt::NoTextInteraction);

        QObject::connect(control, SIGNAL(updateRequest(QRectF)),
                         qq, SLOT(_q_update(QRectF)));
        QObject::connect(control, SIGNAL(documentSizeChanged(QSizeF)),
                         qq, SLOT(_q_updateBoundingRect(QSizeF)));
        QObject::connect(control, SIGNAL(visibilityRequest(QRectF)),
                         qq, SLOT(_q_ensureVisible(QRectF)));
        QObject::connect(control, SIGNAL(linkActivated(QString)),
                         qq, SIGNAL(linkActivated(QString)));
        QObject::connect(control, SIGNAL(linkHovered(QString)),
                         qq, SIGNAL(linkHovered(QString)));

        const QSizeF pgSize = control->document()->pageSize();
        if (pgSize.height() != -1) {
            qq->removeFromIndex();
            that->dd->boundingRect.setSize(pgSize);
            qq->addToIndex();
        } else {
            that->dd->_q_updateBoundingRect(control->size());
        }
    }
    return control;
}

/*!
    \fn QGraphicsTextItem::linkActivated(const QString &link)

    This signal is emitted when the user clicks on a link on a text item
    that enables Qt::LinksAccessibleByMouse or Qt::LinksAccessibleByKeyboard.
    \a link is the link that was clicked.

    \sa setTextInteractionFlags()
*/

/*!
    \fn QGraphicsTextItem::linkHovered(const QString &link)

    This signal is emitted when the user hovers over a link on a text item
    that enables Qt::LinksAccessibleByMouse. \a link is
    the link that was hovered over.

    \sa setTextInteractionFlags()
*/

/*!
    Sets the flags \a flags to specify how the text item should react to user
    input.

    The default for a QGraphicsTextItem is Qt::NoTextInteraction. Setting a
    value different to Qt::NoTextInteraction will also set the ItemIsFocusable
    QGraphicsItem flag.

    By default, the text is read-only. To transform the item into an editor,
    set the Qt::TextEditable flag.
*/
void QGraphicsTextItem::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
    if (flags & Qt::NoTextInteraction)
        setFlags(this->flags() & ~QGraphicsItem::ItemIsFocusable);
    else
        setFlags(this->flags() | QGraphicsItem::ItemIsFocusable);
    dd->textControl()->setTextInteractionFlags(flags);
}

/*!
    Returns the current text interaction flags.

    \sa setTextInteractionFlags()
*/
Qt::TextInteractionFlags QGraphicsTextItem::textInteractionFlags() const
{
    if (!dd->control)
        return Qt::NoTextInteraction;
    return dd->control->textInteractionFlags();
}

/*!
    \property QGraphicsTextItem::openExternalLinks

    Specifies whether QGraphicsTextItem should automatically open links using
    QDesktopServices::openUrl() instead of emitting the
    anchorClicked signal.

    The default value is false.
*/
void QGraphicsTextItem::setOpenExternalLinks(bool open)
{
    dd->textControl()->setOpenExternalLinks(open);
}

bool QGraphicsTextItem::openExternalLinks() const
{
    if (!dd->control)
        return false;
    return dd->control->openExternalLinks();
}

/*!
    \property QGraphicsTextItem::textCursor

    This property represents the visible text cursor in an editable
    text item.
*/
void QGraphicsTextItem::setTextCursor(const QTextCursor &cursor)
{
    dd->textControl()->setTextCursor(cursor);
}

QTextCursor QGraphicsTextItem::textCursor() const
{
    if (!dd->control)
        return QTextCursor();
    return dd->control->textCursor();
}

class QGraphicsSimpleTextItemPrivate : public QAbstractGraphicsShapeItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsSimpleTextItem)
public:
    inline QGraphicsSimpleTextItemPrivate() {
        pen.setStyle(Qt::NoPen);
        brush.setStyle(Qt::SolidPattern);
    }
    QString text;
    QFont font;
    QRectF boundingRect;

    void updateBoundingRect();
};

static QRectF setupTextLayout(QTextLayout *layout)
{
    layout->setCacheEnabled(true);
    layout->beginLayout();
    while (layout->createLine().isValid())
        ;
    layout->endLayout();
    qreal maxWidth = 0;
    qreal y = 0;
    for (int i = 0; i < layout->lineCount(); ++i) {
        QTextLine line = layout->lineAt(i);
        maxWidth = qMax(maxWidth, line.naturalTextWidth());
        line.setPosition(QPointF(0, y));
        y += line.height();
    }
    return QRectF(0, 0, maxWidth, y);
}

void QGraphicsSimpleTextItemPrivate::updateBoundingRect()
{
    Q_Q(QGraphicsSimpleTextItem);
    q->removeFromIndex();
    if (text.isEmpty()) {
        boundingRect = QRectF();
    } else {
        QString tmp = text;
        tmp.replace(QLatin1Char('\n'), QChar::LineSeparator);
        QStackTextEngine engine(tmp, font);
        QTextLayout layout(&engine);
        boundingRect = setupTextLayout(&layout);
    }
    q->addToIndex();
}

/*!
    \class QGraphicsSimpleTextItem
    \brief The QGraphicsSimpleTextItem class provides a text item that you can add to
    a QGraphicsScene.
    \since 4.2
    \ingroup multimedia

    To set the item's text, pass a QString to QGraphicsSimpleTextItem's
    constructor, or call setText().

    QGraphicsTextItem uses the text's formatted size and the associated font
    to provide a reasonable implementation of boundingRect(), shape(),
    and contains(). You can set the font by calling setFont().

    QGraphicsSimpleText does not display rich text. It is optimized for drawing
    plain text fast and with low memory overhead.

    \sa QGraphicsTextItem, QGraphicsPathItem, QGraphicsRectItem, QGraphicsEllipseItem,
    QGraphicsPixmapItem, QGraphicsPolygonItem, QGraphicsLineItem, {The
    Graphics View Framework}
*/

/*!
    Constructs a QGraphicsSimpleTextItem.

    \a parent and \a scene are passed to QGraphicsItem's constructor.
*/
QGraphicsSimpleTextItem::QGraphicsSimpleTextItem(QGraphicsItem *parent, QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsSimpleTextItemPrivate, parent, scene)
{
}

/*!
    Constructs a QGraphicsSimpleTextItem, using \a text as the default plain text.

    \a parent and \a scene are passed to QGraphicsItem's constructor.
*/
QGraphicsSimpleTextItem::QGraphicsSimpleTextItem(const QString &text, QGraphicsItem *parent , QGraphicsScene *scene)
    : QAbstractGraphicsShapeItem(*new QGraphicsSimpleTextItemPrivate, parent, scene)
{
    setText(text);
}

/*!
    Destroys the QGraphicsSimpleTextItem.
*/
QGraphicsSimpleTextItem::~QGraphicsSimpleTextItem()
{
}

/*!
    Sets the item's text to \a text. The text will be displayed as
    plain text. Newline characters ('\n') as well as characters of
    type QChar::LineSeparator will cause item to break the text into
    multiple lines.
*/
void QGraphicsSimpleTextItem::setText(const QString &text)
{
    Q_D(QGraphicsSimpleTextItem);
    d->text = text;
    d->updateBoundingRect();
}

/*!
    Returns the item's text.
*/
QString QGraphicsSimpleTextItem::text() const
{
    Q_D(const QGraphicsSimpleTextItem);
    return d->text;
}

/*!
    Sets the font that is used to draw the item's text to \a font.
*/
void QGraphicsSimpleTextItem::setFont(const QFont &font)
{
    Q_D(QGraphicsSimpleTextItem);
    d->font = font;
    d->updateBoundingRect();
}

/*!
    Returns the font that is used to draw the item's text.
*/
QFont QGraphicsSimpleTextItem::font() const
{
    Q_D(const QGraphicsSimpleTextItem);
    return d->font;
}

/*!
    \reimp
*/
QRectF QGraphicsSimpleTextItem::boundingRect() const
{
    Q_D(const QGraphicsSimpleTextItem);
    return d->boundingRect;
}

/*!
    \reimp
*/
QPainterPath QGraphicsSimpleTextItem::shape() const
{
    Q_D(const QGraphicsSimpleTextItem);
    QPainterPath path;
    path.addRect(d->boundingRect);
    return path;
}

/*!
    \reimp
*/
bool QGraphicsSimpleTextItem::contains(const QPointF &point) const
{
    Q_D(const QGraphicsSimpleTextItem);
    return d->boundingRect.contains(point);
}

/*!
    \reimp
*/
void QGraphicsSimpleTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_D(QGraphicsSimpleTextItem);

    painter->setFont(d->font);

    QString tmp = d->text;
    tmp.replace(QLatin1Char('\n'), QChar::LineSeparator);
    QStackTextEngine engine(tmp, d->font);
    QTextLayout layout(&engine);
    setupTextLayout(&layout);

    QPen p;
    p.setBrush(d->brush);
    painter->setPen(p);
    if (d->pen.style() == Qt::NoPen && d->brush.style() == Qt::SolidPattern) {
        painter->setBrush(Qt::NoBrush);
    } else {
        QTextLayout::FormatRange range;
        range.start = 0;
        range.length = layout.text().length();
        range.format.setTextOutline(d->pen);
        QList<QTextLayout::FormatRange> formats;
        formats.append(range);
        layout.setAdditionalFormats(formats);
    }

    layout.draw(painter, QPointF(0, 0));

    if (option->state & (QStyle::State_Selected | QStyle::State_HasFocus)) {
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(d->boundingRect);
    }
}

/*!
    \reimp
*/
bool QGraphicsSimpleTextItem::isObscuredBy(const QGraphicsItem *item) const
{
    return QAbstractGraphicsShapeItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsSimpleTextItem::opaqueArea() const
{
    return QAbstractGraphicsShapeItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsSimpleTextItem::type() const
{
    return Type;
}

/*!
    \internal
*/
bool QGraphicsSimpleTextItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsSimpleTextItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsSimpleTextItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

/*!
    \class QGraphicsItemGroup
    \brief The QGraphicsItemGroup class provides treating a group of items as
    one.
    \since 4.2
    \ingroup multimedia

    A QGraphicsItemGroup is a special type of compound item that
    treats itself and all its children as one item (i.e., all events
    and geometries for all children are merged together). It's common
    to use item groups in presentation tools, when the user wants to
    group several smaller items into one big item in order to simplify
    moving and copying of items.

    If all you want is to store items inside other items, you can use
    any QGraphicsItem directly by passing a suitable parent to
    setParentItem().

    The boundingRect() function of QGraphicsItemGroup returns the
    bounding rectangle of all items in the item group. In addition,
    item groups have handlesChildEvents() enabled by default, so all
    events sent to a member of the group go to the item group (i.e.,
    selecting one item in a group will select them all).

    There are two ways to construct an item group. The easiest and
    most common approach is to pass a list of items (e.g., all
    selected items) to QGraphicsScene::createItemGroup(), which
    returns a new QGraphicsItemGroup item. The other approach is to
    manually construct a QGraphicsItemGroup item, add it to the scene
    calling QGraphicsScene::addItem(), and then add items to the group
    manually, one at a time by calling addToGroup(). To dismantle
    ("ungroup") an item group, you can either call
    QGraphicsScene::destroyItemGroup(), or you can manually remove all
    items from the group by calling removeFromGroup().

    \code
      // Group all selected items together
      QGraphicsItemGroup *group = scene->createItemGroup(scene->selecteditems());

      // Destroy the group, and delete the group item
      scene->destroyItemGroup(group);
    \endcode

    The operation of adding and removing items preserves the items'
    scene-relative position and transformation, as opposed to calling
    setParentItem(), where only the child item's parent-relative
    position and transformation are kept.

    The addtoGroup() function reparents the target item to this item
    group, keeping the item's position and transformation intact
    relative to the scene. Visually, this means that items added via
    addToGroup() will remain completely unchanged as a result of this
    operation, regardless of the item or the group's current position
    or transformation; although the item's position and matrix are
    likely to change.

    The removeFromGroup() function has similar semantics to
    setParentItem(); it reparents the item to the parent item of the
    item group. As with addToGroup(), the item's scene-relative
    position and transformation remain intact.

    \sa QGraphicsItem, {The Graphics View Framework}
*/

class QGraphicsItemGroupPrivate : public QGraphicsItemPrivate
{
public:
    QRectF itemsBoundingRect;
};

/*!
    Constructs a QGraphicsItemGroup. The \a parent and \a scene
    parameters are passed to QGraphicsItem's constructor.
*/
QGraphicsItemGroup::QGraphicsItemGroup(QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsItem(*new QGraphicsItemGroupPrivate, parent, scene)
{
    setHandlesChildEvents(true);
}

/*!
    Destroys the QGraphicsItemGroup.
*/
QGraphicsItemGroup::~QGraphicsItemGroup()
{
}

/*!
    Adds the given \a item to this item group. The item will be
    reparented to this group, but its position and transformation
    relative to the scene will stay intact.

    \sa removeFromGroup(), QGraphicsScene::createItemGroup()
*/
void QGraphicsItemGroup::addToGroup(QGraphicsItem *item)
{
    Q_D(QGraphicsItemGroup);
    if (!item) {
        qWarning("QGraphicsItemGroup::addToGroup: cannot add null item");
        return;
    }
    if (item == this) {
        qWarning("QGraphicsItemGroup::addToGroup: cannot add a group to itself");
        return;
    }

    QMatrix oldSceneMatrix = item->sceneMatrix();
    item->setPos(mapFromItem(item, 0, 0));
    item->setParentItem(this);
    item->setMatrix(oldSceneMatrix
                    * sceneMatrix().inverted()
                    * QMatrix().translate(-item->x(), -item->y()));
    item->d_func()->setIsMemberOfGroup(true);
    removeFromIndex();
    d->itemsBoundingRect |= (item->matrix() * QMatrix().translate(item->x(), item->y()))
                            .mapRect(item->boundingRect() | item->childrenBoundingRect());
    addToIndex();
}

/*!
    Removes the specified \a item from this group. The item will be
    reparented to this group's parent item, or to 0 if this group has
    no parent.  Its position and transformation relative to the scene
    will stay intact.

    \sa addToGroup(), QGraphicsScene::destroyItemGroup()
*/
void QGraphicsItemGroup::removeFromGroup(QGraphicsItem *item)
{
    Q_D(QGraphicsItemGroup);
    if (!item) {
        qWarning("QGraphicsItemGroup::removeFromGroup: cannot remove null item");
        return;
    }

    QGraphicsItem *newParent = parentItem();
    QPointF oldPos = item->mapToItem(newParent, 0, 0);
    item->setParentItem(newParent);
    item->setPos(oldPos);
    item->d_func()->setIsMemberOfGroup(item->group() != 0);

    // ### Quite expensive. But removeFromGroup() isn't called very often.
    d->itemsBoundingRect = childrenBoundingRect();
}

/*!
    \reimp

    Returns the bounding rect of this group item, and all its children.
*/
QRectF QGraphicsItemGroup::boundingRect() const
{
    Q_D(const QGraphicsItemGroup);
    return d->itemsBoundingRect;
}

/*!
    \reimp
*/
void QGraphicsItemGroup::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
    Q_UNUSED(widget);
    if (option->state & QStyle::State_Selected) {
        Q_D(QGraphicsItemGroup);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(d->itemsBoundingRect);
    }
}

/*!
    \reimp
*/
bool QGraphicsItemGroup::isObscuredBy(const QGraphicsItem *item) const
{
    return QGraphicsItem::isObscuredBy(item);
}

/*!
    \reimp
*/
QPainterPath QGraphicsItemGroup::opaqueArea() const
{
    return QGraphicsItem::opaqueArea();
}

/*!
    \reimp
*/
int QGraphicsItemGroup::type() const
{
    return Type;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, QGraphicsItem *item)
{
    if (!item) {
        debug << "QGraphicsItem(0)";
        return debug;
    }

    QStringList flags;
    if (item->isVisible()) flags << QLatin1String("isVisible");
    if (item->isEnabled()) flags << QLatin1String("isEnabled");
    if (item->isSelected()) flags << QLatin1String("isSelected");
    if (item->hasFocus()) flags << QLatin1String("HasFocus");

    debug << "QGraphicsItem(this =" << ((void*)item)
          << ", parent =" << ((void*)item->parentItem())
          << ", pos =" << item->pos()
          << ", z =" << item->zValue() << ", flags = {"
          << flags.join(QLatin1String("|")) << " })";
    return debug;
}
#endif

#include "moc_qgraphicsitem.cpp"

#endif // QT_NO_GRAPHICSVIEW
