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

#include "s3c24a0mousehandler.h"

#ifdef QT_QWS_S3C24A0
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
    unsigned short pressure;
    unsigned short x;
    unsigned short y;
    unsigned short pad;
} ts_packet;


S3c24a0MouseHandler::S3c24a0MouseHandler()
    : m_raw(false), mouseIdx(0)
{
    qWarning( "***Loaded s3c24a0 touchscreen plugin!");
    setObjectName( "s3c24a0 Mouse Handler" );
    numSamples=0;
    currSample=0;
    openTs();
}

S3c24a0MouseHandler::~S3c24a0MouseHandler()
{
    closeTs();
}

void S3c24a0MouseHandler::openTs()
{
    if ((mouseFD = open("/dev/ts", O_RDONLY | O_NDELAY)) < 0) {
      qWarning("Cannot open /dev/ts (%s)", strerror(errno));
      return;
    } else
      qWarning("Opened /dev/ts as touchscreen input");

    m_notify = new QSocketNotifier( mouseFD, QSocketNotifier::Read, this );
    connect( m_notify, SIGNAL(activated(int)), this, SLOT(readMouseData()));
}

void S3c24a0MouseHandler::closeTs()
{
    if (mouseFD)
        close(mouseFD);
    mouseFD = 0;

    delete m_notify;
    m_notify = 0;
    m_raw = false;
}

void S3c24a0MouseHandler::suspend()
{
    m_notify->setEnabled( false );
}

void S3c24a0MouseHandler::resume()
{
    m_notify->setEnabled( true );
}

void S3c24a0MouseHandler::readMouseData()
{
    if(!qt_screen)
        return;

    int n;
    do {
      n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx );
      if ( n > 0 )
        mouseIdx += n;
    //} while ( n > 0 && mouseIdx < mouseBufSize );

      ts_packet *data;
      int idx = 0;

      while (mouseIdx-idx >= (int)sizeof(ts_packet)) {
        uchar *mb = mouseBuf+idx;
        data = (ts_packet *) mb;
        qLog(Input)<<"ts:x="<<data->x<<",y="<<data->y<<",p="<<data->pressure;

        if(data->pressure >= QT_QWS_TP_PRESSURE_THRESHOLD) {
          samples[currSample] = QPoint( data->x, data->y );
          numSamples++;

          if ( numSamples > QT_QWS_TP_MINIMUM_SAMPLES ) {
            mousePos = QPoint( 0, 0 );
            QPoint totalMousePos( 0, 0);
            int count=0;

            for(int i=1;i<numSamples;i++) {
              QPoint dp = samples[i] - samples[i-1];
              int dxSqr = dp.x() * dp.x();
              int dySqr = dp.y() * dp.y();
              if ( dxSqr + dySqr < (QT_QWS_TP_MOVE_LIMIT * QT_QWS_TP_MOVE_LIMIT) ) {
                totalMousePos = QPoint(totalMousePos.x()+samples[i].x(),totalMousePos.y()+samples[i].y());
                count++;
              }
            }
            if(count) {
              mousePos = QPoint(totalMousePos.x() / count, totalMousePos.y() / count);
              mousePos = transform( mousePos );
              oldmouse = mousePos;
              qLog(Input) << "Mouse Down:x=" << oldmouse.x() << ",y=" << oldmouse.y();
              emit mouseChanged(oldmouse,Qt::LeftButton);
            } 
            currSample=0;
            numSamples=0;
          } else {
            currSample++;
          }
        } else {
          qLog(Input) << "Mouse Up  :x=" << oldmouse.x() << ",y=" << oldmouse.y();
          emit mouseChanged( oldmouse, 0);
          currSample=0;
          numSamples=0;
        }
        idx += sizeof(ts_packet);
      }
      int surplus = mouseIdx - idx;
      for (int i = 0; i < surplus; i++)
        mouseBuf[i] = mouseBuf[idx+i];
      mouseIdx = surplus;
    } while (n > 0);  
}

#endif // QT_QWS_S3C24A0
