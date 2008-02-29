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

#include "qtcolorbutton.h"
#include <QColorDialog>
#include <QPainter>

using namespace qdesigner_internal;

namespace qdesigner_internal {

class QtColorButtonPrivate
{
    QtColorButton *q_ptr;
    Q_DECLARE_PUBLIC(QtColorButton)
public:
    QColor m_color;
    bool m_backgroundTransparent;

    void slotEditColor();
};

}

void QtColorButtonPrivate::slotEditColor()
{
    bool ok;
    QRgb rgba = QColorDialog::getRgba(m_color.rgba(), &ok, q_ptr);
    if (ok == false)
        return;
    QColor c;
    c.setRgba(rgba);
    q_ptr->setColor(c);
    emit q_ptr->colorChanged(m_color);
}

///////////////

QtColorButton::QtColorButton(QWidget *parent)
    : QToolButton(parent)
{
    d_ptr = new QtColorButtonPrivate;
    d_ptr->q_ptr = this;
    d_ptr->m_backgroundTransparent = true;

    connect(this, SIGNAL(clicked()), this, SLOT(slotEditColor()));
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
}

QtColorButton::~QtColorButton()
{
    delete d_ptr;
}

void QtColorButton::setColor(const QColor &color)
{
    if (d_ptr->m_color == color)
        return;
    d_ptr->m_color = color;
    update();
}

QColor QtColorButton::color() const
{
    return d_ptr->m_color;
}

void QtColorButton::setBackgroundTransparent(bool transparent)
{
    if (d_ptr->m_backgroundTransparent == transparent)
        return;
    d_ptr->m_backgroundTransparent = transparent;
    update();
}

bool QtColorButton::backgroundTransparent() const
{
    return d_ptr->m_backgroundTransparent;
}

void QtColorButton::paintEvent(QPaintEvent *e)
{
    QToolButton::paintEvent(e);
    if (!isEnabled())
        return;

    int pixSize = 20;
    QBrush br(d_ptr->m_color);
    if (!d_ptr->m_backgroundTransparent) {
        QPixmap pm(2 * pixSize, 2 * pixSize);
        QPainter pmp(&pm);
        pmp.fillRect(0, 0, pixSize, pixSize, Qt::lightGray);
        pmp.fillRect(pixSize, pixSize, pixSize, pixSize, Qt::lightGray);
        pmp.fillRect(0, pixSize, pixSize, pixSize, Qt::darkGray);
        pmp.fillRect(pixSize, 0, pixSize, pixSize, Qt::darkGray);
        pmp.fillRect(0, 0, 2 * pixSize, 2 * pixSize, d_ptr->m_color);
        br = QBrush(pm);
    }

    QPainter p(this);
    int corr = 2;
    QRect r = rect().adjusted(corr, corr, -corr, -corr);
    p.setBrushOrigin((r.width() % pixSize + pixSize) / 2 + corr, (r.height() % pixSize + pixSize) / 2 + corr);
    p.fillRect(r, br);
}

#include "moc_qtcolorbutton.cpp"
