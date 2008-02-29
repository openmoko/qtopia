/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "todayoptions.h"

#include <qtopia/resource.h>
#include <qtopia/config.h>

#include <qlayout.h>
#include <qtoolbutton.h>
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

    connect(inactiveView, SIGNAL( clicked(QListViewItem *) ),
	    this, SLOT( addItemToView(QListViewItem *) ) );
    inactiveView->setAllColumnsShowFocus( TRUE );
    QWhatsThis::add(inactiveView, tr("Lists the plugins which are available but not enabled.  Tap a plugin to enable it.") );
}

void TodayOptions::accept()
{
    writeConfig();

    QDialog::accept();
}

void TodayOptions::writeConfig()
{
    Config config("today");
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

