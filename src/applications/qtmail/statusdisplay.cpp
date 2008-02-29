/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#include "statusdisplay.h"

#include <qtopiaapplication.h>
#include <QPhoneStyle>


class StatusProgressBarStyle : public QPhoneStyle
{
public:
    void drawControl( ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget ) const
    {
        if (ce != CE_ProgressBarGroove)
            QPhoneStyle::drawControl(ce, opt, p, widget);
    }
};


StatusProgressBar::StatusProgressBar( QWidget* parent ) :
    QProgressBar(parent), txtchanged(false)
{
    QPalette p(palette());
    p.setBrush(QPalette::Base,p.brush(QPalette::Window));
    p.setBrush(QPalette::HighlightedText,p.brush(QPalette::WindowText));
    setPalette(p);
    setAlignment(Qt::AlignHCenter);
    setStyle(new StatusProgressBarStyle);
}

StatusProgressBar::~StatusProgressBar()
{
    delete style();
}

QSize StatusProgressBar::sizeHint() const
{
    return QProgressBar::sizeHint()-QSize(0,8);
}

void StatusProgressBar::setText(const QString& s)
{
    if ( txt != s ) {
        txt = s;
        txtchanged = true;

        if ( value() == maximum() )
            reset();

        update();
    }
}

QString StatusProgressBar::text() const
{
    static const Qt::TextElideMode mode(QApplication::isRightToLeft() ? Qt::ElideLeft : Qt::ElideRight);
    static QString last;

    if (txtchanged) {
        QFontMetrics fm(font());
        last = fm.elidedText(txt, mode, width());
        txtchanged = false;
    } 
    return last;
}


StatusDisplay::StatusDisplay(QWidget* parent)
    : StatusProgressBar(parent), suppressed(false)
{
}

void StatusDisplay::showStatus(bool visible)
{
    suppressed = !visible;
    setVisible(visible);
}

void StatusDisplay::displayStatus(const QString& txt)
{
    setVisible(true);
    setText(txt);
}

void StatusDisplay::displayProgress(uint value, uint range)
{
    if (range == 0) {
        reset();
    } else {
        setVisible(true);

        if (static_cast<int>(range) != maximum())
            setRange(0, range);

        setValue(qMin(value, range));
    }
}

void StatusDisplay::clearStatus()
{
    reset();
    setText(QString());

    if (suppressed)
        setVisible(false);
}


