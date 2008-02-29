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
#include "kvncconnectdlg.h"
#include "krfbconnection.h"
#include "krfbcanvas.h"
#include "krfboptions.h"
#include "krfbbuffer.h"

#include <qtopia/config.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qaction.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qiconset.h>
#include <qtimer.h>

KRFBCanvas::KRFBCanvas( QWidget *parent, const char *name )
  : QScrollView( parent, name )
{
    connection_ = new KRFBConnection();
    connect( connection_, SIGNAL( passwordRequired(KRFBConnection*) ),
	    this, SLOT( passwordRequired(KRFBConnection*) ) );
    connect( connection_, SIGNAL( loggedIn() ),
	    this, SLOT( loggedIn() ) );

    loggedIn_ = false;

    viewport()->setFocusPolicy( QWidget::StrongFocus );
    viewport()->setFocus();

    autoTimer = new QTimer( this );
    connect( autoTimer, SIGNAL(timeout()), this, SLOT(doAutoScroll()) );
}

KRFBCanvas::~KRFBCanvas()
{
}

void KRFBCanvas::setViewScale( int s )
{
    connection_->buffer()->setScaling( s );
}

void KRFBCanvas::openConnection()
{
  KVNCConnectDlg dlg( connection_, this, "connect dialog" ); // No tr
  if ( dlg.exec() ) {
    QCString host = dlg.hostname().latin1();
    password = dlg.password();
    connection_->connectTo( host, dlg.display() );
  }
}

void KRFBCanvas::openURL( const QUrl &url )
{
  if ( loggedIn_ ) {
      qWarning( "openURL invoked when logged in\n" );
      return;
  }

  QCString host = url.host().latin1();
  int display = url.port();
  if ( url.hasPassword() )
    connection_->setPassword( url.password().latin1() );

  connection_->connectTo( host, display );
}

void KRFBCanvas::closeConnection()
{
  loggedIn_ = false;
  connection_->disconnect();

  viewport()->setMouseTracking( false );
  viewport()->setBackgroundMode( PaletteDark );
  setBackgroundMode( PaletteDark );
  update();
  resizeContents( 0, 0 );
}

void KRFBCanvas::passwordRequired( KRFBConnection *con )
{
  con->setPassword( password.latin1() );
}

void KRFBCanvas::bell()
{
  if ( connection_->options()->deIconify ) {
    topLevelWidget()->raise();
    topLevelWidget()->show();
  }
}

void KRFBCanvas::updateSizeChanged(int w, int h)
{
    resizeContents(w, h);
    viewportUpdate(0, 0, w, h);
}

void KRFBCanvas::loggedIn()
{
  qWarning( "Ok, we're logged in" );

  //
  // Get ready for action
  //
  loggedIn_ = true;
  viewport()->setMouseTracking( true );
  viewport()->setBackgroundMode( NoBackground );
  setBackgroundMode( NoBackground );

  // Start using the buffer
  connect( connection_->buffer(), SIGNAL( sizeChanged(int,int) ),
           this, SLOT( updateSizeChanged(int,int) ) );
  connect( connection_->buffer(), SIGNAL( updated(int,int,int,int) ),
           this, SLOT( viewportUpdate(int,int,int,int) ) );
  connect( connection_->buffer(), SIGNAL( bell() ),
           this, SLOT( bell() ) );
  connect( qApp->clipboard(), SIGNAL( dataChanged() ),
           this, SLOT( clipboardChanged() ) );
}

void KRFBCanvas::viewportPaintEvent( QPaintEvent *e )
{
    QPainter p(viewport());
    QRect r = e->rect();

    if ( loggedIn_ ) {
	QRegion bgr = QRegion(r) - QRect(0, 0, contentsWidth(), contentsHeight());
	for ( uint i = 0; i < bgr.rects().count(); i++ )
	    p.fillRect( bgr.rects()[i], colorGroup().dark() );
	p.translate( -contentsX(), -contentsY() );
	connection_->buffer()->paint( &p, r.x()+contentsX(),
		r.y()+contentsY(), r.width(), r.height() );
    }
}

void KRFBCanvas::viewportUpdate( int x, int y, int w, int h )
{
  updateContents( x, y, w, h );
}

void KRFBCanvas::contentsMousePressEvent( QMouseEvent *e )
{
  if ( loggedIn_ )
    connection_->buffer()->mouseEvent( e );
}

void KRFBCanvas::contentsMouseReleaseEvent( QMouseEvent *e )
{
    if ( loggedIn_ ) {
	connection_->buffer()->mouseEvent( e );
	dx = dy = 0;
	autoTimer->stop();
    }
}

void KRFBCanvas::contentsMouseMoveEvent( QMouseEvent *e )
{
    if ( loggedIn_ )
	connection_->buffer()->mouseEvent( e );
    if ( loggedIn_ && e->state() & (LeftButton|RightButton|MidButton) ) {
	dx = dy = 0;
	autoTimer->stop();
	QPoint vpos = contentsToViewport( e->pos() );
	if ( vpos.x() >= viewport()->width()-1 && e->pos().x() < contentsWidth()-1 ) {
	    dx = 8;
	} else if ( vpos.x() <= 0 && e->pos().x() > 0 ) {
	    dx = -8;
	}
	if ( vpos.y() >= viewport()->height()-1 && e->pos().y() < contentsHeight()-1 ) {
	    dy = 8;
	} else if ( vpos.y() <= 0 && e->pos().y() > 0 ) {
	    dy = -8;
	}
	if ( dx || dy ) {
	    autoMouseEvent = new QMouseEvent(e->type(), vpos, e->button(), e->state());
	    autoTimer->start( 50, TRUE );
	}
    }
}

void KRFBCanvas::keyPressEvent( QKeyEvent *e )
{
  if ( loggedIn_ )
    connection_->buffer()->keyPressEvent( e );
}

void KRFBCanvas::keyReleaseEvent( QKeyEvent *e )
{
  if ( loggedIn_ )
    connection_->buffer()->keyReleaseEvent( e );
}

void KRFBCanvas::refresh()
{
  if ( loggedIn_ )
    connection_->refresh();
}

void KRFBCanvas::clipboardChanged()
{
  if ( loggedIn_ ) {
      connection_->sendCutText( qApp->clipboard()->text() );
  }
}

void KRFBCanvas::doAutoScroll()
{
    if ( dx || dy ) {
	scrollBy( dx, dy );
	QApplication::postEvent( viewport(), autoMouseEvent );
	dx = 0;
	dy = 0;
    }
}

