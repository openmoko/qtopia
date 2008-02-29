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
    : QComboBox( parent, name )
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

    int i,
	j,
	saveMe,
	recCount;
    QStringList::Iterator it;
    // now add in all the items...
    recCount = recCats.count();
    saveMe = -1;
    if ( recCount > 1 && loadOk ) {
	it = slApp.begin();
	for ( j = 0; j< (int)(slApp.count()-1); ++it, j++ ) {
	    // grr... we have to go through and compare...
	    if ( j < int(d->mAppCats.size()) ) {
		for ( i = 0; i < recCount; i++ ) {
		    if ( recCats[i] == d->mAppCats[j] ) {
			(*it).append( tr(" (multi.)","short 'multiple'") );
			if ( saveMe < 0 )
			    saveMe = j;
			// no need to continue through the list.
			break;
		    }
		}
	    }
	    insertItem( *it );
	}
	insertItem( *it );
    } else
	insertStringList( slApp );

    if ( recCount > 0 && loadOk ) {
	for ( i = 0; i < int(d->mAppCats.size()); i++ ) {
	    if ( d->mAppCats[i] == recCats[0] ) {
		setCurrentItem( i );
		break;
	    }
	}
    } else {
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

    int i,
	j,
	saveMe,
	recCount;
    QStringList::Iterator it;
    // now add in all the items...
    recCount = results.count();
    saveMe = -1;
    if ( recCount > 1 && loadOk ) {
	it = slApp.begin();
	for ( j = 0; j< (int)(slApp.count()-1); ++it, j++ ) {

	    // grr... we have to go through and compare...
	    if ( j < int(d->mAppCats.size()) ) {
		for ( i = 0; i < recCount; i++ ) {
		    if ( results[i] == d->mAppCats[j] ) {
			(*it).append( tr(" (multi.)","short 'multiple'") );
			if ( saveMe < 0 )
			    saveMe = j;
			// no need to continue through the list.
			break;
		    }
		}
	    }
	    insertItem( *it );
	}
	insertItem( *it );
    } else
	insertStringList( slApp );

    if ( recCount > 0 && loadOk ) {
	for ( i = 0; i < int(d->mAppCats.size()); i++ ) {
	    if ( d->mAppCats[i] == results[0] ) {
		setCurrentItem( i );
		break;
	    }
	}
    } else
    {
	setCurrentItem( slApp.count()-1 );  // unfiled
    }
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
	r =  d->mAppCats[i];

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
    CategorySelectPrivate( QObject *parent, bool all )
	: QObject(parent, "CategorySelectPrivate" ), mRec(),
	  usingAll( all ), mVisibleName(), editMode( FALSE ),
	  canEdit( TRUE ), type( CategorySelect::ComboBox ), container( 0 ),
	  layout(0), fixedWidth(0), catSelector( 0 )
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

    QArray<int> mRec;
    bool usingAll;
    QString mVisibleName;
    bool editMode;
    bool canEdit;
    CategorySelect::SelectorWidget type;
    QWidget *container;
    QGridLayout *layout;
    int fixedWidth;

    // so far, the CheckedListView is used only for Qtopia Desktop
    // but could be used for bigger PDA screen sizes

    // the following methods and variables are used only when
    // type == ComboBox
    CheckedListView *catSelector;
    QPushButton *editButton;
    Categories cats;
    QString appName;

    void reloadCatSelector( const QStringList &checked )
	{
	    catSelector->clear();
	    catSelector->addCheckableList( cats.labels( appName ) );
	    catSelector->setChecked( checked );
	}

    void reloadCatSelector()
	{
	    // labels() from qtopia1
	    QStringList strs = cats.globalGroup().labels( mRec );
	    strs += cats.appGroupMap()[appName].labels( mRec );
	    reloadCatSelector( strs );
	}
    void pruneDeletedCats()
	{
	    QArray<int> checkedCats;
	    for ( int i = 0; i < (int) mRec.count();++i )
		if ( cats.label( appName, mRec[i] ) != QString::null ) {
		    checkedCats.resize( i+1 );
		    checkedCats[i] = mRec[i];
		}

	    mRec = checkedCats;
	}

public slots:
    void itemClicked( QListViewItem * )
	{
	    mRec = cats.ids( appName, catSelector->checked() );
	}

};


/*!
  \overload

  This constructor accepts an array \a vl of integers representing Categories.
  \a visibleName is the string used when the name of this widget is required
  to be displayed. \a width is an integer used as the fixed width of the widget.
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

class EditDlg : public QDialog
{
    Q_OBJECT
public:
    EditDlg(QWidget *parent, const char *name, bool modal,
	    const QArray<int> &vlRecs, const QString &appName, const QString &visibleName)
	: QDialog(parent, name, modal)
    {
	setCaption( tr("Edit Categories") );
	QVBoxLayout *vb = new QVBoxLayout( this );
	ce = new CategoryEdit( vlRecs, appName, visibleName, this );
	vb->addWidget( ce );
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

/*!
  This slot is called when the user pushes the button to edit Categories.
*/

void CategorySelect::slotDialog()
{
    EditDlg editDlg( this, "categories", TRUE, d->mRec, mStrAppName, d->mVisibleName ); // No tr
#ifndef QTOPIA_DESKTOP
    editDlg.showMaximized();
    // need to add OKAY, CANCEL button
#endif

    d->editMode = TRUE;
    if ( editDlg.exec() ) {
	d->mRec = editDlg.newCategories();
	cmbCat->initCombo( d->mRec, mStrAppName, d->mVisibleName );
	if ( d->usingAll )
	    cmbCat->insertItem( tr( "All" ), cmbCat->count() );
    }
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
	    cmbCat->initComboWithRefind( d->mRec, mStrAppName );
	    if ( d->usingAll ) {
		cmbCat->insertItem( tr( "All" ), cmbCat->count() );
	    }
	    cmbCat->setCurrentCategory( prevCat );

	    // Test if category is still valid.  If it isn't we need to inform our parent about a selection change
	    if ( cmbCat->currentCategory() != prevCat ) {
		cmbCat->setCurrentCategory(-1);
		emit signalSelected( cmbCat->currentCategory() );
	    }
	}
	else {
	    QStringList current = d->catSelector->checked();
	    d->cats.load( categoryFileName() );
	    d->pruneDeletedCats();
	    d->reloadCatSelector( current );
	}
    }
}

/*!
  This slot is called when a new Category is available.
*/

void CategorySelect::slotNewCat( int newUid )
{
    if ( newUid != -1 ) {
	bool alreadyIn = false;
	for ( uint it = 0; it < d->mRec.count(); ++it ) {
	    if ( d->mRec[(int)it] == newUid ) {
		alreadyIn = true;
		break;
	    }
	}
	if ( !alreadyIn ) {
	    d->mRec.resize( 1 );
	    d->mRec[ 0 ] = newUid;
	}
    } else
    d->mRec.resize(0);  // now Unfiled.
    emit signalSelected( currentCategory() );
}

/*!
  \overload

  Resets the CategorySelect to select the \a vlCats for
  the Categories assoicated with \a appName.

  This function should only be called if <i>filtering</i>
  on Categories and not selecting and therefore possibly
  allowing the user to edit Categories.
*/
QString CategorySelect::setCategories( const QArray<int> &rec,
				    const QString &appName )
{
    return setCategories( rec, appName, appName );
}


/*!
  Resets the CategorySelect to select the \a vlCats for
  the Categories assoicated with \a appName and displays
  the \a visibleName if the user is selecting and therefore editing
  new Categories.
 */
QString CategorySelect::setCategories( const QArray<int> &rec,
				    const QString &appName,
				    const QString &visibleName )
{
    d->mVisibleName = visibleName;
    mStrAppName = appName;
    d->appName = appName;
    if ( d->type == ComboBox )
	d->mRec = cmbCat->initComboWithRefind( rec, appName );
    else {
	d->mRec = rec.copy();
	d->reloadCatSelector();
    }
    return Qtopia::Record::idsToString(d->mRec);
}

#ifdef QTOPIA_DESKTOP
void CategorySelect::init(int width, bool usingAll )
{
#else
void CategorySelect::init(int width)
{
    const bool usingAll = FALSE;
#endif
    if ( d ) {
	delete d->layout;
	delete d->container;
	d->usingAll = usingAll;
	d->setType();
    } else {
	d = new CategorySelectPrivate( this, usingAll );
    }
    d->container = new QWidget( this, "CategorySelect container" ); // No tr

    if ( d->type == ComboBox ) {
	d->layout = new QGridLayout( d->container );

	delete cmbCat;
	cmbCat = new CategoryCombo( d->container, "category combo", width); // No tr
	d->layout->addWidget( cmbCat, 0, 0 );
	QObject::connect( cmbCat, SIGNAL(sigCatChanged(int)),
			  this, SLOT(slotNewCat(int)) );

	if ( d->canEdit ) {
	    cmdCat = new QToolButton( d->container, "category button" ); // No tr
	    d->layout->addWidget( cmdCat, 0, 1 );

	    cmdCat->setTextLabel( "...", FALSE );
	    cmdCat->setFocusPolicy( TabFocus );
	    cmdCat->setFixedHeight( cmbCat->sizeHint().height() );
	    cmdCat->setUsesTextLabel( true );
	    QObject::connect( cmdCat, SIGNAL(clicked()), this, SIGNAL(editCategoriesClicked()) );
	    QObject::connect( cmdCat, SIGNAL(clicked()), this, SLOT(slotDialog()) );
	}
    }
    else {
	d->layout = new QGridLayout( d->container, 2, 2 );
	d->cats.load( categoryFileName() );
	d->catSelector = (CheckedListView *) new QListView( d->container, "catSelector" );
	d->catSelector->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum ) );
	d->catSelector->addColumn( tr("Categories") );
	d->catSelector->setSelectionMode( QListView::NoSelection );
	d->layout->addMultiCellWidget( d->catSelector, 0, 0, 0, 1 );

	QSpacerItem *horiSpacer = new QSpacerItem( 1, 1, QSizePolicy::Expanding,
						   QSizePolicy::Expanding );
	d->layout->addItem( horiSpacer, 1, 0 );

	d->editButton = new QPushButton( tr("Edit Categories"), d->container );
	d->layout->addWidget( d->editButton, 1, 1 );

	connect( d->catSelector, SIGNAL( clicked( QListViewItem *) ), d, SLOT( itemClicked( QListViewItem *) ) );
	connect( d->catSelector, SIGNAL( spacePressed( QListViewItem *) ), d, SLOT( itemClicked( QListViewItem *) ) );
	connect( d->editButton, SIGNAL( clicked() ),
		 this, SIGNAL( editCategoriesClicked() ) );
#ifndef QTOPIA_DESKTOP
	connect( d->editButton, SIGNAL( clicked() ),
		 SLOT( slotDialog() ) );
#endif
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
    if ( d->type == ComboBox )
	cmbCat->setCurrentCategory( newCatUid );
    else d->catSelector->setChecked( d->cats.label( mStrAppName, newCatUid ) );
}

/*!
  Returns a shallow copy of the categories in this CategorySelect.
 */
const QArray<int> &CategorySelect::currentCategories() const
{
    return d->mRec;
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
    if ( !d->usingAll && all && d->catSelector ) {
	createCmb = TRUE;
	reinit = TRUE;
    }
    else if ( d->usingAll && !all && !d->catSelector )
	reinit = TRUE;

    if ( reinit ) {
#ifdef QTOPIA_DESKTOP
	init( d->fixedWidth, all );
#else
	init( d->fixedWidth );
	d->usingAll = all;
#endif
	setCategories( d->mRec, d->appName, d->mVisibleName );
    }

    if ( cmdCat ) {
	if ( all ) {
	    cmbCat->insertItem( tr( "All" ), cmbCat->count() );
	    cmbCat->setCurrentItem( cmbCat->count() - 1 );
	} else if ( !createCmb && cmbCat->text( cmbCat->count()-1) == tr("All") ) {
	    cmbCat->removeItem( cmbCat->count() - 1 );
	}
    }

    d->usingAll = all;
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
