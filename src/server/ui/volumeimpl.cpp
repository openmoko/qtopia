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

#include "volumeimpl.h"
#include <QKeyEvent>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QVBoxLayout>

class VolumeWidget : public QWidget
{

public:
    VolumeWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0)
        : QWidget(parent, flags), m_steps(10), m_current(0)
    {
    }

    ~VolumeWidget() {}

    int heightForWidth(int width) const
    {
        return qRound(0.33 * width);
    }

    int value() const
    {
        return m_current;
    }

    void setCurrent(int i)
    {
        if ((i <= 0) || (i > m_steps))
            return;

        m_current = i;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        int w = rect().width() - (3 * m_steps);
        barWidth = qRound(w / (m_steps - 1));
        barHeight = qRound(rect().height() / (m_steps - 1));

        QPainter painter(this);
        painter.setPen(palette().text().color());
        painter.setBrush(palette().highlight());

        for (int n = 1; n < m_current; n++) {
            QRect r;
            r.setTopLeft(QPoint(((n-1) * barWidth) + (n * 3) - 1, (m_steps-1-n)*barHeight));
            r.setWidth(barWidth);
            r.setHeight(n * barHeight);
            painter.drawRect(r);
        }
    }

    QSizePolicy sizePolicy() const
    {
        return QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    }

    QSize sizeHint() const
    {
        return QSize(27, 9);
    }

private:
    int m_steps;
    int m_current;
    int barWidth;
    int barHeight;
};


VolumeDialogImpl::VolumeDialogImpl( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), m_tid(0), m_oldValue(0)
{
    QRect d = QApplication::desktop()->screenGeometry();
    int dw = d.width();
    int dh = d.height();
    setGeometry(20*dw/100, 30*dh/100, 60*dw/100, 40*dh/100);

    QColor c(Qt::black);
    c.setAlpha(180);

    setAttribute(Qt::WA_SetPalette, true);

    QPalette p = palette();
    p.setBrush(QPalette::Window, c);
    setPalette(p);

    QVBoxLayout *vBox = new QVBoxLayout(this);
    QHBoxLayout *hBox = new QHBoxLayout(this);

    QIcon icon(":icon/sound");

    QLabel *l = new QLabel(this);
    l->setPixmap(icon.pixmap(64, 64));

    hBox->addStretch();
    hBox->addWidget(l);
    hBox->addStretch();
    vBox->addLayout(hBox);

    volumeWidget = new VolumeWidget(this);
    vBox->addWidget(volumeWidget);
}

void VolumeDialogImpl::timerEvent( QTimerEvent *e )
{
    Q_UNUSED(e)
    close();
}

void VolumeDialogImpl::setVolume( bool up )
{
    m_oldValue = volumeWidget->value();
    volumeWidget->setCurrent( up ? m_oldValue + 1 : m_oldValue - 1 );

    int value = volumeWidget->value();
    if ( m_oldValue < value )
        emit volumeChanged( true );
    else if ( m_oldValue > value )
        emit volumeChanged( false );

    resetTimer();
}

void VolumeDialogImpl::resetTimer()
{
    killTimer( m_tid );
    m_tid = startTimer( TIMEOUT );
}
