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
#include <qlayout.h>
#include <qtimer.h>
#include <qvbox.h>
#include "graph.h"
#include "storage.h"
#include <qtopia/storage.h>

#include <stdio.h>
#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
#include <sys/vfs.h>
#include <mntent.h>
#include <errno.h>
#endif

StorageInfoView::StorageInfoView( QWidget *parent, const char *name )
    : QWidget( parent, name ), vb(0)
{
    sinfo = new StorageInfo(this);
    vb = 0;
    updateMounts();
    connect(sinfo, SIGNAL(disksChanged()), this, SLOT(updateMounts()));
    startTimer(5000);
}

void StorageInfoView::timerEvent(QTimerEvent*)
{
    if (isVisible()) {
        sinfo->update();
        emit updated();
    }
}

void StorageInfoView::resizeEvent(QResizeEvent*)
{
    if ( vb )
	setVBGeom();
}

void StorageInfoView::setVBGeom()
{
    vb->setGeometry(4,0,width()-8,height());
}

QSize StorageInfoView::sizeHint() const
{
    QSize s = vb ? vb->sizeHint() : QSize();
    return QSize( s.width()+8, s.height() );
}


void StorageInfoView::updateMounts()
{
    const QList<FileSystem>& sifs(sinfo->fileSystems());
    QListIterator<FileSystem> sit(sifs);

    int n = sifs.count();
    delete vb;
    vb = new QVBox( this );
    vb->setSpacing( n > 3 ? 1 : 5 );
    bool frst=TRUE;
    FileSystem* fs;
    for ( ; (fs=sit.current()); ++sit ) {
	if ( !frst ) {
	    QFrame *f = new QFrame( vb );
	    f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	    f->show();
	} else frst=FALSE;
	MountInfo* mi = new MountInfo(fs, vb);
	connect(this, SIGNAL(updated()), mi, SLOT(refresh()));
    }
    if ( n < 3 ) // add a filler
	(new QWidget(vb))->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setVBGeom();
    updateGeometry();
    vb->show();
}


MountInfo::MountInfo( const FileSystem* f, QWidget *parent, const char *name )
    : QWidget( parent, name ), title(f->name())
{
    fs = f;
    QVBoxLayout *vb = new QVBoxLayout( this, 7 );

    totalSize = new QLabel( this );
    vb->addWidget( totalSize );

    data = new GraphData();

    graph = new BarGraph( this );
    graph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( graph, 1 );
    graph->setData( data );

    legend = new GraphLegend( this );
    vb->addWidget( legend );
    legend->setData( data );

    refresh();
}

MountInfo::~MountInfo()
{
    delete data;
}

void MountInfo::refresh()
{
    long mult = 0;
    long div = 0;
    if ( fs->blockSize() ) {
	mult = fs->blockSize() / 1024;
	div = 1024 / fs->blockSize();
    }
    if ( !mult ) mult = 1;
    if ( !div ) div = 1;
    long total = fs->totalBlocks() * mult / div;
    long avail = fs->availBlocks() * mult / div;
    long used = total - avail;
    data->clear();
    if (total < 10240 ) {
        totalSize->setText( title + ": " + tr("%1 kB").arg( total ) );
        data->addItem( tr("Used (%1 kB)").arg(used), used );
        data->addItem( tr("Free (%1 kB)").arg(avail), avail );
    }
    else {
        totalSize->setText( title + ": " + tr("%1 MB").arg( total/1024 ) );
        data->addItem( tr("Used (%1 MB)").arg(used/1024), used );
        data->addItem( tr("Free (%1 MB)").arg(avail/1024), avail );
    }
    graph->repaint( FALSE );
    legend->update();
    graph->show();
    legend->show();
}
