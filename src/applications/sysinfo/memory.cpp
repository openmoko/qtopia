/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qlabel.h>
#include <qtimer.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlayout.h>
#include "graph.h"
#include "memory.h"

MemoryInfo::MemoryInfo( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    QVBoxLayout *vb = new QVBoxLayout( this, 5 );

    totalMem = new QLabel( this );
    vb->addWidget( totalMem );

    data = new GraphData();
//    graph = new PieGraph( this );
    graph = new BarGraph( this );
    graph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( graph, 1 );
    graph->setData( data );

    legend = new GraphLegend( this );
    vb->addWidget( legend );
    legend->setData( data );

    vb->addStretch( 1 );
    updateData();

    QTimer *t = new QTimer( this );
    connect( t, SIGNAL( timeout() ), this, SLOT( updateData() ) );
    t->start( 5000 );
}

MemoryInfo::~MemoryInfo()
{
    delete data;
}

void MemoryInfo::updateData()
{
    QFile file( "/proc/meminfo" );

    if ( file.open( IO_ReadOnly ) ) {
	QTextStream t( &file );
	QString dummy = t.readLine();	// title
	t >> dummy;
	int total, used, memfree, shared, buffers, cached;
	t >> total;
	total /= 1024;
	t >> used;
	used /= 1024;
	t >> memfree;
	memfree /= 1024;
	t >> shared;
	shared /= 1024;
	t >> buffers;
	buffers /= 1024;
	t >> cached;
	cached /= 1024;
	int realUsed = total - ( buffers + cached + memfree );
	data->clear();
	data->addItem( tr("Used (%1 kB)").arg(realUsed), realUsed );
	data->addItem( tr("Buffers (%1 kB)").arg(buffers), buffers );
	data->addItem( tr("Cached (%1 kB)").arg(cached), cached );
	data->addItem( tr("Free (%1 kB)").arg(memfree), memfree );
	totalMem->setText( tr( "Total Memory: %1 kB" ).arg( total ) );
	graph->repaint( FALSE );
	legend->update();
    }
}


