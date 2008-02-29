/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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
#include <qtopia/qpeapplication.h>
#include <qtopia/storage.h>
#include <qtopia/contextmenu.h>
#include <qtopia/contextbar.h>
#include <qtopia/categoryselect.h>
#include <qtopia/categorydialog.h>
#include <qtopia/config.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qwidgetstack.h>
#include <qmenubar.h>
#include <qdir.h>
#include <stdlib.h>
#include "action.h"
#include "fileinfo.h"
#include "mediaselector.h"
#include "mediaselectorwidget.h"
#include "documentlist.h"
#include "loadingwidget.h"


#include <qtopia/categoryselect.h>
#include <qtopia/categories.h>
#include <qwhatsthis.h>


#ifndef QTOPIA_PHONE
class CategorySelectX : public CategorySelect, public ContainsCategoryInterface {
    public:
	CategorySelectX(QWidget *parent = 0, const char *name = 0, int width = 0) :
	    CategorySelect(parent, name, width) {
	}
	bool containsCategoryInterface( const QArray<int>& categories ) {
	    return containsCategory(categories);
	}
	void connectSelectedToSlot(QObject *obj, const char *slot) {
	    connect(this, SIGNAL(signalSelected(int)), obj, slot);
	}
	int currentCategoryId() {
	    return currentCategory();
	}
	void setCurrentCategoryId(int id) {
	    setCurrentCategory(id);
	}
};
#endif

class CategorySelectDialogX : public CategorySelectDialog, public ContainsCategoryInterface {
    public:
	CategorySelectDialogX(const QString &appName, QWidget *parent=0, const char *name=0, bool modal=FALSE) :
	    CategorySelectDialog(appName, parent, name, modal) {
	}
	bool containsCategoryInterface( const QArray<int>& categories ) {
	    return containsCategory(categories);
	}
	void connectSelectedToSlot(QObject *obj, const char *slot) {
	    connect(this, SIGNAL(selected(int)), obj, slot);
	}
	int currentCategoryId() {
	    return currentCategory();
	}
	void setCurrentCategoryId(int id) {
	    setCurrentCategory(id);
	}
};


/*!
  \class MediaSelectorWidget mediaselectorwidget.h
  \brief This class is specific to the mediaplayer apps.

    It is currently private to those apps, and its API,
    and therefore binary compatibilty may change in later
    versions.

    This class loads and displays the MediaSelector or the
    MediaScanningProgress depending if finding documents or not.
*/


class MediaSelectorWidgetPrivate {
public:
    QToolBar *bar;

    Action *tbPlay;
    Action *tbAdd;
    Action *tbRemove;
    Action *tbAddRemove;
    Action *tbUp;
    Action *tbDown;
    Action *tbLoop;
    Action *tbFull;
    Action *tbScale;
    Action *tbShuffle;
    Action *tbSort;
    Action *tbInfo;
    Action *tbSelect;
    Action *actionCategory;

    QMenuBar *menu;
    QPopupMenu *playlistMenu;
    int playlistMenuId;
    QPopupMenu *optionsMenu;
    int optionsMenuId;
    bool playlistInMenu; 
    QPopupMenu *skinMenu;
    int skinMenuId;
    bool skinInMenu; 
    QStringList skinID;
    unsigned int skinCount;

    MediaSelector *fileList;
    QVBox *fileListVBox;
    MediaScanningProgress *loadingWidget;
    QVBox *loadingWidgetVBox;
    QWidgetStack *stack;
    AppDocumentList *list;
    ContainsCategoryInterface *catSel;

    bool setDocumentActive;
    DocLnk setDoc;
};


MediaSelectorWidget::MediaSelectorWidget(QWidget *parent, const char* name, QString filter, bool videoButtons, WFlags fl)
    : QMainWindow(parent, name, fl), MediaSelectorBase()
{
    d = new MediaSelectorWidgetPrivate;
    d->setDocumentActive = FALSE;
    d->skinMenu = 0;

    setToolBarsMovable( FALSE );
    setBackgroundMode( PaletteButton );

    d->stack = new QWidgetStack(this);

    d->loadingWidgetVBox = new QVBox(d->stack);
    d->stack->addWidget(d->loadingWidgetVBox, 0);
    d->loadingWidget = new MediaScanningProgress(d->loadingWidgetVBox);
    d->stack->raiseWidget(0);

    d->fileListVBox = new QVBox(d->stack);

#ifndef QTOPIA_PHONE
    // Create Toolbar
    QHBox *hbox = new QHBox(d->fileListVBox);
    QToolBar *toolbar = new QToolBar("toolbar", this, hbox );
    toolbar->setHorizontalStretchable( true );

    // Create Menubar
    d->menu = new QMenuBar( toolbar );
    d->menu->setMargin( 0 );
    d->playlistMenu = new QPopupMenu( d->menu );
    d->playlistMenuId = d->menu->insertItem( tr( "Playlist" ), d->playlistMenu );

    d->optionsMenu = new QPopupMenu( d->menu );
    d->optionsMenuId = d->menu->insertItem( tr( "Options" ), d->optionsMenu );

    d->skinMenu = new QPopupMenu( d->menu );
    d->skinMenuId = d->optionsMenu->insertItem( tr( "Skin" ), d->skinMenu );
    d->skinInMenu = false;

    // Create Toolbar
    d->bar = new QToolBar( "toolbar", this, hbox );
    d->bar->setLabel( tr( "Play Operations" ) );
#endif

    d->stack->addWidget(d->fileListVBox, 1);
    d->fileList = new MediaSelector(d->fileListVBox);
    d->list = new AppDocumentList( filter, this, "list" ); // No tr

#ifndef QTOPIA_PHONE
    QHBox *hb = new QHBox( d->fileListVBox );
    StorageInfo si;
    QComboBox *locMenu = new QComboBox( hb );
    locMenu->insertStringList( si.fileSystemNames() );
    locMenu->insertItem( MediaSelector::tr("All Locations") );
    locMenu->setCurrentItem( locMenu->count() - 1 );
    d->fileList->setLocationFilter( locMenu );
    connect(locMenu, SIGNAL(activated(int)), d->list, SLOT(resend()));
    Categories c;
    QArray<int> vl( 0 );
    c.load(categoryFileName());
    CategorySelectX *catSel = new CategorySelectX( hb );
    catSel->setRemoveCategoryEdit(true);
    catSel->setCategories( vl, "Document View", // No tr
	tr("Document View") );
    catSel->setAllCategories(true);
    QWhatsThis::add( catSel, tr("Show documents in this category") );
    d->catSel = catSel;
#else
    CategorySelectDialogX *catDlg = new CategorySelectDialogX("Document View", this, 0, true);
    catDlg->setAllCategories(true);
    d->catSel = catDlg;
#endif

#ifdef QTOPIA_PHONE
    // select all if not going to give feedback on current category
    // TODO: when out of string freeze, create a label
    // that gives feedback on id != -2.
    d->catSel->setCurrentCategoryId(-2);
#else
    Config cfg( "MediaPlayer" );
    cfg.setGroup("PlayListSettings");
    d->catSel->setCurrentCategoryId(cfg.readNumEntry("Category", 0));
#endif

    d->fileList->setCategoryFilter(d->catSel);
    d->catSel->connectSelectedToSlot(d->list, SLOT(resend()));
    connect( d->list, SIGNAL( added(const DocLnk&) ), d->fileList, SLOT( addLink(const DocLnk&) ) );
    connect( d->list, SIGNAL( changed(const DocLnk&,const DocLnk&) ), d->fileList, SLOT( changeLink(const DocLnk&,const DocLnk&) ) );
    connect( d->list, SIGNAL( removed(const DocLnk&) ), d->fileList, SLOT( removeLink(const DocLnk&) ) );
    connect( d->list, SIGNAL( allRemoved() ), d->fileList, SLOT( removeAll() ) );
    connect( d->list, SIGNAL( percentDone(int) ), this, SLOT( scanProgress(int) ) );

    d->fileList->setFocus(); 

    d->tbPlay    = new Action( this, tr( "Play" ),	 "play",	mediaPlayerState, SLOT(setPlaying(bool)), TRUE );
    d->tbInfo    = new Action( this, tr( "Properties" ), "info",	this,		  SLOT(showInfo()) );
    d->tbUp      = new Action( this, tr( "Move Up" ),	 "up",		d->fileList,	  SLOT(moveSelectedUp()) );
    d->tbDown    = new Action( this, tr( "Move Down" ),	 "down",	d->fileList,	  SLOT(moveSelectedDown()) );
    d->tbShuffle = new Action( this, tr( "Randomize" ),  "shuffle",	this,		  SLOT(shuffle()) );
    d->tbSort	 = new Action( this, tr( "Sort" ),	 "sort",	this,		  SLOT(sort()) );
    d->tbLoop    = new Action( this, tr( "Loop" ),       "loop",	mediaPlayerState, SLOT(setLooping(bool)), TRUE );
    d->tbFull    = new Action( this, tr( "Fullscreen" ), "fullscreen",	mediaPlayerState, SLOT(setFullscreen(bool)), TRUE );
    d->tbScale   = new Action( this, tr( "Scale" ),      "scale",	mediaPlayerState, SLOT(setScaled(bool)), TRUE );
    d->tbSelect  = new Action( this, tr( "Select All" ), "selectAll",	this,		  SLOT(selectAll()) );
    d->tbAdd     = new Action( this, tr( "Add to Category..." ), "categorize",	this,		  SLOT(categorize()) );

    d->actionCategory = new Action(this, tr("View Category..."),  "",	this,		  SLOT(selectCategory()));

#ifdef QTOPIA_PHONE
    // Create context menu
    ContextMenu *contextMenu = new ContextMenu( this );

    // Add actions to the context menu.
    d->tbInfo->addTo( contextMenu );
    d->tbUp->addTo( contextMenu );
    d->tbDown->addTo( contextMenu );
    if ( videoButtons ) {
        d->tbFull->addTo( contextMenu );
        d->tbScale->addTo( contextMenu );
    }

    d->tbSelect->addTo(contextMenu);
    d->tbAdd->addTo(contextMenu);
    d->actionCategory->addTo(contextMenu);
#else
    // Add actions to menu and toolbar
    d->tbPlay->addTo( d->bar );
    d->tbInfo->addTo( d->bar );
    d->tbUp->addTo( d->bar );
    d->tbDown->addTo( d->bar );
    d->tbLoop->addTo( d->optionsMenu );
    if ( videoButtons ) {
	d->tbFull->addTo( d->optionsMenu );
	d->tbScale->addTo( d->optionsMenu );
    }

    d->tbShuffle->addTo( d->playlistMenu );
    d->tbSort->addTo( d->playlistMenu );
    d->tbSelect->addTo( d->playlistMenu );
    d->tbAdd->addTo( d->playlistMenu );

    QString skinPath = QPEApplication::qpeDir() + "pics/mediaplayer/skins/"; 
    QDir skinDir( skinPath );
    d->skinCount = 0;
    d->skinID.clear();
    for ( unsigned int i = 0; i < skinDir.count(); i++ ) {
	if ( skinDir[i] != "." && skinDir[i] != ".." ) {
	    QString cfgfile = skinPath + skinDir[i] + "/config";
	    Config cfg(cfgfile,Config::File);
	    if ( cfg.isValid() ) {
		cfg.setGroup("Skin");
		d->skinMenu->insertItem( cfg.readEntry("Name"), this, SLOT( skinChanged(int) ), 0, d->skinCount );
	        if ( skinDir[i] == mediaPlayerState->skin() ) 
	            d->skinMenu->setItemChecked( d->skinCount, TRUE );
		d->skinID.append(skinDir[i]);
		d->skinCount++;
	    }
	}
    }
#endif

    connect( mediaPlayerState, SIGNAL( playingToggled(bool) ),    d->tbPlay,    SLOT( setOn2(bool) ) );
    connect( mediaPlayerState, SIGNAL( loopingToggled(bool) ),    d->tbLoop,    SLOT( setOn2(bool) ) );
    connect( mediaPlayerState, SIGNAL( shuffledToggled(bool) ),   d->tbShuffle, SLOT( setOn2(bool) ) );
    connect( mediaPlayerState, SIGNAL( fullscreenToggled(bool) ), d->tbFull,    SLOT( setOn2(bool) ) );
    connect( mediaPlayerState, SIGNAL( scaledToggled(bool) ),     d->tbScale,   SLOT( setOn2(bool) ) );
    connect( mediaPlayerState, SIGNAL( viewChanged(View) ),	  this,         SLOT( setView(View) ) );
    connect( mediaPlayerState, SIGNAL( viewClosed() ),		  this,         SLOT( viewClosed() ) );
    connect( mediaPlayerState, SIGNAL( next() ),		  this,         SLOT( next() ) );
    connect( mediaPlayerState, SIGNAL( prev() ),		  this,         SLOT( prev() ) );

    setCentralWidget(d->stack);

    initializeStates();
}



MediaSelectorWidget::~MediaSelectorWidget()
{
    Config cfg( "MediaPlayer" );
    cfg.setGroup("PlayListSettings");
    cfg.writeEntry("Category", d->catSel->currentCategoryId());
    delete d;
}


void MediaSelectorWidget::selectAll()
{
    d->fileList->selectAll();
}


void MediaSelectorWidget::categorize()
{
    d->fileList->categorize();
}


void MediaSelectorWidget::selectCategory()
{
#ifdef QTOPIA_PHONE
    QPEApplication::execDialog((CategorySelectDialogX*)d->catSel);
#endif
}


void MediaSelectorWidget::sort()
{
    d->fileList->setSorting(1);
    d->fileList->sort();
    d->fileList->setSorting(-1);
}


void MediaSelectorWidget::shuffle()
{
    d->fileList->setSorting(-1);
    d->fileList->setUpdatesEnabled(false);
    QList<QListViewItem> shuffledList;
    QListViewItem *item = d->fileList->firstChild();
    int count = 0;
    while (item) {
	unsigned int randomPos = (unsigned int)((double)rand() * count / RAND_MAX);
	randomPos = QMIN( randomPos, (unsigned int)count );
	shuffledList.insert( randomPos, item );
	count++;
	QListViewItem *nextItem = item->nextSibling();
	d->fileList->takeItem(item);
	item = nextItem;
    }
    while (count--)
	d->fileList->insertItem(shuffledList.at(count));
    d->fileList->setUpdatesEnabled(true);
    QTimer::singleShot(0,d->fileList,SLOT(updateContents()));
}


void MediaSelectorWidget::resizeEvent( QResizeEvent * )
{
#ifndef QTOPIA_PHONE
    QFontMetrics fm( qApp->font() );
    int w = width() - fm.width( tr( "Playlist" ) + tr( "Options" ) + tr( "Skin" ) );

    // Add it back in based on the width
    if ( w < 150 ) {
	if ( d->skinInMenu ) {
	    d->menu->removeItem( d->skinMenuId );
	    d->skinMenuId = d->optionsMenu->insertItem( tr( "Skin" ), d->skinMenu );
	    d->skinInMenu = FALSE;
	}
    } else {
	if ( !d->skinInMenu ) {
	    d->optionsMenu->removeItem( d->skinMenuId );
	    d->skinMenuId = d->menu->insertItem( tr( "Skin" ), d->skinMenu );
	    d->skinInMenu = TRUE;
	}
    }
#endif
}


void MediaSelectorWidget::scanProgress(int percent)
{
    int count = 0;
    d->loadingWidget->setProgress(percent);
    switch ( percent ) {
        case 0:
            d->stack->raiseWidget(0);
	    d->fileList->setUpdatesEnabled(false);
	    d->fileList->setSorting(-1);
	    break;
        case 100:
	    d->fileList->setUpdatesEnabled(true);
            d->stack->raiseWidget(1);
	    count = d->fileList->childCount();
#ifdef QTOPIA_PHONE
            if (count)
                ContextBar::setLabel( this, Key_Select, "play", tr("Play") );
            else
                ContextBar::setLabel( this, Key_Select, ContextBar::NoLabel );
#endif
	    d->tbPlay->setEnabled( count );
	    d->tbInfo->setEnabled( count );
	    d->tbUp->setEnabled( count > 1 );
	    d->tbDown->setEnabled( count > 1 );
	    break;
        default:
            break;
    }
}


bool MediaSelectorWidget::listHasFocus()
{
    return d->fileList->hasFocus();
}


void MediaSelectorWidget::loadFiles() 
{
    d->list->start();
}


void MediaSelectorWidget::skinChanged( int id )
{
    // Already checked, therefore this is the current skin, do nothing
    if ( !d->skinMenu || d->skinMenu->isItemChecked( id ) )
	return;
    // Update the checked menu item
    for ( int i = 0; i < (int)d->skinCount; i++ ) 
        d->skinMenu->setItemChecked( i, (i == id) );
    // Update the skin state
    mediaPlayerState->setSkin( d->skinID[id] );
}


void MediaSelectorWidget::initializeStates()
{
    d->tbPlay->setOn( mediaPlayerState->playing() );
    d->tbLoop->setOn( mediaPlayerState->looping() );
    d->tbShuffle->setOn( mediaPlayerState->shuffled() );
    d->tbFull->setOn( mediaPlayerState->fullscreen() );
    d->tbScale->setOn( mediaPlayerState->scaled() );
}


void MediaSelectorWidget::setDocument(const QString& fileref)
{
    d->setDocumentActive = true;
    d->setDoc = DocLnk( fileref );
}


void MediaSelectorWidget::setActiveWindow()
{
    mediaPlayerState->setView( mediaPlayerState->view() );
}


const DocLnk *MediaSelectorWidget::current()
{
    if ( d->setDocumentActive ) 
	return &d->setDoc;
    return d->fileList->current();
}


void MediaSelectorWidget::prev()
{
    mediaPlayerState->setPlaying(false);
    d->setDocumentActive = false;
    if ( d->fileList->prev() )
        mediaPlayerState->setPlaying(true);
    else
	mediaPlayerState->setList();
}


void MediaSelectorWidget::next()
{
    mediaPlayerState->setPlaying(false);
    d->setDocumentActive = false;
    if ( d->fileList->next() )
        mediaPlayerState->setPlaying(true);
    else
	mediaPlayerState->setList();
}


void MediaSelectorWidget::viewClosed()
{
    if ( d->setDocumentActive ) {
	mediaPlayerState->setPlaying(false);
	QTimer::singleShot(100, qApp, SLOT(quit()));
    } else {
	mediaPlayerState->setList();
    }
}


void MediaSelectorWidget::setView( View view )
{
    if ( view == ListView ) {
	d->setDocumentActive = false;
	d->list->resume();
    } else {
        d->list->pause();
    }
}


void MediaSelectorWidget::showInfo()
{
    const DocLnk *cur = current();
    if ( cur ) {
	MediaFileInfoWidget *infoWidget = new MediaFileInfoWidget( *cur, this );
	QPEApplication::execDialog( infoWidget );
    }
}

