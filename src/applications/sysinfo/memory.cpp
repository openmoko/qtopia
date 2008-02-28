/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <QLabel>
#include <QLayout>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include "graph.h"
#include "memory.h"

MemoryInfo::MemoryInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(30, this, SLOT(init()));
}

MemoryInfo::~MemoryInfo()
{
    delete data;
}

void MemoryInfo::init()
{
    QVBoxLayout *vb = new QVBoxLayout( this );

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

    vb->addStretch(100);

    updateData();

    startTimer(5000);
}

void MemoryInfo::timerEvent(QTimerEvent *)
{
    if (isVisible())
        updateData();
}

void MemoryInfo::updateData()
{
    QFile file( "/proc/meminfo" );
    if ( file.open( QIODevice::ReadOnly ) ) {
        QTextStream t( &file );
        QString all = t.readAll();
        int total=0, memfree=0, buffers=0, cached = 0;
        int pos = 0;
        QRegExp regexp("(MemTotal:|MemFree:|Buffers:|\\bCached:)\\s*(\\d+) kB");
        while ( (pos = regexp.indexIn( all, pos )) != -1 ) {
            if ( regexp.cap(1) == "MemTotal:" )
                total = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "MemFree:" )
                memfree = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "Buffers:" )
                buffers = regexp.cap(2).toInt();
            else if ( regexp.cap(1) == "Cached:" )
                cached = regexp.cap(2).toInt();
            pos += regexp.matchedLength();
        }

        int realUsed = total - ( buffers + cached + memfree );
        data->clear();
        data->addItem( tr("Used (%1 kB)").arg(realUsed), realUsed );
        data->addItem( tr("Buffers (%1 kB)").arg(buffers), buffers );
        data->addItem( tr("Cached (%1 kB)").arg(cached), cached );
        data->addItem( tr("Free (%1 kB)").arg(memfree), memfree );
        totalMem->setText( tr( "Total Memory: %1 kB" ).arg( total ) );
        graph->repaint();
        legend->update();
    }
}

