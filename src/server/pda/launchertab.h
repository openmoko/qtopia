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
#ifndef LAUNCHERTAB_H
#define LAUNCHERTAB_H

#include <qcolor.h>
#include <qstring.h>
#include <qwidget.h>
#include <qrect.h>
#include <qiconset.h>
#include <qlist.h>
#include <qdict.h>
#include <qtabbar.h>

class LauncherView;

class LauncherTab : public QTab
{
public:
    LauncherTab( const QString &idStr, LauncherView *viewPtr,
		 const QIconSet &iconSet,
		 const QString &textStr=QString::null )
	: QTab( iconSet, textStr )
    {
	type = idStr;
	view = viewPtr;
    }

    QString type;
    LauncherView *view;
    QColor bgColor;
    QColor fgColor;
};

class LauncherTabBar : public QTabBar
{
    Q_OBJECT
public:
    LauncherTabBar( QWidget *parent=0, const char *name=0 );
    ~LauncherTabBar();

    void insertTab( LauncherTab *, int index = -1 );
    void removeTab( QTab * );

    LauncherView *currentView() {
	return currentLauncherTab()->view;
    }

    LauncherTab *currentLauncherTab() {
	return (LauncherTab*) tab( currentTab() );
    }

    LauncherTab *launcherTab( const QString &id ) const { return tabs[id]; }

    void nextTab();
    void prevTab();

public slots:
    void showTab(const QString&);
    virtual void layoutTabs();

protected:
    virtual void paint( QPainter *p, QTab *t, bool f ) const;
    void paintLabel( QPainter* p, const QRect& br, QTab* t, bool has_focus ) const;

private:
    QList<LauncherTab> items;
    QDict<LauncherTab> tabs;
};

#endif
