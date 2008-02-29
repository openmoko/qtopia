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
    QVBoxLayout *vb = new QVBoxLayout( this, 7 );

    totalMem = new QLabel( this );
    vb->addWidget( totalMem );

    data = new GraphData();
    graph = new BarGraph( this );
    graph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( graph, 1 );
    graph->setData( data );

    legend = new GraphLegend( this );
    vb->addWidget( legend );
    legend->setData( data );
    
#ifndef QTOPIA_PHONE
    vb->addStretch(1);
#else
    vb->addStretch(100);
#endif
    updateData();

    startTimer(5000);
}

MemoryInfo::~MemoryInfo()
{
    delete data;
}

void MemoryInfo::timerEvent(QTimerEvent *) 
{
    if (isVisible())
        updateData();
}

void MemoryInfo::updateData()
{
    QFile file( "/proc/meminfo" );

    if ( file.open( IO_ReadOnly ) ) {
	QTextStream t( &file );
        QString word;
	int total, memfree, buffers, cached;
        while ( ! t.atEnd() ) {
            t >> word;
            if ( word == "MemTotal:" ) {
                t >> total;
            } else if ( word == "MemFree:" ) {
                t >> memfree;
            } else if ( word == "Buffers:" ) {
                t >> buffers;
            } else if ( word == "Cached:" ) {
                t >> cached;
                break;
            }
        }
        
	int realUsed = total - ( buffers + cached + memfree );
	data->clear();
#ifdef SYSINFO_GEEK_MODE
	// Geek mode
	data->addItem( tr("Used (%1 kB)").arg(realUsed), realUsed );
	data->addItem( tr("Buffers (%1 kB)").arg(buffers), buffers );
	data->addItem( tr("Cached (%1 kB)").arg(cached), cached );
	data->addItem( tr("Free (%1 kB)").arg(memfree), memfree );
	totalMem->setText( tr( "Total Memory: %1 kB" ).arg( total ) );
#else
	// User mode
	//  - 'cached' is free-whenever-you-need-it
	//  - 'buffers' is used-and-expensive-to-recover
	realUsed += buffers;
	memfree += cached;
        QString unit = tr("kB");
        if (total > 10240) {
            realUsed = realUsed / 1024;
            memfree = memfree / 1024;
            total = total / 1024;
            unit = tr("MB");
        }
    	data->addItem( tr("Used (%1 %2)", "%1 = number, %2 = unit").arg(realUsed).arg(unit), realUsed );
	data->addItem( tr("Free (%1 %2)", "%1 = number, %2 = unit").arg(memfree).arg(unit), memfree );
	totalMem->setText( tr( "Total Memory: %1 %2", "%1 = 512 %2 = MB/kB" ).arg( total ).arg( unit ));
#endif
	graph->repaint( FALSE );
	legend->update();
    }
}


