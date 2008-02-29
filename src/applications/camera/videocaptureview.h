/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <qwidget.h>
#include <qimage.h>
#include <qlist.h>


namespace camera
{
class VideoCaptureDevice;
}


class VideoCaptureView : public QWidget
{
    Q_OBJECT

public:
    VideoCaptureView(QWidget *parent = 0, Qt::WFlags fl = 0);
    ~VideoCaptureView();

    bool available() const;

    QImage image() const { return m_image; }
    void setLive(int period=0);
    void setStill(const QImage&);

    QList<QSize> photoSizes() const;
    QList<QSize> videoSizes() const;

    QSize recommendedPhotoSize() const;
    QSize recommendedVideoSize() const;
    QSize recommendedPreviewSize() const;

    QSize captureSize() const;
    void setCaptureSize( QSize size );

    uint refocusDelay() const;

protected:
    void moveEvent(QMoveEvent* moveEvent);
    void resizeEvent(QResizeEvent* resizeEvent);
    void paintEvent(QPaintEvent* paintEvent);
    void timerEvent(QTimerEvent* timerEvent);

private:
    bool                m_cleared;
    int                 m_tidUpdate;
    QImage              m_image;
    camera::VideoCaptureDevice  *m_capture;
};

#endif

