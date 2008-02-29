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
#ifndef AUDIO_WIDGET_H
#define AUDIO_WIDGET_H


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
#include <qrect.h>
#include "controlwidgetbase.h"
#include "mediaplayerstate.h"
#include "action.h"


class Ticker : public QFrame {
    Q_OBJECT
public:
    Ticker( QWidget* parent = 0 );
    ~Ticker();

    void setText( const QString& text );

protected:
    void resizeEvent( QResizeEvent * );
    void timerEvent( QTimerEvent * );
    void drawContents( QPainter *p );
//    void paintEvent( QPaintEvent *p );

private:
    QString scrollText;
    QPixmap scrollTextPixmap;
    int pos;
};


class AudioWidget : public ControlWidgetBase {
    Q_OBJECT
public:
    AudioWidget( QWidget* parent, const QString& skin, const char* name );
    ~AudioWidget();
    void setTickerText( const QString &text ) { songInfo.setText( text ); }

public slots:
    void setView( View );

protected:
    void virtualPaint( QPaintEvent *pe );
    void virtualResize();
    void virtualUpdateSlider();

private:
    Ticker  songInfo;
};


#endif // AUDIO_WIDGET_H

