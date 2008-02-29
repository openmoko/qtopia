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

#define INCLUDE_MENUITEM_DEF

#include "startmenu.h"
#include "sidething.h"
#include "mrulist.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/applnk.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>

#include <qdict.h>

#include <stdlib.h>


// #define USE_CONFIG_FILE


StartMenu::StartMenu(QWidget *parent) : QLabel( parent )
{
    loadOptions();

    int sz = AppLnk::smallIconSize()+3;
    QPixmap pm;
    pm.convertFromImage(Resource::loadImage(startButtonPixmap).smoothScale(sz,sz));
    setPixmap(pm);
    setFocusPolicy( NoFocus );
    //setFlat( startButtonIsFlat );

    apps = 0;
    launchMenu = 0;
    reloadApps();
}


void StartMenu::mousePressEvent( QMouseEvent * )
{
    launch();
}


StartMenu::~StartMenu()
{
    delete apps;
}


void StartMenu::loadOptions()
{
#ifdef USE_CONFIG_FILE
    // Read configuration file
    Config config("StartMenu");
    config.setGroup( "StartMenu" );
    QString tmpBoolString1 = config.readEntry( "UseWidePopupMenu", "FALSE" );
    useWidePopupMenu  = ( tmpBoolString1 == "TRUE" ) ? TRUE : FALSE;
    QString tmpBoolString2 = config.readEntry( "StartButtonIsFlat", "TRUE" );
    startButtonIsFlat = ( tmpBoolString2 == "TRUE" ) ? TRUE : FALSE;
    QString tmpBoolString3 = config.readEntry( "UseMRUList", "TRUE" );
    popupMenuSidePixmap = config.readEntry( "PopupMenuSidePixmap", "sidebar" );
    startButtonPixmap = config.readEntry( "StartButtonPixmap", "go" );
#else
    // Basically just #include the .qpe_menu.conf file settings
    useWidePopupMenu = FALSE;
    popupMenuSidePixmap = "sidebar";
    startButtonIsFlat = TRUE;
    startButtonPixmap = "go"; // No tr
#endif
}


void StartMenu::createMenu()
{
    delete launchMenu;
//    if ( useWidePopupMenu )
//	launchMenu = new PopupWithLaunchSideThing( this, &popupMenuSidePixmap );
 //   else
        launchMenu = new StartPopupMenu( this );
    loadMenu( apps, launchMenu );
}

void StartMenu::reloadApps()
{
    Config cfg("Taskbar");
    cfg.setGroup("Menu");
    bool ltabs = cfg.readBoolEntry("LauncherTabs",TRUE);
    bool lot = cfg.readBoolEntry("LauncherOther",TRUE);
    bool lt = ltabs || lot;
    if ( launchMenu && apps && !lt )
	return; // nothing to do

    if ( lt ) {
	delete apps;
	apps = new AppLnkSet( QPEApplication::qpeDir() + "apps" );
    }
    if ( launchMenu ) {
	int i;
	for (i=0; i<(int)launchMenu->count(); i++) {
	    QMenuItem* item = launchMenu->findItem(launchMenu->idAt(i));
	    if ( item && item->isSeparator() ) {
		i++;
		break;
	    }
	}
	while (i<(int)launchMenu->count())
	    launchMenu->removeItemAt(i);
	loadMenu(apps,launchMenu);
    } else {
	createMenu();
    }
}

void StartMenu::itemSelected( int id )
{
    const AppLnk *app = apps->find( id );
    if ( app )
	app->execute();
}

bool StartMenu::loadMenu( AppLnkSet *folder, QPopupMenu *menu )
{
    bool result = FALSE;

    Config cfg("Taskbar");
    cfg.setGroup("Menu");

    bool ltabs = cfg.readBoolEntry("LauncherTabs",TRUE);
    bool lot = cfg.readBoolEntry("LauncherOther",TRUE);
    bool sepfirst = !ltabs && !lot;
    if ( ltabs || lot ) {
	QDict<QPopupMenu> typpop;
	QStringList typs = folder->types();
	for (QStringList::Iterator tit=typs.begin(); tit!=typs.end(); ++tit) {
	    if ( !(*tit).isEmpty() ) {
		QPopupMenu *new_menu;
		if ( ltabs ) {
		    new_menu = new StartPopupMenu( menu );
		    connect( new_menu, SIGNAL(activated(int)), SLOT(itemSelected(int)) );
		    menu->insertItem( folder->typePixmap(*tit), *tit, new_menu );
		} else {
		    new_menu = (QPopupMenu*)1;
		}
		typpop.insert(*tit, new_menu);
	    }
	}

	QListIterator<AppLnk> it( folder->children() );
	bool f=TRUE;
	for ( ; it.current(); ++it ) {
	    AppLnk *app = it.current();
	    if ( app->type() == "Separator" ) { // No tr
		if ( lot ) {
		    menu->insertSeparator();
		    sepfirst = f && !ltabs;
		}
	    } else {
		f = FALSE;
		QString t = app->type();
		QPopupMenu* pmenu = typpop.find(t);
		bool sort = FALSE;
		if ( ltabs ) {
		    sort = pmenu;
		    if ( !pmenu && lot )
			pmenu = menu;
		} else {
		    if ( !pmenu )
			pmenu = menu;
		    else
			pmenu = 0;
		}
		if ( pmenu ) {
		    QString t = app->name();
		    t.replace(QRegExp("&"),"&&"); // escape shortcut character
		    int idx=-1;
		    if ( sort ) {
			// Overall, sorting takes O(n^3), because of text() is O(n)
			// Fortunately, 20^3 is still small. If it is a problem, this
			// whole function needs a re-write.
			for (idx=0; idx < (int)pmenu->count() && pmenu->text(pmenu->idAt(idx)) < t; ++idx)
			    ;
		    }
		    QIconSet i = Resource::loadIconSet(app->icon());
		    if (i.pixmap().isNull()) {
			i = Resource::loadIconSet("UnknownDocument");
		    }
		    pmenu->insertItem( i, t, app->id(), idx );
		}
		result=TRUE;
	    }
	}
	if ( !menu->count() )
	    sepfirst = TRUE;
    }

    launchMenu->setName(sepfirst ? "accessories" : "accessories_need_sep"); // No tr

    if ( result )
	connect( menu, SIGNAL(activated(int)), SLOT(itemSelected(int)) );

    return result;
}


void StartMenu::launch()
{
    int y = mapToGlobal( QPoint() ).y() - launchMenu->sizeHint().height();

    if ( launchMenu->isVisible() ) 
        launchMenu->hide();
    else
        launchMenu->popup( QPoint( 1, y ) );
}

const AppLnk* StartMenu::execToLink(const QString& appname)
{
    const AppLnk* a = apps->findExec( appname );
    return a;
}

void StartPopupMenu::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_F33 || e->key() == Key_Space ) {
	// "OK" button, little hacky
	QKeyEvent ke(QEvent::KeyPress, Key_Enter, 13, 0);
	QPopupMenu::keyPressEvent( &ke );
    } else {
	QPopupMenu::keyPressEvent( e );
    }
}
