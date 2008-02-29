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
    adjustedPalette.setColor( QPalette::Inactive,
			      QColorGroup::Highlight,
			      origPalette.active().base() );
    adjustedPalette.setColor( QPalette::Inactive,
			      QColorGroup::HighlightedText,
			      origPalette.active().text() );
    if ( recording )
	setPalette( adjustedPalette );
    else
	setPalette( origPalette );
}

