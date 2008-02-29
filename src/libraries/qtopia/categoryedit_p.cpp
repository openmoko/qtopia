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

#include <qtopia/categories.h>
#include <qtopia/global.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextmenu.h>
#endif
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>

#include <qdir.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qstringlist.h>
#include <qtoolbutton.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qheader.h>
#include <qaction.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qpushbutton.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#ifndef Q_OS_WIN32
#include <unistd.h>
#endif

QIconSet qtopia_internal_loadIconSet( const QString &pix );

using namespace Qtopia;

enum RenameResult{
    Ok, Failed, MadeGlobal
};

#ifdef QTOPIA_DESKTOP
// This is used to allow the Qtopia Desktop setCategories function to use the 
// regular version. The extra parameter is temporarily stored here.
static bool gCheckable = FALSE;
#endif

void qpe_translateLabels(QStringList& strs);
QString qpe_translateLabel(const QString& strs);


class CategoryEditListItem : public QCheckListItem {
public:
    CategoryEditListItem(QListView* parent, const QString& l, bool checkable) :
	QCheckListItem(parent,qpe_translateLabel(l),checkable ? QCheckListItem::CheckBox : QCheckListItem::Controller),
	id(l)
    {
    }

    void paintCell( QPainter *p,  const QColorGroup & cg,
                    int column, int width, int alignment )
    {
	if ( type() == QCheckListItem::CheckBox )
	    QCheckListItem::paintCell(p,cg,column,width,alignment);
	else
	    QListViewItem::paintCell(p,cg,column,width,alignment);
    }

    bool editable() const
    {
	return text( 0 ) == id;
    }

    void setText(const QString& l)
    {
	ASSERT(id == text(0)); // Otherwise, this must not be called.

	QCheckListItem::setText(0,l);
	id = l;
    }

    QString label() const { return id; }

private:
    QString id;
};



//  Note that there is a duplicate of this class in libqtopia to allow access to this private class
class CategoryEditPrivate : public QObject
{
    Q_OBJECT
public:
    CategoryEditPrivate( QWidget *parent, const QString &appName )
	: QObject( 0, 0 ),
	mCategoryEdit( 0 ), mCategories( parent, "" ),
	editItem( 0 ), mStrApp( appName ),
	checkable( TRUE ), nameChanged( FALSE ),
	w(parent), settingItem( FALSE ), editDialogHack( FALSE ),
	editingNew( FALSE )
    {
	mCategories.load( categoryFileName() );
    }

    bool isGlobal( QListViewItem *item )
    {
	return (item->pixmap( 1 ) && !item->pixmap( 1 )->isNull());
    }

    void setGlobal( QListViewItem *item, bool global )
    {
	if ( global )
	    item->setPixmap( 1, pm_globe );
	else
	    item->setPixmap( 1, QPixmap() );
    }

    CategoryEdit *mCategoryEdit;
    Categories mCategories;
    CategoryEditListItem *editItem;
    QString mStrApp;
    QString mVisible;
    bool checkable, nameChanged;
    QWidget *w;
    QString orgName;
    bool settingItem;
    bool editDialogHack;
    bool changedGlobal;
    bool editingNew;
    QAction *newAction;
    QAction *editAction;
    QAction *deleteAction;
    QPixmap pm_globe;
#ifndef QTOPIA_PHONE
    QPushButton *newBtn;
    QPushButton *editBtn;
    QPushButton *deleteBtn;
#endif

public slots:
    void slotEdit()
    {
	if ( mCategoryEdit == 0 || editItem == 0 )
	    return;

	QDialog editDialog( mCategoryEdit, 0, TRUE );
	if ( editingNew )
	    editDialog.setCaption( tr("New Category") );
	else
	    editDialog.setCaption( tr("Edit Category") );
	QVBoxLayout *vb = new QVBoxLayout( &editDialog, 6, 3 );
	QLineEdit *le = new QLineEdit( &editDialog );
	QCheckBox *cb = new QCheckBox( tr("Global"), &editDialog );
	vb->addWidget( le );

#ifdef QTOPIA_DESKTOP
	QWidget *buttons = new QWidget( &editDialog );
	QGridLayout *gl = new QGridLayout( buttons );

	QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding );
	QPushButton *ok = new QPushButton( tr("OK"), buttons );
	QPushButton *cancel = new QPushButton( tr("Cancel"), buttons );
	gl->addItem( spacer, 0, 0 );
	gl->addWidget( ok, 0, 1 );
	gl->addWidget( cancel, 0, 2 );

	vb->addWidget( buttons );

	connect( ok, SIGNAL(clicked()), &editDialog, SLOT(accept()) );
	connect( cancel, SIGNAL(clicked()), &editDialog, SLOT(reject()) );
#endif

	bool wasGlobal = isGlobal( editItem );
	le->setText( editItem->text( 0 ) );
	if ( !editItem->editable() ) {
	    // Translated. Not editable. Fixed system category.
	    le->setReadOnly(TRUE);
	    cb->setEnabled(FALSE);
	}
	vb->addWidget( cb );
	cb->setChecked( wasGlobal );

	bool dialogResult;
	QString oldname = le->text();
#ifdef QTOPIA_PHONE
	dialogResult = QPEApplication::execDialog( &editDialog );
#else
	dialogResult = editDialog.exec();
#endif
	QString newname = le->text();
	// this is a new item that hasn't been renamed or an existing item that has been named ""
	if ( dialogResult && ((editingNew && newname == oldname) || newname == "") ) {
	    // reject it
	    dialogResult = FALSE;
	    // don't let it's global status be changed
	    cb->setChecked( wasGlobal );
	}

	if ( dialogResult ) {
	    /// strip _'s from start (that means "System")
	    while (newname.length() && newname[0]=='_')
		newname = newname.mid(1);
	    nameChanged = newname != editItem->text( 0 );
	    changedGlobal = wasGlobal != cb->isChecked();

	    editItem->setText( newname );

	    editDialogHack = TRUE;
	    mCategoryEdit->tryAccept();
	    editDialogHack = FALSE;
	} else {
	    if ( editingNew ) {
		QTimer::singleShot( 0, deleteAction, SIGNAL( activated() ) );
	    }
	}

	QTimer::singleShot( 0, this, SLOT( finishedEditingNew() ) );
    }

    void finishedEditingNew()
    {
	editingNew = FALSE;
    }
};


CategoryEdit::CategoryEdit( QWidget *parent, const char *name )
    : CategoryEditBase( parent, name )
{
    d = 0;
    lvView->setMinimumHeight( 1 );
    lvView->setColumnAlignment( 1, AlignHCenter );
#ifndef QTOPIA_DESKTOP
    lvView->header()->hide();
#endif
#ifdef QTOPIA_PHONE
    lvView->setFrameStyle(QFrame::NoFrame);
#endif
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
    lvView->setColumnAlignment( 1, AlignHCenter );
#ifndef QTOPIA_DESKTOP
    lvView->header()->hide();
#endif
#ifdef QTOPIA_PHONE
    lvView->setFrameStyle(QFrame::NoFrame);
#endif
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
    if ( d->checkable ) {
	for ( QListViewItemIterator it( lvView ); it.current(); ++it ) {
	    CategoryEditListItem *chk = (CategoryEditListItem*)it.current();
	    if ( chk->isOn() )
		l.append( d->mCategories.id( d->mStrApp, chk->label() ) );
	}
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
    if ( !d ) {
	d = new CategoryEditPrivate( (QWidget*)parent(), name()  );
	d->mCategoryEdit = this;
#ifndef QTOPIA_DESKTOP
	lvView->header()->resizeSection( 0, QApplication::desktop()->width() - 32 );
	lvView->header()->resizeSection( 1, 16 );
	lvView->header()->setResizeEnabled( FALSE, 0 );
	lvView->header()->setResizeEnabled( FALSE, 1 );
#endif
	if ( d->pm_globe.isNull() ) {
	    d->pm_globe = qtopia_internal_loadIconSet("globe").pixmap();
	}

	d->newAction = new QAction( tr("New"), qtopia_internal_loadIconSet("new"), QString::null, 0, this );
	d->editAction = new QAction( tr("Edit"), qtopia_internal_loadIconSet("edit"), QString::null, 0, this );
	d->deleteAction = new QAction( tr("Delete"), qtopia_internal_loadIconSet("trash"), QString::null, 0, this );
	connect( d->newAction, SIGNAL( activated() ), this, SLOT( slotAdd() ) );
	connect( d->newAction, SIGNAL( activated() ), d, SLOT( slotEdit() ) );
	connect( d->editAction, SIGNAL( activated() ), d, SLOT( slotEdit() ) );
	connect( d->deleteAction, SIGNAL( activated() ), this, SLOT( slotRemove() ) );

#if defined(QTOPIA_PHONE)
	ContextMenu *menu = new ContextMenu( (QWidget *)parent() );
	d->newAction->addTo( menu );
	d->editAction->addTo( menu );
	d->deleteAction->addTo( menu );
#else
	d->newBtn = new QPushButton( qtopia_internal_loadIconSet("new"), tr("New"), this );
	d->editBtn = new QPushButton( qtopia_internal_loadIconSet("edit"), tr("Edit"), this );
	d->deleteBtn = new QPushButton( qtopia_internal_loadIconSet("trash"), tr("Delete"), this );
	connect( d->newBtn, SIGNAL( clicked() ), d->newAction, SIGNAL( activated() ) );
	connect( d->editBtn, SIGNAL( clicked() ), d->editAction, SIGNAL( activated() ) );
	connect( d->deleteBtn, SIGNAL( clicked() ), d->deleteAction, SIGNAL( activated() ) );

	QLayout *l = this->layout();
#ifdef QTOPIA_DESKTOP
	l->remove( lvView );
#else
	for ( QLayoutIterator it = l->iterator(); it.current() != 0; ++it ) {
	    if ( it.current()->widget() == lvView ) {
		it.deleteCurrent();
		break;
	    }
	}
#endif
	QGridLayout *gl = new QGridLayout( l, 1, 3);
	gl->addMultiCellWidget( lvView, 0, 0, 0, 2 );
	gl->addWidget( d->newBtn, 1, 0 );
	gl->addWidget( d->editBtn, 1, 1 );
	gl->addWidget( d->deleteBtn, 1, 2 );
#endif
    }
    d->mStrApp = appName;
    d->mVisible = visibleName;
#ifdef QTOPIA_DESKTOP
    // This was set before this function call
    d->checkable = gCheckable;
#else
    d->checkable = TRUE;
#endif

    QStringList appCats = d->mCategories.labels( d->mStrApp );
    QArray<int> cats = d->mCategories.ids(d->mStrApp, appCats);
    lvView->clear();

    QStringList::ConstIterator it;
    int i, j;
    for ( i = 0, it = appCats.begin(); it != appCats.end(); i++, ++it ) {
	CategoryEditListItem *chk
	    = new CategoryEditListItem( lvView, (*it), d->checkable );
	d->setGlobal( chk, d->mCategories.isGlobal((*it)) );
	if ( d->checkable ) {
	    // Is this record using this category, then we should check it
	    for ( j = 0; j < (int)recCats.count(); j++ ) {
		if ( cats[i] == recCats[j] ) {
		    chk->setOn( true );
		    break;
		}
	    }
	}
    }
    lvView->setSorting( 0, TRUE );
    lvView->sort();
    if ( lvView->childCount() >= 1 ) {
	lvView->setSelected( lvView->firstChild(), TRUE );
    }
    enableButtons();
}

#ifdef QTOPIA_DESKTOP
void CategoryEdit::setCategories( const QArray<int> &recCats,
				  QString appName, QString visibleName, bool checkable )
{
    // setCategories will grab this value and put it into the d pointer
    gCheckable = checkable;
    setCategories( recCats, appName, visibleName );
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

    d->editItem = (CategoryEditListItem*)newItem;
    d->nameChanged = FALSE;

    if ( !d->editItem )
	return;

    d->orgName = d->editItem->text(0);
}

void CategoryEdit::slotAdd()
{
    QString name = "";
    d->mCategories.addCategory( d->mStrApp, name );

    QListViewItem *chk = new CategoryEditListItem( lvView, name, d->checkable );
    d->setGlobal( chk, FALSE );

    lvView->setSelected( chk, TRUE );
    d->nameChanged = TRUE;
    enableButtons();
    d->editingNew = TRUE;
}

void CategoryEdit::slotRemove()
{
    d->editItem = (CategoryEditListItem*)lvView->selectedItem();
    if ( d->editItem ) {
	QListViewItem *nextItem = d->editItem->itemBelow();
	if ( !nextItem )
	    nextItem = d->editItem->itemAbove();

	if ( !d->editingNew ) {
	    if ( d->isGlobal( d->editItem ) ) {
		switch( QMessageBox::warning( this, tr( "Removing Category" ),
			    tr( "<qt>Deleting a global category "
				"effects all applications. "
				"Any items in this category "
				"will become unfiled."
				"<br>Are you sure you want to do this?</qt>"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default ) ) {
		    case QMessageBox::Yes:
			break;
		    default:
			return;
		}
	    } else {
		switch( QMessageBox::warning( this, tr( "Removing Category" ),
			    tr( "<qt>Deleting a local category will make "
				"all items in this category unfiled."
				"<br>Are you sure you want to do this?</qt>"),
			    QMessageBox::Yes, QMessageBox::No ) ) {

		    case QMessageBox::Yes:
			break;
		    default:
			return;
		}

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
	    d->setGlobal( d->editItem, isChecked );
	} else {
	    switch( QMessageBox::warning( this,
			tr( "Global Category" ),
			tr( "<qt>Making this category local "
			    "will delete it from all other apps. "
			    "Any items in this category will become "
			    "unfiled.<br>Continue?</qt>" ),
			QMessageBox::Yes, QMessageBox::No ) ) {

		case QMessageBox::Yes : break;
		default:
		    return;
	    }
	    d->setGlobal( d->editItem, isChecked );
	}

	d->mCategories.setGlobal( d->mStrApp, d->orgName, isChecked );
    }
}

void CategoryEdit::slotTextChanged( const QString &str )
{
    if ( d->editItem == 0 || d->settingItem )
	return;

    d->nameChanged = TRUE;
    d->editItem->setText( str );
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
	    CategoryEditListItem* item = (CategoryEditListItem*)it.current();
	    if ( item->isOn() )
		l.append( d->mCategories.id( d->mStrApp, item->label() ) );
	}
	int i = 0;
	a.resize( l.count() );
	for ( QValueList<int>::ConstIterator lit = l.begin(); lit != l.end(); ++lit ) {
	    a[(int)i++] = *lit;
	}
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
    if ( d->editDialogHack ) {
	slotSetText( d->editItem );
	if ( d->changedGlobal )
	    slotSetGlobal( !d->isGlobal( d->editItem ) );
	return TRUE;
    } else {
	return updateInline();
    }
}

bool CategoryEdit::updateInline()
{
    bool result = TRUE;
    disconnect( lvView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotSetText(QListViewItem*) ) );
    if ( d->nameChanged && d->editItem ) {
	d->settingItem = TRUE;

	QString newName = d->editItem->text( 0 );
	RenameResult r = (RenameResult) tryRename(newName, d->isGlobal( d->editItem ) );
	if ( r == Ok ) {
	    d->editItem->setText( newName );
	} else if ( r == Failed ) {
	    d->editItem->setText( d->orgName );
	    result = FALSE;
	} else if ( r == MadeGlobal ) {
	    d->editItem->setText( d->orgName );

	    QListViewItem *c = new CategoryEditListItem( lvView, newName, d->checkable );

	    d->setGlobal( c, TRUE );
	    result = FALSE;
	}

	d->settingItem = FALSE;
    }

    connect( lvView, SIGNAL( selectionChanged(QListViewItem*) ), this, SLOT( slotSetText(QListViewItem*) ) );
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
	switch( QMessageBox::warning( d->w, tr("Duplicate categories"),
				  tr("<qt>There already exists a local category named "
				  "<b>%1</b> in another application."
				  "<br>Make that category a global group?</qt>").arg(newName),
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
        //append number to cat name until we find a name that doesn't exist yet
        int version = 2;
        QString tmpName;
        while (!success) {
            tmpName = newName;
            tmpName += QString::number(version);
            if (global) {
	        success = d->mCategories.renameGlobalCategory( d->orgName, tmpName );
            } else {
	        success = d->mCategories.renameCategory( d->mStrApp, d->orgName, tmpName );
            }
            if (success) 
                d->orgName = tmpName;
            version++;
        }

	QMessageBox::warning( d->w, tr("Duplicate categories"),
			      tr("<qt>There is already a category named "
				 "%1. Please choose another name, or delete "
				 "the duplicate.</qt>").arg(newName) );
	return (int) Failed;
    } else
	return (int) Ok;
}

void CategoryEdit::enableButtons()
{
    CategoryEditListItem *editItem = (CategoryEditListItem*)lvView->currentItem();
    bool e = editItem?editItem->editable():FALSE;
    d->editAction->setEnabled(e);
    d->deleteAction->setEnabled(e);
#ifndef QTOPIA_PHONE
    d->editBtn->setEnabled(e);
    d->deleteBtn->setEnabled(e);
#endif
}

#include "categoryedit_p.moc"
