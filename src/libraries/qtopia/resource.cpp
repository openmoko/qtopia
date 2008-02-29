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


#include "resource.h"
#include "config.h"
#include "mimetype.h"
#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qpixmapcache.h>
#include <qpainter.h>

#include "inlinepics_p.h"

#include "qpeapplication.h"

static bool qpe_fast_findPixmap = FALSE;

/*!
  \class Resource resource.h
  \brief The Resource class provides access to named resources.

  The resources may be provided from files or other sources.

  The allSounds() function returns a list of all the sounds available.
  A particular sound can be searched for using findSound().

  Images can be loaded with loadImage(), loadPixmap(), loadBitmap()
  and loadIconSet().

  \ingroup qtopiaemb
*/

/*!
  \fn Resource::Resource()
  \internal
*/

/*!
  Returns the QPixmap called \a pix. You should avoid including
  any filename type extension (e.g. .png, .xpm).
*/
QPixmap Resource::loadPixmap( const QString &pix )
{
    QPixmap pm;
    QString key="QPE_"+pix;
    if ( !QPixmapCache::find(key,pm) ) {
	pm.convertFromImage(loadImage(pix));
	QPixmapCache::insert(key,pm);
    }
    return pm;
}

/*!
  Returns the QBitmap called \a pix. You should avoid including
  any filename type extension (e.g. .png, .xpm).
*/
QBitmap Resource::loadBitmap( const QString &pix )
{
    QBitmap bm;
    bm = loadPixmap(pix);
    return bm;
}

/*!
  Returns the filename of a pixmap called \a pix. You should avoid including
  any filename type extension (e.g. .png, .xpm).

  Normally you will use loadPixmap() rather than this function.
*/
QString Resource::findPixmap( const QString &pix )
{
    QString picsPath = QPEApplication::qpeDir() + "pics/";

    QString f;

    // Common case optimizations...
    f = picsPath + pix + ".png";
    if ( QFile( f ).exists() )
	return f;
    f = picsPath + pix + ".xpm";
    if ( QFile( f ).exists() )
	return f;

#ifdef Q_WS_QWS
    if ( !qpe_fast_findPixmap ) {
	qDebug( "Doing slow search for image: %s", pix.latin1() );
	// All formats...
	QStrList fileFormats = QImageIO::inputFormats();
	QString ff = fileFormats.first();
	while ( fileFormats.current() ) {
	    QStringList exts = MimeType("image/"+ff.lower()).extensions();
	    for ( QStringList::ConstIterator it = exts.begin(); it!=exts.end(); ++it ) {
		QString f = picsPath + pix + "." + *it;
		if ( QFile(f).exists() )
		    return f;
	    }
	    ff = fileFormats.next();
	}

	// Finally, no (or existing) extension...
	if ( QFile( picsPath + pix ).exists() )
	    return picsPath + pix;
    }
#endif

    //qDebug("Cannot find pixmap: %s", pix.latin1());
    return QString();
}

/*!
  Returns a sound file for a sound called \a name.

  You should avoid including any filename type extension (e.g. .wav),
  as the system will search for only those fileformats which are supported
  by the library.

  Currently, only WAV files are supported.
*/
QString Resource::findSound( const QString &name )
{
    QString picsPath = QPEApplication::qpeDir() + "sounds/";

    QString result;
    if ( QFile( (result = picsPath + name + ".wav") ).exists() )
	return result;

    return QString();
}

/*!
  Returns a list of all sound names.
*/
QStringList Resource::allSounds()
{
    QDir resourcedir( QPEApplication::qpeDir() + "sounds/" , "*.wav" );
    QStringList entries = resourcedir.entryList();
    QStringList result;
    for (QStringList::Iterator i=entries.begin(); i != entries.end(); ++i)
	result.append((*i).replace(QRegExp("\\.wav"),""));
    return result;
}


/*!
  Returns the QImage called \a name. You should avoid including
  any filename type extension (e.g. .png, .xpm).

  For performance reasons, names ending in "_disabled" are only
  supported as XPM or PNG files.
*/
QImage Resource::loadImage( const QString &name)
{
    QImage img = qembed_findImage(name.latin1());
    if ( img.isNull() ) {
	// No inlined image, try file
	// For *_disabled images, only search for XPM and PNG
	bool disabled = name[(int)name.length()-1]=='d' && name.right(9)=="_disabled"; // No tr
	if ( disabled )
	    qpe_fast_findPixmap = TRUE;
	QString f = Resource::findPixmap(name);
	if ( !f.isEmpty() )
	    img.load(f);
	if ( disabled )
	    qpe_fast_findPixmap = FALSE;
    }
    return img;
}


QIconSet qtopia_internal_loadIconSet( const QString &pix ) 
// DUPLICATION from Resource::loadIconSet
{
    static int iconSetSize = -1;
    if ( iconSetSize < 0 ) {
	Config config( "qpe" );
	config.setGroup( "Appearance" );
	iconSetSize = config.readNumEntry("IconSize",0);
    }

    if ( iconSetSize ) {
	// Force the icon to iconSetSize for small icons and 50% larger fo large icons.
	QImage dimg = Resource::loadImage( pix + "_disabled" );
	QImage img = Resource::loadImage(pix);
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
	QPixmap dpm = Resource::loadPixmap( pix + "_disabled" );
	QPixmap pm = Resource::loadPixmap( pix );
	QIconSet is( pm );
	if ( !dpm.isNull() )
	    is.setPixmap( dpm, pm.width() <= 22 ? QIconSet::Small : QIconSet::Large, QIconSet::Disabled );
	return is;
    }
}
