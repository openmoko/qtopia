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

#ifndef __CATEGORYMENU_H__
#define __CATEGORYMENU_H__

#include <qtopia/qpeglobal.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qarray.h>
#include <qmap.h>

class CategoryMenuPrivate;
class QTOPIA_EXPORT CategoryMenu : public QPopupMenu
{
    Q_OBJECT
public:
    CategoryMenu( const QString &appName, bool, QWidget *parent = 0, 
	    const char *name = 0 );
    ~CategoryMenu();

    bool isSelected(const QArray<int> &cUids) const;

    void setCurrentCategory( int newCatUid );
    void setCurrentCategoryAll( );
    void setCurrentCategoryUnfiled( );

signals:
    void categoryChange();

public slots:
    void reload();

private slots:
    void mapMenuId(int);

private:
    QString appName;
    bool includeGlobal;

    QMap<int, int> menuToId;
    QMap<int, int> idToMenu;
    int currentMid;

    CategoryMenuPrivate *d;
};

#define QTOPIA_DEFINED_CATEGORYMENU
#include <qtopia/qtopiawinexport.h>

#endif
