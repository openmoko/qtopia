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
#include "mediaplayerstate.h"
#include "action.h"


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


/*
 *  Abstract base class used by AudioWidget and VideoWidget classes
 */
class ControlWidgetBase : public QWidget {
    Q_OBJECT
public:
    ControlWidgetBase( QWidget* parent, const QString& skin, const QString& audio, const char* name );
    ~ControlWidgetBase();

public slots:
    void sliderPressed();
    void sliderReleased();
    void setPosition( long );
    void setLength( long );
    void setPaused( bool b );
    void setLooping( bool b );
    void setPlaying( bool b );
    void setFullscreen( bool b );
    void setSkin( const QString& skin );

    void loadImages();
    void scaleImages();

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
    virtual void internalResize() = 0; // Make the class abstract
    virtual void internalPaint( QPaintEvent *event ) = 0;

    void mouseMoveEvent( QMouseEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void closeEvent( QCloseEvent *event );
    void resizeEvent( QResizeEvent *event );
    void paintEvent( QPaintEvent *event );

    virtual void updateSlider( long, long );
    virtual QString timeAsString( long );
    virtual void paintButton( QPainter& p, int i );

    void updateSliderBase( long, long );
    void setButtonData( MediaButton *mediaButtons, int count );
    int resizeObjects( int w, int h, int scaleW, int scaleH );
    void setToggleButton( MediaButtonType, bool );
    void toggleButton( int );
    void paintAllButtons( QPainter& p );
    void stopPlaying();
    void getButtonPix( bool down );
    void getButtonMask( int i );

    int skinW, skinH;
    int skinScaleW, skinScaleH;
    int actualScaleW, actualScaleH;
    QString skinType;
    QString skinName;
    QString skin;

    MediaButton *buttons;
    int buttonCount;
    QImage  imgButtonMask;

    // Unscaled
    QImage tmpButtonImg[2];
    bool tmpButtonImgLoaded[2];
    QImage tmpButtonMask[MaxButtonTypes];
    bool tmpButtonMaskLoaded[MaxButtonTypes];

    // Scaled
    QPixmap buttonPix[2];
    bool buttonPixCreated[2];
    QBitmap buttonMask[MaxButtonTypes];
    bool buttonMaskCreated[MaxButtonTypes];

    QImage bgImg;
    bool bgImgLoaded;
    QPixmap bgPix;
    bool bgNeedsScaling;
    bool bgPixCreated;

    bool imagesLoaded;
    bool imagesScaled;

    bool hadFirstResize;
    bool resized;

    MySlider slider;
    QLineEdit time;
    int xoff, yoff;
    bool sliderBeingMoved;
};


#endif // CONTROL_WIDGET_BASE_H

