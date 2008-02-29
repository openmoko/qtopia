/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H


#include <qwidget.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qrect.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qimage.h>
#include <qpushbutton.h>
#include "controlwidgetbase.h"
#include "mediaplayerstate.h"
#include "action.h"
#include "framebuffer.h"


class VideoWidget;


class VideoOutput : public QWidget {
    Q_OBJECT
public:
    VideoOutput( VideoWidget* parent );
    ~VideoOutput();
    bool playVideo();
protected:
    void paintEvent( QPaintEvent *pe );
    void mouseReleaseEvent( QMouseEvent *me );
private:
    VideoWidget *parentWidget;
    QRect targetRect;
    QImage *currentFrame;
    QImage *rotatedFrame;
    FrameBuffer imageFb;
    FrameBuffer rotatedBuffer;
    FrameBuffer directFb;
};


class VideoWidget : public ControlWidgetBase {
    Q_OBJECT
public:
    VideoWidget( QWidget* parent, const QString& skin, const char* name );
    ~VideoWidget();

    enum Mode {
	InvalidMode,
	Fullscreen,
	Large,
	Normal
    };

    bool playVideo();
    Mode mode() { return screenMode; }
public slots:
    void makeVisible();
    void setView( View );
    void setMode( int );
    void setNextMode();
    void doModeMenu();

protected:
    void virtualPaint( QPaintEvent *pe );
    void virtualResize();
    void virtualUpdateSlider();
    void paintButton( QPainter& p, int i );

private:
    bool showCornerButton() { return width() >= 315; }
    void updateVideoOutputGeometry();
    QRect innerMovieArea;
    QRect outerMovieArea;
    int movieBorder;
    QPushButton cornerButton;
    QPopupMenu cornerMenu;
    Mode screenMode;
    VideoOutput videoOutput;
    int buttonHeight;
};


#endif // VIDEO_WIDGET_H



