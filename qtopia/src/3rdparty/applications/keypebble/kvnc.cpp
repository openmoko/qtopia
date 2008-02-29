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
#include <qiconset.h>
#include <qdialog.h>
#include <qpixmap.h>
#include <qdom.h>
#include <qaction.h>
#include <qtopia/qpemenubar.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtopia/qpetoolbar.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/global.h>
#include <assert.h>

#include "kvnc.h"
#include "krfbcanvas.h"
#include "kvncoptionsdlg.h"
#include "krfbconnection.h"
#include "krfboptions.h"


const int StatusTextId = 0;

/*!
  \class KVNC
  \brief The KVNC class is a VNC client.

  \legalese
    Keypebble, the VNC client for Qtopia, is based on Keystone from
    the KDE project. Keystone and Keypebble are distributed
    under the terms of the GNU General Public License. The VNC server
    code in Qt/Embedded is copyright Trolltech.

    A primary copyright holder is <a href="mailto:rich@kde.org">Richard Moore</a>.

    This software owes a lot to Markus Wuebben's kvncviewer,
    which some of the code is derived from. Thanks also go to ORL for
    making the RFB protocol and servers GPL.
*/

// QDOC_SKIP_BEGIN

/*! \a name */

KVNC::KVNC( QWidget* parent, const char* name, WFlags fl) :
QMainWindow( parent, name, fl )
{
    setCaption( tr("VNC Viewer") );
    fullscreen = false;
    isConnected = false;

    canvas = new KRFBCanvas( this, "canvas" ); // No tr
    setCentralWidget( canvas );

    connect( canvas->connection(), SIGNAL(statusChanged(const QString&)),
	    this, SLOT(statusMessage(const QString&)) );
    connect( canvas->connection(), SIGNAL(error(const QString&)),
	    this, SLOT(error(const QString&)) );
    connect( canvas->connection(), SIGNAL(connected()), this, SLOT(connected()) );
    connect( canvas->connection(), SIGNAL(loggedIn()), this, SLOT(loggedIn()) );
    connect( canvas->connection(), SIGNAL(disconnected()), this, SLOT(disconnected()) );

    setupActions();

    cornerButton = new QPushButton( this );
    cornerButton->setPixmap( Resource::loadPixmap("menu" ) );
    connect( cornerButton, SIGNAL(pressed()), this, SLOT(showMenu()) );
    canvas->setCornerWidget( cornerButton );

    if ( qApp->argc() > 1 ) {
        openURL( QUrl( qApp->argv()[1]) );
    } else {
    QTimer::singleShot( 0, canvas, SLOT(openConnection()) );
    }
}

/*! */

KVNC::~KVNC()
{

}

/*! \a url */

void KVNC::openURL( const QUrl &url )
{
    canvas->openURL( url );
}

/*! */

void KVNC::setupActions()
{
    cornerMenu = new QPopupMenu( this );

    fullScreenAction = new QAction( tr("Full Screen"), QString::null, 0, 0 );
    connect( fullScreenAction, SIGNAL(toggled(bool)), this, SLOT( toggleFullScreen(bool) ) );
    fullScreenAction->setToggleAction( true );
    fullScreenAction->addTo( cornerMenu );
    fullScreenAction->setEnabled( false );

    zoomAction = new QAction( tr("Zoom"), QString::null, 0, 0 );
    connect( zoomAction, SIGNAL(toggled(bool)),
	    this, SLOT( zoom(bool) ) );
    zoomAction->setToggleAction( true );
    zoomAction->addTo( cornerMenu );
    zoomAction->setEnabled( false );

    optionsAction = new QAction( tr("Settings..."), QString::null, 0, 0 );
    connect( optionsAction, SIGNAL(activated()), this, SLOT( showOptions() ) );
    optionsAction->addTo( cornerMenu );

    connectAction = new QAction( tr("Connect..."), QString::null, 0, 0 );
    connect( connectAction, SIGNAL(activated()), this, SLOT(connectToServer()) );
    connectAction->addTo( cornerMenu );
}

/*! \a f */

void KVNC::toggleFullScreen( bool f )
{
    if ( f ) {
	canvas->setFrameStyle( QFrame::NoFrame );
	canvas->reparent( 0,WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop,
		QPoint(0,0),false);
	canvas->resize(qApp->desktop()->width(), qApp->desktop()->height());
	canvas->raise();
	canvas->setFocus();
	canvas->grabKeyboard();
	canvas->show();
    } else {
	canvas->releaseKeyboard();
	canvas->reparent( this, 0, QPoint(0,0), false );
	canvas->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	setCentralWidget( canvas );
	canvas->show();
    }

    fullscreen = f;
    fullScreenAction->setOn(fullscreen);
}

/*! */
void KVNC::connectToServer()
{
    if ( isConnected ) {
	if ( fullscreen )
	    toggleFullScreen( false );
	canvas->closeConnection();
    } else {
	canvas->openConnection();
    }
}

/*! \a z */

void KVNC::zoom( bool z )
{
    if (isConnected) {
	canvas->setViewScale( z ? 2 : 1 );
    }
}

/*! */

void KVNC::showMenu()
{
    QPoint pt = mapToGlobal(cornerButton->pos());
    QSize s = cornerMenu->sizeHint();
    pt.ry() -= s.height();
    pt.rx() -= s.width();
    cornerMenu->popup( pt );
}

/*! */

void KVNC::connected()
{
    static QString msg = tr( "Connected to remote host" );
    statusMessage( msg );
    fullScreenAction->setEnabled( true );
    zoomAction->setEnabled( true );
    connectAction->setText( tr("Disconnect") );
    isConnected = true;
}

/*! */

void KVNC::loggedIn()
{
    static QString msg = tr( "Logged in to remote host" );
    statusMessage( msg );
}

/*! */

void KVNC::disconnected()
{
    static QString msg = tr( "Connection closed" );
    statusMessage( msg );

    isConnected = false;
    fullScreenAction->setEnabled( false );
    fullScreenAction->setOn( false );
    zoomAction->setEnabled( false );
    zoomAction->setOn( false );
    connectAction->setText( tr("Connect...") );
}

/*! \a m */

void KVNC::statusMessage( const QString &m )
{
    Global::statusMessage( m );
}

/*! \a msg */

void KVNC::error( const QString &msg )
{
    QMessageBox::warning( this, tr("VNC Viewer"), msg );
}

/*! */

void KVNC::showOptions()
{
    if ( fullscreen )
	toggleFullScreen( false );
    KVNCOptionsDlg *wdg = new KVNCOptionsDlg( canvas->connection()->options(), this );
    if ( QPEApplication::execDialog(wdg) == QDialog::Accepted ) {
	canvas->connection()->options()->save();
    }
    delete wdg;
}

// QDOC_SKIP_END
