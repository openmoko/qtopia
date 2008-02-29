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
    sinfo->update();
    emit updated();
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
    QVBoxLayout *vb = new QVBoxLayout( this, 3 );

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
    totalSize->setText( title + tr(" total: %1 kB").arg( total ) );
    data->clear();
    data->addItem( tr("Used (%1 kB)").arg(used), used );
    data->addItem( tr("Available (%1 kB)").arg(avail), avail );
    graph->repaint( FALSE );
    legend->update();
    graph->show();
    legend->show();
}
