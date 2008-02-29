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

#include <qframe.h>
#include <qarray.h>
#include <qstringlist.h>

class GraphData
{
public:
    void clear();
    void addItem( const QString &name, int value );

    const QString &name( int i ) const { return names[i]; }
    int value( int i ) const { return values[i]; }
    unsigned count() const { return values.size(); }

private:
    QStringList names;
    QArray<int> values;
};

class Graph : public QFrame
{
    Q_OBJECT
public:
    Graph( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

    void setData( const GraphData *p ) { data = p; }

protected:
    const GraphData *data;
};

class PieGraph : public Graph
{
    Q_OBJECT
public:
    PieGraph( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

protected:
    virtual void drawContents( QPainter *p );
};

class BarGraph : public Graph
{
    Q_OBJECT
public:
    BarGraph( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

protected:
    virtual void drawContents( QPainter *p );
    void drawSegment( QPainter *p, const QRect &r, const QColor &c );
};

class GraphLegend : public QFrame
{
    Q_OBJECT
public:
    GraphLegend( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );

    void setData( const GraphData *p );
    virtual QSize sizeHint() const;
    void setOrientation(Orientation o);

protected:
    virtual void drawContents( QPainter *p );

private:
    const GraphData *data;
    bool horz;
};
