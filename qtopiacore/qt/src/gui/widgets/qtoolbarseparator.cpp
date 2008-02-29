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

#include "qtoolbarseparator_p.h"

#ifndef QT_NO_TOOLBAR

#include <qstyle.h>
#include <qstyleoption.h>
#include <qtoolbar.h>
#include <qpainter.h>

static QStyleOption getStyleOption(const QToolBarSeparator *tbs)
{
    QStyleOption opt;
    opt.init(tbs);
    if (tbs->orientation() == Qt::Horizontal)
        opt.state |= QStyle::State_Horizontal;
    return opt;
}

QToolBarSeparator::QToolBarSeparator(QToolBar *parent)
    : QWidget(parent), orient(parent->orientation())
{ setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); }

void QToolBarSeparator::setOrientation(Qt::Orientation orientation)
{
    orient = orientation;
    update();
}

Qt::Orientation QToolBarSeparator::orientation() const
{ return orient; }

QSize QToolBarSeparator::sizeHint() const
{
    QStyleOption opt = getStyleOption(this);
    const int extent = style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, &opt, this);
    return QSize(extent, extent);
}

void QToolBarSeparator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOption opt = getStyleOption(this);
    style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, &p, this);
}

#endif // QT_NO_TOOLBAR
