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

#include <qtopia/categories.h>
#include <qtopia/private/palmtoprecord.h>
#include <qtopia/qpeapplication.h>
#include <qlayout.h>
#include <qlistbox.h>

#include "categorydialog.h"

#include <stdlib.h>


class CategorySelectDialogPrivate
{
public:
    QString appName;
    QArray<int> appCats;
    Categories cats;
    QListBox *listBox;
    bool all;
};



CategorySelectDialog::CategorySelectDialog(const QString &appName, QWidget *parent, const char *name, bool modal)
    : QDialog(parent, name, modal)
{
    setCaption(tr("Select Category"));

    d = new CategorySelectDialogPrivate;
    d->appName = appName;
    d->cats.load(categoryFileName());
    d->all = FALSE;

    QVBoxLayout *vb = new QVBoxLayout(this);
    d->listBox = new QListBox(this);
    d->listBox->setMargin(0);
    d->listBox->setFrameStyle(QFrame::NoFrame);
    connect(d->listBox, SIGNAL(selected(int)), this, SLOT(catSelected()));
    connect(d->listBox, SIGNAL(mouseButtonClicked(int,QListBoxItem*,const QPoint&)), this, SLOT(catSelected()) );
    vb->addWidget(d->listBox);

    load();

    QPEApplication::setMenuLike(this, TRUE);

    connect(qApp, SIGNAL(categoriesChanged()), this, SLOT(categoriesChanged()));
}

CategorySelectDialog::~CategorySelectDialog()
{
    delete d;
}

void qpe_translateLabels(QStringList& strs);
QString qpe_translateLabel(const QString&);

void CategorySelectDialog::load()
{
    int cur = currentCategory();

    d->listBox->clear();

    bool loadOk = d->cats.load(categoryFileName());
    QStringList slApp = d->cats.labels( d->appName, TRUE, Categories::UnfiledLabel );
    d->appCats = d->cats.ids(d->appName, slApp);
    qpe_translateLabels(slApp);

    d->listBox->insertStringList( slApp );

    if (d->all)
	d->listBox->insertItem( qpe_translateLabel("_All"), d->listBox->count() );

    if ( cur != -1 && cur != -2 && loadOk ) {
	for ( int i = 0; i < int(d->appCats.size()); i++ ) {
	    if ( d->appCats[i] == cur ) {
		d->listBox->setCurrentItem( i );
		break;
	    }
	}
    } else if (d->all) {
	d->listBox->setCurrentItem( d->listBox->count()-1 );  // unfiled
    } else {
	d->listBox->setCurrentItem( slApp.count()-1 );  // unfiled
    }
}

void CategorySelectDialog::setAllCategories( bool all )
{
    if (d->all != all) {
	d->all = all;
	load();
    }
}

int CategorySelectDialog::currentCategory() const
{
    int i = d->listBox->currentItem();

    int r;
    if ( i == (int)d->appCats.count() )
	r = -1;
    else if ( i == -1 || i > (int)d->appCats.count() )  // only happen on "All"
	r = -2;
    else
	r =  d->appCats[i];

    return r;
}

void CategorySelectDialog::setCurrentCategory( int newCatUid )
{
    if ( newCatUid == -1 ) {
	d->listBox->setCurrentItem( d->appCats.count() );
    } else if ( newCatUid == -2 ) {
	d->listBox->setCurrentItem( d->appCats.count()+1 );
    } else {
	for ( int i = 0; i < int(d->appCats.size()); i++ ) {
	    if ( d->appCats[i] == newCatUid )
		d->listBox->setCurrentItem( i );
	}
    }
}


void CategorySelectDialog::catSelected()
{
    emit selected(currentCategory());
    if (isModal())
	accept();
}

/*!
  Note that this text is translated. It cannot be used as
  a parameter to Categories label functions.

  \sa currentCategoryLabel()
*/
QString CategorySelectDialog::currentCategoryText() const
{
    return qpe_translateLabel(currentCategoryLabel());
}

/*!
  Note that this label is not translated. It can be used as
  a parameter to Categories label functions.

  \sa currentCategoryText()
*/
QString CategorySelectDialog::currentCategoryLabel() const
{
    QString label;

    int id = currentCategory();
    if (id == -1) {
	label = "_Unfiled";
    } else if (id == -2) {
	label = "_All";
    } else {
	label = d->cats.label(d->appName, id);
    }

    return label;
}

/*!
  Returns whether any category in \a cats is selected.
*/
bool CategorySelectDialog::containsCategory( const QArray<int>& cats )
{
    int catId = currentCategory();
    if (catId == -2 || cats.contains(catId) || (catId == -1 && cats.isEmpty()))
	return TRUE;

    return FALSE;
}

void CategorySelectDialog::categoriesChanged()
{
    int prevCat = currentCategory();
    load();
    if (currentCategory() != prevCat) {
	emit selected(currentCategory());
    }
}
