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

#include "q3groupbox.h"

#include "qlayout.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "q3accel.h"
#include "qradiobutton.h"
#include "qdrawutil.h"
#include "qapplication.h"
#include "qstyle.h"
#include "qcheckbox.h"
#include "qaccessible.h"
#include "qstyleoption.h"
#include "qdebug.h"

/*!
    \class Q3GroupBox
    \brief The Q3GroupBox widget provides a group box frame with a title.

    \compat

    A group box provides a frame, a title and a keyboard shortcut, and
    displays various other widgets inside itself. The title is on top,
    the keyboard shortcut moves keyboard focus to one of the group
    box's child widgets, and the child widgets are usually laid out
    horizontally (or vertically) inside the frame.

    The simplest way to use it is to create a group box with the
    desired number of columns (or rows) and orientation, and then just
    create widgets with the group box as parent.

    It is also possible to change the orientation() and number of
    columns() after construction, or to ignore all the automatic
    layout support and manage the layout yourself. You can add 'empty'
    spaces to the group box with addSpace().

    Q3GroupBox also lets you set the title() (normally set in the
    constructor) and the title's alignment().

    You can change the spacing used by the group box with
    setInsideMargin() and setInsideSpacing(). To minimize space
    consumption, you can remove the right, left and bottom edges of
    the frame with setFlat().

    \sa QButtonGroup
*/

class QCheckBox;

class Q3GroupBoxPrivate
{
public:
    Q3GroupBoxPrivate():
        vbox(0), grid(0), row(0), col(0), nRows(0), nCols(0), dir(Qt::Horizontal),
        spac(5), marg(11),
        checkbox(0) {}

    QVBoxLayout *vbox;
    QGridLayout *grid;
    int row;
    int col;
    int nRows, nCols;
    Qt::Orientation dir;
    int spac, marg;

    QCheckBox *checkbox;
};




/*!
    Constructs a group box widget with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.

    This constructor does not do automatic layout.
*/

Q3GroupBox::Q3GroupBox(QWidget *parent, const char *name)
    : QGroupBox(parent, name)
{
    init();
}

/*!
    Constructs a group box with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.

    This constructor does not do automatic layout.
*/

Q3GroupBox::Q3GroupBox(const QString &title, QWidget *parent, const char *name)
    : QGroupBox(parent, name)
{
    init();
    setTitle(title);
}

/*!
    Constructs a group box with no title. Child widgets will be
    arranged in \a strips rows or columns (depending on \a
    orientation).

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3GroupBox::Q3GroupBox(int strips, Qt::Orientation orientation,
                    QWidget *parent, const char *name)
    : QGroupBox(parent, name)
{
    init();
    setColumnLayout(strips, orientation);
}

/*!
    Constructs a group box titled \a title. Child widgets will be
    arranged in \a strips rows or columns (depending on \a
    orientation).

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3GroupBox::Q3GroupBox(int strips, Qt::Orientation orientation,
                    const QString &title, QWidget *parent,
                    const char *name)
    : QGroupBox(parent, name)
{
    init();
    setTitle(title);
    setColumnLayout(strips, orientation);
}

/*!
    Destroys the group box.
*/
Q3GroupBox::~Q3GroupBox()
{
    delete d;
}

void Q3GroupBox::init()
{
    d = new Q3GroupBoxPrivate();
}


/*! \reimp
*/
void Q3GroupBox::resizeEvent(QResizeEvent *e)
{
    QGroupBox::resizeEvent(e);
}


/*!
    Adds an empty cell at the next free position. If \a size is
    greater than 0, the empty cell takes \a size to be its fixed width
    (if orientation() is \c Horizontal) or height (if orientation() is
    \c Vertical).

    Use this method to separate the widgets in the group box or to
    skip the next free cell. For performance reasons, call this method
    after calling setColumnLayout() or by changing the \l
    Q3GroupBox::columns or \l Q3GroupBox::orientation properties. It is
    generally a good idea to call these methods first (if needed at
    all), and insert the widgets and spaces afterwards.
*/
void Q3GroupBox::addSpace(int size)
{
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    if (d->nCols <= 0 || d->nRows <= 0)
        return;

    if (d->row >= d->nRows || d->col >= d->nCols)
        d->grid->expand(d->row+1, d->col+1);

    if (size > 0) {
        QSpacerItem *spacer
            = new QSpacerItem((d->dir == Qt::Horizontal) ? 0 : size,
                              (d->dir == Qt::Vertical) ? 0 : size,
                              QSizePolicy::Fixed, QSizePolicy::Fixed);
        d->grid->addItem(spacer, d->row, d->col);
    }

    skip();
}

/*!
    \property Q3GroupBox::columns
    \brief the number of columns or rows (depending on \l Q3GroupBox::orientation) in the group box

    Usually it is not a good idea to set this property because it is
    slow (it does a complete layout). It is best to set the number
    of columns directly in the constructor.
*/
int Q3GroupBox::columns() const
{
    if (d->dir == Qt::Horizontal)
        return d->nCols;
    return d->nRows;
}

void Q3GroupBox::setColumns(int c)
{
    setColumnLayout(c, d->dir);
}

/*!
    Returns the width of the empty space between the items in the
    group and the frame of the group.

    Only applies if the group box has a defined orientation.

    The default is usually 11, by may vary depending on the platform
    and style.

    \sa setInsideMargin(), orientation
*/
int Q3GroupBox::insideMargin() const
{
    return d->marg;
}

/*!
    Returns the width of the empty space between each of the items
    in the group.

    Only applies if the group box has a defined orientation.

    The default is usually 5, by may vary depending on the platform
    and style.

    \sa setInsideSpacing(), orientation
*/
int Q3GroupBox::insideSpacing() const
{
    return d->spac;
}

/*!
    Sets the the width of the inside margin to \a m pixels.

    \sa insideMargin()
*/
void Q3GroupBox::setInsideMargin(int m)
{
    d->marg = m;
    setColumnLayout(columns(), d->dir);
}

/*!
    Sets the width of the empty space between each of the items in
    the group to \a s pixels.

    \sa insideSpacing()
*/
void Q3GroupBox::setInsideSpacing(int s)
{
    d->spac = s;
    setColumnLayout(columns(), d->dir);
}

/*!
    \property Q3GroupBox::orientation
    \brief the group box's orientation

    A horizontal group box arranges its children in columns, while a
    vertical group box arranges them in rows.

    Usually it is not a good idea to set this property because it is
    slow (it does a complete layout). It is better to set the
    orientation directly in the constructor.
*/
void Q3GroupBox::setOrientation(Qt::Orientation o)
{
    setColumnLayout(columns(), o);
}


Qt::Orientation Q3GroupBox::orientation() const
{
    return d->dir;
}

/*!
    Changes the layout of the group box. This function is only useful
    in combination with the default constructor that does not take any
    layout information. This function will put all existing children
    in the new layout. It is not good Qt programming style to call
    this function after children have been inserted. Sets the number
    of columns or rows to be \a strips, depending on \a direction.

    \sa orientation columns
*/
void Q3GroupBox::setColumnLayout(int strips, Qt::Orientation direction)
{
    if (layout())
        delete layout();

    d->vbox = 0;
    d->grid = 0;

    if (strips < 0) // if 0, we create the d->vbox but not the d->grid. See below.
        return;

    d->vbox = new QVBoxLayout(this, d->marg, 0);

    d->nCols = 0;
    d->nRows = 0;
    d->dir = direction;

    // Send all child events and ignore them. Otherwise we will end up
    // with doubled insertion. This won't do anything because d->nCols ==
    // d->nRows == 0.
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);

    // if 0 or smaller , create a vbox-layout but no grid. This allows
    // the designer to handle its own grid layout in a group box.
    if (strips <= 0)
        return;

    d->dir = direction;
    if (d->dir == Qt::Horizontal) {
        d->nCols = strips;
        d->nRows = 1;
    } else {
        d->nCols = 1;
        d->nRows = strips;
    }
    d->grid = new QGridLayout(d->nRows, d->nCols, d->spac);
    d->row = d->col = 0;
    d->grid->setAlignment(Qt::AlignTop);
    d->vbox->addLayout(d->grid);

    // Add all children
    QObjectList childList = children();
    if (!childList.isEmpty()) {
        for (int i = 0; i < childList.size(); ++i) {
            QObject *o = childList.at(i);
            if (o->isWidgetType() && o != d->checkbox)
                insertWid(static_cast<QWidget *>(o));
        }
    }
}

/*!\reimp */
void Q3GroupBox::childEvent(QChildEvent *c)
{
    QGroupBox::childEvent(c);
    if (!c->inserted() || !c->child()->isWidgetType())
        return;
    if (d->grid) {
        insertWid((QWidget*)c->child());
    }
}

void Q3GroupBox::insertWid(QWidget* w)
{
    if (d->row >= d->nRows || d->col >= d->nCols)
        d->grid->expand(d->row+1, d->col+1);
    d->grid->addWidget(w, d->row, d->col);
    skip();
}


void Q3GroupBox::skip()
{
    // Same as QGrid::skip()
    if (d->dir == Qt::Horizontal) {
        if (d->col+1 < d->nCols) {
            d->col++;
        } else {
            d->col = 0;
            d->row++;
        }
    } else { //Vertical
        if (d->row+1 < d->nRows) {
            d->row++;
        } else {
            d->row = 0;
            d->col++;
        }
    }
}


/*! \reimp */
void Q3GroupBox::changeEvent(QEvent *ev)
{
    QGroupBox::changeEvent(ev);
}


/*!
    \fn void Q3GroupBox::setFrameRect(QRect)
    \internal
*/

/*!
    \fn QRect Q3GroupBox::frameRect() const
    \internal
*/
/*!
    \enum Q3GroupBox::DummyFrame
    \internal

    \value Box
    \value Sunken
    \value Plain
    \value Raised
    \value MShadow
    \value NoFrame
    \value Panel 
    \value StyledPanel
    \value HLine 
    \value VLine 
    \value GroupBoxPanel
    \value WinPanel 
    \value ToolBarPanel 
    \value MenuBarPanel 
    \value PopupPanel 
    \value LineEditPanel 
    \value TabWidgetPanel 
    \value MShape
*/

/*!
    \fn void Q3GroupBox::setFrameShadow(DummyFrame)
    \internal
*/

/*!
    \fn DummyFrame Q3GroupBox::frameShadow() const
    \internal
*/

/*!
    \fn void Q3GroupBox::setFrameShape(DummyFrame)
    \internal
*/

/*!
    \fn DummyFrame Q3GroupBox::frameShape() const
    \internal
*/

/*!
    \fn void Q3GroupBox::setFrameStyle(int)
    \internal
*/

/*!
    \fn int Q3GroupBox::frameStyle() const
    \internal
*/

/*!
    \fn int Q3GroupBox::frameWidth() const
    \internal
*/

/*!
    \fn void Q3GroupBox::setLineWidth(int)
    \internal
*/

/*!
    \fn int Q3GroupBox::lineWidth() const
    \internal
*/

/*!
    \fn void Q3GroupBox::setMargin(int margin)
    \since 4.2

    Sets the width of the margin around the contents of the widget to \a margin.
    
    This function uses QWidget::setContentsMargins() to set the margin.
    \sa margin(), QWidget::setContentsMargins()
*/

/*!
    \fn int Q3GroupBox::margin() const 
    \since 4.2

    Returns the with of the the margin around the contents of the widget.
    
    This function uses QWidget::getContentsMargins() to get the margin.
    \sa setMargin(), QWidget::getContentsMargins()
*/

/*!
    \fn void Q3GroupBox::setMidLineWidth(int)
    \internal
*/

/*!
    \fn int Q3GroupBox::midLineWidth() const
    \internal
*/
