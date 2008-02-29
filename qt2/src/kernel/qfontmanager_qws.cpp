/****************************************************************************
** $Id: qt/src/kernel/qfontmanager_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Definition of QFontFactory for Truetype class for Embedded Qt
**
** Created : 940721
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qfontmanager_qws.h"
#include "qfontfactoryttf_qws.h"
#include "qfontfactorybdf_qws.h"
#include "qfontdata_p.h"
#include "qfile.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//
// $QTDIR/lib/fonts/fontdir lists a sequence of:
//
//    <name> <file> <renderer> <italic> <weight> <size> <flags>
//
// eg.
//      <name> = Helvetica
//      <file> = /usr/local/qt-embedded/lib/fonts/helvR0810.bdf or
//                 /usr/local/qt-embedded/lib/fonts/verdana.ttf, etc.
//  <renderer> = BDF or FT
//    <italic> = y or n
//    <weight> = 50 is Normal, 75 is Bold, etc.
//      <size> = 0 for scalable or 10 time pointsize (eg. 120 for 12pt)
//     <flags> = flag characters:
//                 s = smooth (anti-aliased)
//                 u = unicode range when saving (default is Latin 1)
//                 a = ascii range when saving (default is Latin 1)
//
// and of course...
//    # This is a comment
//

QString qws_topdir()
{
    const char* r = getenv("QTDIR");
    if ( !r ) r = "/usr/local/qt-embedded";
    return r;
}

QFontManager * qt_fontmanager=0;

/*!
  \class QFontManager qfontmanager_qws.h
  \brief There is one and only one QFontManager per Qt/Embedded
  application (qt_fontmanager is a global variable that points to it).
  It keeps a list of font factories, a cache of rendered fonts and a list
  of fonts available on disk. QFontManager is called when a new font needs
  to be rendered from a Freetype-compatible or BDF font on disk; this
  only happens if there isn't an appropriate QPF font already available.
*/

/*!
  \fn void QFontManager::initialize()
  Creates a new QFontManager and points qt_fontmanager to it
*/

void QFontManager::initialize()
{
    qt_fontmanager=new QFontManager();
}

/*!
  \fn void QFontManager::cleanup()
  Destroys the font manager
*/

void QFontManager::cleanup()
{
    delete qt_fontmanager;
    qt_fontmanager = 0;
}

/*!
  \class QRenderedFont qfontmanager_qws.h
  \brief A QRenderedFont is the rendered version of a particular font (as 
  specified by QFont - that is, at a given weight,family and point size, 
  italic or not, and so on). There is one and only one QRenderedFont
  for each particular QFont specification; if you specify two Times 10pt
  bold italic QFonts they will both refer to the same QRenderedFont.
  QRenderedFonts are cached by QFontManager and are reference counted;
  when there is no QGfx referring to a particular QRenderedFont it is deleted.
  Each QRenderedFont renders glyphs (that is, images of characters) on demand
  and caches the rendered glyphs. It can be subclassed by individual factories
  since they create new QRenderedFonts.
*/

/*!
  \fn QRenderedFont::QRenderedFont(QDiskFont * df, const QFontDef &d)
  This constructs a QRenderedFont; the QDiskFont and QFontDef are needed by 
  the font factory to render glyphs.
*/

QRenderedFont::QRenderedFont(QDiskFont * df, const QFontDef &d)
{
    diskfont = df;
    ptsize=d.pointSize;
    refcount=0;
    ref();
    fleftbearing=0;
    frightbearing=0;
    fmaxwidth=0;
    smooth = df->flags.contains('s');
    if ( df->flags.contains('u') ) {
	maxchar = 65535;
    } else if ( df->flags.contains('a') ) {
	maxchar = 127;
    } else {
	maxchar = 255;
    }
}

/*!
\fn QRenderedFont::~QRenderedFont()
Destroys a QRenderedFont
*/
 
QRenderedFont::~QRenderedFont()
{
}

QFontDef QRenderedFont::fontDef() const
{
    QFontDef r;
    r.family=diskfont->name;
    r.italic=diskfont->italic;
    r.weight=diskfont->weight;
    r.pointSize=ptsize;
    return r;
}

// Triggering a whole font metrics call is bad, so right now return
// some best guesses

/*!
\fn int QRenderedFont::minLeftBearing()
Returns the minimum left bearing (distance before the start of a character)
of any character in the font. Unimplemented.
*/

int QRenderedFont::minLeftBearing()
{
    return 0;
}

/*!
\fn int QRenderedFont::minLeftBearing()
Returns the minimum right bearing (distance from the end of a character)
of any character in the font. Unimplemented.
*/

int QRenderedFont::minRightBearing()
{
    return 0;
}

/*!
\fn int QRenderedFont::maxWidth()
Returns the maximum width in pixels of any character in the font.
*/

int QRenderedFont::maxWidth()
{
    // Actually max advance
    return fmaxwidth;
}

/*!
\fn QFontManager::QFontManager()
Creates a font manager. This method reads in the font definition file
from $QTDIR/lib/fonts/fontdir (or /usr/local/qt-embedded/lib/fonts/fontdir
if QTDIR isn't defined) and creates a list of QDiskFonts to hold the 
information in the file. It also constructs any defined font factories.
*/

QFontManager::QFontManager()
{
    factories.setAutoDelete(true);
    diskfonts.setAutoDelete(true);

#ifndef QT_NO_FREETYPE
    factories.append(new QFontFactoryFT());
#endif
#ifndef QT_NO_BDF
    factories.append(new QFontFactoryBDF());
#endif

    // Load in font definition file
    QString fn = qws_topdir() + "/lib/fonts/fontdir";
    FILE* fontdef=fopen(fn.local8Bit(),"r");
    if(!fontdef) {
	QCString temp=fn.local8Bit();
	qWarning("Cannot find font definition file %s - is $QTDIR set correctly?",
	       temp.data());
	exit(1);
	//return;
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
	    QFontFactory * factoryp;
	    int weight=50;
	    int size=0;
	    flags[0]=0;
	    sscanf(buf,"%s %s %s %s %d %d %s",name,file,render,isitalic,&weight,&size,flags);
	    QString filename;
	    if ( file[0] != '/' )
		filename = qws_topdir() + "/lib/fonts/";
	    filename += file;
	    if ( QFile::exists(filename) ) {
		for(factoryp=factories.first();factoryp;factoryp=factories.next()) {
		    if( factoryp->name() == render ) {
			QDiskFont * qdf=new QDiskFont(factoryp,name,isitalic[0]=='y',
						      weight,size,flags,filename);
			diskfonts.append(qdf);
			break;
		    }
		}
	    }
	}
	fgets(buf,200,fontdef);
    }
    fclose(fontdef);
}

/*!
\fn QFontManager::~QFontManager()
Destroys the QFontManager and sets qt_fontmanager to 0.
*/

QFontManager::~QFontManager()
{
    if ( qt_fontmanager == this )
	qt_fontmanager = 0;
}

extern bool qws_savefonts; //in qapplication_qws.cpp

int QFontManager::cmpFontDef(const QFontDef & goal, const QFontDef & choice)
{
    int r = 100;
    if (goal.family.lower() == choice.family.lower())
	r += 1000;
    // Match closest weight
    r -= abs(goal.weight-choice.weight);
    // Favour italicness ahead of weight
    if (goal.italic==choice.italic)
	r += 100;
    if ( choice.pointSize ) {
	// A bit smaller is better than a bit bigger.
	if ( choice.pointSize > goal.pointSize) {
	    r += 1 - (choice.pointSize - goal.pointSize)*2;
	} else {
	    r += 1 - (goal.pointSize - choice.pointSize);
	}
    }
    return r;
}

/*!
Returns the QDiskFont that best matches \a f,
based on family, weight, italicity and font size.
*/

QDiskFont * QFontManager::get(const QFontDef & f)
{
    QDiskFont * qdf;
    QDiskFont * bestmatch=diskfonts.first();
    int bestmatchval=0;

    for(qdf=diskfonts.first();qdf;qdf=diskfonts.next()) {
	QFontDef def = qdf->fontDef();
	int mymatchval = cmpFontDef(f,def);
	if ( mymatchval>bestmatchval) {
	    bestmatchval=mymatchval;
	    bestmatch=qdf;
	}
    }

    return bestmatch;
}

/*!
  Loads the disk font as a rendered font.
*/
QRenderedFont* QDiskFont::load(const QFontDef & f)
{
    factory->load(this);
    QRenderedFont * ret = factory->get(f,this);
    return ret;
}

/*!
  Returns a QFontDef equivalent to this disk font.  The pointSize
  may be 0 indicating this is a scalable font.
*/
QFontDef QDiskFont::fontDef() const
{
    QFontDef r;
    r.family = name;
    r.italic = italic;
    r.weight = weight;
    r.pointSize = size;
    return r;
}

/*!
\fn int QRenderedFont::ascent()
Returns the font's ascent (the distance from the baseline to the top of
the tallest character)
*/

/*!
\fn int QRenderedFont::descent()
Returns the font's descent (the distance from the baseline to the bottom
of the lowest character)
*/

/*!
\fn int QRenderedFont::width(int)
Returns the width in pixels of the unicode character specified
*/

/*!
\fn int QRenderedFont::width(const QString & s,int l=-1 );
Returns the width in pixels of the first l characters of the string s,
or the whole string if the value for l is not specified. This should be
used in preference to adding up the widths of each character in the string
since it can take account of kerning and inter-character spacing
*/

/*!
\fn int leftBearing(int)
Returns the left bearing (distance in pixels before the start of the letter)
of the character specified
*/

/*!
\fn int QRenderedFont::rightBearing(int)
Returns the right bearing (distance in pixels after the end of the letter)
of the character specified.
*/

/*!
\fn bool QRenderedFont::inFont(QChar ch)
Returns true if the unicode character ch is in the font.
*/

/*!
\fn virtual QGlyph QRenderedFont::render(QChar ch)
Renders the unicode character ch, returning a QGlyph.
A QGlyph has two members, metrics and data. Metrics contains
information on the size, advance width and so forth of the character,
data a pointer to the raw data for the character - either a 1 bit per pixel
bitmap or an 8 bit per pixel alpha map of the character, the linestep
of which is specified in QGlyphMetrics.
*/
