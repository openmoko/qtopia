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
#include "helppreprocessor.h"

#define QTOPIA_INTERNAL_LANGLIST
#include <qtopia/global.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qmap.h>

HelpPreProcessor::HelpPreProcessor( const QString &file )
    : mFile( file )
{
}

QString HelpPreProcessor::text()
{
    QFile f( mFile );
    if ( !f.exists() ) {
        QStringList helpPaths = Global::helpPath();
        QStringList::Iterator it;
        for ( it = helpPaths.begin(); it != helpPaths.end(); it++ ) {
            QString file = (*it) + "/" + mFile;
            f.setName( file );
            if ( f.exists() )
                break;
        }
        if ( it == helpPaths.end() )
            return tr("Could not locate %1", "%1 - file").arg( mFile );
    }
    //qDebug( "***** HelpPreProcessor::text %s", mFile.latin1() );
    f.open( IO_ReadOnly );
    QByteArray data = f.readAll();
    QTextStream ts( data, IO_ReadOnly );
    ts.setEncoding( QTextStream::UnicodeUTF8 );
    QString text;

    // setup the config variable
    QStringList config;
#ifdef QTOPIA_PHONE
    config << "PHONE";
    if ( Global::mousePreferred() )
	config << "TOUCH";
    else
	config << "KEYPAD";
#else
    config << "PDA";
#endif

    QValueList<QStringList> skip;
    QMap<QString,QString> replace;

    QString line;
    bool found;
    int offset;
    int matchLen;
    QStringList tag;
    QRegExp tagStart( "%%[^%]+%%" );
    QRegExp tagEnd( "@@[^@]+@@" );
    QRegExp varSet( "^\\{[a-zA-Z0-9._]+\\}=\\{[^\\}]+\\}$" );
    QRegExp varStart( "^$$\\{[a-zA-Z0-9._]+\\}" );
    QRegExp varNorm( "[^\\\\]$$\\{[a-zA-Z0-9._]+\\}" );
    QRegExp varIgnore( "\\\\$$\\{[a-zA-Z0-9._]+\\}" );
    QRegExp allWhiteSpace( "^\\s*$" );

    for ( line = ts.readLine(); line != QString::null; line = ts.readLine() ) {
	found = FALSE;
	offset = 0;
	matchLen = 0;
	tag.clear();
	while ( (offset = tagStart.match( line, offset + matchLen, &matchLen, FALSE )) != -1 ) {
	    found = TRUE;
	    tag << line.mid( offset + 2, matchLen - 4 );
	}
	if ( found ) {
	    QString word = tag.join(" ");
	    //qDebug( "found start tag %s", word.latin1() );
	    if ( !skip.contains( tag ) ) {
		skip.append( tag );
	    } else {
		qWarning( "%s already in use", word.latin1() );
	    }
	    line.replace( tagStart, "" );
	    if ( line.find( allWhiteSpace ) == 0 )
		continue;
	}

	found = FALSE;
	offset = 0;
	matchLen = 0;
	tag.clear();
	while ( (offset = tagEnd.match( line, offset + matchLen, &matchLen, FALSE )) != -1 ) {
	    found = TRUE;
	    tag << line.mid( offset + 2, matchLen - 4 );
	}
	if ( found ) {
	    QString word = tag.join(" ");
	    //qDebug( "found end tag %s", word.latin1() );
	    if ( skip.contains( tag ) ) {
		skip.remove( tag );
	    } else {
		qWarning( "%s not in use", word.latin1() );
	    }
	    line.replace( tagEnd, "" );
	    if ( line.find( allWhiteSpace ) == 0 )
		continue;
	}

	int print = 0;
	QValueList<QStringList>::ConstIterator it;
	for ( it = skip.begin(); it != skip.end(); ++it ) {
	    const QStringList &tag = (*it);
	    QStringList::ConstIterator jt;
	    bool found = FALSE;
	    for ( jt = tag.begin(); jt != tag.end(); ++jt ) {
		if ( config.contains( *jt ) )
		    found = TRUE;
	    }
	    if ( found )
		print++;
	}

	// ensure we're in a valid block
	if ( print != (int)skip.count() )
	    continue;

	if ( varSet.match( line, 0, &matchLen, FALSE ) != -1 ) {
	    int var1close = line.find("}");
	    QString key = line.mid( 1, var1close - 1 );
	    QString value = line.mid( var1close + 3, matchLen - var1close - 4);
	    //qDebug( "setting varible %s to %s", key.latin1(), value.latin1() );
	    replace[key] = value;
	    continue;
	}

	if ( varStart.match( line, 0, &matchLen, FALSE ) != -1 ) {
	    QString key = line.mid( 3, matchLen - 4 );
	    line.replace( 0, matchLen, replace[key] );
	}
	offset = 0;
	matchLen = 0;
	while ( (offset = varNorm.match( line, offset + matchLen, &matchLen, FALSE )) != -1 ) {
	    QString key = line.mid( offset + 4, matchLen - 5 );
	    //qDebug( "matched %s", key.latin1() );
	    line.replace( offset + 1, matchLen - 1, replace[key] );
	}
	offset = 0;
	matchLen = 0;
	while ( (offset = varIgnore.match( line, offset + matchLen, &matchLen, FALSE )) != -1 ) {
	    line.remove( offset, 1 );
	}

	offset = 0;
	matchLen = 0;

	//qDebug( "line: %s", line.latin1() );
	text += line + "\n";
    }
    if ( text.find( allWhiteSpace ) == 0 )
	text = tr("No content for the current config (%1) exists in file %2").arg( config.join(" ") ).arg( mFile );
    return text;
}

