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

#include "private/qtoolbarhandle_p.h"

#ifndef QT_NO_TOOLBAR

#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbar.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qevent.h>

#include <private/qmainwindowlayout_p.h>

static QStyleOption getStyleOption(const QToolBarHandle *tbh)
{
    QStyleOption opt;
    opt.init(tbh);
    if (tbh->orientation() == Qt::Horizontal)
	opt.state |= QStyle::State_Horizontal;
    return opt;
}

QToolBarHandle::QToolBarHandle(QToolBar *parent)
    : QWidget(parent), state(0)
{
    setOrientation(parent->orientation());
#ifndef QT_NO_CURSOR
    setCursor(Qt::SizeAllCursor);
#endif
}

void QToolBarHandle::setOrientation(Qt::Orientation orientation)
{
    orient = orientation;

    if (orientation == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    } else {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    }

    // if we're dragging - swap the offset coords around as well
    if (state) {
	QPoint p = state->offset;
	state->offset = QPoint(p.y(), p.x());
    }

    update();
}

Qt::Orientation QToolBarHandle::orientation() const
{ return orient; }

QSize QToolBarHandle::sizeHint() const
{
    QStyleOption opt = getStyleOption(this);
    const int extent = style()->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, this);
    return QSize(extent, extent);
}

void QToolBarHandle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt = getStyleOption(this);
    style()->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
}

void QToolBarHandle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QToolBar *toolBar = qobject_cast<QToolBar *>(parentWidget());
    Q_ASSERT_X(toolBar != 0, "QToolBar", "internal error");
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(toolBar->parentWidget());
    Q_ASSERT_X(mainWindow != 0, "QMainWindow", "internal error");
    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout *>(mainWindow->layout());
    Q_ASSERT_X(layout != 0, "QMainWindow", "internal error");

    // cannot drag if the toolbar has not been added to the mainwindow
    QLayoutItem *item;
    bool found = false;
    int index = 0;
    while (!found && (item = layout->itemAt(index++)) != 0)
        found = item->widget() == toolBar;
    if (!found)
        return;

    if (state != 0)
        delete state;

    state = new DragState;
    state->offset = mapTo(parentWidget(), event->pos());
    if (orientation() == Qt::Horizontal) {
	state->offset = QStyle::visualPos(QApplication::layoutDirection(),
					  parentWidget()->rect(), state->offset);
    }
}

void QToolBarHandle::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;
    delete state;
    state = 0;
}

void QToolBarHandle::mouseMoveEvent(QMouseEvent *event)
{
    if (!state)
        return;

    QToolBar *toolBar = qobject_cast<QToolBar *>(parentWidget());
    Q_ASSERT_X(toolBar != 0, "QToolBar", "internal error");
    QMainWindow *mainWindow = qobject_cast<QMainWindow *>(toolBar->parentWidget());
    Q_ASSERT_X(mainWindow != 0, "QMainWindow", "internal error");
    QMainWindowLayout *layout = qobject_cast<QMainWindowLayout *>(mainWindow->layout());
    Q_ASSERT_X(layout != 0, "QMainWindow", "internal error");

    QPoint p = toolBar->mapFromGlobal(event->globalPos());
    if (orientation() == Qt::Horizontal)
	p = QStyle::visualPos(QApplication::layoutDirection(), toolBar->rect(), p);
    p -= state->offset;

    // offset is measured from the widget origin
    if (orientation() == Qt::Vertical)
        p.setX(state->offset.x() + p.x());
    else
        p.setY(state->offset.y() + p.y());

    // re-position toolbar
    Qt::ToolBarArea oldArea = layout->toolBarArea(toolBar);
    bool toolBarPositionSwapped = layout->dropToolBar(toolBar, event->globalPos(), p);
    Qt::ToolBarArea newArea = layout->toolBarArea(toolBar);

    // ensure modified toolbar areas are repainted
    if (toolBarPositionSwapped) {
        layout->updateToolbarsInArea(oldArea);
        if (newArea != oldArea)
            layout->updateToolbarsInArea(newArea);
    }
}

#endif // QT_NO_TOOLBAR
