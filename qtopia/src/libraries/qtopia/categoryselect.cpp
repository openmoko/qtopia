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
#include <qdir.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qtimer.h>
#include <qheader.h>
#include <qtopia/contextbar.h>

#include "categoryedit_p.h"
#include "categoryselect.h"

#include <stdlib.h>

/*! \enum CategorySelect::SelectorWidget
  Chooses a type of widget to use as the selection widget.

  \value ComboBox
  \value ListView
*/

/*!
  \class CategorySelect
  \brief The CategorySelect widget allows users to select Categories with a
  combobox interface.

  CategorySelect is useful to provide a QComboBox of Categories for
  filtering (such as in the Contacts table view) or to allow the user
  to select multiple Categories. The allCategories variable sets
  whether the CategorySelect is in filtering or selecting mode.

  In filtering mode, the All and Unfiled categories are added. The

  In selecting mode, the CategorySelect may either be a QComboBox and
  a QToolButton or a QListView with checkable items depending on the
  screen size.

  CategorySelect automatically updates itself if Categories has been
  changed elsewhere in the environment.

  Signals and slots are provided to notify the application of the users
  selections.  A QToolButton is also provided so that users can edit the
  Categories manually.

  \ingroup qtopiaemb
*/

class CategoryCheckListItem : public QCheckListItem {
public:
    CategoryCheckListItem(QListView* parent, const QString& l, int i) :
	QCheckListItem(parent, l, CheckBox),
	id(i)
    {
    }

    const int id;
};

class CategoryComboPrivate
{
public:
    CategoryComboPrivate() { }
    QArray<int> mAppCats;
    QString mStrAppName;
    QString mStrVisibleName;
    Categories mCats;
};

CategoryCombo::CategoryCombo( QWidget *parent, const char *name , int width)
    : QComboBox( FALSE, parent, name )
{
    QSizePolicy p = sizePolicy();
    p.setHorData(QSizePolicy::Expanding);
    setSizePolicy(p);
    d = new CategoryComboPrivate();
    if (width)
      setFixedWidth(width);
}

void CategoryCombo::initCombo( const QArray<int> &recCats,
			       const QString &appName )
{
    initCombo( recCats, appName, appName );
}

void qpe_translateLabels(QStringList& strs);
QString qpe_translateLabel(const QString& s);

void CategoryCombo::initCombo( const QArray<int> &recCats,
			       const QString &appName,
			       const QString &visibleName )
{
    d->mStrAppName = appName;
    d->mStrVisibleName = visibleName;
    clear();
    QStringList slApp;

    bool loadOk = d->mCats.load( categoryFileName() );
    QObject::connect( this, SIGNAL(activated(int)),
		      this, SLOT(slotValueChanged(int)) );
    slApp = d->mCats.labels( d->mStrAppName, TRUE, Categories::UnfiledLabel );

    d->mAppCats = d->mCats.ids( d->mStrAppName, slApp);
    qpe_translateLabels(slApp);

    int recCount = recCats.count();
    insertStringList( slApp );

    if ( recCount == 1 && loadOk ) {
	for ( int i = 0; i < int(d->mAppCats.size()); i++ ) {
	    if ( d->mAppCats[i] == recCats[0] ) {
		setCurrentItem( i );
		break;
	    }
	}
    } else if (recCount == 0 || !loadOk) {
	setCurrentItem( slApp.count()-1 );  // unfiled
    }

}

// this is a new function by SHARP instead of initCombo()
QArray<int> CategoryCombo::initComboWithRefind( const QArray<int> &recCats,
			       const QString &appName)
{
    QString visibleName = appName;
    d->mStrAppName = appName;
    d->mStrVisibleName = visibleName;
    clear();
    QStringList slApp;
    QArray<int> results;

    QObject::disconnect( this, SIGNAL(activated(int)),
		      this, SLOT(slotValueChanged(int)) );

    QObject::connect( this, SIGNAL(activated(int)),
		      this, SLOT(slotValueChanged(int)) );
    bool loadOk = d->mCats.load( categoryFileName() );
    slApp = d->mCats.labels( d->mStrAppName, TRUE, Categories::UnfiledLabel );

    d->mAppCats = d->mCats.ids( d->mStrAppName, slApp);
    qpe_translateLabels(slApp);

    // addition part
    // make new recCats
    if (loadOk){
      int i,j;
      int value;
      int rCount = recCats.count();
      int mCount = d->mAppCats.count();

      for (i=0; i<rCount; i++){
	value = 0;
	for (j=0; j<mCount; j++){
	  if (recCats[i] == d->mAppCats[j]){
	    value = recCats[i];
	    break;
	  }
	}
	if (value != 0){
	  int tmp = results.size();
	  results.resize( tmp + 1 );
	  results[ tmp ] = value;
	}
      }
    }
    else{
      results = recCats.copy();
    }
    // addition end

    int recCount = results.count();
    insertStringList( slApp );

    if ( recCount == 1 && loadOk ) {
	for ( int i = 0; i < int(d->mAppCats.size()); i++ ) {
	    if ( d->mAppCats[i] == results[0] ) {
		setCurrentItem( i );
		break;
	    }
	}
    } else if (recCount == 0 || !loadOk) {
	setCurrentItem( slApp.count()-1 );  // unfiled
    } // else handled later in CategorySelect::setCategories,
    //which calls this function

/*
    QObject::connect( this, SIGNAL(activated(int)),
		      this, SLOT(slotValueChanged(int)) );
*/
    return results;
}


CategoryCombo::~CategoryCombo()
{
    delete d;
}

int CategoryCombo::currentCategory() const
{
    int i = currentItem();

    int r;
    if ( i == (int)d->mAppCats.count() )
	r = -1;
    else if ( i > (int)d->mAppCats.count() )  // only happen on "All"
	r = -2;
    else
	r = d->mAppCats[i];

    return r;
}

void CategoryCombo::setCurrentCategory( int newCatUid )
{
    if ( newCatUid == -1 ) {
	setCurrentItem( d->mAppCats.count() );
    } else if ( newCatUid == -2 ) {
	setCurrentItem( d->mAppCats.count()+1 );
    } else {
	int i;
	for ( i = 0; i < int(d->mAppCats.size()); i++ ) {
	    if ( d->mAppCats[i] == newCatUid )
		setCurrentItem( i );
	}
    }
}

void CategoryCombo::setCurrentText( const QString &str )
{
    int i;
    int stop;
    stop = count();
    for ( i = 0; i < stop; i++ ) {
	if ( text( i ) == str ) {
	    setCurrentItem( i );
	    break;
	}
    }
}

void CategoryCombo::slotValueChanged( int )
{
    emit sigCatChanged( currentCategory() );
}

class CategorySelectPrivate : public QObject
{
    Q_OBJECT
public:
    CategorySelectPrivate( QObject *parent )
	: QObject(parent, "CategorySelectPrivate" ), mChosen(),
	  usingAll( FALSE ), mVisibleName(), editMode( FALSE ),
	  canEdit( TRUE ), type( CategorySelect::ComboBox ), container( 0 ),
	  layout(0), fixedWidth(0), pendingCategorySets( 0 ), catSelector( 0 )
    {
	setType();
    }

    void setType()
    {
#ifndef QTOPIA_DESKTOP
	type = CategorySelect::ComboBox;
#else
	if ( qApp->desktop()->height() >= 600 && !usingAll )
	    type = CategorySelect::ListView;
	else
	    type = CategorySelect::ComboBox;
#endif
    }

    QArray<int> mChosen; // The checked categories.
    bool usingAll;
    QString mVisibleName;
    bool editMode;
    bool canEdit;
    CategorySelect::SelectorWidget type;
    QWidget *container;
    QGridLayout *layout;
    int fixedWidth;
    int lastCat;
    int pendingCategorySets;

    // so far, the QListView is used only for Qtopia Desktop
    // but could be used for bigger PDA screen sizes

    // the following methods and variables are used only when
    // type == ComboBox
    QListView *catSelector;
    QPushButton *editButton;
    Categories cats;
    QString appName;

    void reloadCatSelector( const QArray<int>& checked )
    {
	catSelector->clear();

	QStringList sl = cats.labels( appName );

	// Qtopia2:
	//QArray<int> id = cats.ids( appName );
	// Qtopia0:
	QArray<int> id = cats.ids( appName, sl );

	qpe_translateLabels(sl);
	int i=0;
	for (QStringList::ConstIterator it=sl.begin(); it!=sl.end(); ++it,++i) {
	    CategoryCheckListItem *item = new CategoryCheckListItem(catSelector,*it,id[i]);
	    if ( checked.find(id[i])>=0 )
		item->setOn(TRUE);
	}
    }

    void reloadCatSelector()
    {
	reloadCatSelector( mChosen );
    }
    void pruneDeletedCats(QArray<int>& l)
    {
	QArray<int> checkedCats;
	for ( int i = 0; i < (int) l.count();++i )
	    if ( cats.label( appName, l[i] ) != QString::null ) {
		checkedCats.resize( i+1 );
		checkedCats[i] = l[i];
	    }

	l = checkedCats;
    }

    void setComboItem( QComboBox *combo, int item )
    {
	pendingCategorySets++;
	combo->setCurrentItem( item );
	QTimer::singleShot( 0, this, SLOT( turnOnEditDialog() ) );
    }

public slots:
    void itemClicked( QListViewItem *i )
    {
	if ( i == 0 )
	    return;
	CategoryCheckListItem* item = (CategoryCheckListItem*)i;
	int id = item->id;
	if ( item->isOn() ) {
	    if ( mChosen.count() == 0 || mChosen.find(id) < 0 ) {
		mChosen.resize(mChosen.size()+1);
		mChosen[(int)mChosen.size()-1] = id;
	    }
	} else {
	    if ( mChosen.count() > 0 ) {
		int i = mChosen.find(id);
		if ( i >= 0 ) {
		    int t = mChosen[(int)mChosen.size()-1];
		    mChosen[i] = t;
		    mChosen.resize(mChosen.size()-1);
		}
	    }
	}
    }

    void turnOnEditDialog()
    {
	pendingCategorySets--;
    }
};


/*!
  \overload

  This constructor accepts an array \a vl of integers representing Categories
  for application \a appName.
  \a visibleName is the string used when the name of this application is required
  to be displayed.
    \a width is an integer used as the fixed width of the widget.
    The \a parent and \a name parameters are the standard Qt parent parameters.
*/
CategorySelect::CategorySelect( const QArray<int> &vl,
				const QString &appName,
				const QString &visibleName,
				QWidget *parent, const char *name , int width)
    : QHBox( parent, name ),
      cmbCat( 0 ),
      cmdCat( 0 ),
      d( 0 )
{
    init(width);
    setCategories( vl, appName, visibleName );
}

/*!
  \overload
  This constructor accepts an array \a vl of integers representing Categories.
  \a appName is used as the visible name string.
    \a width is an integer used as the fixed width of the widget.
    The \a parent and \a name parameters are the standard Qt parent parameters.
*/

CategorySelect::CategorySelect( const QArray<int> &vl,
				const QString &appName, QWidget *parent,
				const char *name ,int width)
    : QHBox( parent, name ),
      cmbCat( 0 ),
      cmdCat( 0 ),
      d( 0 )
{
    init(width);
    setCategories( vl, appName, appName );
}


/*!
  Constructs a category selector with parent \a parent, name \a name and
  fixed width \a width.

  This constructor is provided to make integration with Qt Designer easier.
*/

CategorySelect::CategorySelect( QWidget *parent, const char *name,int width)
    : QHBox( parent, name ),
      cmbCat( 0 ),
      cmdCat( 0 ),
      d( 0 )
{
    init(width);
}


/*!
  Destructs a CategorySelect widget.
*/
CategorySelect::~CategorySelect()
{
}

// ===========================================================================

class EditDlg : public QDialog
{
    Q_OBJECT
public:
    EditDlg(QWidget *parent, const char *name, bool modal,
	    const QArray<int> &vlRecs, const QString &appName, const QString &visibleName)
	: QDialog(parent, name, modal)
    {
	setCaption( tr("Categories") );
	QVBoxLayout *vb = new QVBoxLayout( this );
	ce = new CategoryEdit( vlRecs, appName, visibleName, this );
	vb->addWidget( ce );

#ifdef QTOPIA_DESKTOP
	QWidget *buttons = new QWidget( this );
	QGridLayout *gl = new QGridLayout( buttons );

	QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding );
	QPushButton *ok = new QPushButton( tr("OK"), buttons );
	QPushButton *cancel = new QPushButton( tr("Cancel"), buttons );
	gl->addItem( spacer, 0, 0 );
	gl->addWidget( ok, 0, 1 );
	gl->addWidget( cancel, 0, 2 );

	vb->addWidget( buttons );

	connect( ok, SIGNAL(clicked()), this, SLOT(accept()) );
	connect( cancel, SIGNAL(clicked()), this, SLOT(reject()) );

	setCaption( tr("Edit Categories") );
#endif
    }

    QArray<int> newCategories() { return ce->newCategories(); };

protected:
    void accept()
    {
	if ( !ce->tryAccept() )
	    return;

	QDialog::accept();
    }

private:
    CategoryEdit *ce;
};

// ===========================================================================

/*!
  This slot is called when the user pushes the button to edit Categories.
*/
void CategorySelect::slotDialog()
{
#ifdef QTOPIA_DESKTOP
    emit editCategoriesClicked(this);
    categoriesChanged();
#else
    EditDlg editDlg( this, "categories", TRUE, d->mChosen, mStrAppName, d->mVisibleName ); // No tr
    editDlg.showMaximized();

    d->editMode = TRUE;
    if ( editDlg.exec() ) {
	QArray<int> prev = d->mChosen;
	d->mChosen = editDlg.newCategories();
	if ( d->type == ComboBox ) {
	    cmbCat->initCombo( d->mChosen, mStrAppName, d->mVisibleName );
	    if ( d->usingAll ) {
		cmbCat->insertItem( tr( "All" ), cmbCat->count() );
	    } else {
		if ( d->mChosen.count() > 1 ) {
		    cmbCat->insertItem( tr( "(multi) ..." ), cmbCat->count() );
		    d->setComboItem( cmbCat, cmbCat->count() - 1 );
		} else
		    cmbCat->insertItem( tr( "..." ), cmbCat->count() );
	    }
	    d->lastCat = cmbCat->currentItem();
	} else {
	    QArray<int> t = d->mChosen;
	    d->cats.load( categoryFileName() );
	    d->pruneDeletedCats(t);
	    d->reloadCatSelector( t );
	}

	// WORKAROUND: signal not always emitted when multi used!
	if ( prev != d->mChosen ) {
	    emit signalSelected( cmbCat->currentCategory() );
	}
    } else if ( !d->usingAll ) {
	d->setComboItem( cmbCat, d->lastCat );
    }
#endif
}

/*!
  This slot is called when the available Categories have been changed.
*/

void CategorySelect::categoriesChanged()
{
    if ( d->editMode ) {
	//only one can edit at a time, so if we're the one we can ignore this signal
	// as it will be handled in slotDialog
	d->editMode = FALSE;
    } else {
	if ( d->type == ComboBox ) {
	    int prevCat = cmbCat->currentCategory();
	    cmbCat->initComboWithRefind( d->mChosen, mStrAppName );
	    if ( d->usingAll ) {
		cmbCat->insertItem( tr( "All" ), cmbCat->count() );
	    } else {
		if ( d->mChosen.count() > 1 ) {
		    cmbCat->insertItem( tr( "(multi) ..." ), cmbCat->count() );
		    d->setComboItem( cmbCat, cmbCat->count() - 1 );
		} else
		    cmbCat->insertItem( tr( "..." ), cmbCat->count() );
	    }
	    cmbCat->setCurrentCategory( prevCat );

	    // Test if category is still valid.  If it isn't we need to inform our parent about a selection change
	    if ( cmbCat->currentCategory() != prevCat ) {
		cmbCat->setCurrentCategory(-1);
		emit signalSelected( cmbCat->currentCategory() );
	    }
	}
	else {
	    QArray<int> t = d->mChosen;
	    d->cats.load( categoryFileName() );
	    d->pruneDeletedCats(t);
	    d->reloadCatSelector( t );
	}
    }
}

/*!
  This slot is called when a new Category is available,
  passing the \a newUid created.
*/

void CategorySelect::slotNewCat( int newUid )
{
    if ( !d->usingAll ) {
	// This is for handling the '...' entry
	if ( newUid == -2 ) {
	    if ( d->pendingCategorySets == 0 ) {
		slotDialog();
	    }
	    return;
	}
	d->lastCat = cmbCat->currentItem();
    }

    if ( newUid != -1 && newUid != -2 ) {
	bool alreadyIn = false;
	for ( uint it = 0; it < d->mChosen.count(); ++it ) {
	    if ( d->mChosen[(int)it] == newUid ) {
		alreadyIn = true;
		break;
	    }
	}
	if ( !alreadyIn ) {
	    d->mChosen.resize( 1 );
	    d->mChosen[ 0 ] = newUid;
	}
    } else
    d->mChosen.resize(0);  // now Unfiled.
    emit signalSelected( currentCategory() );
}

/*!
  \overload

  Resets the CategorySelect to select \a cats as
  the Categories associated with \a appName.

  This function should only be called if <i>filtering</i>
  on Categories and not selecting and therefore possibly
  allowing the user to edit Categories.
*/
QString CategorySelect::setCategories( const QArray<int> &cats,
				    const QString &appName )
{
    return setCategories( cats, appName, appName );
}


/*!
  Resets the CategorySelect to select the \a cals for
  the Categories assoicated with \a appName and displays
  the \a visibleName if the user is selecting and therefore editing
  new Categories.
 */
QString CategorySelect::setCategories( const QArray<int> &cals,
				    const QString &appName,
				    const QString &visibleName )
{
    d->mVisibleName = visibleName;
    mStrAppName = appName;
    d->appName = appName;
    if ( d->type == ComboBox ) {
	d->mChosen = cmbCat->initComboWithRefind( cals, appName );
	if ( d->usingAll ) {
	    cmbCat->insertItem( tr( "All" ), cmbCat->count() );
	} else {
	    if ( d->mChosen.count() > 1 ) {
		cmbCat->insertItem( tr( "(multi) ..." ), cmbCat->count() );
		d->setComboItem( cmbCat, cmbCat->count() - 1 );
	    } else
		cmbCat->insertItem( tr( "..." ), cmbCat->count() );
	}
	d->lastCat = cmbCat->currentItem();
    } else {
	d->mChosen = cals.copy();
	d->reloadCatSelector();
    }
    return Qtopia::Record::idsToString(d->mChosen);
}

void CategorySelect::init(int width)
{
#ifdef QTOPIA_PHONE
    ContextBar::setLabel( this, Key_Select, ContextBar::Select );
#endif

    if ( d ) {
	delete d->layout;
	delete d->container;
	d->setType();
    } else {
	d = new CategorySelectPrivate( this );
    }
    d->container = new QWidget( this ); // No tr

    if ( d->type == ComboBox ) {
	d->layout = new QGridLayout( d->container );

	cmbCat = new CategoryCombo( d->container, 0, width); // No tr
	d->layout->addWidget( cmbCat, 0, 0 );
	QObject::connect( cmbCat, SIGNAL(sigCatChanged(int)),
			  this, SLOT(slotNewCat(int)) );

	if ( d->canEdit && d->usingAll ) {
	    cmdCat = new QToolButton( d->container ); // No tr
	    d->layout->addWidget( cmdCat, 0, 1 );

	    cmdCat->setTextLabel( "...", FALSE );
	    cmdCat->setFocusPolicy( TabFocus );
	    cmdCat->setFixedHeight( cmbCat->sizeHint().height() );
	    cmdCat->setUsesTextLabel( true );
	    QObject::connect( cmdCat, SIGNAL(clicked()), this, SLOT(slotDialog()) );
	}
    } else {
	d->layout = new QGridLayout( d->container, 2, 2 );
	d->cats.load( categoryFileName() );
	d->catSelector = new QListView( d->container, "catSelector" );
	d->catSelector->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
	d->catSelector->addColumn( tr("Categories") );
	d->catSelector->setSelectionMode( QListView::NoSelection );
	d->catSelector->header()->hide();
	d->layout->addMultiCellWidget( d->catSelector, 0, 0, 0, 1 );

	QSpacerItem *horiSpacer = new QSpacerItem( 1, 1, QSizePolicy::Expanding,
						   QSizePolicy::Expanding );
	d->layout->addItem( horiSpacer, 1, 0 );

	d->editButton = new QPushButton( tr("Edit Categories"), d->container );
	d->layout->addWidget( d->editButton, 1, 1 );

	connect( d->catSelector, SIGNAL( clicked(QListViewItem*) ), d, SLOT( itemClicked(QListViewItem*) ) );
	connect( d->catSelector, SIGNAL( spacePressed(QListViewItem*) ), d, SLOT( itemClicked(QListViewItem*) ) );
	connect( d->editButton, SIGNAL( clicked() ), SLOT( slotDialog() ) );
    }

#ifndef QTOPIA_DESKTOP
    connect(qApp, SIGNAL( categoriesChanged() ), this, SLOT( categoriesChanged() ) );
#endif

    d->fixedWidth = width;
}

/*!
  Return the value of the currently selected category.
 */
int CategorySelect::currentCategory() const
{
    if ( d->type == ComboBox )
	return cmbCat->currentCategory();

    return currentCategories()[0];
}

void CategorySelect::setCurrentCategory( int newCatUid )
{
    if ( d->type == ComboBox ) {
	cmbCat->setCurrentCategory( newCatUid );
    } else {
	CategoryCheckListItem *i = (CategoryCheckListItem*)d->catSelector->firstChild();
	while (i) {
	    if ( i->id == newCatUid ) {
		i->setOn(TRUE);
		break;
	    }
	    i = (CategoryCheckListItem*)i->nextSibling();
	}
    }
}

/*!
  Returns a shallow copy of the categories in this CategorySelect.
 */
const QArray<int> &CategorySelect::currentCategories() const
{
    return d->mChosen;
}

/*!
  Hides the edit section of the CategorySelect widget if \a remove is TRUE.
 */
void CategorySelect::setRemoveCategoryEdit( bool remove )
{
    d->canEdit = !remove;
    if ( !cmdCat )
	return;

    if ( remove ) {
	cmdCat->setEnabled( FALSE );
	cmdCat->hide();
    } else {
	cmdCat->setEnabled( TRUE );
	cmdCat->show();
    }
}

/*!
  Changes this CategorySelect to the All category if \a all is TRUE.
 */
void CategorySelect::setAllCategories( bool all )
{
    // if showing all category, then that means the
    // user isn't just selecting

    bool createCmb = FALSE;
    bool reinit = FALSE;
    if ( d->usingAll != all ) {
	reinit = TRUE;
	if ( d->catSelector )
	    createCmb = TRUE;
    }

    if ( reinit ) {
	d->usingAll = all;
	init( d->fixedWidth );
	setCategories( d->mChosen, d->appName, d->mVisibleName );
    }

    if ( cmdCat ) {
	QString qpe_translateLabel(const QString&);
	QString allcat = qpe_translateLabel("_All");
	if ( all ) {
	    cmbCat->insertItem( allcat, cmbCat->count() );
	} else if ( !createCmb && cmbCat->text( cmbCat->count()-1) == allcat ) { // ugly
	    cmbCat->removeItem( cmbCat->count() - 1 );
	}
    }
    if ( all )
	cmbCat->setCurrentItem( cmbCat->count() - 1 );
}

// 01.12.21 added
/*!
  Sets the fixed width of the widget to \a width.
  */
void CategorySelect::setFixedWidth(int width)
{
    if ( d->type == ComboBox ) {
	width -= cmdCat->width();
	cmbCat->setFixedWidth(width);
    }
    else  d->catSelector->setFixedWidth( width );

    d->fixedWidth = width;
}

#include "categoryselect.moc"
