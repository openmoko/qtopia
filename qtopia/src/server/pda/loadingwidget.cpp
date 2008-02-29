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
#include <qtopia/config.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qpainter.h>
#include <qlabel.h>
#include "loadingwidget.h"
#include "launcherview.h"


LoadingWidget::LoadingWidget( QWidget *parent, const char *name )
    : LauncherView( parent, name )
{
    docLoadingWidgetEnabled = false;
    docLoadingWidgetProgress = 0;

    hideIcons();
    QVBox *docLoadingVBox = new QVBox( this );

    docLoadingVBox->setSpacing( 20 );
    docLoadingVBox->setMargin( 10 );

    QWidget *space1 = new QWidget( docLoadingVBox );
    docLoadingVBox->setStretchFactor( space1, 1 );

    QLabel *waitPixmap = new QLabel( docLoadingVBox );
    waitPixmap->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, waitPixmap->sizePolicy().hasHeightForWidth() ) );
    waitPixmap->setPixmap( Resource::loadPixmap( "bigwait" ) );
    waitPixmap->setAlignment( int( QLabel::AlignCenter ) );

    QLabel *textLabel = new QLabel( docLoadingVBox );
    textLabel->setText( tr( "<b>Finding Documents...</b>" ) );
    textLabel->setAlignment( int( QLabel::AlignCenter ) );

    docLoadingWidgetProgress = new QProgressBar( docLoadingVBox );
    docLoadingWidgetProgress->setProgress( 0 );
    docLoadingWidgetProgress->setCenterIndicator( TRUE );

    // Still got flicker, was fixed for a bit, but this doesn't seem to be
    // the right solution without changing Qt (backport newer QProgressBar?)
    // docLoadingWidgetProgress->setBackgroundMode( NoBackground ); // No flicker

    setProgressStyle();

    QWidget *space2 = new QWidget( docLoadingVBox );
    docLoadingVBox->setStretchFactor( space2, 1 );
}

LoadingWidget::~LoadingWidget()
{
}

void LoadingWidget::setProgressStyle()
{
    if (docLoadingWidgetProgress) {
	docLoadingWidgetProgress->setFrameShape( QProgressBar::Box );
	docLoadingWidgetProgress->setFrameShadow( QProgressBar::Plain );
	docLoadingWidgetProgress->setMargin( 1 );
	docLoadingWidgetProgress->setLineWidth( 1 );
    }
}

void LoadingWidget::setEnabled( bool v )
{
    docLoadingWidgetEnabled = v;
}

void LoadingWidget::setProgress( int percent )
{
    docLoadingWidgetProgress->setProgress( percent );
}



