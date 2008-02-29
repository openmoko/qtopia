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

#include "qdesigner_widget_p.h"
#include "qdesigner_command_p.h"
#include "layout_p.h"
#include "invisible_widget_p.h"

#include <QtDesigner/QtDesigner>
#include <QtDesigner/QExtensionManager>

#include <QtGui/QBitmap>
#include <QtGui/QToolButton>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>
#include <QtGui/qevent.h>

#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

static void paintGrid(QWidget *widget, QDesignerFormWindowInterface *formWindow, QPaintEvent *e, bool needFrame = false)
{
    QPainter p(widget);

    p.fillRect(e->rect(), widget->palette().brush(widget->backgroundRole()));

    p.setPen(widget->palette().dark().color());
    QPoint grid = formWindow->grid();

    int xstart = e->rect().x();
    int ystart = e->rect().y();
    xstart = (xstart/grid.x())*grid.x();
    ystart = (ystart/grid.y())*grid.y();

    int xend = e->rect().right();
    xend = (xend/grid.x())*grid.x();

    int yend = e->rect().bottom();
    yend = (yend/grid.x())*grid.y();

    int pointCount = ((xend - xstart) / grid.x()) *((yend - ystart) * grid.y());


    static const int BUF_SIZE = 4096;
    QPoint points[BUF_SIZE];

    int i = 0;
    int x = xstart;
    int y = ystart;
    while (pointCount > 0) {
        while (i < pointCount && i < BUF_SIZE) {
            points[i] = QPoint(x, y);
            ++i;
            x += formWindow->grid().x();
            if (x > xend) {
                x = xstart;
                y += formWindow->grid().y();
                if (y > yend) // probably never reached..
                    break;
            }
        }
        p.drawPoints(points, i);
        pointCount -= i;
        i = 0;
    }
    if (needFrame) {
        p.setPen(widget->palette().dark().color());
        p.drawRect(e->rect());
    }
}

void QDesignerDialog::paintEvent(QPaintEvent *e)
{
    if (m_formWindow && m_formWindow->currentTool() == 0 && m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature)) {
        paintGrid(this, m_formWindow, e);
    } else {
        QPainter p(this);
        p.fillRect(e->rect(), palette().brush(QPalette::Window));
    }
}

void QDesignerLabel::updateBuddy()
{
    if (myBuddy.isEmpty()) {
        QLabel::setBuddy(0);
        return;
    }

    QList<QWidget *> widgets = qFindChildren<QWidget*>(topLevelWidget(), QString::fromUtf8(myBuddy));
    QListIterator<QWidget *> it(widgets);
    while (it.hasNext()) {
        QWidget *widget = it.next();
        if (widget && !widget->isHidden()) {
            QLabel::setBuddy(widget);
            return;
        }
    }
    QLabel::setBuddy(0);
}

QDesignerWidget::QDesignerWidget(QDesignerFormWindowInterface* formWindow, QWidget *parent)
    : QWidget(parent), m_formWindow(formWindow)
{
    need_frame = true;
    setBackgroundRole(QPalette::Window);
}

QDesignerWidget::~QDesignerWidget()
{
}

void QDesignerWidget::paintEvent(QPaintEvent *e)
{
    if (m_formWindow && m_formWindow->currentTool() == 0 && m_formWindow->hasFeature(QDesignerFormWindowInterface::GridFeature))
        paintGrid(this, m_formWindow, e);
    else
        QWidget::paintEvent(e);
}

void QDesignerWidget::dragEnterEvent(QDragEnterEvent *)
{
//    e->setAccepted(QTextDrag::canDecode(e));
}

QDesignerLabel::QDesignerLabel(QWidget *parent)
    : QLabel(parent)
{
}

void QDesignerLabel::setBuddy(QWidget *widget)
{
    QLabel::setBuddy(widget);
}
