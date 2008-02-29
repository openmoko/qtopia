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

#include <qfile.h>
#include <qtl.h>
#include <math.h>
#include <limits.h>
#include <errno.h>
#include <qdatastream.h>
#ifdef Q_WS_QWS
#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>
#else
#include <qapplication.h>
#endif
#include "combining.h"
#include "char.h"

#define QIMPEN_MATCH_THRESHOLD	    200000

const QIMPenSpecialKeys qimpen_specialKeys[] = {
    { Qt::Key_Escape,		QT_TRANSLATE_NOOP("Handwriting","[Esc]") },
    { Qt::Key_Tab,		QT_TRANSLATE_NOOP("Handwriting","[Tab]") },
    { Qt::Key_Backspace,	QT_TRANSLATE_NOOP("Handwriting","[BackSpace]") },
    { Qt::Key_Return,		QT_TRANSLATE_NOOP("Handwriting","[Return]") },
    { QIMPenChar::Caps,		QT_TRANSLATE_NOOP("Handwriting","[Uppercase]") },
    { QIMPenChar::CapsLock,	QT_TRANSLATE_NOOP("Handwriting","[Caps Lock]") },
    { QIMPenChar::Shortcut,	QT_TRANSLATE_NOOP("Handwriting","[Shortcut]") },
    { QIMPenChar::Punctuation,  QT_TRANSLATE_NOOP("Handwriting","[Punctuation]") },
    { QIMPenChar::Symbol,	QT_TRANSLATE_NOOP("Handwriting","[Symbol]") },
    { QIMPenChar::NextWord,	QT_TRANSLATE_NOOP("Handwriting","[Next Word]") },
    { QIMPenChar::WordPopup,	QT_TRANSLATE_NOOP("Handwriting","[Word Menu]") }, // word popup
    { QIMPenChar::SymbolPopup,	QT_TRANSLATE_NOOP("Handwriting","[Symbol Menu]") },
    { QIMPenChar::ModePopup,	QT_TRANSLATE_NOOP("Handwriting","[Mode Menu]") },
    { Qt::Key_unknown,	        0 } };

/*!
  \class QIMPenChar char.h
  \brief The QIMPenChar class handles a single character.

  Can calculate closeness of match to
  another character.
*/

QIMPenChar::QIMPenChar()
{
    flags = 0;
    strokes.setAutoDelete( TRUE );
}

QIMPenChar::QIMPenChar( const QIMPenChar &chr )
{
    strokes.setAutoDelete( TRUE );
    ch = chr.ch;
    flags = chr.flags;
    QIMPenStrokeIterator it( chr.strokes );
    while ( it.current() ) {
        strokes.append( new QIMPenStroke( *it.current() ) );
        ++it;
    }
}

QIMPenChar &QIMPenChar::operator=( const QIMPenChar &chr )
{
    strokes.clear();
    ch = chr.ch;
    flags = chr.flags;
    QIMPenStrokeIterator it( chr.strokes );
    while ( it.current() ) {
        strokes.append( new QIMPenStroke( *it.current() ) );
        ++it;
    }

    return *this;
}

QString QIMPenChar::name() const
{
    return name(ch);
}

QString QIMPenChar::name(unsigned int ch)
{
    QString n;

    if ( (ch & 0x0000FFFF) == 0 ) {
	int code = ch >> 16;
	for ( int i = 0; qimpen_specialKeys[i].code != Qt::Key_unknown; i++ ) {
	    if ( qimpen_specialKeys[i].code == code ) {
		n = qApp->translate("Handwriting", qimpen_specialKeys[i].name);
		break;
	    }
	}
    } else {
	n = QChar( ch & 0x0000FFFF );
    }

    return n;
}

void QIMPenChar::clear()
{
    ch = 0;
    flags = 0;
    strokes.clear();
}

unsigned int QIMPenChar::strokeLength( int s ) const
{
    QIMPenStrokeIterator it( strokes );
    while ( it.current() && s ) {
	++it;
	--s;
    }

    if ( it.current() )
	return it.current()->length();

    return 0;
}

/*!
  Add a stroke, \a st, to the character
*/
void QIMPenChar::addStroke( QIMPenStroke *st )
{
    QIMPenStroke *stroke = new QIMPenStroke( *st );
    strokes.append( stroke );
}

/*!
  Return an indicator of the closeness of this character to \a pen.
  Lower value is better.
*/
int QIMPenChar::match( QIMPenChar *pen )
{
/*
    if ( strokes.count() > pen->strokes.count() )
        return INT_MAX;
*/
    int err = 0;
    int maxErr = 0;
    int diff = 0;
    QIMPenStrokeIterator it1( strokes );
    QIMPenStrokeIterator it2( pen->strokes );
    err = it1.current()->match( it2.current() );
    if ( err > maxErr )
	maxErr = err;
    ++it1;
    ++it2;
    /* currently the multi-stroke gravity checks are not
       canvas height dependent, they should be */
    while ( err < 400000 && it1.current() && it2.current() ) {
	// the difference between the center of this stroke
	// and the center of the first stroke.
        QPoint p1 = it1.current()->boundingRect().center() -
		    strokes.getFirst()->boundingRect().center();

	// scale to canvas height
	p1 = p1 * 75 / it1.current()->canvasHeight();

	// the difference between the center of this stroke
	// and the center of the first stroke.
        QPoint p2 = it2.current()->boundingRect().center() -
		    pen->strokes.getFirst()->boundingRect().center();

	// scale to canvas height
	p1 = p1 * 75 / it1.current()->canvasHeight();

        int xdiff = QABS( p1.x() - p2.x() ) - 6;
        int ydiff = QABS( p1.y() - p2.y() ) - 5;
	if ( xdiff < 0 )
	    xdiff = 0;
	if ( ydiff < 0 )
	    ydiff = 0;
	if ( xdiff > 10 || ydiff > 10 ) { // not a chance
#ifdef DEBUG_QIMPEN
	    qDebug( "char %c, stroke starting pt diff excessive", pen->ch );
#endif
	    return INT_MAX;
	}
        diff += xdiff*xdiff + ydiff*ydiff;
	err = it1.current()->match( it2.current() );
	if ( err > maxErr )
	    maxErr = err;
        ++it1;
        ++it2;
    }

    maxErr += diff * diff * 6; // magic weighting :)

#ifdef DEBUG_QIMPEN
    qDebug( "char: %c, maxErr %d, diff %d, (%d)", pen->ch, maxErr, diff, strokes.count() );
#endif
    return maxErr;
}

/*!
  Return the bounding rect of this character.  It may have sides with
  negative coords since its origin is where the user started drawing
  the character.
*/
QRect QIMPenChar::boundingRect()
{
    QRect br;
    QIMPenStroke *st = strokes.first();
    while ( st ) {
        br |= st->boundingRect();
        st = strokes.next();
    }

    return br;
}


/*!
  Write the character's data to the stream.
*/
QDataStream &operator<< (QDataStream &s, const QIMPenChar &ws)
{
    s << ws.ch;

    // never write data, its old hat.
    if ( ws.flags & QIMPenChar::Data )
	s << (ws.flags ^ QIMPenChar::Data);
    else 
	s << ws.flags;
    s << ws.strokes.count();
    QIMPenStrokeIterator it( ws.strokes );
    while ( it.current() ) {
        s << *it.current();
        ++it;
    }

    return s;
}

/*!
  Read the character's data from the stream.
*/
QDataStream &operator>> (QDataStream &s, QIMPenChar &ws)
{
    s >> ws.ch;
    s >> ws.flags;
    if ( ws.flags & QIMPenChar::Data ) {
	QString d;
	s >> d;
	// then throw away.
    }
    unsigned size;
    s >> size;
    for ( unsigned i = 0; i < size; i++ ) {
        QIMPenStroke *st = new QIMPenStroke();
        s >> *st;
        ws.strokes.append( st );
    }

    return s;
}

//===========================================================================

bool QIMPenCharMatch::operator>( const QIMPenCharMatch &m )
{
    return error > m.error;
}

bool QIMPenCharMatch::operator<( const QIMPenCharMatch &m )
{
    return error < m.error;
}

bool QIMPenCharMatch::operator<=( const QIMPenCharMatch &m )
{
    return error <= m.error;
}

//===========================================================================

/*!
  \class QIMPenCharSet qimpenchar.h
  \brief The QIMPenCharSet class maintains a set of related characters.
*/

QIMPenCharSet::QIMPenCharSet()
{
    chars.setAutoDelete( TRUE );
    desc = qApp->translate("Handwriting","Unnamed","Character set name");
    csTitle = "abc";
    csType = Unknown;
    maxStrokes = 0;
}

/*!
  Construct and load a characters set from file \a fn.
*/
QIMPenCharSet::QIMPenCharSet( const QString &fn )
{
    chars.setAutoDelete( TRUE );
    desc = qApp->translate("Handwriting","Unnamed","Character set name");
    csTitle = "abc"; // No tr;
    csType = Unknown;
    maxStrokes = 0;
    load( fn );
}

void QIMPenCharSet::clear()
{
    if (count() > 0) {
	chars.clear();
    }
}

void QIMPenCharSet::setDescription( const QString &d )
{
    if (d != desc) {
	desc = d;
    }
}

void QIMPenCharSet::setTitle( const QString &t )
{
    if (t != csTitle) {
	csTitle = t;
    }
}

void QIMPenCharSet::setType( Type t )
{
    if (t != csType) {
	csType = t;
    }
}

const QString &QIMPenCharSet::filename( ) const
{
    return userFilename;
}

void QIMPenCharSet::setFilename( const QString &fn )
{
    if (fn != userFilename) {
	userFilename = fn;
    }
}

#ifdef Q_WS_QWS
QString QIMPenCharSet::systemPath( ) const
{
    static const QString sysPath(QPEApplication::qpeDir() + "etc/qimpen/"); // no tr

    return sysPath + userFilename;
}

QString QIMPenCharSet::userPath() const
{
    return Global::applicationFileName("qimpen",userFilename); // no tr
}
#endif

/*!
  Load a character set from file \a fn, in the domain \a d.
*/
bool QIMPenCharSet::load( const QString &fn )
{
    clear();
    if (!fn.isEmpty())
	setFilename( fn );

    bool ok = FALSE;
#ifdef Q_WS_QWS
    for (int isUser = 0; isUser < 2; ++isUser) {
	QString path;

	if (isUser == 1)
	    path = userPath();
	else
	    path = systemPath();
#else
	QString path = fn;
#endif

	QFile file( path );
	if ( file.open( IO_ReadOnly ) ) {
	    QDataStream ds( &file );
	    QString version;
	    ds >> version;
	    ds >> csTitle;
	    ds >> desc;
	    int major = version.mid( 4, 1 ).toInt();
	    int minor = version.mid( 6 ).toInt();
	    if ( major >= 1 && minor > 0 ) {
		ds >> (Q_INT8 &)csType;
	    } else {
		if ( csTitle == "abc" ) // no tr
		    csType = Lower;
		else if ( csTitle == "ABC" ) // no tr
		    csType = Upper;
		else if ( csTitle == "123" ) // no tr
		    csType = Numeric;
		else if ( fn == "Combining" ) // No tr
		    csType = Combining;
	    }
	    while ( !ds.atEnd() ) {
		QIMPenChar *pc = new QIMPenChar;
		ds >> *pc;
#ifdef Q_WS_QWS
		if ( isUser == 1 )
#endif
		    markDeleted( pc->character() ); // override system
		addChar( pc );
	    }
	    if ( file.status() == IO_Ok )
		ok = TRUE;
	}
#ifdef Q_WS_QWS
    }
#endif

    return ok;
}

/*!
  Save this character set, in the domain \a d.
*/
bool QIMPenCharSet::save( ) const
{
    bool ok = FALSE;

#ifdef Q_WS_QWS
    QString fn = userPath();
#else
    QString fn = userFilename;
#endif
    QString tmpFn = fn + ".new"; // no tr
    QFile file( tmpFn );
    if ( file.open( IO_WriteOnly|IO_Raw ) ) {
        QDataStream ds( &file );
	ds << QString( "QPT 1.1" ); // no tr
        ds << csTitle;
        ds << desc;
	ds << (Q_INT8)csType;
        QIMPenCharIterator ci( chars );
        for ( ; ci.current(); ++ci ) {
	    QIMPenChar *pc = ci.current();
#ifdef Q_WS_QWS
	    // only save user char's, not system char's.
	    if ( !pc->testFlag( QIMPenChar::System ) ) {
		ds << *pc;
	    }
#else
		ds << *pc;
#endif
	    if ( file.status() != IO_Ok )
		break;
        }
	if ( file.status() == IO_Ok )
	    ok = TRUE;
    }

    if ( ok ) {
	if ( ::rename( tmpFn.latin1(), fn.latin1() ) < 0 ) {
	    qWarning( "problem renaming file %s to %s, errno: %d",
		    tmpFn.latin1(), fn.latin1(), errno );
	    // remove the tmp file, otherwise, it will just lay around...
	    QFile::remove( tmpFn.latin1() );
	    ok = FALSE;
	}
    }

    return ok;
}

QIMPenChar *QIMPenCharSet::at( int i )
{
    return chars.at(i);
}

void QIMPenCharSet::markDeleted( uint ch )
{
    QIMPenCharIterator ci( chars );
    for ( ; ci.current(); ++ci ) {
	QIMPenChar *pc = ci.current();
	if ( pc->character() == ch && pc->testFlag( QIMPenChar::System ) )
	    pc->setFlag( QIMPenChar::Deleted );
    }
}

/*!
  Find the best matches for \a ch in this character set.
*/
QIMPenCharMatchList QIMPenCharSet::match( QIMPenChar *ch )
{
    QIMPenCharMatchList matches;

    QIMPenCharIterator ci( chars );
    // for each character in set.
    for ( ; ci.current(); ++ci ) {
	QIMPenChar *tmplChar = ci.current();
        if ( tmplChar->testFlag( QIMPenChar::Deleted ) ) {
            continue;
        }
	int err;
	// if the stroke to match against has equal or less strokes
	if ( ch->penStrokes().count() <= tmplChar->penStrokes().count() ) {
	    // check how well it matches, 
	    err = ch->match( tmplChar );
	    // and if it is less than the threshold
	    if ( err <= QIMPEN_MATCH_THRESHOLD ) {
		// compare stroke count again
		if (tmplChar->penStrokes().count() != ch->penStrokes().count())
		    err = QMIN(err*3, QIMPEN_MATCH_THRESHOLD);
		QIMPenCharMatchList::Iterator it;
		// Correct the error if an existing char match
		// has greater error (e.g. two ways of writing 'a')
		for ( it = matches.begin(); it != matches.end(); ++it ) {
		    if ( (*it).penChar->character() == tmplChar->character() &&
			 (*it).penChar->penStrokes().count() == tmplChar->penStrokes().count() ) {
			if ( (*it).error > err )
			    (*it).error = err;
			break;
		    }
		}
		// if this char isn't already a match
		if ( it == matches.end() ) {
		    // add it as a match
		    QIMPenCharMatch m;
		    m.error = err;
		    m.penChar = tmplChar;
		    matches.append( m );
		}
	    }
	}
    }
    // sort and return.
    qHeapSort( matches );
    return matches;
}

/*!
  Add a character \a ch to this set.
  QIMPenCharSet will delete this character when it is no longer needed.
*/
void QIMPenCharSet::addChar( QIMPenChar *ch )
{
    if ( ch->penStrokes().count() > maxStrokes )
        maxStrokes = ch->penStrokes().count();
    chars.append( ch );
}

/*!
  Remove a character by reference \a ch from this set.
  QIMPenCharSet will delete this character.
*/
void QIMPenCharSet::removeChar( QIMPenChar *ch )
{
    chars.remove( ch );
}

/*!
  Move the character \a ch up the list of characters.
*/
void QIMPenCharSet::up( QIMPenChar *ch )
{
    int idx = chars.findRef( ch );
    if ( idx > 0 ) {
        chars.take();
        chars.insert( idx - 1, ch );
    }
}

/*!
  Move the character \a ch down the list of characters.
*/
void QIMPenCharSet::down( QIMPenChar *ch )
{
    int idx = chars.findRef( ch );
    if ( idx >= 0 && idx < (int)chars.count() - 1 ) {
        chars.take();
        chars.insert( idx + 1, ch );
    }
}

