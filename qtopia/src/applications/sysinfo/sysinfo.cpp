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

#include "memory.h"
#include "load.h"
#include "storage.h"
//#include "graphics.h"
#include "versioninfo.h"
#include "sysinfo.h"
#include "dataview.h"

#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>

#include <qtabwidget.h>
#include <qlayout.h>
#include <qscrollview.h>
#include <qpushbutton.h>


SystemInfo::SystemInfo( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f ), wizard(0)
{
    QScrollView *sv;
    setIcon( Resource::loadPixmap( "SystemInfo" ) );
    setCaption( tr("System Info") );
    QVBoxLayout *lay = new QVBoxLayout( this );
    tab = new QTabWidget( this );
    lay->addWidget( tab );

    sv = new QScrollView(this, "VersionSV");
    sv->setHScrollBarMode(QScrollView::AlwaysOff);
    sv->setResizePolicy(QScrollView::AutoOneFit);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->addChild( new VersionInfo( tab ) );
    tab->addTab( sv, tr("Version") );

#if defined(_OS_LINUX_) || defined(Q_OS_LINUX)
    sv = new QScrollView(this, "StorageSV");
    QWidget *w = new QWidget(sv->viewport());
    sv->setHScrollBarMode(QScrollView::AlwaysOff);
    sv->setResizePolicy(QScrollView::AutoOneFit);
    sv->setFrameStyle(QFrame::NoFrame);
    sv->addChild(w);
    
    QVBoxLayout * vl = new QVBoxLayout(w);
    vl->addWidget( new StorageInfoView(w) );
    QPushButton * cleanup = new QPushButton(tr("Cleanup..."), w, "start cleanup");
    cleanup->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    connect(cleanup, SIGNAL(clicked()), this, SLOT(startCleanupWizard()));
    vl->addWidget( cleanup ) ;
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum,
            QSizePolicy::Expanding);
    vl->addItem(spacer);
    
    connect(qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
            this, SLOT(appMessage(const QCString&, const QByteArray&)));
    
    tab->addTab(sv, tr("Storage"));
#endif

#ifdef SYSINFO_GEEK_MODE
    tab->addTab( new LoadInfo( tab ), tr("CPU") );
    //tab->addTab( new Graphics( tab ), tr("Graphics") );
#endif

    tab->addTab( new MemoryInfo( tab ), tr("Memory") );
    tab->addTab( dataView = new DataView( tab ), tr("Data") );
    resize( 220, 180 );

#ifdef QTOPIA_PHONE
    contextMenu = new ContextMenu( this );
#endif
}

bool SystemInfo::event(QEvent *e) 
{   
    if (e->type() == QEvent::Accel ||  e->type() == QEvent::KeyPress) {
        QKeyEvent *ke = (QKeyEvent*)e;
        if (ke->key() == Key_Up || ke->key() == Key_Down) {
            QWidget *w = tab->currentPage();
            if (w->inherits("QScrollView")) {
                QScrollView *sView = (QScrollView*)(w);
                if (ke->key() == Key_Down)
                    sView->scrollBy(0, 10);
                else
                    sView->scrollBy(0, -10);
                return TRUE;
            }
        }
    }
    return QWidget::event(e);
}
/*@ \service CleanupWizard */
void SystemInfo::appMessage(const QCString& msg, const QByteArray& data) {
    (void)data;
    if ( msg == "showCleanupWizard()" ) {
        /*@ \message 
          Start the cleanup wizard. The wizard allows the deletion of old documents,
          the cleanup of the mailbox and purges old (and finished) events.
        */
        startCleanupWizard();
        tab->showPage( dataView );
    }
}

void SystemInfo::startCleanupWizard() 
{
    QPEApplication::setKeepRunning();
    if (!wizard) 
        wizard = new CleanupWizard(this, "CleanupWizard", WType_Modal);
    wizard->showMaximized();
}
