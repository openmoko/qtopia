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

#include "todayoptions.h"

#include <qtopia/resource.h>
#include <qtopia/config.h>
#include <qtopia/custom.h>

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qlistview.h>
#include <qheader.h>
#include <qlabel.h>

#include <qwhatsthis.h>

TodayOptions::TodayOptions(QWidget *parent, const char *name, WFlags fl)
    : TodayOptionsBase(parent, name, fl)
{
    displayView->setSorting( -1, FALSE );
    displayView->setAllColumnsShowFocus( TRUE );
    QWhatsThis::add(displayView, tr("Lists the plugins and in which order they will be displayed in the today view.") );

    connect(moveUp, SIGNAL( clicked() ), this, SLOT( itemMoveUp() ) );
    QWhatsThis::add(moveUp, tr("Move the selected plugin one step up.") );

    connect(cut, SIGNAL( clicked() ), this, SLOT( itemCut() ) );
    QWhatsThis::add(cut, tr("Disable the selected plugin (Do not show in today view).") );

    connect(moveDown, SIGNAL( clicked() ), this, SLOT( itemMoveDown() ) );
    QWhatsThis::add(moveDown, tr("Move the selected plugin one step down.") );

    connect(inactiveView, SIGNAL( clicked(QListViewItem*) ),
	    this, SLOT( addItemToView(QListViewItem*) ) );
    inactiveView->setAllColumnsShowFocus( TRUE );
    QWhatsThis::add(inactiveView, tr("Lists the plugins which are available but not enabled.  Tap a plugin to enable it.") );
    Config config("today");
    config.setGroup("Start");
#ifndef QPE_DEFAULT_TODAY_MODE
#define QPE_DEFAULT_TODAY_MODE "Never"
#endif
    bool as = config.readEntry("Mode",QPE_DEFAULT_TODAY_MODE) == "Daily";
    autostart->setChecked(as);
}

void TodayOptions::accept()
{
    writeConfig();

    QDialog::accept();
}

void TodayOptions::writeConfig()
{
    Config config("today");
    config.setGroup("Start");
    config.writeEntry("Mode",autostart->isChecked() ? "Daily" : "Never");
    config.setGroup("view");
    config.clearGroup();

    QString str;
    QListViewItemIterator it(inactiveView);
    for ( ; it.current(); ++it) {
	    str = it.current()->text(0) + "_pos";
	    config.writeEntry( str, -1);
    }

    QListViewItemIterator it2(displayView);
    int count = 0;
    for ( ; it2.current(); ++it2) {
	    str = it2.current()->text(0) + "_pos";
	    config.writeEntry( str, count);
	    count++;
    }
    config.write();
}

void TodayOptions::setPlugins(QValueList<TodayPlugin> list)
{
    pList = list;

    QListViewItem *item, *lastItem = NULL;
    TodayPlugin plugin;
    for (uint i = 0; i < list.count(); i++) {
	plugin = pList[i];

	if ( plugin.active ) {
	    lastItem = new QListViewItem(displayView, lastItem, plugin.viewer->name() );
	    lastItem->setPixmap(0, plugin.viewer->icon() );
	} else {
	    item = new QListViewItem(inactiveView, plugin.viewer->name() );
	    item->setPixmap(0, plugin.viewer->icon() );
	}
    }
    displayView->setSelected( displayView->currentItem(), TRUE );
}

void TodayOptions::addItemToView(QListViewItem *item)
{
    if ( item ) {
	QListViewItem *i = new QListViewItem(displayView, item->text(0) );
	i->setPixmap(0, *item->pixmap(0) );

	delete item;
    }
}

void TodayOptions::itemMoveUp()
{
    QListViewItem *item = displayView->selectedItem();
    if ( item && item->itemAbove() )
	item->itemAbove()->moveItem( item );

    displayView->ensureItemVisible( item );
}

void TodayOptions::itemMoveDown()
{
    QListViewItem *item = displayView->selectedItem();
    if ( item && item->itemBelow() )
	item->moveItem( item->itemBelow() );

    displayView->ensureItemVisible( item );
}

void TodayOptions::itemCut()
{
    QListViewItem *item = displayView->selectedItem();
    if ( item ) {
	QListViewItem *i = new QListViewItem(inactiveView, item->text(0) );
	i->setPixmap(0, *item->pixmap(0) );

	delete item;
    }

    displayView->setSelected( displayView->currentItem(), TRUE );
    displayView->ensureItemVisible( displayView->selectedItem() );
}

