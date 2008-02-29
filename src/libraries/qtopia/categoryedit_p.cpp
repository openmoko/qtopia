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

#include "categoryedit_p.h"

#include <qtopia/categories.h>
#include <qtopia/global.h>

#include <qdir.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qstringlist.h>
#include <qtoolbutton.h>
#include <qmessagebox.h>
#include <qapplication.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#endif


using namespace Qtopia;

enum RenameResult{
    Ok, Failed, MadeGlobal
};

//  Note that there is a duplicate of this class in libqtopia to allow access to this private class
class CategoryEditPrivate
{
public:
    CategoryEditPrivate( QWidget *parent, const QString &appName )
	: mCategories( parent, "" ),
	  mStrApp( appName ), w(parent)
    {
	editItem = 0;
	checkable = TRUE;
	nameChanged = FALSE;
	settingItem = FALSE;
	mCategories.load( categoryFileName() );
    }

    Categories mCategories;
    QListViewItem *editItem;
    QString mStrApp;
    QString mVisible;
    bool checkable, nameChanged;
    QWidget *w;
    QString orgName;
    bool settingItem;
};

CategoryEdit::CategoryEdit( QWidget *parent, const char *name )
    : CategoryEditBase( parent, name )
{
    d = 0;
    lvView->setMinimumHeight( 1 );
    connect(lvView, SIGNAL(currentChanged(QListViewItem*)),
	this, SLOT(enableButtons()));

#ifndef QTOPIA_DESKTOP
    connect( qApp, SIGNAL( categoriesChanged() ), SLOT( reloadCategories() ) );
#endif
}

CategoryEdit::CategoryEdit( const QArray<int> &recCats,
			    const QString &appName, const QString &visibleName,
			    QWidget *parent, const char *name )
    : CategoryEditBase( parent, name )
{
    d = 0;
    lvView->setMinimumHeight( 1 );
    setCategories( recCats, appName, visibleName );
    connect(lvView, SIGNAL(currentChanged(QListViewItem*)),
	this, SLOT(enableButtons()));

#ifndef QTOPIA_DESKTOP
    connect( qApp, SIGNAL( categoriesChanged() ), SLOT( reloadCategories() ) );
#endif
}

void CategoryEdit::reloadCategories()
{
    if ( !d )
	return;

    QValueList<int> l;
    for ( QListViewItemIterator it( lvView ); it.current(); ++it ) {
	if ( reinterpret_cast<QCheckListItem*>(it.current())->isOn() )
	    l.append( d->mCategories.id( d->mStrApp, it.current()->text(0) ) );
    }
    int i = 0;
    QArray<int> currentSelected( l.count() );
    for ( QValueList<int>::ConstIterator lit = l.begin(); lit != l.end(); ++lit )
	currentSelected[(int)i++] = *lit;

    d->mCategories.load( categoryFileName() );
    setCategories( currentSelected, d->mStrApp, d->mVisible );
}

void CategoryEdit::setCategories( const QArray<int> &recCats,
				  const QString &appName, const QString &visibleName )
{
    if ( !d )
	d = new CategoryEditPrivate( (QWidget*)parent(), name()  );
    d->mStrApp = appName;
    d->mVisible = visibleName;

    QStringList appCats = d->mCategories.labels( d->mStrApp );
    QArray<int> cats = d->mCategories.ids(d->mStrApp, appCats);
    lvView->clear();

    QStringList::ConstIterator it;
    int i, j;
    for ( i = 0, it = appCats.begin(); it != appCats.end(); i++, ++it ) {
	QCheckListItem *chk;
	chk = new QCheckListItem( lvView, (*it), QCheckListItem::CheckBox );
	if ( !d->mCategories.isGlobal((*it)) )
	    chk->setText( 1, tr(d->mVisible) );
	else
	    chk->setText( 1, tr("All") );
	// Is this record using this category, then we should check it
	for ( j = 0; j < int(recCats.count()); j++ ) {
	    if ( cats[i] == recCats[j] ) {
		chk->setOn( true );
		break;
	    }
	}
    }
    lvView->setSorting( 0, TRUE );
    lvView->sort();
    if ( lvView->childCount() < 1 )
	txtCat->setEnabled( FALSE );
    else {
	lvView->setSelected( lvView->firstChild(), true );
    }
    enableButtons();
}

#ifdef QTOPIA_DESKTOP
void CategoryEdit::setCategories( const QArray<int> &recCats,
				  QString appName, QString visibleName, bool checkable )
{
    delete d; d = 0;
    if ( !d )
	d = new CategoryEditPrivate( (QWidget*)parent(), name()  );
    d->mStrApp = appName;
    d->mVisible = visibleName;
    d->checkable = checkable;

    QStringList appCats = d->mCategories.labels( d->mStrApp );
    QArray<int> cats = d->mCategories.ids(d->mStrApp, appCats);
    lvView->clear();

    QStringList::ConstIterator it;
    int i, j;
    for ( i = 0, it = appCats.begin(); it != appCats.end(); i++, ++it ) {
	if ( d->checkable ) {
	    QCheckListItem *chk;
	    chk = new QCheckListItem( lvView, (*it), QCheckListItem::CheckBox );
	    if ( !d->mCategories.isGlobal((*it)) )
		chk->setText( 1, tr(d->mVisible) );
	    else
		chk->setText( 1, tr("All") );
	    // Is this record using this category, then we should check it
	    for ( j = 0; j < int(recCats.count()); j++ ) {
		if ( cats[i] == recCats[j] ) {
		    chk->setOn( true );
		    break;
		}
	    }
	} else {
	    QListViewItem *l = new QListViewItem( lvView, (*it) );
	    if ( !d->mCategories.isGlobal((*it)) )
		l->setText( 1, tr(d->mVisible) );
	    else
		l->setText( 1, tr("All") );
	}
    }
    lvView->setSorting( 0, TRUE );
    lvView->sort();
    if ( lvView->childCount() < 1 )
	txtCat->setEnabled( FALSE );
    else {
	lvView->setSelected( lvView->firstChild(), true );
    }
    enableButtons();
}

void CategoryEdit::refresh()
{
    if ( !d )
	return;
    d->mCategories.load( categoryFileName() );
    setCategories( QArray<int>(), d->mStrApp, d->mVisible, d->checkable );
}
#endif

CategoryEdit::~CategoryEdit()
{
    if ( d )
	delete d;
}

void CategoryEdit::slotSetText( QListViewItem *newItem )
{
    updateInline();

    d->editItem = newItem;
    d->nameChanged = FALSE;

    if ( !d->editItem )
	return;

    d->orgName = d->editItem->text(0);

    // avoid textChanged signal (could connect,disconnect, but I suspect this is a bit faster...)
    d->settingItem = TRUE;
    txtCat->setText( d->editItem->text(0) );
    txtCat->setEnabled( true );

    if ( d->editItem->text(1) == tr("All") )
        chkGlobal->setChecked( true );
    else
        chkGlobal->setChecked( false );

    d->settingItem = FALSE;
}

void CategoryEdit::slotAdd()
{
    QString name = tr( "New Category" );
    bool insertOk = FALSE;
    int num = 0;
    while ( !insertOk ) {
	if ( num++ > 0 )
	    name = tr("New Category ") + QString::number(num);
	if ( chkGlobal->isChecked() )
	    insertOk = d->mCategories.addGlobalCategory( name );
	else
	    insertOk = d->mCategories.addCategory( d->mStrApp, name );
    }

    QListViewItem *chk;
    if ( d->checkable )
	chk = (QListViewItem *) new QCheckListItem( lvView, name, QCheckListItem::CheckBox);
    else
	chk = new QListViewItem( lvView, name);

    if ( !chkGlobal->isChecked() )
	chk->setText( 1, tr(d->mVisible) );
    else
	chk->setText( 1, tr("All") );

    lvView->setSelected( chk, TRUE );
    txtCat->selectAll();
    txtCat->setFocus();
    d->nameChanged = TRUE;
    enableButtons();
}

void CategoryEdit::slotRemove()
{
    d->editItem = lvView->selectedItem();
    if ( d->editItem ) {
	QListViewItem *nextItem = d->editItem->itemBelow();
	if ( !nextItem )
	    nextItem = d->editItem->itemAbove();

	if ( chkGlobal->isChecked() ) {
	    switch( QMessageBox::warning( this, tr( "Removing Category" ), "<qt>" +
				  tr( "Deleting a global category "
				      "will delete it from all applications. "
				      "Any items in this category "
				      "will become unfiled. "
				      "Are you sure you want to do this?") + "</qt>",
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default ) ) {
		case QMessageBox::Yes : break;
		default: return;
	    }
	} else {
	    switch( QMessageBox::warning( this, tr( "Removing Category" ), "<qt>" +
				  tr( "Deleting a local category will make "
				      "all items in this category unfiled. "
				      "Are you sure you want to do this?") + "</qt>",
				  QMessageBox::Yes, QMessageBox::No ) ) {

		case QMessageBox::Yes : break;
		default: return;
	    }

	}

	d->mCategories.removeCategory( d->mStrApp, d->orgName );

	d->settingItem = TRUE;
	delete d->editItem;
	d->editItem = 0;
	d->settingItem = FALSE;

	if ( nextItem )
	    lvView->setSelected( nextItem, TRUE );
    }
    if ( lvView->childCount() < 1 ) {
	d->settingItem = TRUE;
	txtCat->clear();
	txtCat->setEnabled( FALSE );
	d->settingItem = FALSE;
    }
    enableButtons();
}

void CategoryEdit::slotSetGlobal( bool isChecked )
{
    if ( d->settingItem )
	return;

    if ( d->editItem ) {
	if ( isChecked ) {
	    d->editItem->setText( 1, tr("All") );
	} else {
	    switch( QMessageBox::warning( this,
			      tr( "Removing Category" ), "<qt>" +
			      tr( "Making a global category local "
				  "will delete it from all other applications "
				  "that may use it. Any items in this category "
				  "will become unfiled. "
				  "Are you sure you want to do this?") + "</qt>",
				QMessageBox::Yes, QMessageBox::No ) ) {

		case QMessageBox::Yes : break;
		default: chkGlobal->setChecked( TRUE ); return;
	    }
	    d->editItem->setText( 1, tr(d->mVisible) );
	}

	d->mCategories.setGlobal( d->mStrApp, d->orgName, isChecked );
    }
}

void CategoryEdit::slotTextChanged( const QString &str)
{
    if ( !d->editItem || d->settingItem )
	return;

    d->nameChanged = TRUE;
    d->editItem->setText( 0, str);
}

QArray<int> CategoryEdit::newCategories()
{
    QArray<int> a;
    if ( d ) {
	QStringList sl;
	d->mCategories.save( categoryFileName() );
	QListViewItemIterator it( lvView );
	QValueList<int> l;
	for ( ; it.current(); ++it ) {
	    if ( reinterpret_cast<QCheckListItem*>(it.current())->isOn() )
		l.append( d->mCategories.id( d->mStrApp, it.current()->text(0) ) );
	}
	int i = 0;
	a.resize( l.count() );
	for ( QValueList<int>::ConstIterator lit = l.begin(); lit != l.end(); ++lit )
	    a[(int)i++] = *lit;
    }
    return a;
}

void CategoryEdit::accept()
{
}

#ifdef QTOPIA_DESKTOP
static QString mCatFileName;

void Categories::setCategoryFileName(const QString &s)
{
    mCatFileName = s;
}
#endif

QString categoryFileName()
{
#ifdef QTOPIA_DESKTOP
    return mCatFileName;
#else

    QDir dir = QString(QDir::homeDirPath() + "/Settings");
    if ( !dir.exists() )
#ifndef Q_OS_WIN32
	mkdir( dir.path().local8Bit(), 0700 );
#else
        dir.mkdir(dir.path());
#endif
    return dir.path() + "/" + "Categories" + ".xml"; // No tr
#endif // QTOPIA_DESKTOP
}

void CategoryEdit::kludge()
{
    lvView->setMaximumHeight( 130 );
}

bool CategoryEdit::tryAccept()
{
    return updateInline();
}

bool CategoryEdit::updateInline()
{
    bool result = TRUE;
    disconnect( lvView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotSetText( QListViewItem* ) ) );
    if ( d->nameChanged && d->editItem ) {
	d->settingItem = TRUE;

	QString newName = txtCat->text().stripWhiteSpace();

	RenameResult r = (RenameResult) tryRename(newName, chkGlobal->isChecked());
	if ( r == Ok ) {
	    d->editItem->setText(0, newName );
	} else if ( r == Failed ) {
	    d->editItem->setText(0, d->orgName );
	    txtCat->setText( d->orgName );
	    result = FALSE;
	} else if ( r == MadeGlobal ) {
	    d->editItem->setText(0, d->orgName);
	    txtCat->setText( d->orgName );

	    QListViewItem *c;
	    if ( d->checkable )
		c = (QListViewItem *) new QCheckListItem( lvView, newName, QCheckListItem::CheckBox );
	    else
		c = new QListViewItem(lvView, newName);

	    c->setText(1, tr("All") );
	    result = FALSE;
	}

	d->settingItem = FALSE;
    }

    connect( lvView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotSetText( QListViewItem* ) ) );
    return result;
}

static bool contains(const Categories &c, const QString &label)
{
    if ( c.globalGroup().contains(label) )
	return TRUE;

    for ( QMap<QString, CategoryGroup>::ConstIterator it = c.appGroupMap().begin();
	    it != c.appGroupMap().end(); ++it ) {

	if ( (*it).contains(label) )
	    return TRUE;
    }

    return FALSE;
}

static void setGlobal(Categories &c, const QString &catname)
{
    for ( QMap<QString, CategoryGroup>::ConstIterator it = c.appGroupMap().begin();
	    it != c.appGroupMap().end(); ++it ) {

	if ( (*it).contains(catname) ) {
	    c.setGlobal( it.key(), catname, TRUE );
	    return;
	}
    }
}

int CategoryEdit::tryRename(const QString &newName, bool global)
{
    QString name = newName.stripWhiteSpace();
    if ( name == d->orgName )
	return (int) Ok;
    if (name.isEmpty() )
	return (int) Failed;

    if ( contains(d->mCategories, newName) && !d->mCategories.globalGroup().contains(newName)
	    && !d->mCategories.appGroupMap()[d->mStrApp].contains(newName) ) {
	switch( QMessageBox::warning( d->w, qApp->translate("Qtopia", "Duplicate categories"),
			      qApp->translate("Qtopia", "<qt>There already exists a local category named"
				  " <b>%1</b> in another application.<p>"
				  "Make that category a global group?</qt>").arg(newName),
				  QMessageBox::Yes, QMessageBox::No) ) {

	    case QMessageBox::Yes:
		setGlobal(d->mCategories, newName);
		return (int) MadeGlobal;
	    case QMessageBox::No: return (int) Failed;
	}
    }

    bool success;
    if ( global ) {
	success = d->mCategories.renameGlobalCategory( d->orgName, newName );
    } else {
	success = d->mCategories.renameCategory( d->mStrApp, d->orgName, newName );
    }
    if ( !success ) {
	QMessageBox::warning( d->w, qApp->translate("Qtopia", "Duplicate categories"),
			      qApp->translate("Qtopia", "There is already a category named\n"
				  "%1.\n"
				  "Please choose another name, or delete\n"
				  "the duplicate.").arg(newName) );
	return (int) Failed;
    } else
	return (int) Ok;
}

void CategoryEdit::enableButtons()
{
    cmdDel->setEnabled(lvView->selectedItem() != 0);
}

