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

#include "waitindicator.h"
#include "qtopiaserverapplication.h"

#include <QValueSpaceItem>
#include <QBitmap>
#include <QPainter>
#include <QTimer>
#include <QDesktopWidget>
#include <QImageReader>


static const int FadeOutTime = 500;

/*!
  \class WaitIndicator
  \brief The WaitIndicator class provides widget that is shown while an application starts.

  This task provides a widget that is Qtopia's equivalent to an hour glass.
  It is only visible while the system is busy starting a Qtopia application. In order to determine 
  whether an application is in the process of being started this task monitors the valuespace
  path \c /System/applications/Info/Busy.

  This class is part of the Qtopia server and cannot be used by other Qtopia applications.
  */

/*! \internal */
WaitIndicator::WaitIndicator()
    : QWidget(0, Qt::Tool|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint)
    , angle(180)
{
    QPalette pal = palette();
    pal.setBrush(QPalette::Window, QBrush(QColor(0,0,0,0)));
    setPalette(pal);
    vsi = new QValueSpaceItem("/System/Applications/Info/BusyCount", this);
    connect(vsi, SIGNAL(contentsChanged()), this, SLOT(busyChanged()));
    timeline.setLoopCount(1);
    timeline.setStartFrame(10);
    timeline.setEndFrame(70);
    timeline.setUpdateInterval(100);
    timeline.setDuration(FadeOutTime);
    connect(&timeline, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
    connect(&timeline, SIGNAL(finished()), this, SLOT(finished()));
    QSize dsize = QApplication::desktop()->screenGeometry().size();
    int size = qMin(dsize.width(), dsize.height()) / 4 + 1;
    QImageReader imgReader(":image/clock");
    imgReader.setScaledSize(QSize(size, size));
    waitIcon = QPixmap::fromImage(imgReader.read());
    setMask(waitIcon.mask());
}

/*! \internal */
void WaitIndicator::paintEvent(QPaintEvent* /*e*/)
{
    QPainter p(this);
//    p.setCompositionMode(QPainter::CompositionMode_Clear);
//    p.eraseRect(rect());
//    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawPixmap(0,0,waitIcon);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(Qt::black, 2));
    p.translate(qreal(rect().width())/2.0, qreal(rect().height())/2.0);
    p.rotate(angle);
    p.drawLine(0,0,0,rect().height()/2-waitIcon.width()/8);
}

void WaitIndicator::frameChanged(int frame)
{
    setWindowOpacity(qreal(frame)/100.0);
}

void WaitIndicator::finished()
{
    if (timeline.direction() == QTimeLine::Backward) {
        hide();
        angle = 180;
        timer.stop();
    }
}

/*! \internal */
void WaitIndicator::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timer.timerId()) {
        angle += 6;
        if (angle >= 360)
            angle = 0;
        update();
    } else {
        QWidget::timerEvent(e);
    }
}

void WaitIndicator::busyChanged()
{
    if (vsi->value().toBool()) {
        timeline.setCurrentTime(FadeOutTime);
        if (!isVisible()) {
            QRect srect = QApplication::desktop()->availableGeometry();
            setGeometry(srect.x()+(srect.width()-waitIcon.width())/2,
                    srect.y()+(srect.height()-waitIcon.height())/2,
                    waitIcon.width(), waitIcon.height());
            show();
        }
        timer.start(250, this);
    } else {
        timeline.setDirection(QTimeLine::Backward);
        timeline.start();
    }
}

QTOPIA_TASK( WaitIndicator, WaitIndicator );
