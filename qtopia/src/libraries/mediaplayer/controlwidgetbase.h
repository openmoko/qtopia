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
#ifndef CONTROL_WIDGET_BASE_H
#define CONTROL_WIDGET_BASE_H


#include <qwidget.h>
#include <qpainter.h>
#include <qdrawutil.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qstring.h>
#include <qslider.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qtimer.h>
#include "mediaplayerstate.h"
#include "action.h"
#include "ticker.h"


enum MediaButtonType {
    PlayButton,
    StopButton,
    PauseButton,
    PreviousButton,
    NextButton,
    ForwardButton,
    BackwardButton,
    VolumeUpButton,
    VolumeDownButton,
    LoopButton,
    PlayListButton,
    FullscreenButton,
    MaxButtonTypes
};


struct MediaButton {
    bool isToggle, isHeld, isDown, hasImage;
    const char *skinImage;
    MediaButtonType buttonType;
};


class ScalableImage {
    public:
	QImage	unscaledImg;
	QPixmap scaledPix;
	bool	loaded;
	bool	scaled;
	QPoint	offset;
};


/*
 *  Abstract base class used by AudioWidget and VideoWidget classes
 */
class ControlWidgetBase : public QWidget {
    Q_OBJECT
public:
    ControlWidgetBase( QWidget* parent, const QString& skin, const QString& audio, const char* name );
    ~ControlWidgetBase();

    // Format seconds as 0:00
    static QString toTimeString( long );

    virtual void setTickerText( const QString & ) { }
    virtual void updateVolume();
    virtual void makeVisible() { }

public slots:
    void sliderPressed();
    void sliderReleased();
    void setPosition( long );
    void setLength( long );
    void setPaused( bool b );
    void setSeekable( bool b );
    void setLooping( bool b );
    void setPlaying( bool b );
    void setFullscreen( bool b );
    void setSkin( const QString& skin );

    void loadImages();
    void scaleImages();
    void hideVolumeOverlay();

    void stop();
    void play();

signals:
    void sliderMoved( long );

    void moreClicked();
    void lessClicked();
    void moreReleased();
    void lessReleased();

    void forwardClicked();
    void backwardClicked();
    void forwardReleased();
    void backwardReleased();

protected:
    virtual void virtualResize() = 0; // Make the class abstract
    virtual void virtualPaint( QPaintEvent *event ) = 0;
    virtual void virtualUpdateSlider() = 0;
    virtual void paintButton( QPainter& p, int i );
    virtual void drawOverlays();
    QString timeAsString( long );

    void keyPressEvent( QKeyEvent *event );
    void keyReleaseEvent( QKeyEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void closeEvent( QCloseEvent *event );
    void resizeEvent( QResizeEvent *event );
    void paintEvent( QPaintEvent *event );
    void resetButtons();
    void updateSlider();

    void setButtonData( MediaButton *mediaButtons, int count );
    void resizeObjects( int w, int h, int scaleW, int scaleH, int cornerWidgetWidth = 0 );
    void setToggleButton( MediaButtonType, bool );
    void toggleButton( int );
    void paintAllButtons( QPainter& p );
    void getButtonPix( bool down );
    void getButtonMask( int i );
    bool videoGeometry(QRect &);
    void drawVolumeOverlay(bool inPaintEvent=true);

    int skinW, skinH;
    int skinScaleW, skinScaleH;
    int actualScaleW, actualScaleH;
    QString skinType;
    QString skinFile;
    QString skin;

    MediaButton *buttons;
    int buttonCount;
    int buttonMap[MaxButtonTypes];

    QImage imgButtonMask;

    ScalableImage mediaButtonImg[2];
    ScalableImage mediaButtonMask[MaxButtonTypes];
    ScalableImage background;
    bool bgNeedsScaling;

    bool imagesLoaded;
    bool imagesScaled;

    bool hadFirstResize;
    bool resized;
    bool canPaint;

    int drawnVolume;
    MySlider slider;
    Ticker time;
    bool sliderBeingMoved;
    QTimer volumeTimeout;
    bool drawVolume;
    int xoff, yoff;
};


#endif // CONTROL_WIDGET_BASE_H

