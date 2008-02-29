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

#ifndef __CATEGORYEDIT_H__
#define __CATEGORYEDIT_H__

#include "categoryeditbase_p.h"
#include <qarray.h>

class CategoryEditPrivate;

class CategoryEdit : public CategoryEditBase
{
    Q_OBJECT

public:
    CategoryEdit( QWidget *parent = 0, const char *name = 0 );
    CategoryEdit( const QArray<int> &vlRecs, const QString &appName,
		  const QString &visibleName,
		  QWidget *parent = 0, const char *name = 0 );
    ~CategoryEdit();
    void setCategories( const QArray<int> &vlRecs,
			const QString &appName, const QString &visibleName );
    QArray<int> newCategories();
    void kludge();

protected:
    void accept();

protected slots:
    void slotAdd();
    void slotRemove();
    void slotSetText( QListViewItem *selected );
    void slotSetGlobal( bool isChecked );
    void slotTextChanged( const QString &strNew );

private:
    CategoryEditPrivate *d;
};

QString categoryFileName();

#endif
