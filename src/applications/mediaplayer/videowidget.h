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
#include "controlwidgetbase.h"
#include "mediaplayerstate.h"
#include "action.h"


class VideoWidget : public ControlWidgetBase {
    Q_OBJECT
public:
    VideoWidget( QWidget* parent, const QString& skin, const char* name );
    ~VideoWidget();

    bool playVideo();

public slots:
    void makeVisible();
    void setView( View );

protected:
    void internalPaint( QPaintEvent *pe );
    void internalResize();
    void mouseReleaseEvent( QMouseEvent *me );
    void updateSlider( long, long );
    void paintButton( QPainter& p, int i );
    QString timeAsString( long );

private:
    int	scaledWidth;
    int scaledHeight;
    QRect innerMovieArea;
    QRect outerMovieArea;
    int movieBorder;

    QImage *currentFrame;
};


#endif // VIDEO_WIDGET_H



