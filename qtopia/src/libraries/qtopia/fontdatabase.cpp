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

#include "pluginloader_p.h"
#include "qpeapplication.h"
#include "fontfactoryinterface.h"
#include "fontdatabase.h"

#ifdef QWS
#include <qfontmanager_qws.h>
#endif
#include <qdir.h>
#include <qdict.h>
#include <stdio.h>
#include <stdlib.h>

static QString fontDir()
{
#ifndef Q_OS_WIN32
    QString qtdir = getenv("QTDIR");
    if ( qtdir.isEmpty() ) qtdir = "/usr/local/qt-embedded";
    return qtdir+"/lib/fonts/";
#else
    QString fontDir, qtDir;
    if ( getenv("QTDIR") )
	qtDir = QString(getenv("QTDIR")).stripWhiteSpace();
    if ( qtDir.isNull() || qtDir.isEmpty() )
	fontDir = QPEApplication::qpeDir();
    else
	fontDir = qtDir + QDir::separator();
    QDir d (fontDir + "lib/fonts/");
    if (d.exists())
	fontDir.append("lib/fonts/");
    else
	fontDir.append("dist/embedded/lib/fonts/"); // internal development	    
    return fontDir;
#endif
}

#ifdef QT_NO_FONTDATABASE
static QString fontFamily( const QString& key )
{
    int u0 = key.find('_');
    int u1 = key.find('_',u0+1);
    int u2 = key.find('_',u1+1);
    QString family = key.left(u0);
    //int pointSize = key.mid(u0+1,u1-u0-1).toInt();
    //int weight = key.mid(u1+1,u2-u1-1).toInt();
    //bool italic = key.mid(u2-1,1) == "i";
    // #### ignores _t and _I fields
    return family;
}
#endif

QValueList<FontFactory> *FontDatabase::factoryList = 0;
PluginLoaderIntern *FontDatabase::loader = 0;

/*!
  \class FontDatabase fontdatabase.h
  \brief The FontDatabase class provides information about available fonts.

  Most often you will simply want to query the database for the
  available font families().

  Use FontDatabase rather than QFontDatabase when you may need access
  to fonts that are not normally available. For example, if the
  freetype library and the Qtopia freetype plugin are installed,
  TrueType fonts will be available to your application. Font renderer
  plugins have greater resource requirements than system fonts so they
  should be used only when necessary. You can force the loading of
  font renderer plugins with loadRenderers().

  Note that on the SHARP SL5500, some fonts return pointSizes() that
  are 10 times too large. A heuristic for detecting this case is that
  the first size in the list is more than 70.

  \ingroup qtopiaemb
*/

/*!
  Constructs a FontDatabase object.
*/
FontDatabase::FontDatabase()
#ifndef QT_NO_FONTDATABASE
    : QFontDatabase()
#endif
{
    if ( !factoryList )
	loadRenderers();
}

/*!
  Returns a list of names of all the available font families.
*/
QStringList FontDatabase::families() const
{
#ifndef QT_NO_FONTDATABASE
    QStringList f = QFontDatabase::families();
    QStringList r;
    for (QStringList::ConstIterator it=f.begin(); it!=f.end(); ++it) {
	QString s = *it;
	int dash=s.find('-');
	if ( dash>=0 ) {
	    // Skip foundry-based duplicates.
	    // This allows a "font family" to include fonts from
	    // different foundries. In particular, it allows bitmap
	    // and TTF versions of fonts.
	    s=s.mid(dash+1);
	    if ( r.contains(s) )
		continue;
	}
	r.append(s);
    }
    return r;
#else
    QStringList list;
    QDict<void> familyDict;
    QDiskFont *qdf;
    for ( qdf=qt_fontmanager->diskfonts.first(); qdf!=0;
            qdf=qt_fontmanager->diskfonts.next()) {
	QString familyname = qdf->name;
	if ( !familyDict.find( familyname ) ) {
	    familyDict.insert( familyname, (void *)1 );
	    list.append( familyname );
	}
    }

    QDir dir(fontDir(),"*.qpf");
    for (int i=0; i<(int)dir.count(); i++) {
	QString familyname = fontFamily(dir[i]);
	if ( !familyDict.find( familyname ) ) {
	    familyDict.insert( familyname, (void *)1 );
	    list.append( familyname );
	}
    }

    return list;
#endif
}

#ifdef QT_NO_FONTDATABASE
/*!
  Returns a list of standard fontsizes.
*/
QValueList<int> FontDatabase::standardSizes()
{
    static int s[]={ 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28,
		     36, 48, 72, 0 };
    static bool first = TRUE;
    static QValueList<int> sList;
    if ( first ) {
	first = FALSE;
	int i = 0;
	while( s[i] )
	    sList.append( s[i++] );
    }
    return sList;
}

#endif

/*!
  Load any font renderer plugins that are available and make the fonts
  that the plugins can read available.
*/
void FontDatabase::loadRenderers()
{
#if !defined(QT_NO_COMPONENT) && (QT_VERSION-0 < 0x030000)
    if ( !factoryList )
	factoryList = new QValueList<FontFactory>;
    if ( !loader )
	loader = new PluginLoaderIntern("fontfactories");

    QValueList<FontFactory>::Iterator mit;
    for ( mit = factoryList->begin(); mit != factoryList->end(); ++mit ) {
	qt_fontmanager->factories.setAutoDelete( false );
	qt_fontmanager->factories.removeRef( (*mit).factory );
	qt_fontmanager->factories.setAutoDelete( true );
	loader->releaseInterface((*mit).interface);
    }
    factoryList->clear();

    QStringList list = loader->list();
    QStringList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
	FontFactoryInterface *iface = 0;
	if ( loader->queryInterface( *it, IID_FontFactory, (QUnknownInterface**)&iface ) == QS_OK ) {
	    FontFactory factory;
	    factory.interface = iface;
	    factory.factory = factory.interface->fontFactory();
	    factoryList->append( factory );
	    qt_fontmanager->factories.append( factory.factory );
	    readFonts( factory.factory );
	}
    }
#endif
}

/*!
  \internal
*/
void FontDatabase::readFonts( QFontFactory *factory )
{
    // Load in font definition file(s)
    QDir fd( fontDir(), "fontdir.*" );
    QStringList fontdirs = fd.entryList();
    fontdirs.prepend( "fontdir" );

    QStringList::Iterator it;
    for ( it = fontdirs.begin(); it != fontdirs.end(); ++it ) {
	QString fn = fontDir() + *it;
	FILE* fontdef=fopen(fn.local8Bit(),"r");
	if(!fontdef) {
	    QCString temp=fn.local8Bit();
	    qWarning("Cannot find font definition file %s - is $QTDIR set correctly?",
		   temp.data());
	    continue;
	}
	char buf[200]="";
	char name[200]="";
	char render[200]="";
	char file[200]="";
	char flags[200]="";
	char isitalic[10]="";
	fgets(buf,200,fontdef);
	while(!feof(fontdef)) {
	    if ( buf[0] != '#' ) {
		int weight=50;
		int size=0;
		flags[0]=0;
		sscanf(buf,"%s %s %s %s %d %d %s",name,file,render,isitalic,&weight,&size,flags);
		QString filename;
		if ( file[0] != '/' )
		    filename = fontDir();
		filename += file;
		if ( QFile::exists(filename) ) {
		    if( factory->name() == render ) {
			QDiskFont * qdf=new QDiskFont(factory,name,isitalic[0]=='y',
						      weight,size,flags,filename);
			qt_fontmanager->diskfonts.append(qdf);
#if QT_VERSION >= 232
			QFontDatabase::qwsAddDiskFont( qdf );
#endif
		    }
		}
	    }
	    fgets(buf,200,fontdef);
	}
	fclose(fontdef);
    }
}

