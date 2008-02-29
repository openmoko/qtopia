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

#include <qmessagebox.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qfile.h>

#include "categorywidget.h"
#include "categoryselect.h"

#include <stdlib.h>

static QString categoryEdittingFileName()
{
    QString str = getenv("HOME");
    str +="/.cateditting";
    return str;
}

class CategoryComboPrivate
{
public:
    CategoryComboPrivate(QObject *o)
	: mCat( o )
    {
    }
    QArray<int> mAppCats;
    QString mStrAppName;
    QString mStrVisibleName;
    Categories mCat;
};

class CategorySelectPrivate
{
public:
    CategorySelectPrivate( const QArray<int> &cats)
	: mRec( cats ),
	  usingAll( false )
    {
    }
    CategorySelectPrivate()
    {
    }
    QArray<int> mRec;
    bool usingAll;
    QString mVisibleName;
};

CategoryCombo::CategoryCombo( QWidget *parent, const char *name , int width)
    : QComboBox( parent, name )
{
    QSizePolicy p = sizePolicy();
    p.setHorData(QSizePolicy::Expanding);
    setSizePolicy(p);
    d = new CategoryComboPrivate(this);
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

    QObject::connect( this, SIGNAL(activated(int)),
		      this, SLOT(slotValueChanged(int)) );
    bool loadOk = d->mCat.load( categoryFileName() );
    slApp = d->mCat.labels( d->mStrAppName, TRUE, Categories::UnfiledLabel );

    d->mAppCats = d->mCat.ids( d->mStrAppName, slApp);

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
			(*it).append( tr(" (Multi.)") );
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
    bool loadOk = d->mCat.load( categoryFileName() );
    slApp = d->mCat.labels( d->mStrAppName, TRUE, Categories::UnfiledLabel );

    d->mAppCats = d->mCat.ids( d->mStrAppName, slApp);

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
			(*it).append( tr(" (Multi.)") );
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
    int returnMe;
    returnMe = currentItem();

    if ( returnMe == (int)d->mAppCats.count() )
	returnMe = -1;
    else if ( returnMe > (int)d->mAppCats.count() )  // only happen on "All"
	returnMe = -2;
    else
	returnMe =  d->mAppCats[returnMe];
    return returnMe;
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

CategorySelect::CategorySelect( QWidget *parent, const char *name,int width)
    : QHBox( parent, name ),
      cmbCat( 0 ),
      cmdCat( 0 ),
      d( 0 )
{
    d = new CategorySelectPrivate();
    init(width);
}

CategorySelect::CategorySelect( const QArray<int> &vl,
				const QString &appName, QWidget *parent,
				const char *name ,int width)
    : QHBox( parent, name )
{
    d = new CategorySelectPrivate( vl );

    init(width);

    setCategories( vl, appName, appName );
}

CategorySelect::CategorySelect( const QArray<int> &vl,
				const QString &appName,
				const QString &visibleName,
				QWidget *parent, const char *name , int width)
    : QHBox( parent, name )
{
    d = new CategorySelectPrivate( vl );
    init(width);
    setCategories( vl, appName, visibleName );
}

CategorySelect::~CategorySelect()
{
    delete d;
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

    QDialog editDlg( this, 0, TRUE );
    editDlg.setCaption( tr("Edit Categories") );
    QVBoxLayout *vb = new QVBoxLayout( &editDlg );
    CategoryWidget ce( d->mRec, mStrAppName, d->mVisibleName, &editDlg );
    vb->addWidget( &ce );
    editDlg.showMaximized();

    if ( editDlg.exec() ) {
	d->mRec = ce.newCategories();
	cmbCat->initCombo( d->mRec, mStrAppName, d->mVisibleName );
    }

    f.close();
    QFile::remove( categoryEdittingFileName() );
}

void CategorySelect::slotNewCat( int newUid )
{
    if ( newUid != -1 ) {
	bool alreadyIn = false;
	for ( uint it = 0; it < d->mRec.count(); ++it ) {
	    if ( d->mRec[it] == newUid ) {
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
    d->mRec = cmbCat->initComboWithRefind( rec, appName );
    return Qtopia::Record::idsToString(d->mRec);
}

void CategorySelect::init(int width)
{
    cmbCat = new CategoryCombo( this, 0, width);

    QObject::connect( cmbCat, SIGNAL(sigCatChanged(int)),
		      this, SLOT(slotNewCat(int)) );
    cmdCat = new QToolButton( this );
    QObject::connect( cmdCat, SIGNAL(clicked()), this, SLOT(slotDialog()) );
    cmdCat->setTextLabel( "...", FALSE );
    cmdCat->setUsesTextLabel( true );
    cmdCat->setMaximumSize( cmdCat->sizeHint() );
    cmdCat->setFocusPolicy( TabFocus );
    cmdCat->setFixedHeight( cmbCat->sizeHint().height() );
}


int CategorySelect::currentCategory() const
{
    return cmbCat->currentCategory();
}

void CategorySelect::setCurrentCategory( int newCatUid )
{
    cmbCat->setCurrentCategory( newCatUid );
}


const QArray<int> &CategorySelect::currentCategories() const
{
    return d->mRec;
}

void CategorySelect::setRemoveCategoryEdit( bool remove )
{
    if ( remove ) {
	cmdCat->setEnabled( FALSE );
	cmdCat->hide();
    } else {
	cmdCat->setEnabled( TRUE );
	cmdCat->show();
    }
}

void CategorySelect::setAllCategories( bool add )
{
    d->usingAll = add;
    if ( add ) {
	cmbCat->insertItem( tr( "All" ), cmbCat->count() );
	cmbCat->setCurrentItem( cmbCat->count() - 1 );
    } else
	cmbCat->removeItem( cmbCat->count() - 1 );
}

// 01.12.21 added
void CategorySelect::setFixedWidth(int width)
{
  width -= cmdCat->width();
  cmbCat->setFixedWidth(width);
}
