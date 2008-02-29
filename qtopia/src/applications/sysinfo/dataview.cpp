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

#include "dataview.h"
#include "graph.h"

#include <qfileinfo.h>
#include <qdir.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtopia/applnk.h>
#include <qtopia/storage.h>
#include <qtopia/global.h>


DataView::DataView( QWidget *parent, const char *name, WFlags f) 
    : QWidget(parent, name, f)
{
    storage = new StorageInfo( this );
    QVBoxLayout *vb = new QVBoxLayout(this, 7);

    data = new GraphData();

    graph = new BarGraph( this );
    graph->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    vb->addWidget(graph, 1);
    graph->setData( data );

    legend = new GraphLegend(this);
    vb->addWidget(legend);
    legend->setData(data);

    vb->addStretch( 1 );

    updateData();

    startTimer( 5000 );
}

DataView::~DataView()
{
    delete data;
}

void DataView::timerEvent(QTimerEvent *)
{
    if (isVisible()) {
        storage->update();
        updateData();
    }
}

void DataView::updateData()
{
    const FileSystem *fs;
    fs = storage->fileSystemOf(Global::homeDirPath());
    long av = 0, to = -1;
    if (fs)
        fileSystemMetrics(fs, &av, &to);
    long total = to;
    long avail = av;
    
    const QList<FileSystem>& filesystems(storage->fileSystems());
    QListIterator<FileSystem> iter(filesystems);
    
    for ( ; iter.current(); ++iter )
    {
        long av = 0, to = 0;
        if ((*iter)->isRemovable()) {
            fileSystemMetrics(*iter, &av, &to);
            total += to;
            avail += av;
        }
    }

    int mail = 0;
    QString dirName = Global::homeDirPath() + "/Applications/qtmail";
    QDir mailDir(dirName);
    const QFileInfoList *list = mailDir.entryInfoList();
    if (list) {
        QFileInfo * info;
        QFileInfoListIterator it(*list);
        while ((info = it.current())) {
            mail += info->size()/1024;
            ++it;
        }
    }
   
    QString filter = "image/*"; 
    int images = documentSize(filter);
    filter = "audio/*";
    int audio = documentSize(filter);
    filter = "video/*";
    int video = documentSize(filter);
    filter = "text/*";
    int txt = documentSize(filter);
    
    data->clear();

    QString unitKB = tr("kB"," short for kilobyte");
    QString unitMB = tr("MB", "short for megabyte");
    
    if (audio < 10240) 
        data->addItem(tr("Audio (%1 %2)").arg(audio).arg(unitKB), audio);
    else
        data->addItem(tr("Audio (%1 %2)").arg(audio/1024).arg(unitMB), audio);
    
    if (images < 10240) 
        data->addItem(tr("Images (%1 %2)").arg(images).arg(unitKB), images);
    else
        data->addItem(tr("Images (%1 %2)").arg(images/1024).arg(unitMB), images);
    
    if (mail < 10240)
        data->addItem(tr("Mailbox (%1 %2)").arg(mail).arg(unitKB), mail);
    else
        data->addItem(tr("Mailbox (%1 %2)").arg(mail/1024).arg(unitMB), mail);

    if (txt < 10240)
        data->addItem(tr("Text (%1 %2)").arg(txt).arg(unitKB),txt);
    else
        data->addItem(tr("Text (%1 %2)").arg(txt/1024).arg(unitMB),txt);
    
    if (video < 1024)
        data->addItem(tr("Video (%1 %2)").arg(video).arg(unitKB), video);
    else
        data->addItem(tr("Video (%1 %2)").arg(video/1024).arg(unitMB), video);

    if (avail >= 0)
        if (avail < 10240)
            data->addItem(tr("Free (%1 %2)").arg(avail).arg(unitKB), avail);
        else
            data->addItem(tr("Free (%1 %2)").arg(avail/1024).arg(unitMB), avail);
            

    graph->repaint( FALSE );
    legend->update();
}

int DataView::documentSize(QString filter) 
{
    DocLnk *dl;
    DocLnkSet allDocs;
    Global::findDocuments( &allDocs, filter );
    const QList<DocLnk> list = allDocs.children();
    QListIterator<DocLnk> it(list);

    uint sum = 0;
    while ((dl=it.current())) {
        sum += QFileInfo(dl->file()).size()/1024;
        ++it;
    }

    return sum;
}

void DataView::fileSystemMetrics(const FileSystem *fs, long *avail, long *total)
{
    long mult = 0;
    long div = 0;
    if ( fs->blockSize() ) {
        mult = fs->blockSize() / 1024;
        div = 1024 / fs->blockSize();
    }
    if ( !mult ) mult = 1;
    if ( !div ) div = 1;

    *total = fs->totalBlocks() * mult / div;
    *avail = fs->availBlocks() * mult / div;
}

