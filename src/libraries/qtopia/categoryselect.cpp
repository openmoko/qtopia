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

#include "categorywidget.h"
#include "categoryselect.h"

#include <stdlib.h>

static QString categoryEdittingFileName()
{
QString str = QDir::homeDirPath();
#ifdef QTOPIA_DESKTOP
    str += "/.palmtopcenter/";
#endif

    str +="/.cateditting";
    return str;
}

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
      results = recCats;
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
	    if ( qApp->desktop()->height() >= 600 && !usingAll )
		type = CategorySelect::ListView;
	    else
		type = CategorySelect::ComboBox;
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

    void reloadCatSelector()
	{
	    catSelector->clear();
	    catSelector->addCheckableList( cats.labels( appName ) );

	    // labels() from qtopia1
	    QStringList strs = cats.globalGroup().labels( mRec );
	    strs += cats.appGroupMap()[appName].labels( mRec );

	    catSelector->setChecked( strs );
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


CategorySelect::CategorySelect( QWidget *parent, const char *name,int width)
    : QHBox( parent, name ),
      cmbCat( 0 ),
      cmdCat( 0 ),
      d( 0 )
{
    init(width);
}

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


CategorySelect::~CategorySelect()
{
}

void CategorySelect::slotDialog()
{
    if (QFile::exists( categoryEdittingFileName() )){
        QMessageBox::warning(this,tr("Error"),
	   tr("Sorry, another application is\nediting categories.") );
        return;
    }

    QFile f( categoryEdittingFileName() );
    if ( !f.open( IO_WriteOnly) ){
        return;
    }

    QDialog editDlg( this, "categories", TRUE ); // No tr
    editDlg.setCaption( tr("Edit Categories") );
    QVBoxLayout *vb = new QVBoxLayout( &editDlg );
    CategoryWidget ce( d->mRec, mStrAppName, d->mVisibleName, &editDlg );
    vb->addWidget( &ce );
    editDlg.showMaximized();

    d->editMode = TRUE;
    if ( editDlg.exec() ) {
	d->mRec = ce.newCategories();
	cmbCat->initCombo( d->mRec, mStrAppName, d->mVisibleName );
    }

    f.close();
    QFile::remove( categoryEdittingFileName() );
}

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
	    d->cats.load( categoryFileName() );
	    d->pruneDeletedCats();
	    d->reloadCatSelector();
	}
    }
}

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

QString CategorySelect::setCategories( const QArray<int> &rec,
				    const QString &appName )
{
    return setCategories( rec, appName, appName );
}

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
	d->mRec = rec;
	d->reloadCatSelector();
    }
    return Qtopia::Record::idsToString(d->mRec);
}

void CategorySelect::init(int width, bool usingAll )
{
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


const QArray<int> &CategorySelect::currentCategories() const
{
    return d->mRec;
}

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
	init( d->fixedWidth, all );
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
