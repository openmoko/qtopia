/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "sendwindow.h"
#include "qirserver.h"

#include <qtopia/global.h>

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qprogressbar.h>

SendWindow::SendWindow( QIrServer *irserver, QWidget *parent, const char * name )
    : QMainWindow( parent, name ) //, WStyle_StaysOnTop )
{
    init();

    server = irserver;

    connect( w->cancelButton, SIGNAL( clicked() ), this, SLOT( canceled() ) );
}

SendWindow::~SendWindow()
{
    delete w;
}

void SendWindow::init()
{
    setCaption( tr("Infrared send") );

    w = new SendWidgetBase(this);
    setCentralWidget(w);
}

void SendWindow::add(BeamItem item)
{
    beamItems.append( item );

    (void) new QListViewItem(w->queuedView, item.name, item.fileName);
}

void SendWindow::beamingFirst()
{
    BeamItem item = beamItems.first();
    currentItem = item.name;
    if ( !currentItem.length() )
	currentItem = item.fileName;
    
    setTotalSteps( item.fileSize );
    w->cancelButton->setEnabled( TRUE );
}

void SendWindow::itemBeamed()
{
    QValueList<BeamItem>::Iterator it = beamItems.begin();
    if ( it != beamItems.end() )
	beamItems.remove( it );
    
    QListViewItem *item = w->queuedView->firstChild();
    if ( item ) {
	QString txt = item->text(0);
	if ( txt.isEmpty() )
	    item->text(1);
	w->queuedView->takeItem( w->queuedView->firstChild() );
    
	setStatus( tr("Beamed %1").arg(txt) );
    }
    w->progressBar->setProgress( w->progressBar->totalSteps() );
    w->cancelButton->setEnabled( count() );
}

void SendWindow::setTotalSteps(int i)
{
    w->progressBar->reset();
    w->progressBar->setTotalSteps(i);
}

void SendWindow::setProgress(int i)
{
    w->progressBar->setProgress(i);
    if ( !isVisible() ) {
	int percent;
	//avoid null division (file len = 0 )
	if ( w->progressBar->totalSteps() )
	    percent = ( w->progressBar->progress() * 100 / w->progressBar->totalSteps() );
	else
	    percent = 100;

	if (currentItem.length() > 20 ) {
	    currentItem.truncate(17);
	    currentItem += "...";
	}

	QString str = QString("%1 (%2\%)").arg(currentItem).arg(percent);
	Global::statusMessage(str);
    }
}

void SendWindow::initBeam()
{
    connect( server, SIGNAL( statusMsg(const QString&) ), this, SLOT( statusMsg(const QString&) ) );
    connect( server, SIGNAL( progressSend(int) ), this, SLOT( setProgress(int) ) );
}

void SendWindow::finished()
{
    disconnect( server, SIGNAL( statusMsg(const QString&) ), this, SLOT( statusMsg(const QString&) ) );
    disconnect( server, SIGNAL( progressSend(int) ), this, SLOT( setProgress(int) ) );
}

void SendWindow::failed()
{
    w->cancelButton->setEnabled( FALSE );
    w->queuedView->clear();
    beamItems.clear();

    setStatus( tr("Failed") );
    finished();
}

void SendWindow::statusMsg(const QString &str)
{
    setStatus( str );
    if ( !isVisible() )
	Global::statusMessage( str );
}

void SendWindow::setStatus(const QString &s)
{
    w->statusLabel->setText(s);
}

void SendWindow::canceled()
{
    server->cancelBeam();
}

void SendWindow::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Escape) {
//	canceled();
    }

    QMainWindow::keyPressEvent(k);
}

