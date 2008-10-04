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

#include "qglobal.h"

#ifndef QT_NO_GRAPHICSVIEW

#include "qgraphicslayout.h"
#include "qgraphicsscene.h"
#include "qgraphicslayoutitem.h"
#include "qgraphicslayoutitem_p.h"
#include "qwidget.h"

#include <QtDebug>

QT_BEGIN_NAMESPACE

/*
    COMBINE_SIZE() is identical to combineSize(), except that it
    doesn't evaluate 'size' unless necessary.
*/
#define COMBINE_SIZE(result, size) \
    do { \
        if ((result).width() < 0 || (result).height() < 0) \
            combineSize((result), (size)); \
    } while (false)

static void combineSize(QSizeF &result, const QSizeF &size)
{
    if (result.width() < 0)
        result.setWidth(size.width());
    if (result.height() < 0)
        result.setHeight(size.height());
}

static void boundSize(QSizeF &result, const QSizeF &size)
{
    if (size.width() >= 0 && size.width() < result.width())
        result.setWidth(size.width());
    if (size.height() >= 0 && size.height() < result.height())
        result.setHeight(size.height());
}

static void expandSize(QSizeF &result, const QSizeF &size)
{
    if (size.width() >= 0 && size.width() > result.width())
        result.setWidth(size.width());
    if (size.height() >= 0 && size.height() > result.height())
        result.setHeight(size.height());
}

static void normalizeHints(qreal &minimum, qreal &preferred, qreal &maximum, qreal &descent)
{
    if (minimum >= 0 && maximum >= 0 && minimum > maximum)
        minimum = maximum;

    if (preferred >= 0) {
        if (minimum >= 0 && preferred < minimum) {
            preferred = minimum;
        } else if (maximum >= 0 && preferred > maximum) {
            preferred = maximum;
        }
    }

    if (minimum >= 0 && descent > minimum)
        descent = minimum;
}

/*!
    \internal
*/
QGraphicsLayoutItemPrivate::QGraphicsLayoutItemPrivate(QGraphicsLayoutItem *par, bool layout)
    : parent(par), isLayout(layout)
{
}

/*!
    \internal
*/
void QGraphicsLayoutItemPrivate::init()
{
    sizeHintCacheDirty = true; 
    sizePolicy = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

/*!
    \internal
*/
QSizeF *QGraphicsLayoutItemPrivate::effectiveSizeHints(const QSizeF &constraint) const
{
    Q_Q(const QGraphicsLayoutItem);
    if (!sizeHintCacheDirty && cachedConstraint == constraint)
        return cachedSizeHints;

    for (int i = 0; i < Qt::NSizeHints; ++i) {
        cachedSizeHints[i] = constraint;
        combineSize(cachedSizeHints[i], userSizeHints[i]);
    }

    QSizeF &minS = cachedSizeHints[Qt::MinimumSize];
    QSizeF &prefS = cachedSizeHints[Qt::PreferredSize];
    QSizeF &maxS = cachedSizeHints[Qt::MaximumSize];
    QSizeF &descentS = cachedSizeHints[Qt::MinimumDescent];

    normalizeHints(minS.rwidth(), prefS.rwidth(), maxS.rwidth(), descentS.rwidth());
    normalizeHints(minS.rheight(), prefS.rheight(), maxS.rheight(), descentS.rheight());

    // if the minimum, preferred and maximum sizes contradict each other
    // (e.g. the minimum is larger than the maximum) we give priority to
    // the maximum size, then the minimum size and finally the preferred size
    COMBINE_SIZE(maxS, q->sizeHint(Qt::MaximumSize, maxS));
    combineSize(maxS, QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    expandSize(maxS, prefS);
    expandSize(maxS, minS);
    boundSize(maxS, QSizeF(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));

    COMBINE_SIZE(minS, q->sizeHint(Qt::MinimumSize, minS));
    expandSize(minS, QSizeF(0, 0));
    boundSize(minS, prefS);
    boundSize(minS, maxS);

    COMBINE_SIZE(prefS, q->sizeHint(Qt::PreferredSize, prefS));
    expandSize(prefS, minS);
    boundSize(prefS, maxS);

    // Not supported yet
    // COMBINE_SIZE(descentS, q->sizeHint(Qt::MinimumDescent, constraint));

    cachedConstraint = constraint;
    sizeHintCacheDirty = false;
    return cachedSizeHints;
}

/*!
    \class QGraphicsLayoutItem
    \brief The QGraphicsLayoutItem class can be inherited to allow your custom
    items to be managed by layouts.
    \since 4.4
    \ingroup multimedia
    \ingroup graphicsview-api

    QGraphicsLayoutItem is an abstract class that defines a set of virtual
    functions describing sizes, size policies, and size hints for any object
    arranged by QGraphicsLayout. The API contains functions relevant
    for both the item itself and for the user of the item as most of
    QGraphicsLayoutItem's functions are also part of the subclass' public API.

    In most cases, existing layout-aware classes such as QGraphicsWidget and
    QGraphicsLayout already provide the functionality you require. However,
    subclassing these classes will enable you to create both graphical
    elements that work well with layouts (QGraphicsWidget) or custom layouts
    (QGraphicsLayout).

    \section1 Subclassing QGraphicsLayoutItem

    If you create a subclass of QGraphicsLayoutItem and reimplement its
    virtual functions, you will enable the layout to resize and position your
    item along with other QGraphicsLayoutItems including QGraphicsWidget
    and QGraphicsLayout.

    You can start by reimplementing important functions: the protected
    sizeHint() function, as well as the public setGeometry()
    function. If you want your items to be aware of immediate geometry
    changes, you can also reimplement updateGeometry().

    The geometry, size hint, and size policy affect the item's size and
    position. Calling setGeometry() will always resize and reposition the item
    immediately. Normally, this function is called by QGraphicsLayout after
    the layout has been activated, but it can also be called by the item's user
    at any time. 

    The sizeHint() function returns the item' minimum, preferred and maximum
    size hints. You can override these properties by calling setMinimumSize(),
    setPreferredSize() or setMaximumSize(). You can also use functions such as
    setMinimumWidth() or setMaximumHeight() to set only the width or height
    component if desired.

    The effectiveSizeHint() function, on the other hand, returns a size hint
    for any given Qt::SizeHint, and guarantees that the returned size is bound
    to the minimum and maximum sizes and size hints. You can set the item's
    vertical and horizontal size policy by calling setSizePolicy(). The
    sizePolicy property is used by the layout system to describe how this item
    prefers to grow or shrink.

    \section1 Nesting QGraphicsLayoutItems

    QGraphicsLayoutItems can be nested within other QGraphicsLayoutItems,
    similar to layouts that can contain sublayouts. This is done either by
    passing a QGraphicsLayoutItem pointer to QGraphicsLayoutItem's
    protected constructor, or by calling setParentLayoutItem(). The
    parentLayoutItem() function returns a pointer to the item's layoutItem
    parent. If the item's parent is 0 or if the the parent does not inherit
    from QGraphicsItem, the parentLayoutItem() function then returns 0.
    isLayout() returns true if the QGraphicsLayoutItem subclass is itself a
    layout, or false otherwise.

    Qt uses QGraphicsLayoutItem to provide layout functionality in the
    \l{The Graphics View Framework}, but in the future its use may spread
    throughout Qt itself.

    \sa QGraphicsWidget, QGraphicsLayout, QGraphicsLinearLayout,
    QGraphicsGridLayout
*/

/*!
    Constructs the QGraphicsLayoutItem object. \a parent becomes the object's
    parent. If \a isLayout is true the item is a layout, otherwise
    \a isLayout is false.
*/
QGraphicsLayoutItem::QGraphicsLayoutItem(QGraphicsLayoutItem *parent, bool isLayout)
    : d_ptr(new QGraphicsLayoutItemPrivate(parent, isLayout))
{
    Q_D(QGraphicsLayoutItem);
    d->init();
    d->q_ptr = this;
}

/*!
    \internal
*/
QGraphicsLayoutItem::QGraphicsLayoutItem(QGraphicsLayoutItemPrivate &dd)
    : d_ptr(&dd)
{
    Q_D(QGraphicsLayoutItem);
    d->q_ptr = this;
}

/*!
    Destroys the QGraphicsLayoutItem object.
*/
QGraphicsLayoutItem::~QGraphicsLayoutItem()
{
    delete d_ptr;
}

/*!
    \fn virtual QSizeF QGraphicsLayoutItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const = 0;

    This pure virtual function returns the size hint for \a which of the
    QGraphicsLayoutItem, using the width or height of \a constraint to
    constrain the output.

    Reimplement this function in a subclass of QGraphicsLayoutItem to
    provide the necessary size hints for your items.

    \sa effectiveSizeHint()
*/

/*!
    Sets the size policy to \a policy. The size policy describes how the item
    should grow horizontally and vertically when arranged in a layout.

    QGraphicsLayoutItem's default size policy is (QSizePolicy::Fixed,
    QSizePolicy::Fixed, QSizePolicy::DefaultType), but it is common for
    subclasses to change the default. For example, QGraphicsWidget defaults
    to (QSizePolicy::Preferred, QSizePolicy::Preferred,
    QSizePolicy::DefaultType).

    \sa sizePolicy(), QWidget::sizePolicy()
*/
void QGraphicsLayoutItem::setSizePolicy(const QSizePolicy &policy)
{
    Q_D(QGraphicsLayoutItem);
    if (d->sizePolicy == policy)
        return;
    d->sizePolicy = policy;
    updateGeometry();
}

/*!
    \overload

    This function is equivalent to calling
    setSizePolicy(QSizePolicy(\a hPolicy, \a vPolicy, \a controlType)).

    \sa sizePolicy(), QWidget::sizePolicy()
*/
void QGraphicsLayoutItem::setSizePolicy(QSizePolicy::Policy hPolicy,
                                        QSizePolicy::Policy vPolicy,
                                        QSizePolicy::ControlType controlType)
{
    setSizePolicy(QSizePolicy(hPolicy, vPolicy, controlType));
}

/*!
    Returns the current size policy. 

    \sa setSizePolicy(), QWidget::sizePolicy()
*/
QSizePolicy QGraphicsLayoutItem::sizePolicy() const
{
    Q_D(const QGraphicsLayoutItem);
    return d->sizePolicy;
}

/*!
    Sets the minimum size to \a size. This property overrides sizeHint() for
    Qt::MinimumSize and ensures that effectiveSizeHint() will never return
    a size smaller than \a size. In order to unset the minimum size, use an
    invalid size.

    \sa minimumSize(), maximumSize(), preferredSize(), Qt::MinimumSize,
    sizeHint(), setMinimumWidth(), setMinimumHeight()
*/
void QGraphicsLayoutItem::setMinimumSize(const QSizeF &size)
{
    Q_D(QGraphicsLayoutItem);
    if (size == d->userSizeHints[Qt::MinimumSize])
        return;

    d->userSizeHints[Qt::MinimumSize] = size;
    updateGeometry();
}

/*!
    \fn QGraphicsLayoutItem::setMinimumSize(qreal w, qreal h)

    This convenience function is equivalent to calling
    setMinimumSize(QSizeF(\a w, \a h)).

    \sa minimumSize(), setMaximumSize(), setPreferredSize(), sizeHint()
*/

/*!
    Returns the minimum size.

    \sa setMinimumSize(), preferredSize(), maximumSize(), Qt::MinimumSize,
    sizeHint()
*/
QSizeF QGraphicsLayoutItem::minimumSize() const
{
    return effectiveSizeHint(Qt::MinimumSize);
}

/*!
    Sets the minimum width to \a width.

    \sa minimumWidth(), setMinimumSize(), minimumSize()
*/
void QGraphicsLayoutItem::setMinimumWidth(qreal width)
{
    Q_D(QGraphicsLayoutItem);
    qreal &userSizeHint = d->userSizeHints[Qt::MinimumSize].rwidth();
    if (width == userSizeHint)
        return;
    userSizeHint = width;
    updateGeometry();
}

/*!
    Sets the minimum height to \a height.

    \sa minimumHeight(), setMinimumSize(), minimumSize()
*/
void QGraphicsLayoutItem::setMinimumHeight(qreal height)
{
    Q_D(QGraphicsLayoutItem);
    qreal &userSizeHint = d->userSizeHints[Qt::MinimumSize].rheight();
    if (height == userSizeHint)
        return;
    userSizeHint = height;
    updateGeometry();
}


/*!
    Sets the preferred size to \a size. This property overrides sizeHint() for
    Qt::PreferredSize and provides the default value for effectiveSizeHint().
    In order to unset the preferred size, use an invalid size.

    \sa preferredSize(), minimumSize(), maximumSize(), Qt::PreferredSize,
    sizeHint()
*/
void QGraphicsLayoutItem::setPreferredSize(const QSizeF &size)
{
    Q_D(QGraphicsLayoutItem);
    if (size == d->userSizeHints[Qt::PreferredSize])
        return;

    d->userSizeHints[Qt::PreferredSize] = size;
    updateGeometry();
}

/*!
    \fn QGraphicsLayoutItem::setPreferredSize(qreal w, qreal h)

    This convenience function is equivalent to calling
    setPreferredSize(QSizeF(\a w, \a h)).

    \sa preferredSize(), setMaximumSize(), setMinimumSize(), sizeHint()
*/

/*!
    Returns the preferred size.

    \sa setPreferredSize(), minimumSize(), maximumSize(), Qt::PreferredSize,
    sizeHint()
*/
QSizeF QGraphicsLayoutItem::preferredSize() const
{
    return effectiveSizeHint(Qt::PreferredSize);
}

/*!
    Sets the preferred height to \a height.

    \sa preferredWidth(), setPreferredSize(), preferredSize()
*/
void QGraphicsLayoutItem::setPreferredHeight(qreal height)
{
    Q_D(QGraphicsLayoutItem);
    qreal &userSizeHint = d->userSizeHints[Qt::PreferredSize].rheight();
    if (height == userSizeHint)
        return;
    userSizeHint = height;
    updateGeometry();
}

/*!
    Sets the preferred width to \a width.

    \sa preferredHeight(), setPreferredSize(), preferredSize()
*/
void QGraphicsLayoutItem::setPreferredWidth(qreal width)
{
    Q_D(QGraphicsLayoutItem);
    qreal &userSizeHint = d->userSizeHints[Qt::PreferredSize].rwidth();
    if (width == userSizeHint)
        return;
    userSizeHint = width;
    updateGeometry();
}

/*!
    Sets the maximum size to \a size. This property overrides sizeHint() for
    Qt::MaximumSize and ensures that effectiveSizeHint() will never return a
    size larger than \a size. In order to unset the maximum size, use an
    invalid size.

    \sa maximumSize(), minimumSize(), preferredSize(), Qt::MaximumSize,
    sizeHint()
*/
void QGraphicsLayoutItem::setMaximumSize(const QSizeF &size)
{
    Q_D(QGraphicsLayoutItem);
    if (size == d->userSizeHints[Qt::MaximumSize])
        return;

    d->userSizeHints[Qt::MaximumSize] = size;
    updateGeometry();
}

/*!
    \fn QGraphicsLayoutItem::setMaximumSize(qreal w, qreal h)

    This convenience function is equivalent to calling
    setMaximumSize(QSizeF(\a w, \a h)).

    \sa maximumSize(), setMinimumSize(), setPreferredSize(), sizeHint()
*/

/*!
    Returns the maximum size.

    \sa setMaximumSize(), minimumSize(), preferredSize(), Qt::MaximumSize,
    sizeHint()
*/
QSizeF QGraphicsLayoutItem::maximumSize() const
{
    return effectiveSizeHint(Qt::MaximumSize);
}

/*!
    Sets the maximum width to \a width.

    \sa maximumWidth(), setMaximumSize(), maximumSize()
*/
void QGraphicsLayoutItem::setMaximumWidth(qreal width)
{
    Q_D(QGraphicsLayoutItem);
    qreal &userSizeHint = d->userSizeHints[Qt::MaximumSize].rwidth();
    if (width == userSizeHint)
        return;
    userSizeHint = width;
    updateGeometry();
}

/*!
    Sets the maximum height to \a height.

    \sa maximumHeight(), setMaximumSize(), maximumSize()
*/
void QGraphicsLayoutItem::setMaximumHeight(qreal height)
{
    Q_D(QGraphicsLayoutItem);
    qreal &userSizeHint = d->userSizeHints[Qt::MaximumSize].rheight();
    if (height == userSizeHint)
        return;
    userSizeHint = height;
    updateGeometry();
}

/*!
    \fn qreal QGraphicsLayoutItem::minimumWidth() const

    Returns the minimum width.

    \sa setMinimumWidth(), setMinimumSize(), minimumSize()
*/

/*!
    \fn qreal QGraphicsLayoutItem::minimumHeight() const

    Returns the minimum height.

    \sa setMinimumHeight(), setMinimumSize(), minimumSize()
*/

/*!
    \fn qreal QGraphicsLayoutItem::preferredWidth() const

    Returns the preferred width.

    \sa setPreferredWidth(), setPreferredSize(), preferredSize()
*/

/*!
    \fn qreal QGraphicsLayoutItem::preferredHeight() const

    Returns the preferred height.

    \sa setPreferredHeight(), setPreferredSize(), preferredSize()
*/

/*!
    \fn qreal QGraphicsLayoutItem::maximumWidth() const

    Returns the maximum width.

    \sa setMaximumWidth(), setMaximumSize(), maximumSize()
*/

/*!
    \fn qreal QGraphicsLayoutItem::maximumHeight() const

    Returns the maximum height.

    \sa setMaximumHeight(), setMaximumSize(), maximumSize()
*/

/*!
    \fn virtual void QGraphicsLayoutItem::setGeometry(const QRectF &rect) = 0

    This pure virtual function sets the geometry of the QGraphicsLayoutItem to
    \a rect, which is in parent coordinates (e.g., the top-left corner of \a rect
    is equivalent to the item's position in parent coordinates).

    Reimplement this function in a subclass of QGraphicsLayoutItem to enable
    your item to receive geometry updates.

    If \a rect is outside of the bounds of minimumSize and maximumSize, it
    will be adjusted to its closest size so that it is within the legal
    bounds.

    \sa geometry()
*/
void QGraphicsLayoutItem::setGeometry(const QRectF &rect)
{
    Q_D(QGraphicsLayoutItem);
    QSizeF effectiveSize = rect.size().expandedTo(effectiveSizeHint(Qt::MinimumSize))
                                .boundedTo(effectiveSizeHint(Qt::MaximumSize));
    d->geom = QRectF(rect.topLeft(), effectiveSize);
}

/*!
    \fn QRectF QGraphicsLayoutItem::geometry() const

    Returns the item's geometry (e.g., position and size) as a
    QRectF. This function is equivalent to QRectF(pos(), size()).

    \sa setGeometry()
*/
QRectF QGraphicsLayoutItem::geometry() const
{
    Q_D(const QGraphicsLayoutItem);
    return d->geom;
}

/*!
    This virtual function provides the \a left, \a top, \a right and \a bottom
    contents margins for this QGraphicsLayoutItem. The default implementation
    assumes all contents margins are 0. The parameters point to values stored
    in qreals. If any of the pointers is 0, that value will not be updated.

    \sa QGraphicsWidget::setContentsMargins()
*/
void QGraphicsLayoutItem::getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const
{
    if (left)
        *left = 0;
    if (top)
        *top = 0;
    if (right)
        *right = 0;
    if (bottom)
        *bottom = 0;
}

/*!
    Returns the contents rect in local coordinates.

    The contents rect defines the subrectangle used by an associated layout
    when arranging subitems. This function is a convenience function that
    adjusts the item's geometry() by its contents margins. Note that
    getContentsMargins() is a virtual function that you can reimplement to
    return the item's contents margins.

    \sa getContentsMargins(), geometry()
*/
QRectF QGraphicsLayoutItem::contentsRect() const
{
    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    return QRectF(QPointF(), geometry().size()).adjusted(+left, +top, -right, -bottom);
}

/*!
    Returns the effective size hint for this QGraphicsLayoutItem.

    \a which is the size hint in question.
    \a constraint is an optional argument that defines a special constrain
    when calculating the effective size hint. By default, \a constraint is
    QSizeF(-1, -1), which means there is no constraint to the size hint.

    If you want to specify the widget's size hint for a given width or height,
    you can provide the fixed dimension in \a constraint. This is useful for
    widgets that can grow only either vertically or horizontally, and need to
    set either their width or their height to a special value.

    For example, a text paragraph item fit into a column width of 200 may
    grow vertically. You can pass QSizeF(200, -1) as a constraint to get a
    suitable minimum, preferred and maximum height).

    You can adjust the effective size hint by reimplementing sizeHint()
    in a QGraphicsLayoutItem subclass, or by calling one of the following
    functions: setMinimumSize(), setPreferredSize, or setMaximumSize()
    (or a combination of both).

    This function caches each of the size hints and guarantees that
    sizeHint() will be called only once for each value of \a which - unless
    \a constraint is not specified and updateGeometry() has been called.

    \sa sizeHint()
*/
QSizeF QGraphicsLayoutItem::effectiveSizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    // ### should respect size policy???
    return d_ptr->effectiveSizeHints(constraint)[which];
}

/*!
    This virtual function discards any cached size hint information. You
    should always call this function if you change the return value of the
    sizeHint() function. Subclasses must always call the base implementation
    when reimplementing this function.

    \sa effectiveSizeHint()
*/
void QGraphicsLayoutItem::updateGeometry()
{ 
    Q_D(QGraphicsLayoutItem);
    d->sizeHintCacheDirty = true;
}

/*!
    Returns the parent of this QGraphicsLayoutItem, or 0 if there is no parent,
    or if the parent does not inherit from QGraphicsLayoutItem
    (QGraphicsLayoutItem is often used through multiple inheritance with
    QObject-derived classes).

    \sa setParentLayoutItem()
*/
QGraphicsLayoutItem *QGraphicsLayoutItem::parentLayoutItem() const
{
    return d_func()->parent;
}

/*!
    Sets the parent of this QGraphicsLayoutItem to \a parent.

    \sa parentLayoutItem()
*/
void QGraphicsLayoutItem::setParentLayoutItem(QGraphicsLayoutItem *parent)
{ 
    d_func()->parent = parent;
}

/*!
    Returns true if this QGraphicsLayoutItem is a layout (e.g., is inherited
    by an object that arranges other QGraphicsLayoutItem objects); otherwise
    returns false.

    \sa QGraphicsLayout
*/
bool QGraphicsLayoutItem::isLayout() const
{
    return d_func()->isLayout;
}

QT_END_NAMESPACE
        
#endif //QT_NO_GRAPHICSVIEW
