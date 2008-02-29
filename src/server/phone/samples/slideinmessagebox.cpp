/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QWidget>
#include <QApplication>
#include <QMessageBox>
#include <QPaintEvent>
#include <QSoftMenuBar>
#include <QPainter>
#include <QDesktopWidget>
#include <QPainterPath>
#include <QTextDocument>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPalette>
#include <QKeyEvent>
#include <QTimeLine>
#include "slideinmessagebox.h"
#include "qtopiaserverapplication.h"

// declare SlideInMessageBoxPrivate
class SlideInMessageBoxPrivate
{
public:
    SlideInMessageBoxPrivate();
    QString m_title;
    QPixmap m_icon;
    QTimeLine m_timeline;
    QColor fillColor;
    QPixmap topLeft;
    QPixmap topStretch;
    QPixmap topRight;
    QPixmap rightStretch;
    QPixmap leftStretch;
    QPixmap titleLeft;
    QPixmap titleRight;
    QPixmap titleStretch;
    qreal val;
    SlideInMessageBox::Icon m_iconEnum;
    SlideInMessageBox::Button m_button1;
    SlideInMessageBox::Button m_button2;
    SlideInMessageBox::Button m_button3;

    int defaultBtnNum;
    int escapeBtnNum;
    bool customButton;

    bool renderStale;
    QPixmap renderedBox;
    QString m_text;
    int yesKey;
};

// define SlideInMessageBoxPrivate
SlideInMessageBoxPrivate::SlideInMessageBoxPrivate()
: topLeft(":image/samples/groupframe_topleft.png"),
  topStretch(":image/samples/groupframe_top_stretch.png"),
  topRight(":image/samples/groupframe_topright.png"),
  rightStretch(":image/samples/groupframe_right_stretch.png"),
  leftStretch(":image/samples/groupframe_left_stretch.png"),
  titleLeft(":image/samples/title_cap_left.png"),
  titleRight(":image/samples/title_cap_right.png"),
  titleStretch(":image/samples/title_stretch.png"),
  val(0),
  m_iconEnum(SlideInMessageBox::NoIcon),
  m_button1(SlideInMessageBox::NoButton),
  m_button2(SlideInMessageBox::NoButton),
  renderStale(false),
  yesKey(0)
{
}

// define SlideInMessageBox
SlideInMessageBox::SlideInMessageBox(QWidget *parent, Qt::WFlags)
: QAbstractMessageBox(parent, Qt::FramelessWindowHint),
  d(new SlideInMessageBoxPrivate)
{
    QPalette mpal = palette();
    mpal.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
    setPalette(mpal);
    d->fillColor = QColor(50, 50, 50, 200);

    d->m_timeline.setDuration(200);
    d->m_timeline.setCurveShape(QTimeLine::LinearCurve);
    QObject::connect(&d->m_timeline, SIGNAL(valueChanged(qreal)), this, SLOT(valueChanged(qreal)));

    setFixedSize(QApplication::desktop()->availableGeometry().width(),
                 QApplication::desktop()->availableGeometry().height());
}

SlideInMessageBox::~SlideInMessageBox()
{
    delete d;
}

void SlideInMessageBox::renderBox()
{
    if(!d->renderStale)
        return;

    QTextDocument text;
    text.setHtml(d->m_text);
    text.setTextWidth(width() - d->rightStretch.width() - d->leftStretch.width());
    text.adjustSize();

    int boxHeight = 0;
    boxHeight = (int)text.size().height() + d->m_icon.height() + d->topStretch.height();

    QPixmap pix(width(), boxHeight);
    pix.fill(QColor(0,0,0,0));
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    int frameOffset = 0;
    if(d->m_icon.height() > d->titleStretch.height())
        frameOffset = (d->m_icon.height() - d->titleStretch.height()) / 2;

    drawFrame(&painter, QRect(0, frameOffset, width(),
                              boxHeight - frameOffset), d->m_title);

    if(!d->m_icon.isNull()) {
        int midpoint = frameOffset + d->titleStretch.height() / 2;
        painter.drawPixmap(10, midpoint - d->m_icon.height() / 2, d->m_icon);
    }

    int textHeight = qMax(d->m_icon.height(), d->titleStretch.height());
    painter.translate(d->leftStretch.width(), textHeight);
    text.drawContents(&painter);

    painter.end();

    d->renderStale = false;
    d->renderedBox = pix;
}

QString SlideInMessageBox::text() const
{
    return d->m_text;
}

void SlideInMessageBox::setText(const QString &text)
{
    d->m_text = text;
    d->renderStale = true;
    update();
}

QPixmap SlideInMessageBox::pixmap() const
{
    return d->m_icon;
}

void SlideInMessageBox::setPixmap(const QPixmap &icon)
{
    d->m_icon = icon;
    d->m_iconEnum = NoIcon;
    d->renderStale = true;
    update();
}

QString SlideInMessageBox::title() const
{
    return d->m_title;
}

void SlideInMessageBox::setTitle(const QString &title)
{
    d->m_title = title;
    d->renderStale = true;
    update();
}

void SlideInMessageBox::animate()
{
    d->m_timeline.stop();
    d->m_timeline.setCurrentTime(0);
    d->m_timeline.start();
}

void SlideInMessageBox::valueChanged(qreal val)
{
    d->val = val;
    update();
}

SlideInMessageBox::Icon SlideInMessageBox::icon() const
{
    return d->m_iconEnum;
}

void SlideInMessageBox::setIcon(SlideInMessageBox::Icon icon)
{
    switch(icon) {
        case NoIcon:
            setPixmap(QPixmap());
            break;
        case Information:
            setPixmap(QPixmap(":image/alert_info"));
            break;
        case Warning:
            setPixmap(QPixmap(":image/alert_warning"));
            break;
        case Question:
        case Critical:
            setPixmap(QMessageBox::standardIcon((QMessageBox::Icon)icon));
            break;
    }

    d->m_iconEnum = icon;
}

void SlideInMessageBox::setButtons(Button btn0, Button btn1)
{
    d->m_button1 = btn0;
    d->m_button2 = btn1;

    if(!d->yesKey) {
        const QList<int> &cbtns = QSoftMenuBar::keys();
        if (cbtns.count()) {
            if (cbtns[0] != Qt::Key_Back)
                d->yesKey = cbtns[0];
            else if (cbtns.count() > 1)
                d->yesKey = cbtns[cbtns.count()-1];
        }
    }

    if(btn1 != NoButton) {
        if(btn0 == Yes || btn0 == Ok) {
            if (d->yesKey)
                QSoftMenuBar::setLabel(this, d->yesKey, "", btn0 == Yes ? tr("Yes") : tr("OK"));
        }
        if (btn1 == No || btn1 == Cancel) {
            QSoftMenuBar::setLabel(this, Qt::Key_Back, "", btn1 == No ? tr("No") : tr("Cancel"));
        }
    }
}
void SlideInMessageBox::setButtons(const QString &button0Text, const QString &button1Text, const QString &button2Text,
        int defaultButtonNumber, int escapeButtonNumber)
{
    d->customButton = true;

    d->m_button1 = button0Text.isEmpty() ? (Button)-2 : (Button)0;
    d->m_button2 = button1Text.isEmpty() ? (Button)-2 : (Button)1;
    d->m_button3 = button2Text.isEmpty() ? (Button)-2 : (Button)2;
    d->defaultBtnNum = defaultButtonNumber;
    d->escapeBtnNum = escapeButtonNumber;

    if (button1Text.isEmpty() && button2Text.isEmpty()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "", button0Text.isEmpty() ? QString() : button0Text);
        return;
    }

    if (Qtopia::hasKey(Qt::Key_Context1))
        QSoftMenuBar::setLabel(this, Qt::Key_Context1, "", button0Text.isEmpty() ? QString() : button0Text);
    else if (Qtopia::hasKey(Qt::Key_Menu))
        QSoftMenuBar::setLabel(this, Qt::Key_Menu, "", button0Text.isEmpty() ? QString() : button0Text);
    else
        qLog(UI) << "Cannot set context label" << button0Text;

    if (!button1Text.isEmpty() && button2Text.isEmpty()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "", button1Text);
    } else if (button1Text.isEmpty() && !button2Text.isEmpty()) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "", button2Text);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Select, "", button1Text);
        QSoftMenuBar::setLabel(this, Qt::Key_Back, "", button2Text);
    }
}


void SlideInMessageBox::drawFrame(QPainter *painter, const QRect &r, const QString &title)
{
    int radius = 14;
    int radius2 = radius*2;
    QPainterPath clipPath;
    clipPath.moveTo(radius, 0);
    clipPath.arcTo(r.right() - radius2, 0, radius2, radius2, 90, -90);
    clipPath.lineTo(width(), height());
    clipPath.lineTo(0, height());
    clipPath.arcTo(r.left(), r.top(), radius2, radius2, 180, -90);
    painter->setClipPath(clipPath);

    QLinearGradient lg(0, 0, 0, r.height());
    lg.setColorAt(0, QColor(224,224,224));
    lg.setColorAt(1, QColor(255,255,255));
    painter->setPen(Qt::NoPen);
    painter->setBrush(lg);
    painter->drawRect(r.adjusted(0, d->titleStretch.height()/2, 0, 0));
    painter->setClipping(false);

    int topFrameOffset = d->titleStretch.height()/2 - 2;
    painter->drawPixmap(r.topLeft() + QPoint(0, topFrameOffset), d->topLeft);
    painter->drawPixmap(r.topRight() - QPoint(d->topRight.width()-1, 0)
            + QPoint(0, topFrameOffset), d->topRight);

    QRect left = r;
    left.setY(r.y() + d->topLeft.height() + topFrameOffset);
    left.setWidth(d->leftStretch.width());
    left.setHeight(r.height() - d->topLeft.height() - topFrameOffset);
    painter->drawTiledPixmap(left, d->leftStretch);

    QRect top = r;
    top.setX(r.x() + d->topLeft.width());
    top.setY(r.y() + topFrameOffset);
    top.setWidth(r.width() - d->topLeft.width() - d->topRight.width());
    top.setHeight(d->topLeft.height());
    painter->drawTiledPixmap(top, d->topStretch);

    QRect right = r;
    right.setX(r.right() - d->rightStretch.width()+1);
    right.setY(r.y() + d->topRight.height() + topFrameOffset);
    right.setWidth(d->rightStretch.width());
    right.setHeight(r.height() - d->topRight.height()
            - topFrameOffset);
    painter->drawTiledPixmap(right, d->rightStretch);

    // Now draw title
    if(!d->m_title.isEmpty()) {
        int txt_width = fontMetrics().width(title + 20);
        painter->drawPixmap(r.center().x() - txt_width/2, r.top(), d->titleLeft);
        QRect tileRect(r.center().x() - txt_width/2 + d->titleLeft.width(),
                r.top(),
                txt_width,
                d->titleStretch.height());
        painter->drawTiledPixmap(tileRect, d->titleStretch);
        painter->drawPixmap(r.center().x() - txt_width/2 + d->titleLeft.width() +
                txt_width,
                r.top(),
                d->titleRight);

        int opacity = 31;
        painter->setPen(QColor(0, 0, 0, opacity));
        painter->drawText(tileRect.translated(0, 1),
                Qt::AlignVCenter | Qt::AlignHCenter, title);
        painter->drawText(tileRect.translated(2, 1),
                Qt::AlignVCenter | Qt::AlignHCenter, title);
        painter->setPen(QColor(0, 0, 0, opacity * 2));
        painter->drawText(tileRect.translated(1, 1),
                Qt::AlignVCenter | Qt::AlignHCenter, title);
        painter->setPen(Qt::white);
        painter->drawText(tileRect, Qt::AlignVCenter | Qt::AlignHCenter,
                title);
    }
}

void SlideInMessageBox::paintEvent(QPaintEvent *)
{
    renderBox();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    d->fillColor.setAlpha((int)(200.0 * d->val));

    painter.fillRect(0, 0, width(), height(), d->fillColor);

    int ypos = (int)(height() - d->val * d->renderedBox.height());
    painter.drawPixmap(0, ypos, d->renderedBox);
}

void SlideInMessageBox::showEvent(QShowEvent *e)
{
    animate();
    QWidget::showEvent(e);
}

void SlideInMessageBox::keyPressEvent(QKeyEvent *ke)
{
    if (d->customButton) {
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
            done(d->defaultBtnNum);
            ke->accept();
        } else if (ke->key() == Qt::Key_Escape) {
            done(d->escapeBtnNum);
            ke->accept();
        } else if (ke->key() == Qt::Key_Context1) {
            if (d->m_button1 != (Button)-2 && (d->m_button2 != (Button)-2 || d->m_button3 != (Button)-2))
                done(d->m_button1);
            ke->accept();
        } else if (ke->key() == Qt::Key_Select) {
            if (d->m_button1 != (Button)-2 && d->m_button2 != (Button)-2 && d->m_button3 != (Button)-2)
                done(d->m_button2);
            ke->accept();
        } else if (ke->key() == Qt::Key_Back) {
            if (d->m_button1 != (Button)-2 && d->m_button2 == (Button)-2 && d->m_button3 == (Button)-2)
                done(d->m_button1);
            else if (d->m_button1 != (Button)-2 && d->m_button2 != (Button)-2 && d->m_button3 == (Button)-2)
                done(d->m_button2);
            else if (d->m_button1 == (Button)-2 && d->m_button2 != (Button)-2 && d->m_button3 == (Button)-2)
                done(d->m_button2);
            else if (d->m_button1 != (Button)-2 && d->m_button2 == (Button)-2 && d->m_button3 != (Button)-2)
                done(d->m_button3);
            else if (d->m_button1 != (Button)-2 && d->m_button2 != (Button)-2 && d->m_button3 != (Button)-2)
                done(d->m_button3);
            else if (d->m_button1 == (Button)-2 && d->m_button2 == (Button)-2 && d->m_button3 != (Button)-2)
                done(d->m_button3);
            else if (d->m_button1 == (Button)-2 && d->m_button2 != (Button)-2 && d->m_button3 != (Button)-2)
                done(d->m_button3);
            ke->accept();
        } else if (ke->key() == Qt::Key_Hangup) {
            done(d->escapeBtnNum);
            ke->accept();
        }
        return;
    }

    if (ke->key() == Qt::Key_Yes || ke->key() == d->yesKey) {
        ke->accept();
        done(d->m_button1);
    } else if (ke->key() == Qt::Key_No || ke->key() == Qt::Key_Back) {
        ke->accept();
        if(d->m_button2 != NoButton)
            done(d->m_button2);
        else
            done(d->m_button1);
    }
}

QTOPIA_REPLACE_WIDGET(QAbstractMessageBox, SlideInMessageBox);
