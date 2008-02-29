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

#ifndef QIMPENPROFILE_H_
#define QIMPENPROFILE_H_

#include "char.h"

class QIMPenProfile
{
public:
    QIMPenProfile( const QString &fn );
    ~QIMPenProfile() {}

    const QString name() const { return pname; }
    const QString identifier() const;
    const QString description() const { return pdesc; }

    bool canSelectStyle() const { return tstyle; }
    bool canIgnoreStroke() const { return istyle; }

    enum Style { ToggleCases, BothCases };

    Style style() const { return pstyle; }
    void setStyle( Style s );

    int multiStrokeTimeout() const { return msTimeout; }
    void setMultiStrokeTimeout( int t );

    int ignoreStrokeTimeout() const { return isTimeout; }
    void setIgnoreStrokeTimeout( int t );
    
    bool matchWords() const { return wordMatch; }

    // shouldn't use, overly restricts usage of set
    // returns first char set of this type.
    QIMPenCharSet *uppercase();
    QIMPenCharSet *lowercase();
    QIMPenCharSet *numeric();
    QIMPenCharSet *punctuation();
    QIMPenCharSet *symbol();
    QIMPenCharSet *shortcut();
    QIMPenCharSet *find( QIMPenCharSet::Type t );

    // this is more generic, and translateable.
    QIMPenCharSet *charSet( const QString & ); // internal (not translated)
    QString title( const QString & ); // translated
    QString description( const QString & ); // translated

    QStringList charSets(); // internal (not translated)

    void load();
    void save() const;
private:
    QString userConfig() const;
    void loadData();
    void saveData() const;

private:
    struct ProfileSet {
	ProfileSet() : set(0) {}
	ProfileSet(QIMPenCharSet *s) : set(s) {}
	~ProfileSet() { if (set) delete set; }

	QString id;
	QIMPenCharSet *set;
    };

    typedef QList<ProfileSet> ProfileSetList;
    typedef QListIterator<ProfileSet> ProfileSetListIterator;
    ProfileSetList sets;

    QString filename;
    QString pname;
    QString pdesc;
    Style pstyle;
    bool tstyle;
    bool istyle;
    int msTimeout;
    int isTimeout;
    bool wordMatch;
};

#endif
