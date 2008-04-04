/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef VIDEOVIEW_H
#define VIDEOVIEW_H

#include <qwidget.h>
#include <qimage.h>
#include <qlist.h>

#include <QSocketNotifier>


namespace camera
{
class VideoCaptureDevice;
}

class QSlider;
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

    void zoomIn();
    void zoomOut();
    void doZoom();

    int maxZoom() const;
    int minZoom() const;

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

    // Zoom 
    bool m_doZoom;
    int m_maxZoom;
    int m_minZoom;
    int m_zoomlevel;
    float m_zoomfactor;

    bool m_force;

    QSlider *m_zoomWidget;
    void showZoom();
    void hideZoom();
};

#endif

