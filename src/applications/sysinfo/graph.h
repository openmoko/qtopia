/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QFrame>
#include <QVector>
#include <QStringList>

class GraphData
{
public:
    void clear();
    void addItem( const QString &name, int value );

    const QString &name( int i ) const { return names[i]; }
    int value( int i ) const { return values[i]; }
    int count() const { return values.size(); }

private:
    QStringList names;
    QVector<int> values;
};

class Graph : public QFrame
{
    Q_OBJECT
public:
    Graph( QWidget *parent = 0, Qt::WFlags f = 0 );

    void setData( const GraphData *p ) { data = p; }

protected:
    virtual void paintEvent(QPaintEvent *pe);
    virtual void drawContents( QPainter *p ) = 0;
    const GraphData *data;
};

class BarGraph : public Graph
{
    Q_OBJECT
public:
    BarGraph( QWidget *parent = 0, Qt::WFlags f = 0 );

protected:
    virtual void drawContents( QPainter *p );
    void drawSegment( QPainter *p, const QRect &r, const QColor &c );
};

class GraphLegend : public QFrame
{
    Q_OBJECT
public:
    GraphLegend( QWidget *parent = 0, Qt::WFlags f = 0 );

    void setData( const GraphData *p );
    virtual QSize sizeHint() const;
    void setOrientation(Qt::Orientation o);

protected:
    virtual void paintEvent( QPaintEvent *pe );

private:
    const GraphData *data;
    bool horz;
};
