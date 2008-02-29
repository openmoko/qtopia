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

#define INCLUDE_MENUITEM_DEF

#include <qtopia/accessory.h>
#include <qtopia/config.h>

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qguardedptr.h>
#include <qpopupmenu.h>

#include <pda/taskbar.h>
#include <pda/startmenu.h>

class AccessoryData {
public:
    AccessoryData(const QIconSet& icon, const QString& text, QPopupMenu* sm) :
	submenu(sm)
    {
	StartMenu *menu = startMenu();
	QPopupMenu* popup = menu->launchMenu;
	QString n = popup->name();
	if ( n != "accessories" ) { // No tr
	    if ( n == "accessories_need_sep" ) {
		popup->setName("accessories"); // No tr
		popup->insertSeparator(0);
	    } else {
		// First accessory on device that doesn't follow the new protocol.
		Config cfg("Taskbar");
		cfg.setGroup("Menu");
		const int std_items=cfg.readBoolEntry("LauncherOther",TRUE)
			  || cfg.readBoolEntry("LauncherTabs",TRUE);
		if ( std_items ) {
		    popup->setName("accessories_and_std");
		    popup->insertSeparator(0);
		} else {
		    popup->setName("accessories_only");
		    popup->clear();
		}
	    }
	}
	id = popup->insertItem(icon,text,submenu,-1,0);
    }

    ~AccessoryData()
    {
	StartMenu *menu = startMenu();
	if (menu) {
	    QPopupMenu* popup = menu->launchMenu;
	    if ( popup ) {
		int ind = popup->indexOf(id);
		if ( ind >= 0 ) {
		    popup->removeItemAt(ind);
		    QString n = popup->name();
		    if ( n == "accessories_only" && popup->count() == 0
		      || n == "accessories_and_std" && popup->count() == 6
			)
		    {
			// Last accessory on device that doesn't follow the new protocol.
			StartMenu *lastMenu = startMenu();
			if ( lastMenu->launchMenu == popup ) { // sanity check
			    delete lastMenu->launchMenu;
			    lastMenu->launchMenu = 0;
			    // Call function in server's namespace
			    QObject dummy;
			    lastMenu->connect(&dummy,SIGNAL(destroyed()),SLOT(createMenu()));
			}
		    }
		}
	    }
	}
	delete (QPopupMenu *) submenu;
    }

    QPopupMenu* popup()
    {
        return startMenu()->launchMenu;
    }

    StartMenu *startMenu() const
    {
	if ( !smenu ) {
	    QWidgetList *list = QApplication::allWidgets();
	    QWidgetListIt it( *list );
	    QWidget * w;
	    while ( (w=it.current()) != 0 ) {
		++it;
		if ( w->isA("StartMenu") ) {
		    AccessoryData *that = (AccessoryData *)this;
		    that->smenu = (StartMenu *)w;
		    delete list;
		    return that->smenu; // avoid being optimized out
		}
	    }
	    delete list;
	}

	return smenu;
    }

    QGuardedPtr<QPopupMenu> submenu;
    int id;
    QGuardedPtr<StartMenu> smenu;
};

/*!
  \class Accessory
  \brief The Accessory class allows applet plugins to add themselves to
  the Q menu.

  Accessories can either add a single menu entry, or add a sub-menu.

  Accessories are particularly useful when using real estate on the taskbar
  would be wasteful.

  First availability: Qtopia 1.6

  \ingroup qtopiaemb
  \sa TaskbarAppletInterface
*/

/*!
  \fn void Accessory::activated()

  This signal is emitted when the menu item is activated.
*/

/*!
  \fn void Accessory::activated(int)

  This signal is emitted when a menu item in the submenu is activated.
*/

/*!
  Constructs an Accessory object with parent \a parent.

  This constructor places a new menu entry in the Q menu, with the name \a text
  and using the iconset \a icon.

  The activated() signal will be emitted when the user selects this item.
*/
Accessory::Accessory(const QIconSet& icon, const QString& text, QWidget* parent) :
    QObject(parent),
    d(new AccessoryData(icon,text,0))
{
}

/*!
  Constructs an Accessory object with parent \a parent.

  This constructor places a new menu entry in the Q menu, with the name \a text
  and using the iconset \a icon.
  
  The \a submenu, if non-zero, must have no parent object. The activated(int) signal
  will be emitted when the user selects an item from the submenu.
*/
Accessory::Accessory(const QIconSet& icon, const QString& text, QPopupMenu* submenu, QWidget* parent) :
    QObject(parent),
    d(new AccessoryData(icon,text,submenu))
{
    if ( submenu )
	connect(submenu,SIGNAL(activated(int)),this,SIGNAL(activated(int)));
    else
	connect(d->popup(),SIGNAL(activated(int)),this,SLOT(maybeActivated(int)));
}

/*!
  Destructs Accessory.
*/
Accessory::~Accessory()
{
    delete d;
}

void Accessory::maybeActivated(int i)
{
    if ( i == d->id )
	emit activated();
}

/*!
  Returns the QIconSet displayed in the menu.

  \sa setIconSet()
*/
QIconSet Accessory::iconSet() const
{
    return *d->popup()->iconSet(d->id);
}

/*!
  Returns the text displayed in the menu.

  \sa setText()
*/
QString Accessory::text() const
{
    return d->popup()->text(d->id);
}

/*!
  Sets \a icon as the icon shown in the menu.

  \sa iconSet()
*/
void Accessory::setIconSet(const QIconSet& icon)
{
    d->popup()->changeItem(d->id,icon,text());
}

/*!
  Sets \a text as the text shown in the menu.

  \sa text()
*/
void Accessory::setText(const QString& text)
{
    d->popup()->changeItem(d->id,text);
}

/*!
  Returns a pointer to the sub-menu, or 0 if a sub-menu does not
  exist.
*/
QPopupMenu* Accessory::popup() const
{
    return d->submenu;
}
