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

#include <qobjectlist.h>
#include <qwidgetstack.h>
#include <qwidgetlist.h>
#include <qscrollview.h>
#include <qtabwidget.h>

static void addScrollBars(QTabWidget* tw)
{
    // Put a scrollbar on every page.
    // Reasonably general. Works for QTabWidgets from Qt Designer.

    QWidgetStack* stack = (QWidgetStack*)tw->child(0,"QWidgetStack");

    const QObjectList *tabs = stack->children();
    if (tabs) {
	QObject *o;
	QWidgetList pages;
	QObjectListIt oi(*tabs);
	for (; (o = *oi); ++oi) {
	    if (o->isWidgetType() && strcmp(o->className(),"QWidgetStackPrivate::Invisible")!=0)
		pages.append((QWidget*)o);
	}
	QWidget *w;
	QStringList pagelabs;
	QWidget *v = tw->currentPage();
	for (QWidgetListIt wi(pages); (w = *wi); ++wi) {
	    pagelabs.append(tw->tabLabel(w));
	    tw->removePage(w);
	}
	QStringList::ConstIterator li = pagelabs.begin();
	for (QWidgetListIt wi(pages); (w = *wi); ++wi) {
	    QScrollView* sv = new QScrollView(stack,w->name()); // Move name (so hide magic still works)
	    w->setName(0);
	    sv->setFrameStyle(QFrame::NoFrame);
	    sv->setResizePolicy(QScrollView::AutoOneFit);
	    sv->addChild(w);
	    tw->addTab(sv,*li); ++li;
	    if ( w == v )
		v = sv;
	}
	if ( v )
	    tw->showPage(v);
    }
}
