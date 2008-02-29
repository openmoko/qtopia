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

#include "timeprogressbar.h"


#include <qapplication.h>


TimeProgressBar::TimeProgressBar( QWidget *parent, const char *name, WFlags fl )
    : QProgressBar( parent, name, fl ), prevValue( -1 )
{
    setCenterIndicator( TRUE );
    recording = TRUE;
    refreshPalettes();
}


TimeProgressBar::~TimeProgressBar()
{
}


void TimeProgressBar::setRecording()
{
    recording = TRUE;
    setPalette( adjustedPalette );
}


void TimeProgressBar::setPlaying()
{
    recording = FALSE;
    setPalette( origPalette );
}


bool TimeProgressBar::setIndicator( QString& progress_str, int progress, int totalSteps )
{
    if ( !totalSteps )
	return FALSE;
    if ( progress < 0 ) {
	progress_str = QString::fromLatin1( "" );
	return TRUE;
    } else if ( progress != prevValue ) {
	prevValue = progress;
	if ( progress > 60 * 60 ) {
	    progress_str.sprintf( "%02d:%02d:%02d",
				  progress / (60 * 60),
				  (progress / 60) % 60,
				  progress % 60 );
	} else {
	    progress_str.sprintf( "%02d:%02d",
				  progress / 60,
				  progress % 60 );
	}
	return TRUE;
    } else {
	return FALSE;
    }
}


bool TimeProgressBar::event( QEvent *e )
{
    if ( e->type() == QEvent::ApplicationPaletteChange ) {
	refreshPalettes();
    }
    return QProgressBar::event( e );
}


void TimeProgressBar::refreshPalettes()
{
    origPalette = QApplication::palette( this );
    adjustedPalette = origPalette;
    adjustedPalette.setColor( QPalette::Active,
			      QColorGroup::Highlight,
			      origPalette.active().base() );
    adjustedPalette.setColor( QPalette::Active,
			      QColorGroup::HighlightedText,
			      origPalette.active().text() );
    if ( recording )
	setPalette( adjustedPalette );
    else
	setPalette( origPalette );
}

