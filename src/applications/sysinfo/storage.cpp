/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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
#include <QTimer>
#include "graph.h"
#include "storage.h"

#include <qstorage.h>
#include <QScrollArea>
#include <QFont>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QCoreApplication>

#include <stdio.h>
#ifdef Q_OS_LINUX
#include <sys/vfs.h>
#include <mntent.h>
#include <errno.h>
#endif

StorageInfoView::StorageInfoView( QWidget *parent )
    : QWidget( parent ), area(0)
{
    QTimer::singleShot(10, this, SLOT(init()));
}

void StorageInfoView::init()
{
    sinfo = new QStorageMetaInfo(this);
    QLayout *layout = new QVBoxLayout( this );
    layout->setSpacing( 0 );
    layout->setMargin( 0 );
    area = new QScrollArea;
    layout->addWidget( area );

    area->setFocusPolicy( Qt::NoFocus );
    area->setFrameShape( QFrame::NoFrame );

    updateMounts();
    connect(sinfo, SIGNAL(disksChanged()), this, SLOT(updateMounts()));
    startTimer(60000);
}

void StorageInfoView::timerEvent(QTimerEvent*)
{
    if (isVisible()) {
        sinfo->update();
        emit updated();
    }
}

QSize StorageInfoView::sizeHint() const
{
    QSize s = area ? area->sizeHint() : QSize();
    return QSize( s.width()+8, s.height() );
}

void StorageInfoView::updateMounts()
{
    QList<QFileSystem*> sifs = sinfo->fileSystems( 0 );
    QList<QFileSystem*> newfs; 

    //find new/added file systems
    QList<QFileSystem*> newFs;
    foreach( QFileSystem* f, sifs ) {
        if ( !fsHash.contains( f ) ) {
            newFs.append( f );
        }
    }

    //find old/removed file systems
    QList<QFileSystem*> oldFs;
    QList<QFileSystem*> keys = fsHash.keys();
    foreach( QFileSystem* fsHashKey, keys ) {
        if ( !sifs.contains( fsHashKey ) )
            oldFs.append( fsHashKey );
    }

    //don't redraw entire widget if there are no new file systems
    if ( oldFs.count() == 0 && newFs.count() == 0 )
        return;
   
    if ( area->widget() )
        delete area->takeWidget();
    fsHash.clear();
    QWidget *vb = new QWidget;
    area->setWidget( vb );
    area->setWidgetResizable( true );
    QVBoxLayout *vLayout = new QVBoxLayout(vb);
    vLayout->setSpacing( 6 );
    vLayout->setMargin(0);
    foreach( QFileSystem *fs, sifs ) {
        MountInfo* mi = new MountInfo(fs);
        // Force the minimum size (or it gets squashed)
        mi->setMinimumSize( mi->sizeHint() );
        mi->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
        vLayout->addWidget(mi);
        connect(this, SIGNAL(updated()), mi, SLOT(refresh()));
        fsHash.insert( fs, mi ); 
    }
    vLayout->addStretch( 1 );
    vb->updateGeometry();
    vb->show();
}


MountInfo::MountInfo( const QFileSystem* f, QWidget *parent )
    : QWidget( parent ), title(f->name())
{
    fs = f;
    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setSpacing( 7 );
    totalSize = new QLabel;
    vb->addWidget( totalSize );

    data = new GraphData();

    graph = new BarGraph;
    graph->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    vb->addWidget( graph );
    graph->setData( data );

    legend = new GraphLegend;
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
    data->clear();
    double total = (double)fs->totalBlocks() * fs->blockSize();
    QString totalString;
    double free = (double)fs->availBlocks() * fs->blockSize();
    QString freeString;
    double used = (double)(fs->totalBlocks() - fs->availBlocks()) * fs->blockSize();
    QString usedString;

    getSizeString( total, totalString );
    totalSize->setText( QString("%1: %2 %3").arg(title).arg(QString().sprintf("%0.2f", total)).arg(totalString) );

    getSizeString( used, usedString );
    data->addItem( tr("Used (%1 %2)", "1=size,2=B/KB/MB").arg(QString().sprintf("%0.2f", used)).arg(usedString), (fs->totalBlocks() - fs->availBlocks()) );

    getSizeString( free, freeString );
    data->addItem( tr("Free (%1 %2)", "1=size,2=B/KB/MB").arg(QString().sprintf("%0.2f", free)).arg(freeString), fs->availBlocks() );

    graph->update();
    legend->update();
}

void MountInfo::getSizeString( double &size, QString &string )
{
    if ( size < 0 ) size = 0;
    if ( size < 1024 ) {
        string = tr("B","bytes");
        return;
    }
    size /= 1024;
    if ( size < 1024 ) {
        string = tr("kB","kilobytes");
        return;
    }
    size /= 1024;
    if ( size < 1024 ) {
        string = tr("MB","megabytes");
        return;
    }
    size /= 1024;
    if ( size < 1024 ) {
        string = tr("GB","gigabytes");
        return;
    }
}
