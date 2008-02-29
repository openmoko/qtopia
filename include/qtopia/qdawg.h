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
#ifndef QDAWG_H
#define QDAWG_H

#include <qtopia/qpeglobal.h>
#include <qstringlist.h>

class QIODevice;
class QDawgPrivate;

class QTOPIA_EXPORT QDawg {
public:
    QDawg();
    ~QDawg();

    bool readFile(const QString&); // may mmap
    bool read(QIODevice* dev);
    bool write(QIODevice* dev) const;
    bool createFromWords(QIODevice* dev);
    void createFromWords(const QStringList&);
    QStringList allWords() const;

    bool contains(const QString&) const;
    int countWords() const;

    enum {nodebits = 18};

    class Node {
	friend class QDawgPrivate;
	uint let:16;
        uint a : 14 ; //not used
	uint isword:1;
	uint islast:1;
	int offset:nodebits;
        int b : 14 ; //not used
	Node() { a = b = 0; /*set zero for better compression*/ }
    public:
	QChar letter() const { return QChar((ushort)let); }
	bool isWord() const { return isword; }
	bool isLast() const { return islast; }
	const Node* next() const { return islast ? 0 : this+1; }
	const Node* jump() const { return offset ? this+offset : 0; }
    };

    const Node* root() const;

    void dump() const; // debug

private:
    friend class QDawgPrivate;
    QDawgPrivate* d;
};


#endif
