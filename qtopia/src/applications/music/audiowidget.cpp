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
#include <qwidget.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qframe.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#ifdef QTOPIA_PHONE
# include <qtopia/contextmenu.h>
# include <qtopia/contextbar.h>
#endif
#include "qpe/qcopenvelope_qws.h"
#include "action.h"
#include "imageutil.h"
#include "audiowidget.h"


// Layout information for the audioButtons (and if it is a toggle button or not)
MediaButton audioButtons[] = {
    { TRUE,  FALSE, FALSE, FALSE, "play",     PlayButton       }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "stop",     StopButton       }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "next",     NextButton       }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "previous", PreviousButton   }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "up",       VolumeUpButton   }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "down",     VolumeDownButton }, // No tr
    { TRUE,  FALSE, FALSE, FALSE, "loop",     LoopButton       }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "playlist", PlayListButton   },
    { FALSE, FALSE, FALSE, FALSE, "forward",  ForwardButton    }, // No tr
    { FALSE, FALSE, FALSE, FALSE, "backward", BackwardButton   }, // No tr
};


AudioWidget::AudioWidget( QWidget* parent, const QString &skin, const char* name ) :
    ControlWidgetBase(parent, skin, "audio", name), songInfo(this) // No tr
{
    setButtonData( audioButtons, sizeof(audioButtons)/sizeof(MediaButton) );
    connect( mediaPlayerState, SIGNAL( viewChanged(View) ), this, SLOT( setView(View) ) );

    songInfo.setFocusPolicy( QWidget::NoFocus );
    changeTextColor( &songInfo );

#ifdef QTOPIA_PHONE
    // Create context menu
    ContextMenu *contextMenu = new ContextMenu( this, 0, ContextMenu::Modal );
    ContextBar::setLabel( this, Key_Select, "pause", tr("Pause") );

    // Add actions to the context menu.
    Action *tbLoop    = new Action( this, tr( "Loop" ),       "mediaplayer/loop",   mediaPlayerState, SLOT(setLooping(bool)), TRUE );
    Action *tbHide    = new Action( this, tr( "Hide" ),	      "hide",   this, SLOT(backgroundMode()), false );
    tbLoop->addTo( contextMenu );
    tbHide->addTo( contextMenu );
    connect( mediaPlayerState, SIGNAL( loopingToggled(bool) ), tbLoop, SLOT( setOn2(bool) ) );
    tbLoop->setOn( mediaPlayerState->looping() );
#endif
}


AudioWidget::~AudioWidget()
{
}


void AudioWidget::virtualResize()
{
    int h = height();
    int w = width();

    int border = w / 50;
    const int timeHeight = 20;

    int scaleW = w;
    int scaleH = h - 7 * border - 2 * timeHeight;

    songInfo.setGeometry( QRect( border, border, w - 2 * border, timeHeight ) );

    resizeObjects( w, h, scaleW, scaleH );
}


void AudioWidget::backgroundMode()
{
    QWidgetList *list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    for (QWidget *w; (w = it.current()) != 0; ++it) {
        if ( w->isVisible() )
            w->hide();
    }
    delete list;
}


void AudioWidget::setView( View view )
{
    if ( view != AudioView )
	canPaint = FALSE;
    resetButtons();
}


void AudioWidget::virtualUpdateSlider()
{
    updateSlider();
}


void AudioWidget::virtualPaint( QPaintEvent *pe )
{
    if ( !pe->erased() ) {
	// Combine with background and double buffer
	QPixmap pix( pe->rect().size() );
	QPainter p( &pix );
	p.translate( -pe->rect().topLeft().x(), -pe->rect().topLeft().y() );
	p.drawTiledPixmap( pe->rect(), background.scaledPix, pe->rect().topLeft() );
	paintAllButtons( p );
	QPainter p2( this );
	p2.drawPixmap( pe->rect().topLeft(), pix );
    } else {
	QPainter p( this );
	paintAllButtons( p );
    }
}

