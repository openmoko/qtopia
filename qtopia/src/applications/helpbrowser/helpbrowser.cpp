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

#define QTOPIA_INTERNAL_LANGLIST

#include "helpbrowser.h"
#include "helppreprocessor.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/mimetype.h>
#include <qtopia/applnk.h>
#include <qtopia/global.h>
#include <qtopia/stringutil.h>
#include <qtopia/contextbar.h>

#include <qdragobject.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qmenubar.h>
#include <qfile.h>
#include <qvbox.h>

#define HOMEPAGE "index.html"

MagicTextBrowser::MagicTextBrowser( QWidget* parent )
    : QTextBrowser( parent )
{
    setTextFormat( RichText );
}

void MagicTextBrowser::clear()
{
    // Clear text browser
    setCurrent( QString::null );
    
    // Clear backward history
    if( !backStack.isEmpty() ) {
        backStack.clear();
        emit hasBack( FALSE );
    }
    
    // Clear forward history
    if( !forwardStack.isEmpty() ) {
        forwardStack.clear();
        emit hasForward( FALSE );
    }
}

void MagicTextBrowser::setSource( const QString& file )
{
    if( file != current ) {
        // If not first page, add current to backward history
        if( !current.isNull() ) {
            backStack.push( current );
            if( backStack.count() == 1 ) emit hasBack( TRUE );
        }
        
        // Clear forward history
        if( !forwardStack.isEmpty() ) {
            forwardStack.clear();
            emit hasForward( FALSE );
        }
        
        setCurrent( file );
    }
}

void MagicTextBrowser::backward()
{
    if( !backStack.isEmpty() ) {
        // Add current to forward history
        forwardStack.push( current );
        if( forwardStack.count() == 1 ) emit hasForward( TRUE );
        
        // Remove last page from backward history and set current
        setCurrent( backStack.pop() );
        if( backStack.isEmpty() ) emit hasBack( FALSE );
    }
}

void MagicTextBrowser::forward()
{
    if( !forwardStack.isEmpty() ) {
        // Add current to backward history
        backStack.push( current );
        if( backStack.count() == 1 ) emit hasBack( TRUE );
        
        // Remove last page from forward history and set current
        setCurrent( forwardStack.pop() );
        if( forwardStack.isEmpty() ) emit hasForward( FALSE );
    }
}

void MagicTextBrowser::setCurrent( const QString& file )
{
    current = file;
    if ( current.isNull() ) {
        QTextBrowser::setSource( QString::null );
    } else {
        HelpPreProcessor hpp( file );
        QString source = hpp.text();
        if( !( magic( file, "applications", source ) || magic( file, "games", source ) || magic( file, "settings", source ) ) )
            setText( source );
    }
}

bool MagicTextBrowser::magic( const QString& file, const QString& name, const QString& source )
{
    if( "qpe-" + name + ".html" == file ) {
        QRegExp re( "<qtopia-" + name + ">.*</qtopia-" + name + ">" );
        int start, len;
        if( ( start = re.match( source, 0, &len ) ) >= 0 ) {
            QString copy = source;
            setText( copy.replace( start, len, generate( name ) ) );
            return TRUE;
        }
    }
    return FALSE;
}

QString MagicTextBrowser::generate( const QString& name )
{
    QString dir = MimeType::appsFolderName() + "/" + name[ 0 ].upper() + name.mid( 1 );
    AppLnkSet lnkset( dir );
    AppLnk *lnk;
    QString s;
    QMap<QString,AppLnk*> ordered;
    for( QListIterator<AppLnk> it( lnkset.children() ); ( lnk = it.current() ); ++it ) {
	ordered[Qtopia::dehyphenate( lnk->name() )] = lnk;
    }
    for( QMap<QString,AppLnk*>::ConstIterator mit=ordered.begin(); mit!=ordered.end(); ++mit ) {
        QString name = mit.key();
	lnk = mit.data();
        QString icon = lnk->icon();
        QString helpFile = lnk->exec() + ".html";
        QStringList helpPath = Global::helpPath();
        QStringList::ConstIterator it;
        for( it = helpPath.begin(); it != helpPath.end() && !QFile::exists( *it + "/" + helpFile ); ++it )
	    ;
        if( it != helpPath.end() ) {
	    s += "<br><a href=" + helpFile + "><img src=" + icon + "> " + name + "</a>\n";
	}
    }
    return s;
}

HelpBrowser::HelpBrowser( QWidget* parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f )
{
    init();
}

void HelpBrowser::init()
{
#ifdef QTOPIA_PHONE
    pressed = closeOk = FALSE;
#endif
    setIcon( Resource::loadPixmap( "HelpBrowser" ) );
    setBackgroundMode( PaletteButton );
    
#ifdef DEBUG
    QVBox *box = new QVBox( this );
    browser = new MagicTextBrowser( box );
    location = new QLabel( box );
#else
    browser = new MagicTextBrowser( this );
#endif
    connect( browser, SIGNAL( textChanged() ),
	     this, SLOT( textChanged() ) );

#ifdef DEBUG
    setCentralWidget( box );
#else
    setCentralWidget( browser );
#endif
    setToolBarsMovable( FALSE );

    backAction = new QAction( tr( "Back" ), Resource::loadIconSet( "back" ), QString::null, 0, this, 0 );
    backAction->setWhatsThis( tr( "Move backward one page." ) );
    backAction->setEnabled( FALSE );
    connect( backAction, SIGNAL( activated() ), browser, SLOT( backward() ) );
    connect( browser, SIGNAL( hasBack( bool ) ), backAction, SLOT( setEnabled( bool ) ) );

    forwardAction = new QAction( tr( "Forward" ), Resource::loadIconSet( "forward" ), QString::null, 0, this, 0 );
    forwardAction->setWhatsThis( tr( "Move forward one page." ) );
    forwardAction->setEnabled( FALSE );
    connect( forwardAction, SIGNAL( activated() ), browser, SLOT( forward() ) );
    connect( browser, SIGNAL( hasForward( bool ) ), forwardAction, SLOT( setEnabled( bool ) ) );

    QAction *homeAction = new QAction( tr( "Home" ), Resource::loadIconSet( "home" ), QString::null, 0, this, 0 );
    homeAction->setWhatsThis( tr( "Go to the home page." ) );
    connect( homeAction, SIGNAL( activated() ), this, SLOT( goHome() ) );

#ifdef QTOPIA_PHONE
    contextMenu = new ContextMenu(this);

    backAction->addTo( contextMenu );
    forwardAction->addTo( contextMenu );
    homeAction->addTo( contextMenu );
#else
    QToolBar* toolbar = new QToolBar( this );
    toolbar->setHorizontalStretchable( TRUE );
    
    QMenuBar *menu = new QMenuBar( toolbar );
    toolbar = new QToolBar( this );
    
    QPopupMenu *helpMenu = new QPopupMenu( this );
    homeAction->addTo( helpMenu );
    backAction->addTo( helpMenu );
    forwardAction->addTo( helpMenu );
    
    menu->insertItem( tr( "Page" ), helpMenu );

    backAction->addTo( toolbar );
    forwardAction->addTo( toolbar );
    homeAction->addTo( toolbar );
#endif

    setFocusProxy( browser );
    browser->setFrameStyle( QFrame::NoFrame );
    
    connect( qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)),
	     this, SLOT(appMessage(const QCString&,const QByteArray&)) );
             
#ifdef QTOPIA_PHONE
    closeTimer = new QTimer( this );
    connect( closeTimer, SIGNAL( timeout() ), this, SLOT( setBackDisabled() ) );
    connect( browser, SIGNAL( hasBack( bool ) ), this, SLOT( setBack( bool ) ) );
    browser->installEventFilter( this );
#endif
             
    browser->setSource( HOMEPAGE );
}

void HelpBrowser::appMessage(const QCString& msg, const QByteArray& data)
{
    if ( msg == "showFile(QString)" ) {
        QPEApplication::setKeepRunning();
	QDataStream ds(data,IO_ReadOnly);
	QString fn;
	ds >> fn;
	setDocument( fn );
    }
}

void HelpBrowser::setDocument( const QString &doc )
{
    if ( !doc.isEmpty() ) {
        browser->clear();
        browser->setSource( doc );
    }
}

void HelpBrowser::goHome()
{
    browser->setSource( HOMEPAGE );
}

void HelpBrowser::textChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( tr("Help Browser") );
    else
	setCaption( browser->documentTitle() ) ;
#ifdef DEBUG
    location->setText( browser->source() );
#endif
}

#ifdef QTOPIA_PHONE
bool HelpBrowser::eventFilter( QObject*, QEvent* e )
{
#define TIMEOUT 500
    switch( e->type() ) {
    case QEvent::KeyPress:
        {
            QKeyEvent *ke = (QKeyEvent*)e;
            if( (ke->key() == Qt::Key_Back || ke->key() == Key_No) && !ke->isAutoRepeat() ) {
                if( backAction->isEnabled() ) closeTimer->start( TIMEOUT, TRUE );
                else close();
               pressed = TRUE;
               return TRUE;
            }
        }
        break;
    case QEvent::KeyRelease:
        {
            QKeyEvent *ke = (QKeyEvent*)e;
            if( ke->key() == Qt::Key_Back && !ke->isAutoRepeat() && pressed ) {
                if( closeTimer->isActive() ) {
                    closeTimer->stop();
                    browser->backward();
                } else close();
                pressed = FALSE;
                return TRUE;
            }
        }
        break;
    default:
        // Ignore
        break;
    }
    
    return FALSE;
}
#endif

void HelpBrowser::setBackDisabled()
{
#ifdef QTOPIA_PHONE
    setBack( FALSE );
#endif
}

void HelpBrowser::setBack( bool b )
{
#ifdef QTOPIA_PHONE
    if( b ) ContextBar::setLabel( this, Qt::Key_Back, ContextBar::Previous );
    else ContextBar::setLabel( this, Qt::Key_Back, ContextBar::Back );
#else
    Q_UNUSED( b );
#endif 
}

#ifdef QTOPIA_PHONE
void HelpBrowser::close()
{
    closeOk = TRUE;
    QWidget::close();
}
#endif

void HelpBrowser::closeEvent( QCloseEvent* e )
{
#ifdef QTOPIA_PHONE
    if( closeOk ) e->accept();
#else
    browser->clear();
    browser->setSource( HOMEPAGE );
    e->accept();
#endif
}
