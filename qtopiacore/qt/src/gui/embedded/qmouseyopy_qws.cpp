/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qmouseyopy_qws.h"

#ifndef QT_NO_QWS_MOUSE_YOPY
#include "qwindowsystem_qws.h"
#include "qsocketnotifier.h"
#include "qapplication.h"
#include "qscreen_qws.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

QT_BEGIN_NAMESPACE

class QWSYopyMouseHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    QWSYopyMouseHandlerPrivate(QWSYopyMouseHandler *h);
    ~QWSYopyMouseHandlerPrivate();

    void suspend();
    void resume();

private slots:
    void readMouseData();

private:
    int mouseFD;
    int prevstate;
    QSocketNotifier *mouseNotifier;
    QWSYopyMouseHandler *handler;
};

QWSYopyMouseHandler::QWSYopyMouseHandler(const QString &driver, const QString &device)
    : QWSMouseHandler(driver, device)
{
    d = new QWSYopyMouseHandlerPrivate(this);
}

QWSYopyMouseHandler::~QWSYopyMouseHandler()
{
    delete d;
}

void QWSYopyMouseHandler::resume()
{
    d->resume();
}

void QWSYopyMouseHandler::suspend()
{
    d->suspend();
}

QWSYopyMouseHandlerPrivate::QWSYopyMouseHandlerPrivate(QWSYopyMouseHandler *h)
    : handler(h)
{
    if ((mouseFD = open("/dev/ts", O_RDONLY)) < 0) {
        qWarning("Cannot open /dev/ts (%s)", strerror(errno));
        return;
    } else {
        sleep(1);
    }
    prevstate=0;
    mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read,
                                         this);
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
}

QWSYopyMouseHandlerPrivate::~QWSYopyMouseHandlerPrivate()
{
    if (mouseFD >= 0)
        close(mouseFD);
}

#define YOPY_XPOS(d) (d[1]&0x3FF)
#define YOPY_YPOS(d) (d[2]&0x3FF)
#define YOPY_PRES(d) (d[0]&0xFF)
#define YOPY_STAT(d) (d[3]&0x01)

struct YopyTPdata {

  unsigned char status;
  unsigned short xpos;
  unsigned short ypos;

};

void QWSYopyMouseHandlerPrivate::suspend()
{
    mouseNotifier->setEnabled(false);
}


void QWSYopyMouseHandlerPrivate::resume()
{
    prevstate = 0;
    mouseNotifier->setEnabled(true);
}

void QWSYopyMouseHandlerPrivate::readMouseData()
{
    if(!qt_screen)
        return;
    YopyTPdata data;

    unsigned int yopDat[4];

    int ret;

    ret=read(mouseFD,&yopDat,sizeof(yopDat));

    if(ret) {
        data.status= (YOPY_PRES(yopDat)) ? 1 : 0;
        data.xpos=YOPY_XPOS(yopDat);
        data.ypos=YOPY_YPOS(yopDat);
        QPoint q;
        q.setX(data.xpos);
        q.setY(data.ypos);
        if (data.status && !prevstate) {
          handler->mouseChanged(q,Qt::LeftButton);
        } else if(!data.status && prevstate) {
          handler->mouseChanged(q,0);
        }
        prevstate = data.status;
    }
    if(ret<0) {
        qDebug("Error %s",strerror(errno));
    }
}

QT_END_NAMESPACE

#include "qmouseyopy_qws.moc"

#endif //QT_NO_QWS_MOUSE_YOPY
