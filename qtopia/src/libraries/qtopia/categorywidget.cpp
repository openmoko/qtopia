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

#include "categoryedit_p.h"
#include "categorywidget.h"

CategoryWidget::CategoryWidget( QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    ce = new CategoryEdit( this, name );
}

CategoryWidget::CategoryWidget( const QArray<int> &vlRecs,
				const QString &appName, QWidget *parent,
				const char *name )
    : QVBox( parent, name )
{
    ce = new CategoryEdit( vlRecs, appName, appName, this, name );
}

CategoryWidget::CategoryWidget( const QArray<int> &vlRecs,
				const QString &appName, const QString &visibleName,
				QWidget *parent, const char *name )
    : QVBox( parent, name )
{
    ce = new CategoryEdit( vlRecs, appName, visibleName, this, name );
}

CategoryWidget::~CategoryWidget()
{
}

void CategoryWidget::setCategories( const QArray<int> &vlRecs,
				    const QString &appName )
{
    ce->setCategories( vlRecs, appName, appName );
}

void CategoryWidget::setCategories( const QArray<int> &vlRecs,
				    const QString &appName,
				    const QString &visibleName )
{
    ce->setCategories( vlRecs, appName, visibleName );
}

QArray<int> CategoryWidget::newCategories()
{
    return ce->newCategories();
}

void CategoryWidget::kludge()
{
    ce->kludge();
}
