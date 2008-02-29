/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef VIDEO_WIDGET_H
#define VIDEO_WIDGET_H


#include <qslider.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include "controlwidgetbase.h"
#include "mediaplayerstate.h"
#include "action.h"
#include "videooutput.h"


class VideoWidget;


class VideoOutput : public VideoOutputWidget {
    Q_OBJECT
public:
    VideoOutput( VideoWidget* parent );
    ~VideoOutput();
protected:
    void paintEvent( QPaintEvent *pe );
    void mouseReleaseEvent( QMouseEvent *me );
    void keyPressEvent( QKeyEvent *ke );
private:
    VideoWidget *parentWidget;
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

    void makeVisible();
    void updateVolume() { }
    Mode mode() { return screenMode; }
public slots:
    void setView( View );
    void setMode( int );
    void setNextMode();
    void doModeMenu();
    void setFullscreen(bool);

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
};


#endif // VIDEO_WIDGET_H

