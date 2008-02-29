/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "pxa27xmousehandler.h"

#ifdef QT_QWS_PXA27X
#include <QFile>
#include <QTextStream>
#include <QScreen>
#include <QSocketNotifier>
#include <QDebug>

#include <qtopialog.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    unsigned int   dummy1;
    unsigned int   dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int   value;
} ts_packet;


Pxa27xMouseHandler::Pxa27xMouseHandler()
    : m_raw(false), totX(0), totY(0), nX(0), nY(0), mouseIdx(0)
{
    qWarning( "***Loaded Pxa27x touchscreen plugin!");
    setObjectName( "Pxa27x Mouse Handler" );
    openTs();
}

Pxa27xMouseHandler::~Pxa27xMouseHandler()
{
    closeTs();
}

void Pxa27xMouseHandler::openTs()
{
    if ((mouseFD = open("/dev/input/event1", O_RDONLY | O_NDELAY)) < 0) {
      qWarning("Cannot open /dev/input/event1 (%s)", strerror(errno));
      return;
    } else
      qWarning("Opened /dev/input/event1 as touchscreen input");

    m_notify = new QSocketNotifier( mouseFD, QSocketNotifier::Read, this );
    connect( m_notify, SIGNAL(activated(int)), this, SLOT(readMouseData()));
}

void Pxa27xMouseHandler::closeTs()
{
    if (mouseFD)
        close(mouseFD);
    mouseFD = 0;

    delete m_notify;
    m_notify = 0;
    m_raw = false;
}

void Pxa27xMouseHandler::suspend()
{
    m_notify->setEnabled( false );
}

void Pxa27xMouseHandler::resume()
{
    m_notify->setEnabled( true );
}

void Pxa27xMouseHandler::readMouseData()
{
    if(!qt_screen)
        return;

    int     i, j, dx, dy;
    int n=0;

    do {
      n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx);
      if(n > 0) 
        mouseIdx += n;

      ts_packet *data;
      int idx = 0;

      while (mouseIdx-idx >= (int)sizeof(ts_packet)) {
        uchar *mb = mouseBuf+idx;
        data = (ts_packet *) mb;
        if(data->code == 0) {
          // x value
          totX = totX + data->value;
          sx[nX] = data->value;
          nX++;
        } else if(data->code == 1) {
          // y value
          totY = totY + data->value;
          sy[nY] = data->value;
          nY++;
        }
        if(nX >= TS_SAMPLES && nY >= TS_SAMPLES) {
          if(nX < nY) ss=nX;
          else ss=nY;

          for(i=0;i<ss-1;i++) {
            for(j=i+1;j<ss;j++) {
              dx = sx[i] - sx[j];
              if(dx < 0) dx = -dx;
              dy = sy[i] - sy[j];
              if(dy < 0) dy = -dy;
              if(min_x > dx) {
                min_x = dx;
                index_x1 = i;
                index_x2 = j;
              }
              if(min_y > dy) {
                min_y = dy;
                index_y1 = i;
                index_y2 = j;
              }
            }
          }

          QPoint pos( (sx[index_x1] + sx[index_x2])/2,
                  (sy[index_y1] + sy[index_y2])/2);

          nX=0; nY=0; totX=0; totY=0;
          oldmouse = transform( pos );
          if(oldmouse.x() < 0 || oldmouse.x() > 2000 ||
             oldmouse.y() < 0 || oldmouse.y() > 2000) {
            qLog(Input) << "*BAD Mouse sample :x=" << oldmouse.x() << ",y=" << oldmouse.y();
            oldmouse.setX(0);
            oldmouse.setY(0);
          } else {  
          qLog(Input) << "Mouse Down:x=" << oldmouse.x() << ",y=" << oldmouse.y();
          emit mouseChanged( oldmouse, Qt::LeftButton);
          }
        }
        if((data->code == 24) && (data->value == 0)) {
          // Removed pen from screen
          qLog(Input) << "Mouse Up  :x=" << oldmouse.x() << ",y=" << oldmouse.y();
          emit mouseChanged( oldmouse, 0);
          nX=0; nY=0; totX=0; totY=0;
        }
        idx += sizeof(ts_packet);
      }
      int surplus = mouseIdx - idx;
      for (int i = 0; i < surplus; i++)
        mouseBuf[i] = mouseBuf[idx+i];
      mouseIdx = surplus;  
    } while (n > 0);  
}

#endif // QT_QWS_PXA27X
