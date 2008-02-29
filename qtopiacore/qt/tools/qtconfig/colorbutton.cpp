/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "colorbutton.h"

#include <QApplication>
#include <QtEvents>
#include <QColorDialog>
#include <QPainter>
#include <QMimeData>
#include <QStyle>
#include <QStyleOption>

ColorButton::ColorButton(QWidget *parent)
    : QAbstractButton(parent), mousepressed(false)
{
    setAcceptDrops(true);
    col = Qt::black;
    connect(this, SIGNAL(clicked()), SLOT(changeColor()));
}


ColorButton::ColorButton(const QColor &c, QWidget *parent)
    : QAbstractButton(parent)
{
    setAcceptDrops(true);
    col = c;
    connect(this, SIGNAL(clicked()), SLOT(changeColor()));
}


void ColorButton::setColor(const QColor &c)
{
    col = c;
    update();
}


void ColorButton::changeColor()
{
    QColor c = QColorDialog::getColor(col, qApp->activeWindow());

    if (c.isValid()) {
        setColor(c);
        emit colorChanged(color());
    }
}


QSize ColorButton::sizeHint() const
{
    return QSize(40, 25);
}


QSize ColorButton::minimumSizeHint() const
{
    return QSize(40, 25);
}


void ColorButton::drawButton(QPainter *p)
{
    QStyleOptionButton buttonOptions;
    buttonOptions.init(this);
    buttonOptions.features = QStyleOptionButton::None;
    buttonOptions.rect = rect();
    buttonOptions.palette = palette();
    buttonOptions.state = (isDown() ? QStyle::State_Sunken : QStyle::State_Raised);
    style()->drawPrimitive(QStyle::PE_PanelButtonBevel, &buttonOptions, p, this);

    p->save();
    drawButtonLabel(p);
    p->restore();

    QStyleOptionFocusRect frectOptions;
    frectOptions.init(this);
    frectOptions.rect = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &buttonOptions, this);
    if (hasFocus())
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &frectOptions, p, this);
}


void ColorButton::drawButtonLabel(QPainter *p)
{
    QPalette::ColorGroup cg =
        (isEnabled() ? (hasFocus() ? QPalette::Active : QPalette::Inactive) : QPalette::Disabled);

    p->setPen(palette().color(cg, QPalette::ButtonText));
    p->setBrush(col);
    p->drawRect(width() / 4, height() / 4, width() / 2 - 1, height() / 2 - 1);
}


void ColorButton::dragEnterEvent(QDragEnterEvent *e)
{
    if (!e->mimeData()->hasColor()) {
        e->ignore();
        return;
    }
}


void ColorButton::dragMoveEvent(QDragMoveEvent *e)
{
    if (!e->mimeData()->hasColor()) {
        e->ignore();
        return;
    }

    e->accept();
}


void ColorButton::dropEvent(QDropEvent *e)
{
    if (!e->mimeData()->hasColor()) {
        e->ignore();
        return;
    }

    QColor c = qvariant_cast<QColor>(e->mimeData()->colorData());
    setColor(c);
    emit colorChanged(color());
}


void ColorButton::mousePressEvent(QMouseEvent *e)
{
    presspos = e->pos();
    mousepressed = true;
    QAbstractButton::mousePressEvent(e);
}


void ColorButton::mouseReleaseEvent(QMouseEvent *e)
{
    mousepressed = false;
    QAbstractButton::mouseReleaseEvent(e);
}


void ColorButton::mouseMoveEvent(QMouseEvent *e)
{
    if (! mousepressed)
        return;

    if ((presspos - e->pos()).manhattanLength() > QApplication::startDragDistance()) {
        mousepressed = false;
        setDown(false);

        QDrag *drag = new QDrag(this);
        QMimeData *data = new QMimeData;
        data->setColorData(color());
        drag->setMimeData(data);
        drag->start(Qt::CopyAction);
    }
}

void ColorButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawButton(&p);
}
