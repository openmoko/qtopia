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

#define QTOPIA_INTERNAL_MIMEEXT

#include <qtopia/resource.h>
#include <qtopia/config.h>

/*!
  Returns a QIconSet for the pixmap named \a name.
  You should avoid including any filename type extension (eg. .png, .xpm).
  The icon size is determined by the Qtopia font setting.
*/
QIconSet Resource::loadIconSet( const QString &pix ) 
{
    static int iconSetSize = -1;

    if ( iconSetSize < 0 ) {
	Config config( "qpe" );
	config.setGroup( "Appearance" );
	iconSetSize = config.readNumEntry("IconSize",0);
    }

    if ( iconSetSize ) {
	// Force the icon to iconSetSize for small icons and 50% larger fo large icons.
	QImage dimg = loadImage( pix + "_disabled" );
	QImage img = loadImage(pix);
	QPixmap pm;

	QIconSet is;
	if ( img.height()*2 >= iconSetSize*3 ) {
	    pm.convertFromImage(img.smoothScale(iconSetSize,iconSetSize));
	    is.reset(pm,QIconSet::Small);
	    pm.convertFromImage(img.smoothScale(iconSetSize*3/2,iconSetSize*3/2));
	    is.setPixmap(pm,QIconSet::Large);
	    if ( !dimg.isNull() ) {
		pm.convertFromImage(dimg.smoothScale(iconSetSize,iconSetSize));
		is.setPixmap(pm,QIconSet::Small,QIconSet::Disabled);
		pm.convertFromImage(dimg.smoothScale(iconSetSize*3/2,iconSetSize*3/2));
		is.setPixmap(pm,QIconSet::Large,QIconSet::Disabled);
	    }
	} else {
	    pm.convertFromImage(img.smoothScale(iconSetSize,iconSetSize));
	    is.reset(pm,QIconSet::Small);
	    if ( !dimg.isNull() ) {
		pm.convertFromImage(dimg.smoothScale(iconSetSize,iconSetSize));
		is.setPixmap(pm,QIconSet::Small,QIconSet::Disabled);
	    }
	}
	return is;
    } else {
	QPixmap dpm = loadPixmap( pix + "_disabled" );
	QPixmap pm = loadPixmap( pix );
	QIconSet is( pm );
	if ( !dpm.isNull() )
	    is.setPixmap( dpm, pm.width() <= 22 ? QIconSet::Small : QIconSet::Large, QIconSet::Disabled );
	return is;
    }
}


