/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "timeprogressbar.h"


#include <qapplication.h>


TimeProgressBar::TimeProgressBar( QWidget *parent )
    : QProgressBar( parent ), prevValue( -1 )
{
    setAlignment( Qt::AlignCenter );
    recording = true;
    refreshPalettes();
}


TimeProgressBar::~TimeProgressBar()
{
}


void TimeProgressBar::setRecording()
{
    recording = true;
    setPalette( adjustedPalette );
}


void TimeProgressBar::setPlaying()
{
    recording = false;
    setPalette( origPalette );
}


bool TimeProgressBar::setIndicator( QString& progress_str, int progress, int totalSteps )
{
    if ( !totalSteps )
        return false;
    if ( progress < 0 ) {
        progress_str = QString::fromLatin1( "" );
        return true;
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
        return true;
    } else {
        return false;
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
    adjustedPalette.setColor
            ( QPalette::Active,
              QPalette::Highlight,
              origPalette.color( QPalette::Active, QPalette::Base ) );
    adjustedPalette.setColor
            ( QPalette::Active,
              QPalette::HighlightedText,
              origPalette.color( QPalette::Active, QPalette::Text ) );
    adjustedPalette.setColor
            ( QPalette::Inactive,
              QPalette::Highlight,
              origPalette.color( QPalette::Active, QPalette::Base ) );
    adjustedPalette.setColor
            ( QPalette::Inactive,
              QPalette::HighlightedText,
              origPalette.color( QPalette::Active, QPalette::Text ) );
    if ( recording )
        setPalette( adjustedPalette );
    else
        setPalette( origPalette );
}

