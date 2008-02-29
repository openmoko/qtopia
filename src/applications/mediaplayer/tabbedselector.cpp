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

#include "tabbedselector.h"
#include "playlistselection.h"
#include "mediaplayerstate.h"
#include "audiowidget.h"
#include "documentlist.h"

#include <qtopia/fileselector.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/resource.h>
#include <qtopia/categoryselect.h>
#include <qtopia/qpeapplication.h>

#include <qwidget.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qtabbar.h>
#include <qpushbutton.h>
#include <qobjectlist.h>
#include <qwidgetstack.h>


class TabItemData {
public:
    void create( QWidget *parent, bool selectors = TRUE ) {
	tab = new QVBox( parent );
	tab->setMargin( 0 );
	tabList = new PlayListSelection( tab );
	if ( selectors ) {
	    StorageInfo si;
	    QArray<int> vl( 0 );
	    QHBox *hbox = new QHBox( tab );
	    locMenu = new QComboBox( hbox );
	    locMenu->insertStringList( si.fileSystemStrings() );
	    locMenu->insertItem( qApp->translate( "PlayListSelection", "All Locations" ) );
	    locMenu->setCurrentItem( locMenu->count() - 1 );
	    tabList->setLocationFilter( locMenu );
	    catMenu = new CategorySelect( hbox, "MediaPlayer" );
	    catMenu->setRemoveCategoryEdit( TRUE );
	    catMenu->setCategories( vl, "Document View", // No tr
		qApp->translate("FileSelector", "Document View") );
	    catMenu->setAllCategories( TRUE );
	    tabList->setCategoryFilter( catMenu );
	    parent->connect( catMenu, SIGNAL( signalSelected(int) ), parent, SLOT( categoryChanged() ) );
	    parent->connect( locMenu, SIGNAL( activated(int) ), parent, SLOT( locationChanged() ) );
	}
    }
    QVBox		*tab;
    PlayListSelection	*tabList;
    CategorySelect	*catMenu;
    QComboBox		*locMenu;
};


class TabbedSelectorPrivate {
public:
    DocumentList *list;
    TabItemData audioTab; 
    TabItemData videoTab; 
    TabItemData plistTab; 
};


TabbedSelector::TabbedSelector( QWidget *parent, const char *name )
    : QTabWidget( parent, name )
{
    d = new TabbedSelectorPrivate;

    d->audioTab.create( this );
    d->videoTab.create( this );
    d->plistTab.create( this, FALSE );

    d->list = new DocumentList( "video/*;audio/*", this, "list" ); // No tr

    connect( d->list, SIGNAL( added( const DocLnk & ) ), this, SLOT( addLink( const DocLnk & ) ) );
    connect( d->list, SIGNAL( allRemoved() ), this, SLOT( removeAll() ) );
    connect( d->list, SIGNAL( removed( const DocLnk & ) ), d->videoTab.tabList, SLOT( removeLink( const DocLnk & ) ) );
    connect( d->list, SIGNAL( removed( const DocLnk & ) ), d->audioTab.tabList, SLOT( removeLink( const DocLnk & ) ) );
    connect( d->list, SIGNAL( changed( const DocLnk &, const DocLnk & ) ), d->videoTab.tabList, SLOT( changeLink( const DocLnk &, const DocLnk & ) ) );
    connect( d->list, SIGNAL( changed( const DocLnk &, const DocLnk & ) ), d->audioTab.tabList, SLOT( changeLink( const DocLnk &, const DocLnk & ) ) );
//    connect( d->list, SIGNAL( doneForNow() ), this, SLOT( finishedGettingLinks() ) );

    connect( mediaPlayerState, SIGNAL( prevTab() ), this, SLOT( showPrevTab() ) );
    connect( mediaPlayerState, SIGNAL( nextTab() ), this, SLOT( showNextTab() ) );

    addTab( d->audioTab.tab, Resource::loadIconSet("mediaplayer/audio_tab"), tr("Audio") );
    addTab( d->videoTab.tab, Resource::loadIconSet("mediaplayer/video_tab"), tr("Video") );
    addTab( d->plistTab.tab, Resource::loadIconSet("mediaplayer/playlist_tab"), tr("Playlist") );

    setFocusPolicy( QWidget::NoFocus );

    // remove unnecessary border.
    QObjectList *list = queryList ( "QWidgetStack", "tab pages", FALSE, FALSE ); // No tr
    if ( list->first() )
	((QWidgetStack*)list->first())->setFrameStyle( QFrame::NoFrame );
    delete list;
}


TabbedSelector::~TabbedSelector()
{
    delete d;
}


void TabbedSelector::startLoading()
{
    d->list->start();
}


void TabbedSelector::pauseLoading()
{
    d->list->pause();
}


void TabbedSelector::resumeLoading()
{
    d->list->resume();
}


void TabbedSelector::categoryChanged()
{
    d->list->resend();
}


void TabbedSelector::locationChanged()
{
    d->list->resend();
}


void TabbedSelector::moveSelectedUp()
{
    if ( currentPage() == d->audioTab.tab )
	d->audioTab.tabList->moveSelectedUp();
    else if ( currentPage() == d->videoTab.tab )
	d->videoTab.tabList->moveSelectedUp();
    else 
	d->plistTab.tabList->moveSelectedUp();
}


void TabbedSelector::moveSelectedDown()
{
    if ( currentPage() == d->audioTab.tab )
	d->audioTab.tabList->moveSelectedDown();
    else if ( currentPage() == d->videoTab.tab )
	d->videoTab.tabList->moveSelectedDown();
    else 
	d->plistTab.tabList->moveSelectedDown();
}


void TabbedSelector::showPrevTab()
{
    if ( currentPage() == d->audioTab.tab )
	showPage( d->plistTab.tab );
    else if ( currentPage() == d->videoTab.tab )
	showPage( d->audioTab.tab );
    else 
	showPage( d->videoTab.tab );
}


void TabbedSelector::showNextTab()
{
    if ( currentPage() == d->audioTab.tab )
	showPage( d->videoTab.tab );
    else if ( currentPage() == d->videoTab.tab )
	showPage( d->plistTab.tab );
    else 
	showPage( d->audioTab.tab );
}


void TabbedSelector::showPlaylistTab()
{
    showPage( d->plistTab.tab );
}


void TabbedSelector::addLink( const DocLnk& doc )
{
    if ( doc.type().contains("video/") ) {
	d->videoTab.tabList->addLink( doc );
    } else if ( doc.type().contains("audio/") ) {
	d->audioTab.tabList->addLink( doc );
    }
}


void TabbedSelector::removeAll()
{
    d->videoTab.tabList->clear();
    d->audioTab.tabList->clear();
}


TabbedSelector::AVTab TabbedSelector::tab()
{
    if ( currentPage() == d->audioTab.tab )
	return AudioTab;
    if ( currentPage() == d->videoTab.tab )
	return VideoTab;
    return PlaylistTab;
}


void TabbedSelector::clearList()
{
    while ( d->plistTab.tabList->first() )
	d->plistTab.tabList->removeSelected();
}


void TabbedSelector::addAll()
{
    addAllAudio();
    addAllVideo();
}


void TabbedSelector::addAllAudio()
{
    d->audioTab.tabList->first();
    do {
	const DocLnk *lnk = d->audioTab.tabList->current();
	if (lnk != NULL) {
	    d->plistTab.tabList->addToSelection( *lnk );
	}
    } while ( d && d->audioTab.tabList->advanceSelection() );
}


void TabbedSelector::addAllVideo()
{
    d->videoTab.tabList->first();
    do {
	const DocLnk *lnk = d->videoTab.tabList->current();
	if (lnk != NULL) {
	    d->plistTab.tabList->addToSelection( *lnk );
	}
    } while ( d && d->videoTab.tabList->advanceSelection() );
}


void TabbedSelector::addSelected()
{
    if ( currentPage() == d->audioTab.tab )
	addASelected();
    if ( currentPage() == d->videoTab.tab )
	addVSelected();
}


void TabbedSelector::addASelected()
{
    const DocLnk *lnk = d->audioTab.tabList->current();
    if (lnk != NULL) {
	d->plistTab.tabList->addToSelection( *lnk );
	d->audioTab.tabList->advanceSelection();
    }
}


void TabbedSelector::addVSelected()
{
    const DocLnk *lnk = d->videoTab.tabList->current();
    if (lnk != NULL) {
	d->plistTab.tabList->addToSelection( *lnk );
	d->videoTab.tabList->advanceSelection();
    }
}


PlayListSelection *TabbedSelector::audioFiles()
{
    return d->audioTab.tabList;
}


PlayListSelection *TabbedSelector::videoFiles()
{
    return d->videoTab.tabList;
}


PlayListSelection *TabbedSelector::selectedFiles()
{
    return d->plistTab.tabList;
}

