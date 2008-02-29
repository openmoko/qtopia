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

#define QTOPIA_INTERNAL_LANGLIST

#include "helpbrowser.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/resource.h>
#include <qtopia/mimetype.h>
#include <qtopia/applnk.h>
#include <qtopia/global.h>

#include <qstatusbar.h>
#include <qdragobject.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>
#include <qtoolbutton.h>
#include <qiconset.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstylesheet.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobjectlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qdatastream.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qpaintdevicemetrics.h>
#include <qaction.h>

#include <ctype.h>


HelpBrowser::HelpBrowser( QWidget* parent, const char *name, WFlags f )
    : QMainWindow( parent, name, f ),
      selectedURL()
{
    init( "index.html" );
}

class MagicTextBrowser : public QTextBrowser {
public:
    MagicTextBrowser(QWidget* parent) :
	QTextBrowser(parent)
    {
    }
    
    void setSource( const QString& source )
    {
	QTextBrowser::setSource(source);
	if ( magic(source,"applications") || magic(source,"games") || magic(source,"settings") ) // No tr
	    return;
	// Just those are magic (for now). Could do CGI here,
	// or in Qtopia's mime source factory.
    }

    bool magic(const QString& source, const QString& name)
    {
	if ( name+".html" == source ) {
	    QString fn = mimeSourceFactory()->makeAbsolute( source, context() );
	    const QMimeSource* m = mimeSourceFactory()->data( fn, context() );
	    if ( m ) {
		QString txt;
		if ( QTextDrag::decode(m,txt) ) {
		    QRegExp re("<qtopia-"+name+">.*</qtopia-"+name+">");
		    int start,len;
		    if ( (start=re.match(txt,0,&len))>=0 ) {
			QString generated = generate(name);
			txt.replace(start,len,generated);
			setText(txt);
			return TRUE;
		    }
		}
	    }
	}
	return FALSE;
    }

    QString generate(const QString& name) const
    {
	QString dir = MimeType::appsFolderName()+"/"+name[0].upper()+name.mid(1);
	AppLnkSet lnkset(dir);
	AppLnk* lnk;
	QString r;
	for (QListIterator<AppLnk> it(lnkset.children()); (lnk=it.current()); ++it) {
	    QString name = lnk->name();
	    QString icon = lnk->icon();
	    QString helpFile = lnk->exec()+".html";
	    QStringList helpPath = Global::helpPath();
	    bool helpExists = FALSE;
	    for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end() && !helpExists; ++it)
		helpExists = QFile::exists( *it + "/" + helpFile );
	    if ( helpExists ) {
		r += "<h3><a href="+helpFile+"><img src="+icon+">"+name+"</a></h3>\n";
	    }
	}
	return r;
    }
};

void HelpBrowser::init( const QString& _home )
{
    setIcon( Resource::loadPixmap( "HelpBrowser" ) );
    setBackgroundMode( PaletteButton );

    browser = new MagicTextBrowser( this );
    browser->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    connect( browser, SIGNAL( textChanged() ),
	     this, SLOT( textChanged() ) );

    setCentralWidget( browser );
    setToolBarsMovable( FALSE );

    if ( !_home.isEmpty() )
	browser->setSource( _home );

    QPEToolBar* toolbar = new QPEToolBar( this );
    toolbar->setHorizontalStretchable( TRUE );
    QPEMenuBar *menu = new QPEMenuBar( toolbar );

    toolbar = new QPEToolBar( this );
    // addToolBar( toolbar, "Toolbar");

    //QPopupMenu* go = new QPopupMenu( this );
    backAction = new QAction( tr( "Backward" ), Resource::loadIconSet( "back" ), QString::null, 0, this, 0 );
    connect( backAction, SIGNAL( activated() ), browser, SLOT( backward() ) );
    connect( browser, SIGNAL( backwardAvailable( bool ) ),
	     backAction, SLOT( setEnabled( bool ) ) );
    //backAction->addTo( go );
    backAction->addTo( toolbar );
    backAction->setEnabled( FALSE );

    forwardAction = new QAction( tr( "Forward" ), Resource::loadIconSet( "forward" ), QString::null, 0, this, 0 );
    connect( forwardAction, SIGNAL( activated() ), browser, SLOT( forward() ) );
    connect( browser, SIGNAL( forwardAvailable( bool ) ),
	     forwardAction, SLOT( setEnabled( bool ) ) );
    //forwardAction->addTo( go );
    forwardAction->addTo( toolbar );
    forwardAction->setEnabled( FALSE );

    QAction *a = new QAction( tr( "Home" ), Resource::loadIconSet( "home" ), QString::null, 0, this, 0 );
    connect( a, SIGNAL( activated() ), browser, SLOT( home() ) );
    //a->addTo( go );
    a->addTo( toolbar );

    bookm = new QPopupMenu( this );
    bookm->insertItem( tr( "Add Bookmark" ), this, SLOT( addBookmark() ) );
    bookm->insertItem( tr( "Remove from Bookmarks" ), this, SLOT( removeBookmark() ) );
    bookm->insertSeparator();
    connect( bookm, SIGNAL( activated( int ) ),
	     this, SLOT( bookmChosen( int ) ) );

    readBookmarks();

    //menu->insertItem( tr("Go"), go );
    menu->insertItem( tr( "Bookmarks" ), bookm );

    resize( 240, 300 );
    browser->setFocus();
    browser->setFrameStyle( QFrame::NoFrame );

    connect( qApp, SIGNAL(appMessage(const QCString&, const QByteArray&)),
	     this, SLOT(appMessage(const QCString&, const QByteArray&)) );
}

void HelpBrowser::appMessage(const QCString& msg, const QByteArray& data)
{
    if ( msg == "showFile(QString)" ) {
	QDataStream ds(data,IO_ReadOnly);
	QString fn;
	ds >> fn;
	setDocument( fn );
    }
}

void HelpBrowser::setDocument( const QString &doc )
{
    if ( !doc.isEmpty() )
	browser->setSource( doc );
    raise();    
}


void HelpBrowser::textChanged()
{
    if ( browser->documentTitle().isNull() )
	setCaption( tr("Help Browser") );
    else
	setCaption( browser->documentTitle() ) ;

    selectedURL = caption();
}

HelpBrowser::~HelpBrowser()
{
    QStringList bookmarks;
    QMap<int, Bookmark>::Iterator it2 = mBookmarks.begin();
    for ( ; it2 != mBookmarks.end(); ++it2 )
	bookmarks.append( (*it2).name + "=" + (*it2).file );

    QFile f2( Global::applicationFileName("helpbrowser", "bookmarks") );
    if ( f2.open( IO_WriteOnly ) ) {
	QDataStream s2( &f2 );
	s2 << bookmarks;
	f2.close();
    }
}

void HelpBrowser::pathSelected( const QString &_path )
{
    browser->setSource( _path );
}

void HelpBrowser::readBookmarks()
{
    QString file = Global::applicationFileName("helpbrowser", "bookmarks");
    if ( QFile::exists( file ) ) {
	QStringList bookmarks;
	QFile f( file );
	if ( f.open( IO_ReadOnly ) ) {
	    QDataStream s( &f );
	    s >> bookmarks;
	    f.close();
	}
	QStringList::Iterator it = bookmarks.begin();
	for ( ; it != bookmarks.end(); ++it ) {
	    Bookmark b;
	    QString current = *it;
	    int equal = current.find( "=" );
	    if ( equal < 1 || equal == (int)current.length() - 1 )
		continue;
	    b.name = current.left( equal );
	    b.file = current.mid( equal + 1 );
	    mBookmarks[ bookm->insertItem( b.name ) ] = b;
	} 
    }
}

void HelpBrowser::bookmChosen( int i )
{
    if ( mBookmarks.contains( i ) )
	browser->setSource( mBookmarks[ i ].file );
}

void HelpBrowser::addBookmark()
{
    Bookmark b;
    b.name = browser->documentTitle();
    b.file = browser->source();
    if (b.name.isEmpty() ) {
	b.name = b.file.left( b.file.length() - 5 ); // remove .html
    }
    QMap<int, Bookmark>::Iterator it;
    for( it = mBookmarks.begin(); it != mBookmarks.end(); ++it )
	if ( (*it).file == b.file ) return;
    mBookmarks[ bookm->insertItem( b.name ) ] = b;
}

void HelpBrowser::removeBookmark()
{
    QString file = browser->source();
    QMap<int, Bookmark>::Iterator it = mBookmarks.begin();
    for( ; it != mBookmarks.end(); ++it )
	if ( (*it).file == file ) {
	    bookm->removeItem( it.key() );
	    mBookmarks.remove( it );
		break;
	}
}
