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

#define QTOPIA_INTERNAL_MIMEEXT
#define QTOPIA_INTERNAL_LANGLIST

#include "resource.h"
#include "config.h"
#include "mimetype.h"
#include "qpeapplication.h"
#include "global.h"

#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>
#include <qpixmapcache.h>
#include <qpainter.h>

#ifdef QTOPIA_DESKTOP
#include <qdconfig.h>
#endif

bool qpe_fast_findPixmap = FALSE;
QStringList qpe_pathCache[Resource::AllResources + 1];

/*!
  \class Resource resource.h
  \brief The Resource class provides access to named resources.

  The resources may be provided from files or other sources.

  The allSounds() function returns a list of all the sounds available.
  A particular sound can be searched for using findSound().

  Images can be loaded with loadImage(), loadPixmap(), loadBitmap()
  and loadIconSet() and will be put into Qt's
  <a href=http://doc.trolltech.com/2.3/qpixmapcache.html>Pixmap Cache</a>.

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

    // Give the pixmaps some kind of namespace in the pixmapcache
    int index = pix.find('/');
    QString appName1 = qApp->argv()[0];
    appName1 = appName1.replace(QRegExp(".*/"),"");
    QString appName2 = pix.left(index);

    if ( appName2 == "" || appName2 == pix || appName2 == "icons" )
	appName2 = "Global";

    QString appKey1 = "_QPE_" + appName1 + "_" + pix;
    QString appKey2 = "_QPE_" + appName2 + "_" + pix.mid(index+1);

    if ( !QPixmapCache::find(appKey1, pm) ) {
	if ( !QPixmapCache::find(appKey2, pm) ) {
	    QImage img;
	    QString f = findPixmap( pix );
	    if ( !f.isEmpty() ) {
		img.load(f);
		if ( !img.isNull() ) {
		    pm.convertFromImage(img);
		    if ( f.contains(appName1) ) {
			QPixmapCache::insert( appKey1, pm);
		    } else {
			QPixmapCache::insert( appKey2, pm);
		    }
		}
	    }
	}
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

#include "global_qtopiapaths.cpp"

/*!
  Returns the filename of a pixmap called \a pix. You should avoid including
  any filename type extension (e.g. .png, .xpm).

  Normally you will use loadPixmap() rather than this function.
*/
QString Resource::findPixmap( const QString &pix )
{
    QString f;
    QStringList picsPaths(qpe_pathCache[PixResource]);
    QStringList qpepaths = global_qtopiapaths();
    for (QStringList::ConstIterator qit=qpepaths.begin(); qit!=qpepaths.end(); ++qit)
	picsPaths.append(*qit + "pics/");
#ifdef QTOPIA_DESKTOP
    picsPaths.append(QDir::homeDirPath() + "/" + gQtopiaDesktopConfig->dotpalmtopcenter() + "DocumentIconsCache");
#endif
    QString appName = qApp->argv()[0];
    appName.replace(QRegExp(".*/"),"");
#ifdef QTOPIA_DESKTOP
    // Since Qtopia Desktop runs on platforms where the binary isn't
    // called qtopiadesktop, we need to force it here.
    appName = "qtopiadesktop";
    QString currentPlugin = gQtopiaDesktopConfig->currentPlugin();
#endif

    // Common case optimizations...
    QStringList::ConstIterator pathIt;
    //translatable icons
    //if we cant find pic for current lang use en_US as last resort
    QStringList langs = Global::languageList();
    if (!langs.contains("en_US")) 
        langs.append("en_US");
    for ( pathIt = picsPaths.begin(); pathIt!=picsPaths.end(); ++pathIt ) {
	QString prefix = *pathIt;
	if (prefix[(int)prefix.length()-1] != '/')
	    prefix += '/';
#ifdef QTOPIA_DESKTOP
	if ( ! currentPlugin.isEmpty() ) {
	    f = prefix + currentPlugin + '/' + pix + ".png";
	    if ( QFile::exists(f) )
		return f;
	}
#endif
	f = prefix + appName + '/' + pix + ".png";
	if ( QFile::exists(f) )
	    return f;
	f = prefix + pix + ".png";
	if ( QFile::exists(f) )
	    return f;
#ifdef QTOPIA_DESKTOP
	if ( ! currentPlugin.isEmpty() ) {
	    f = prefix + currentPlugin + '/' + pix + ".xpm";
	    if ( QFile::exists(f) )
		return f;
	}
#endif
	f = prefix + appName + '/' +pix + ".xpm";
	if ( QFile::exists(f) )
	    return f;
	f = prefix + pix + ".xpm";
	if ( QFile::exists(f) )
	    return f;
        
	for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
	    QString lang = "i18n/" + (*it);
#ifdef QTOPIA_DESKTOP
            if ( ! currentPlugin.isEmpty() ) {
                f = prefix + currentPlugin + '/' + lang + '/' + pix + ".png";
                if ( QFile::exists(f) )
                    return f;
            }
#endif
            f = prefix + appName + '/' + lang + '/' + pix + ".png";
            if (QFile::exists(f))
                return f;
            f = prefix + lang + '/' + pix + ".png";
            if (QFile::exists(f))
                return f;

#ifdef QTOPIA_DESKTOP
            if ( ! currentPlugin.isEmpty() ) {
                f = prefix + currentPlugin + '/' + lang + '/' + pix + ".xpm";
                if ( QFile::exists(f) )
                    return f;
            }
#endif
            f = prefix + appName + '/' + lang + '/' + pix + ".xpm";
            if (QFile::exists(f))
                return f;
            f = prefix + lang + '/' + pix + ".xpm";
            if (QFile::exists(f))
                return f;
        }
    }

#ifdef Q_WS_QWS
    if ( !qpe_fast_findPixmap ) {
	qDebug( "Doing slow search for image: %s", pix.latin1() );
	// All formats...
	QString png_ext("png");
	QString xpm_ext("xpm");
	QStrList fileFormats = QImageIO::inputFormats();
	QString ff = fileFormats.first();
	while ( fileFormats.current() ) {
	    QStringList exts = MimeType("image/"+ff.lower()).extensions();
	    for ( QStringList::ConstIterator it = exts.begin(); it!=exts.end(); ++it ) {
		if (*it == png_ext || *it == xpm_ext)	// check above.
		    continue;
		for ( pathIt = picsPaths.begin(); pathIt!=picsPaths.end(); ++pathIt ){
		    QString prefix = *pathIt;
		    if (prefix[prefix.length()-1] != '/')
			prefix += '/';
		    f = prefix + appName + "/" + pix + "." + *it;
		    if ( QFile::exists(f) )
			return f;
		    f = prefix + pix + "." + *it;
		    if ( QFile::exists(f) )
			return f;
                    for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
                        QString lang = "i18n/" + (*it);
                        f = prefix + appName + '/' + lang + '/' + pix + '.' + *it;
                        if ( QFile::exists(f) )
                            return f;
                        f = prefix + lang + '/' + pix + '.' + *it;
                        if ( QFile::exists(f) )
                            return f;
                         
                    }
		}
	    }
	    ff = fileFormats.next();
	}

	// Finally, no (or existing) extension...
	for ( pathIt = picsPaths.begin(); pathIt!=picsPaths.end(); ++pathIt ){
	    QString prefix = *pathIt;
	    if (prefix[prefix.length()-1] != '/')
		prefix += '/';
	    f = prefix + pix;
	    if ( QFile::exists(f) )
		return f;
	    f = prefix + appName + "/" + pix;
	    if ( QFile::exists(f) )
		return f;
            for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
                QString lang = "i18n/" + (*it);
                f = prefix + appName + '/' + lang + "/" + pix;
                if ( QFile::exists(f) )
                    return f;
                f = prefix + lang + '/' + pix;
                if ( QFile::exists(f) )
                    return f;
            }
	}
    }
#endif

    qDebug("Cannot find pixmap: %s", pix.latin1());
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
    QStringList soundPaths(qpe_pathCache[SoundResource]);
    QStringList qpepaths = global_qtopiapaths();
    for (QStringList::ConstIterator qit=qpepaths.begin(); qit!=qpepaths.end(); ++qit)
	soundPaths.append(*qit + "sounds/");

    QString result;
    for ( QStringList::ConstIterator soundPrefix = soundPaths.begin(); soundPrefix!=soundPaths.end(); ++soundPrefix ){
	result = *soundPrefix + name + ".wav";
	if ( QFile::exists(result) )
	    return result;
    }

    return QString();
}

/*!
  Returns a list of all sound names.
*/
QStringList Resource::allSounds()
{
    QStringList result, entries;
    QStringList soundPaths(qpe_pathCache[SoundResource]);
    QStringList qpepaths = global_qtopiapaths();
    for (QStringList::ConstIterator qit=qpepaths.begin(); qit!=qpepaths.end(); ++qit)
	soundPaths.append(*qit + "sounds/");
    for ( QStringList::ConstIterator soundPrefix = soundPaths.begin(); soundPrefix!=soundPaths.end(); ++soundPrefix ){
	QDir resourcedir(*soundPrefix, "*.wav");
	entries += resourcedir.entryList();
    }
    
    for (QStringList::Iterator i=entries.begin(); i != entries.end(); ++i)
	result.append((*i).replace(QRegExp("\\.wav"),""));
    return result;
}


/*!
  Returns the QImage called \a name. You should avoid including
  any filename type extension (e.g. .png, .xpm).
*/
QImage Resource::loadImage( const QString &name)
{
    QImage img;
    QString f = Resource::findPixmap(name);
    if ( !f.isEmpty() )
	img.load(f);
    return img;
}

#include "../qtopia1/qpe_load_iconset.cpp"

QIconSet qtopia_internal_loadIconSet( const QString &pix ) 
{
    return qpe_loadIconSet(pix);
}
