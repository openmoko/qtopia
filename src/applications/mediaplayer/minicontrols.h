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
#ifndef MINI_CONTROLS_H
#define MINI_CONTROLS_H


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

/*
enum MediaButtonType {
    PlayButton,
    StopButton,
    PreviousButton,
    NextButton,
    ForwardButton,
    BackwardButton,
    //VolumeUpButton,
    //VolumeDownButton,
    LoopButton,
    PlayListButton
};
*/

class MiniControls : public QWidget {
    Q_OBJECT
public:
    MiniControls( QWidget* parent, const char* name );
    ~MiniControls();

public slots:
    void sliderPressed();
    void sliderReleased();
    void setPosition( long );
    void setLength( long );
    void setPaused( bool b );
    void setLooping( bool b );
    void setPlaying( bool b );

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
    void closeEvent( QCloseEvent *event );

    virtual void updateSlider( long, long );
    virtual QString timeAsString( long );

    void updateSliderBase( long, long );
    //void setToggleButton( MediaButtonType, bool );
    void toggleButton( int );
    void stopPlaying();

    MediaButton *buttons;
    int buttonCount;

    MySlider slider;
    QLineEdit time;
    int xoff, yoff;
    bool sliderBeingMoved;
};


#endif // MINI_CONTROLS_H

