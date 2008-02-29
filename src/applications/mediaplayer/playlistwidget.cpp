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
#include <qpe/qpemenubar.h>
#include <qpe/qpetoolbar.h>
#include <qpe/fileselector.h>
#include <qpe/qpeapplication.h>
#include <qpe/applnk.h>
#include <qpe/config.h>
#include <qpe/global.h>
#include <qpe/resource.h>
#include <qaction.h>
#include <qimage.h>
#include <qdir.h>
#include <qfile.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qlist.h>
#include <qlistbox.h>
#include <qmainwindow.h>
#include <qmessagebox.h>
#include <qtoolbutton.h>

#include "playlistselection.h"
#include "playlistwidget.h"
#include "mediaplayerstate.h"
#include "tabbedselector.h"
#include "action.h"
#include "fileinfo.h"

#include <stdlib.h>


class PlayListWidgetPrivate {
public:
    QPEToolBar *bar;

    Action *tbPlay;
    Action *tbAdd;
    Action *tbRemove;
    Action *tbAddRemove;
    Action *tbUp;
    Action *tbDown;
    Action *tbFull;
    Action *tbLoop;
    Action *tbScale;
    Action *tbShuffle;
    Action *tbInfo;

    TabbedSelector *ts;
    PlayListSelection *audioFiles;
    PlayListSelection *videoFiles;
    PlayListSelection *selectedFiles;

    QPEMenuBar *menu;
    QPopupMenu *playlistMenu;
    int playlistMenuId;
    QPopupMenu *optionsMenu;
    int optionsMenuId;
    QPopupMenu *skinMenu;
    int skinMenuId;
    unsigned int skinCount;

    bool setDocumentActive;
    DocLnk setDoc;
};


PlayListWidget::PlayListWidget( QWidget* parent, const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    d = new PlayListWidgetPrivate;
    d->setDocumentActive = FALSE;

    setCaption( tr("MediaPlayer") );
    setIcon( Resource::loadPixmap( "MPEGPlayer" ) );
    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );

    // Create Toolbar
    QPEToolBar *toolbar = new QPEToolBar( this );
    toolbar->setHorizontalStretchable( TRUE );

    // Create Menubar
    d->menu = new QPEMenuBar( toolbar );
    d->menu->setMargin( 0 );

    QPEToolBar *bar = new QPEToolBar( this );
    d->bar = bar;

    bar->setLabel( tr( "Play Operations" ) );

    d->ts = new TabbedSelector( this );
    d->audioFiles = d->ts->audioFiles();
    d->videoFiles = d->ts->videoFiles();
    d->selectedFiles = d->ts->selectedFiles();

    d->tbPlay   = new Action( this, tr( "Play" ),       "mediaplayer/play",    mediaPlayerState, SLOT(setPlaying(bool)), TRUE );
    d->tbPlay->addTo( bar );
    d->tbInfo   = new Action( this, tr( "Infomation" ), "mediaplayer/info",    this, SLOT(showInfo()) );
    d->tbInfo->addTo( bar );
    d->tbAddRemove = new Action( this, tr( "Add/Remove" ),    "mediaplayer/add_to_playlist",  this, SLOT(addRemoveSelected()) );
    d->tbAddRemove->addTo( bar );
    d->tbUp     = new Action( this, tr( "Move Up" ),   "mediaplayer/up",   d->ts, SLOT(moveSelectedUp()) );
    d->tbUp->addTo( bar );
    d->tbDown   = new Action( this, tr( "Move Down" ), "mediaplayer/down", d->ts, SLOT(moveSelectedDown()) );
    d->tbDown->addTo( bar );

    connect( d->ts, SIGNAL( currentChanged( QWidget * ) ), this, SLOT( updateActions() ) );

    d->playlistMenu = new QPopupMenu( this );
    d->playlistMenuId = d->menu->insertItem( tr( "Playlist" ), d->playlistMenu );
    Action *a = new Action( this, tr( "Clear" ),               "", d->ts,	       SLOT( clearList() ) );
    a->addTo( d->playlistMenu );
    a = new Action( this, tr( "Add all Music" ),       "", d->ts,            SLOT( addAllAudio() ) );
    a->addTo( d->playlistMenu );
    a = new Action( this, tr( "Add all Videos" ),      "", d->ts,            SLOT( addAllVideo() ) );
    a->addTo( d->playlistMenu );
    a = new Action( this, tr( "Add all" ),             "", d->ts,            SLOT( addAll() ) );
    a->addTo( d->playlistMenu );
#ifdef CAN_SAVE_LOAD_PLAYLISTS
    a = new Action( this, tr( "Save PlayList" ),       "", this,             SLOT( saveList() ) );
    a->addTo( d->playlistMenu );
    a = new Action( this, tr( "Load PlayList" ),       "", this,             SLOT( loadList() ) );
    a->addTo( d->playlistMenu );
#endif

    d->optionsMenu = new QPopupMenu( this );
    d->optionsMenuId = d->menu->insertItem( tr( "Options" ), d->optionsMenu );
    d->tbShuffle = new Action( this, tr( "Randomize" ),  "mediaplayer/shuffle", mediaPlayerState, SLOT(setShuffled(bool)), TRUE );
    d->tbShuffle->addTo( d->optionsMenu );
    d->tbLoop    = new Action( this, tr( "Loop" ),       "mediaplayer/loop",    mediaPlayerState, SLOT(setLooping(bool)), TRUE );
    d->tbLoop->addTo( d->optionsMenu );
    d->tbFull    = new Action( this, tr( "Fullscreen" ), "fullscreen",		mediaPlayerState, SLOT(setFullscreen(bool)), TRUE );
    d->tbFull->addTo( d->optionsMenu );
    d->tbScale   = new Action( this, tr( "Scale" ),      "mediaplayer/scale",   mediaPlayerState, SLOT(setScaled(bool)), TRUE );
    d->tbScale->addTo( d->optionsMenu );
    
    d->skinMenu = new QPopupMenu( this );
    d->skinMenuId = d->menu->insertItem( tr( "Skin" ), d->skinMenu );
    QString skinPath = QPEApplication::qpeDir() + "pics/mediaplayer/skins/"; 
    QDir skinDir( skinPath );
    d->skinCount = 0;
    for ( unsigned int i = 0; i < skinDir.count(); i++ ) {
	if ( skinDir[i] != "." && skinDir[i] != ".." ) {
	    QString skinTestPath = "mediaplayer/skins/" + skinDir[i] + "/audio_up";
	    if ( !Resource::findPixmap( skinTestPath ).isNull() ) {
	        d->skinMenu->insertItem( skinDir[i], this, SLOT( skinChanged(int) ), 0, d->skinCount );
	        if ( skinDir[i] == mediaPlayerState->skin() ) 
	            d->skinMenu->setItemChecked( d->skinCount, TRUE );
		d->skinCount++;
	    }
	}
    }

    connect( mediaPlayerState, SIGNAL( playingToggled( bool ) ),    d->tbPlay,    SLOT( setOn2( bool ) ) );
    connect( mediaPlayerState, SIGNAL( loopingToggled( bool ) ),    d->tbLoop,    SLOT( setOn2( bool ) ) );
    connect( mediaPlayerState, SIGNAL( shuffledToggled( bool ) ),   d->tbShuffle, SLOT( setOn2( bool ) ) );
    connect( mediaPlayerState, SIGNAL( fullscreenToggled( bool ) ), d->tbFull,    SLOT( setOn2( bool ) ) );
    connect( mediaPlayerState, SIGNAL( scaledToggled( bool ) ),     d->tbScale,   SLOT( setOn2( bool ) ) );
    connect( mediaPlayerState, SIGNAL( fullscreenToggled( bool ) ), d->tbScale,   SLOT( setEnabled( bool ) ) );

    connect( mediaPlayerState, SIGNAL( viewChanged(View) ),	    this,	  SLOT( setView(View) ) );

    setCentralWidget( d->ts );

    initializeStates();
    updateActions();
}


PlayListWidget::~PlayListWidget()
{
    writePlayList( "MediaPlayer" );
    delete d;
}


void PlayListWidget::resizeEvent( QResizeEvent * )
{
    // Remove it from any menu
    d->menu->removeItem( d->skinMenuId );
    d->menu->removeItem( d->playlistMenuId );
    d->optionsMenu->removeItem( d->skinMenuId );
    d->optionsMenu->removeItem( d->playlistMenuId );

    QFontMetrics fm( qApp->font() );
    int w = width() - fm.width( tr( "Options" ) + tr( "Playlist" ) + tr( "Skin" ) );

    // Add it back in based on the width
    if ( w < 5*30 ) {
	d->skinMenuId = d->optionsMenu->insertItem( tr( "Skin" ), d->skinMenu );
	w += fm.width( tr( "Playlist" ) );
    } else
        d->skinMenuId = d->menu->insertItem( tr( "Skin" ), d->skinMenu );

    // Do the same for the playlist menu
    if ( w < 5*30 ) // If this is a small screen, put the playlist options in 'options'
	d->playlistMenuId = d->optionsMenu->insertItem( tr( "Playlist" ), d->playlistMenu );
    else // Otherwise if there is space put it in the menubar
        d->playlistMenuId = d->menu->insertItem( tr( "Playlist" ), d->playlistMenu );

}


bool PlayListWidget::listHasFocus()
{
    return d->ts->hasFocus() || d->audioFiles->hasFocus() || d->videoFiles->hasFocus() || d->selectedFiles->hasFocus();
}


void PlayListWidget::loadFiles() 
{
    d->ts->startLoading();
    readPlayList( "MediaPlayer" );
}


void PlayListWidget::skinChanged( int id )
{
    // Already checked, therefore this is the current skin, do nothing
    if ( d->skinMenu->isItemChecked( id ) )
	return;
    // Update the checked menu item
    for ( unsigned int i = 0; i < d->skinCount; i++ ) 
        d->skinMenu->setItemChecked( i, ((int)i == id) );
    // Update the skin state
    mediaPlayerState->setSkin( d->skinMenu->text( id ) );
}


void PlayListWidget::addRemoveSelected()
{
    switch ( d->ts->tab() ) {
	case TabbedSelector::AudioTab:
	case TabbedSelector::VideoTab:
	    d->ts->addSelected();
	    break;
	case TabbedSelector::PlaylistTab:
	    d->selectedFiles->removeSelected();
	    break;
    }
}


void PlayListWidget::updateActions()
{
    switch ( d->ts->tab() ) {
	case TabbedSelector::AudioTab:
	case TabbedSelector::VideoTab:
	    d->tbAddRemove->setIconSet( Resource::loadIconSet("mediaplayer/add_to_playlist") );
	    break;
	case TabbedSelector::PlaylistTab:
	    d->tbAddRemove->setIconSet( Resource::loadIconSet("mediaplayer/remove_from_playlist") );
	    break;
	default:
	    break;
    }
}


void PlayListWidget::initializeStates()
{
    d->tbPlay->setOn( mediaPlayerState->playing() );
    d->tbLoop->setOn( mediaPlayerState->looping() );
    d->tbShuffle->setOn( mediaPlayerState->shuffled() );
    d->tbFull->setOn( mediaPlayerState->fullscreen() );
    d->tbScale->setOn( mediaPlayerState->scaled() );
    d->tbScale->setEnabled( mediaPlayerState->fullscreen() );
}


void PlayListWidget::readWinAmpPlayListFile( QString filename )
{
    Config cfg( filename );

    // WinAmp .pls file format fields
    cfg.setGroup("playlist");
    
    QString playlistName = cfg.readEntry( "PlaylistName", 0 );
    QString version = cfg.readEntry( "Version", 0 );
    int noOfFiles = cfg.readNumEntry( "NumberOfEntries", 0 );

    for ( int i = 0; i < noOfFiles; i++ ) {
	QString entryName;
	entryName.sprintf( "File%i", i + 1 );
	QString File = cfg.readEntry( entryName );
	entryName.sprintf( "Title%i", i + 1 );
	QString Title = cfg.readEntry( entryName );
	DocLnk lnk;
	lnk.setFile( File );
	lnk.setName( Title );
	if ( lnk.isValid() )
            addToSelection( lnk );
    }
}


void PlayListWidget::writeWinAmpPlayListFile( QString filename ) const
{
    Config cfg( filename );
    cfg.setGroup("playlist");

    int noOfFiles = 0;

    d->selectedFiles->first();
    do {
	const DocLnk *lnk = d->selectedFiles->current();
	if ( lnk ) {
	    QString entryName;
	    entryName.sprintf( "File%i", noOfFiles + 1 );
	    cfg.writeEntry( entryName, lnk->file() );
	    entryName.sprintf( "Title%i", noOfFiles + 1 );
	    cfg.writeEntry( entryName, lnk->name() );
	    noOfFiles++;
	}
    } while ( d->selectedFiles->advanceSelection() );
    
    cfg.writeEntry("NumberOfEntries", noOfFiles );
}


void PlayListWidget::readPlayList( QString filename )
{
    Config cfg( filename );

    cfg.setGroup("PlayList");
    
    int noOfFiles = cfg.readNumEntry("NumberOfFiles", 0 );

    for ( int i = 0; i < noOfFiles; i++ ) {
	QString entryName;
	entryName.sprintf( "File%i", i + 1 );
	QString linkFile = cfg.readEntry( entryName );
	DocLnk lnk( linkFile );
	if ( lnk.isValid() )
            addToSelection( lnk );
    }
}


void PlayListWidget::writePlayList( QString filename ) const
{
    Config cfg( filename );
    cfg.setGroup("PlayList");

    cfg.clearGroup();

    int noOfFiles = 0;

    d->selectedFiles->first();
    do {
	const DocLnk *lnk = d->selectedFiles->current();
	if ( lnk ) {
	    QString entryName;
	    entryName.sprintf( "File%i", noOfFiles + 1 );
	    cfg.writeEntry( entryName, lnk->linkFile() );
	    // if this link does exist, add it so we have the file
	    // next time...
	    if ( !QFile::exists( lnk->linkFile() ) ) {
		// the way writing lnks doesn't really check for out
		// of disk space, but check it anyway.
		if ( !lnk->writeLink() ) {
		    QMessageBox::critical( 0, tr("Out of space"),
					   tr( "There was a problem saving "
					       "the playlist.\nYour playlist "
					       "may be missing some entries\n"
					       "the next time you start it." )
					   );
		}
	    }		   
	    noOfFiles++;
	}
    } while ( d->selectedFiles->advanceSelection() );
    
    cfg.writeEntry("NumberOfFiles", noOfFiles );
}


void PlayListWidget::addToSelection( const DocLnk& lnk )
{
    d->selectedFiles->addToSelection( lnk );
}


void PlayListWidget::setDocument(const QString& fileref)
{
    d->ts->pauseLoading();

    if ( fileref.isNull() ) {
	QMessageBox::critical( 0, tr( "Invalid File" ), tr( "There was a problem in getting the file." ) );
	return;
    }

    d->setDocumentActive = TRUE;
    d->setDoc = DocLnk( fileref );

    // Start playing
    mediaPlayerState->setPlaying( FALSE );
    QTimer::singleShot( 1, mediaPlayerState, SLOT( setPlaying() ) );
}


void PlayListWidget::setActiveWindow()
{
    // When we get raised we need to ensure that it switches views
    View origView = mediaPlayerState->view();
    mediaPlayerState->setView( ListView ); // invalidate
    mediaPlayerState->setView( origView ); // now switch back
}


const DocLnk *PlayListWidget::current()
{
    if ( d->setDocumentActive ) 
	return &d->setDoc;

    switch ( d->ts->tab() ) {
	case TabbedSelector::AudioTab:
	    return d->audioFiles->current();
	case TabbedSelector::VideoTab:
	    return d->videoFiles->current();
	case TabbedSelector::PlaylistTab:
	    return d->selectedFiles->current();
    }
    return 0;
}


bool PlayListWidget::prev()
{
    if ( d->setDocumentActive ) {
        d->setDocumentActive = FALSE;
	d->ts->resumeLoading();
	return FALSE;
    }

    switch ( d->ts->tab() ) {
	case TabbedSelector::AudioTab:
	    return d->audioFiles->prev();
	case TabbedSelector::VideoTab:
	    return d->videoFiles->prev();
	case TabbedSelector::PlaylistTab:
	    return d->selectedFiles->prev();
    }
    return FALSE;
}


bool PlayListWidget::next()
{
    if ( d->setDocumentActive ) {
        d->setDocumentActive = FALSE;
	d->ts->resumeLoading();
	return FALSE;
    }

    switch ( d->ts->tab() ) {
	case TabbedSelector::AudioTab:
	    return d->audioFiles->next();
	case TabbedSelector::VideoTab:
	    return d->videoFiles->next();
	case TabbedSelector::PlaylistTab:
	    return d->selectedFiles->next();
    }
    return FALSE;
}


bool PlayListWidget::first()
{
    if ( d->setDocumentActive ) {
        d->setDocumentActive = FALSE;
	d->ts->resumeLoading();
	return FALSE;
    }

    switch ( d->ts->tab() ) {
	case TabbedSelector::AudioTab:
	    return d->audioFiles->first();
	case TabbedSelector::VideoTab:
	    return d->videoFiles->first();
	case TabbedSelector::PlaylistTab:
	    return d->selectedFiles->first();
    }
    return FALSE;
}


bool PlayListWidget::last()
{
    if ( d->setDocumentActive ) {
        d->setDocumentActive = FALSE;
	d->ts->resumeLoading();
	return FALSE;
    }

    switch ( d->ts->tab() ) {
	case TabbedSelector::AudioTab:
	    return d->audioFiles->last();
	case TabbedSelector::VideoTab:
	    return d->videoFiles->last();
	case TabbedSelector::PlaylistTab:
	    return d->selectedFiles->last();
    }
    return FALSE;
}


void PlayListWidget::saveList()
{
    QString filename;
//  pseudo code
//  filename = QLineEdit->getText();    
    writePlayList( filename + ".playlist" );
}


void PlayListWidget::loadList()
{
    QString filename;
//  pseudo code
//  filename = FileSelector->openFile( "*.playlist" );
    readPlayList( filename + ".playlist" );
}


void PlayListWidget::setView( View view )
{
    if ( view == ListView ) {
	d->setDocumentActive = FALSE;
	d->ts->resumeLoading();
	showMaximized();
    } else
	hide();
}


void PlayListWidget::showInfo()
{
    const DocLnk *cur = current();
    if ( cur ) {
	MediaFileInfoWidget *infoWidget = new MediaFileInfoWidget( *cur, 0 );
	QPEApplication::execDialog( infoWidget );
    }
}

