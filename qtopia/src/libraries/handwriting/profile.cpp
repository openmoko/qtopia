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

#define QTOPIA_INTERNAL_LOADTRANSLATIONS
#include "combining.h"
#include "profile.h"

#include <qfileinfo.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>


QIMPenProfile::QIMPenProfile( const QString &fn )
    : filename( fn )
{
    sets.setAutoDelete( true );

    load();
}

void QIMPenProfile::load()
{
#ifndef QT_NO_TRANSLATION
    static int translation_installed = 0;
    if (!translation_installed) {
        QPEApplication::loadTranslations("libqmstroke");
        translation_installed++;
    }
#endif
    
    Config config( filename, Config::File );
    config.setGroup( "Handwriting" );

    pname = config.readEntry( "Name" );
    pdesc = config.readEntry( "Description" );

    tstyle = config.readBoolEntry( "CanSelectStyle", false );
    istyle = config.readBoolEntry( "CanIgnoreStroke", false );

    wordMatch = config.readBoolEntry( "MatchWords", true );

    config.setGroup( "Settings" );

    pstyle = BothCases;
    QString s = config.readEntry( "Style", "BothCases" );
    if ( s == "ToggleCases" )
	pstyle = ToggleCases;

    msTimeout = config.readNumEntry( "MultiTimeout", 500 );
    isTimeout = config.readNumEntry( "IgnoreTimeout", 200 );

    // Read user configuration
    Config usrConfig( userConfig() );
    usrConfig.setGroup( "Settings" );
    msTimeout = usrConfig.readNumEntry( "MultiTimeout", msTimeout );

    if ( tstyle && usrConfig.hasKey( "Style" ) ) {
	pstyle = BothCases;
	QString s = usrConfig.readEntry( "Style", "BothCases" );
	if ( s == "ToggleCases" )
	    pstyle = ToggleCases;
    }
    if ( istyle && usrConfig.hasKey( "IgnoreTimeout" ) ) {
	isTimeout = usrConfig.readNumEntry( "IgnoreTimeout", isTimeout );
    }
}

void QIMPenProfile::save() const
{
    Config usrConfig( userConfig() );
    usrConfig.setGroup("Settings");
    usrConfig.writeEntry("MultiTimeout", msTimeout);

    if (tstyle)
	usrConfig.writeEntry("Style", pstyle == BothCases ? "BothCases" : "ToggleCases" );
    if (istyle)
	usrConfig.writeEntry("IgnoreTimeout", isTimeout);

    // each set knows if its modified.
    if (!sets.isEmpty())
	saveData();
}

void QIMPenProfile::setStyle( Style s )
{
    if ( tstyle && s != pstyle ) {
	pstyle = s;
    }
}

void QIMPenProfile::setMultiStrokeTimeout( int t )
{
    if ( t != msTimeout ) {
	msTimeout = t;
    }
}

void QIMPenProfile::setIgnoreStrokeTimeout( int t )
{
    if ( t != isTimeout ) {
	isTimeout = t;
    }
}

const QString QIMPenProfile::identifier() const
{
    QFileInfo fi(filename);
    return fi.baseName();
}

QString QIMPenProfile::userConfig() const
{
    return "handwriting-" + identifier();
}

static const char* untranslatedProfileNames[] = {
    QT_TRANSLATE_NOOP( "QIMPenProfile", "Uppercase" ),
    QT_TRANSLATE_NOOP( "QIMPenProfile", "Lowercase" ),
    QT_TRANSLATE_NOOP( "QIMPenProfile", "Numeric" ),
    QT_TRANSLATE_NOOP( "QIMPenProfile", "Punctuation" ),
    QT_TRANSLATE_NOOP( "QIMPenProfile", "Symbol" ),
    QT_TRANSLATE_NOOP( "QIMPenProfile", "Shortcut" )
};

void QIMPenProfile::loadData()
{
    Config config( filename, Config::File );
    config.setGroup( "CharSets" );

    // accents
    QIMPenCombining *combining = 0;
    QString s = config.readEntry( "Combining" );
    if ( !s.isEmpty() ) {
	combining = new QIMPenCombining( s );
	if ( combining->isEmpty() ) {
	    delete combining;
	    combining = 0;
	}
    }
    // uppercase latin1
    QIMPenCharSet *cs = 0;
    ProfileSet *ps = 0;
    s = config.readEntry( "Uppercase" );
    if ( !s.isEmpty() ) {
	cs = new QIMPenCharSet( s );
	if ( !cs->isEmpty() ) {
	    if ( combining )
		combining->addCombined( cs );
	    ps = new ProfileSet(cs);
	    ps->id = "Uppercase"; // no tr
	    sets.append( ps );
	} else {
	    delete cs;
	}
    }
    // lowercase latin1
    s = config.readEntry( "Lowercase" );
    if ( !s.isEmpty() ) {
	cs = new QIMPenCharSet( s );
	if ( !cs->isEmpty() ) {
	    if ( combining )
		combining->addCombined( cs );
	    ps = new ProfileSet(cs);
	    ps->id = "Lowercase"; // no tr
	    sets.append( ps );
	} else {
	    delete cs;
	}
    }
    // numeric (may comtain punctuation and symbols)
    s = config.readEntry( "Numeric" );
    if ( !s.isEmpty() ) {
	cs = new QIMPenCharSet( s );
	if ( !cs->isEmpty() ) {
	    ps = new ProfileSet(cs);
	    ps->id = "Numeric"; // no tr
	    sets.append( ps );
	} else {
	    delete cs;
	}
    }
    // punctuation
    s = config.readEntry( "Punctuation" );
    if ( !s.isEmpty() ) {
	cs = new QIMPenCharSet( s );
	if ( !cs->isEmpty() ) {
	    ps = new ProfileSet(cs);
	    ps->id = "Punctuation"; // no tr
	    sets.append( ps );
	} else {
	    delete cs;
	}
    }
    // symbol
    s = config.readEntry( "Symbol" );
    if ( !s.isEmpty() ) {
	cs = new QIMPenCharSet( s );
	if ( !cs->isEmpty() ) {
	    ps = new ProfileSet(cs);
	    ps->id = "Symbol"; // no tr
	    sets.append( ps );
	} else {
	    delete cs;
	}
    }
    // shortcut
    s = config.readEntry( "Shortcut" );
    if ( !s.isEmpty() ) {
	cs = new QIMPenCharSet( s );
	if ( !cs->isEmpty() ) {
	    ps = new ProfileSet(cs);
	    ps->id = "Shortcut"; // no tr
	    sets.append( ps );
	} else {
	    delete cs;
	}
    }
    // now read sets that are specified in list variable.

    config.setGroup( "Handwriting" );
    QStringList customSets = config.readListEntry("customSets", ',');
    QStringList::Iterator it;
    for ( it = customSets.begin(); it != customSets.end(); ++it ) {
	QString id(*it);
	config.setGroup(id);
	//qDebug("loading custom set %s (%s : %s)", id.latin1(), config.readEntry("Title").latin1(), config.readEntry("Data").latin1());
	cs = new QIMPenCharSet( config.readEntry("Data") );
	if ( !cs->isEmpty() ) {
	    if ( combining &&
		(cs->type() & (QIMPenCharSet::Lower | QIMPenCharSet::Upper)) != 0)
		combining->addCombined( cs );
	    ps = new ProfileSet(cs);
	    ps->id = id;
	    sets.append( ps );
	} else {
	    delete cs;
	}
    }

    if ( combining )
	delete combining;
}

void QIMPenProfile::saveData() const
{
    ProfileSetListIterator it( sets );
    for ( ; it.current(); ++it )
	it.current()->set->save();
}

/*!
  returns the set for the set identified by \a id.
*/
QIMPenCharSet *QIMPenProfile::charSet( const QString &id )
{
    if ( sets.isEmpty() )
	loadData();
    ProfileSetListIterator it( sets );
    for ( ; it.current(); ++it ) {
	if (it.current()->id == id)
	    return it.current()->set;
    }
    return 0;
}

/*!
  Returns the first set in list that matches the type given
*/
QIMPenCharSet *QIMPenProfile::find( QIMPenCharSet::Type t )
{
    if ( sets.isEmpty() )
	loadData();
    ProfileSetListIterator it( sets );
    for ( ; it.current(); ++it ) {
	if (it.current()->set->type() == t)
	    return it.current()->set;
    }
    return 0;
}

/*!
  Returns the translated title for the set identified by \a id.
*/
QString QIMPenProfile::title( const QString &id )
{
    if ( sets.isEmpty() )
	loadData();
    Config config( filename, Config::File );

    // Try from config file
    config.setGroup( id );
    if (config.hasKey("Title"))
	return config.readEntry("Title", id);
    // Try from Profile Translations
    config.setGroup( "CharSets" );
    if (config.hasKey(id) && qApp)
	return qApp->translate("QIMPenProfile", id);
    return id;
}

/*!
  Returns the translated description for the set identified by \a id.
*/
QString QIMPenProfile::description( const QString &id )
{
    if ( sets.isEmpty() )
	loadData();
    Config config( filename, Config::File );

    // Try from config file
    config.setGroup( id );
    if (config.hasKey("Description"))
	return config.readEntry("Description", id);
    return QString::null;
}

/*!
  Returns the list of identifiers for sets loaded by this profile.
*/
QStringList QIMPenProfile::charSets()
{
    if ( sets.isEmpty() )
	loadData();
    QStringList result;
    ProfileSetListIterator it( sets );
    for ( ; it.current(); ++it ) {
	result.append(it.current()->id);
    }
    return result;
}

QIMPenCharSet *QIMPenProfile::uppercase()
{
    return find(QIMPenCharSet::Upper);
}

QIMPenCharSet *QIMPenProfile::lowercase()
{
    return find(QIMPenCharSet::Lower);
}

QIMPenCharSet *QIMPenProfile::numeric()
{
    return find(QIMPenCharSet::Numeric);
}

QIMPenCharSet *QIMPenProfile::punctuation()
{
    return find(QIMPenCharSet::Punctuation);
}

QIMPenCharSet *QIMPenProfile::symbol()
{
    return find(QIMPenCharSet::Symbol);
}

QIMPenCharSet *QIMPenProfile::shortcut()
{
    return find(QIMPenCharSet::Shortcut);
}
