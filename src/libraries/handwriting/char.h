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

#ifndef QIMPENCHAR_H_
#define QIMPENCHAR_H_

#include <qlist.h>
#include <qvaluelist.h>
#include <qcstring.h>
#include "stroke.h"

struct QIMPenSpecialKeys {
    int code;
    char *name;
};

extern const QIMPenSpecialKeys qimpen_specialKeys[];


class QIMPenChar
{
public:
    QIMPenChar();
    QIMPenChar( const QIMPenChar & );
    ~QIMPenChar() {}

    unsigned int character() const { return ch; }
    void setCharacter( unsigned int c ) { ch = c; }

    QString name() const;
    static QString name(unsigned int);
    bool isEmpty() const { return strokes.isEmpty(); }
    unsigned int strokeCount() const { return strokes.count(); }
    unsigned int strokeLength( int s ) const;
    void clear();
    int match( QIMPenChar *ch );
    const QIMPenStrokeList &penStrokes() const { return strokes; }
    QPoint startingPoint() const { return strokes.getFirst()->startingPoint(); }
    QRect boundingRect();

    void setFlag( int f ) { flags |= f; }
    void clearFlag( int f ) { flags &= ~f; }
    bool testFlag( int f ) { return flags & f; }

    enum Flags { System=0x01,
	Deleted=0x02,
	CombineRight=0x04,
	Data=0x08 // kept so can read old files
    };
    // Correspond to codes in template files.  Do not change values.
    enum Mode {
	ModeBase=0x4000,
	Caps=0x4001,
	Shortcut=0x4002,
	CapsLock=0x4003,
	Punctuation=0x4004,
	Symbol=0x4005,
	NextWord=0x4007, // 6 skipped, for compat reasons.
	WordPopup=0x4008,
	SymbolPopup=0x4009,
	ModePopup=0x400A
    };

    QIMPenChar &operator=( const QIMPenChar &s );

    void addStroke( QIMPenStroke * );

protected:
    unsigned int ch;
    Q_UINT8 flags;
    QIMPenStrokeList strokes;

    friend QDataStream &operator<< (QDataStream &, const QIMPenChar &);
    friend QDataStream &operator>> (QDataStream &, QIMPenChar &);
};

typedef QList<QIMPenChar> QIMPenCharList;
typedef QListIterator<QIMPenChar> QIMPenCharIterator;

QDataStream & operator<< (QDataStream & s, const QIMPenChar &ws);
QDataStream & operator>> (QDataStream & s, QIMPenChar &ws);

struct QIMPenCharMatch
{
    int error;
    QIMPenChar *penChar;

    bool operator>( const QIMPenCharMatch &m );
    bool operator<( const QIMPenCharMatch &m );
    bool operator<=( const QIMPenCharMatch &m );
};

typedef QValueList<QIMPenCharMatch> QIMPenCharMatchList;


class QIMPenCharSet
{
public:
    QIMPenCharSet();
    QIMPenCharSet( const QString &fn );
    ~QIMPenCharSet(){}

    bool isEmpty() const { return chars.isEmpty(); }
    unsigned int count() const { return chars.count(); }
    void clear();

    void setDescription( const QString &d );
    QString description() const { return desc; }
    void setTitle( const QString &t );
    QString title() const { return csTitle; }

    QIMPenCharMatchList match( QIMPenChar *ch );
    void addChar( QIMPenChar *ch );
    void removeChar( QIMPenChar *ch );

    // make it not a pointer?
    QIMPenChar *at( int i );

    unsigned maximumStrokes() const { return maxStrokes; }

    void up( QIMPenChar *ch );
    void down( QIMPenChar *ch );

    enum Type {
	Unknown=0x00,
	Lower=0x01,
	Upper=0x02,
	Combining=0x04,
	Numeric=0x08,
	Punctuation=0x10,
	Symbol=0x20,
	Shortcut=0x40
    };

    //const QIMPenCharList &characters() const { return chars; }

    void setType( Type t );
    Type type() const { return csType; }

    const QString &filename( ) const;
    void setFilename( const QString &fn );

    // loads as if it was user, if no user, as system.
    bool load( const QString & = QString::null );
    // always saves as user.
    bool save( ) const;

    const QIMPenCharList &characters() const { return chars; }

private:
    void markDeleted( uint ch );

#ifdef Q_WS_QWS
    QString userPath() const;
    QString systemPath() const;
#endif

    QString csTitle;
    QString desc;
    QString sysFilename;
    QString userFilename;
    Type csType;
    unsigned maxStrokes;
    QIMPenCharList chars;
    QIMPenCharMatchList matches; // not used?
};

typedef QList<QIMPenCharSet> QIMPenCharSetList;
typedef QListIterator<QIMPenCharSet> QIMPenCharSetIterator;

#endif
