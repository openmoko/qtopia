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
#include <qtopia/resource.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qlabel.h>
#include "loadingwidget.h"


/*!
  \class MediaScanningProgress loadingwidget.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    This class is for displaying a progress bar when
    searching for documents.
*/


MediaScanningProgress::MediaScanningProgress( QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    setSpacing( 10 );
    setMargin( 10 );

    QWidget *space1 = new QWidget( this );
    setStretchFactor( space1, 1 );

    QLabel *waitPixmap = new QLabel( this );
    waitPixmap->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, waitPixmap->sizePolicy().hasHeightForWidth() ) );
    waitPixmap->setPixmap( Resource::loadPixmap( "bigwait" ) );
    waitPixmap->setAlignment( int( QLabel::AlignCenter ) );

    QLabel *textLabel = new QLabel( this );
    textLabel->setText( tr( "<b>Finding Documents...</b>" ) );
    textLabel->setAlignment( int( QLabel::AlignCenter ) );

    docMediaScanningProgressProgress = new QProgressBar( this );
    docMediaScanningProgressProgress->setProgress( 0 );
    docMediaScanningProgressProgress->setCenterIndicator( TRUE );

    // Still got flicker, was fixed for a bit, but this doesn't seem to be
    // the right solution without changing Qt (backport newer QProgressBar?)
    // docMediaScanningProgressProgress->setBackgroundMode( NoBackground ); // No flicker
    setProgressStyle();

    QWidget *space2 = new QWidget( this );
    setStretchFactor( space2, 1 );
}

MediaScanningProgress::~MediaScanningProgress()
{
}

void MediaScanningProgress::setProgressStyle()
{
    if (docMediaScanningProgressProgress) {
	docMediaScanningProgressProgress->setFrameShape( QProgressBar::Box );
	docMediaScanningProgressProgress->setFrameShadow( QProgressBar::Plain );
	docMediaScanningProgressProgress->setMargin( 1 );
	docMediaScanningProgressProgress->setLineWidth( 1 );
    }
}

void MediaScanningProgress::setProgress( int percent )
{
    docMediaScanningProgressProgress->setProgress( percent );
}

