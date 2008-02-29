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

#include "docproperties.h"

#include <qtopia/ir.h>
#include <qtopia/applnk.h>
#include <qtopia/global.h>
#include <qtopia/categorywidget.h>
#include <qtopia/categoryselect.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/filemanager.h>
#include <qtopia/config.h>
#include <qtopia/storage.h>
#include <qtopia/global.h>
#include <qtopia/qpemessagebox.h>

#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qsize.h>
#include <qcombobox.h>
#include <qregexp.h>

#include <qradiobutton.h>
#include <qlayout.h>

#include <stdlib.h>



LocationCombo::LocationCombo( AppLnk * lnk, QWidget *parent )
    : QComboBox( FALSE, parent )
{
    QFileInfo fi( lnk->file() );
    int fileSize = fi.size();
    StorageInfo storage;
    const QList<FileSystem> &fs = storage.fileSystems();
    QListIterator<FileSystem> it ( fs );
    QString s;
    QString homeDir = Global::homeDirPath();
    QString hardDiskHome;
    QString hardDiskPath;
    int index = 0;
    currentLocation = -1;
    for ( ; it.current(); ++it ) {
	// we add 10k to the file size so we are sure we can also save the desktop file
	if ( (ulong)(*it)->availBlocks() * (ulong)(*it)->blockSize() > (ulong)fileSize + 10000 ) {
	    if ( (*it)->isRemovable() ||
		 (*it)->disk() == "/dev/mtdblock1" ||
		 (*it)->disk() == "/dev/mtdblock/1" ) {
		insertItem( (*it)->name(), index );
		locations.append( ((*it)->isRemovable() ? (*it)->path() : homeDir) );
		if ( lnk->file().contains( (*it)->path() ) ) {
		     setCurrentItem( index );
		     currentLocation = index;
		}
		index++;
	    } else if ( (*it)->name().contains( "Hard Disk") &&
			homeDir.contains( (*it)->path() ) &&
			(*it)->path().length() > hardDiskPath.length() ) {
		hardDiskHome = (*it)->name();
		hardDiskPath = (*it)->path();
	    }
	}
    }
    if ( !hardDiskHome.isEmpty() ) {
	insertItem( hardDiskHome );
	QString hardDiskPath = homeDir + "/Documents";
	locations.append( hardDiskPath );
	if ( currentLocation == -1 ) { // assume it's the hard disk
	    setCurrentItem( index );
	    currentLocation = index;
	}
    }
}


bool LocationCombo::isChanged() const
{
    return currentItem() != currentLocation;
}

QString LocationCombo::pathName() const
{
    return locations[ currentItem() ];
}

#if 0
void LocationCombo::apply()
{
}
#endif

class DocPropertiesWidgetPrivate 
{
public:
    LocationCombo *locationCombo;
    CategorySelect *categoryEdit;
    QLineEdit *docname;
    QLabel *doctype;
    QLabel *comment;
    QCheckBox *fastLoad;
};


DocPropertiesWidget::DocPropertiesWidget( AppLnk* l, QWidget* parent )
    : QWidget( parent ), lnk( l )
{
    bool isDocument = lnk->type().contains('/'); // #### better predicate needed
    d = new DocPropertiesWidgetPrivate;

    QVBoxLayout *tll = new QVBoxLayout( this, 0, 0 );


    
    QWidget * main = new QWidget( this );
    tll->addWidget( main );
    
    QGridLayout *grid = new QGridLayout( main, 1, 2, 6, 2 );
    grid->setColStretch( 1, 1 );

    int row = 0;
    
    grid->addWidget( new QLabel( tr("Name:"), main ), row, 0 );
    d->docname = new QLineEdit( main );
    grid->addWidget( d->docname, row, 1 );
    d->docname->setText(lnk->name());
    if ( !isDocument )
	d->docname->setReadOnly( TRUE );
    row++;
    
    
    if ( isDocument ) {
	grid->addWidget( new QLabel( tr("Location:"), main ), row, 0 );
	d->locationCombo = new LocationCombo( lnk, main );
	grid->addWidget( d->locationCombo, row, 1 );
	row++;
    } else {
	d->locationCombo = 0;
    }

    if ( isDocument ) {
	grid->addWidget( new QLabel( tr("Categories:"), main ), row, 0 );
	d->categoryEdit = new CategorySelect( main );
	grid->addWidget( d->categoryEdit, row, 1 );
	d->categoryEdit->setCategories( lnk->categories(),
					"Document View",
					tr("Document View") );
	row++;
    }
    
    if ( !lnk->type().isEmpty() ) {
	grid->addWidget( new QLabel( tr("Type:"), main ), row, 0 );
	d->doctype = new QLabel( main );
	grid->addWidget( d->doctype, row, 1 );
	d->doctype->setText( lnk->type() );    
	row++;
    }
    
    if ( !lnk->comment().isEmpty() ) {
	grid->addWidget( new QLabel( tr("Comment:"), main ), row, 0 );
	d->comment = new QLabel( main );
	grid->addWidget( d->comment, row, 1 );
	d->comment->setText( lnk->comment() );
	row++;
    }

    
#ifdef Q_WS_QWS
    if ( !isDocument
	 && !Global::isBuiltinCommand(lnk->exec()) // builtins are always fast
	 && lnk->property("CanFastload") != "0" )  {
	d->fastLoad = new QCheckBox( tr( "Fast load (consumes memory)" ), main );
	grid->addMultiCellWidget( d->fastLoad, row, row, 0,1 );

    	Config cfg("Launcher");
	cfg.setGroup("Preload");
	QStringList apps = cfg.readListEntry("Apps",',');
	d->fastLoad->setChecked( apps.contains(lnk->exec()) );
	row++;
    } else {
	d->fastLoad = 0;
    }
#endif

    tll->addStretch();

    if ( isDocument ) {
	QFrame *hline = new QFrame( this, "hline" );
	hline->setFrameShadow( QFrame::Sunken );
	hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	tll->addWidget( hline );
	
	QWidget *buttonBox = new QWidget( this );
	tll->addWidget( buttonBox );
	
	QHBoxLayout *hbox = new QHBoxLayout( buttonBox, 3, 5 );
	QPushButton *del = new QPushButton( tr("Delete"), buttonBox );
	hbox->addWidget( del );
	
	QPushButton *copy = new QPushButton( tr("Copy"), buttonBox );
	hbox->addWidget( copy );
	QPushButton *beam = new QPushButton( tr("Beam"), buttonBox );
	hbox->addWidget( beam );
	
	connect(beam,SIGNAL(clicked()),this,SLOT(beamLnk()));
	connect(del,SIGNAL(clicked()),this,SLOT(unlinkLnk()));
	connect(copy,SIGNAL(clicked()),this,SLOT(duplicateLnk()));
    }
}

DocPropertiesWidget::~DocPropertiesWidget()
{
}

void DocPropertiesWidget::applyChanges()
{
    bool changed=FALSE;
    if ( lnk->name() != d->docname->text() ) {
	lnk->setName(d->docname->text());
	changed=TRUE;
    }
    if ( d->categoryEdit->isVisible() ) {
	QArray<int> tmp = d->categoryEdit->currentCategories();
	if ( lnk->categories() != tmp ) {
	    lnk->setCategories( tmp );
	    changed = TRUE;
	}
    }
    if ( !d->fastLoad && d->locationCombo->isChanged() ) {
	moveLnk();
    } else if ( changed ) {
	lnk->writeLink();
    }

    if ( d->fastLoad ) {
	Config cfg("Launcher");
	cfg.setGroup("Preload");
	QStringList apps = cfg.readListEntry("Apps",',');
	QString exe = lnk->exec();
	if ( (apps.contains(exe) > 0) != d->fastLoad->isChecked() ) {
	    if ( d->fastLoad->isChecked() ) {
		apps.append(exe);
#ifndef QT_NO_COP
		QCopEnvelope e("QPE/Application/"+exe.local8Bit(),
			       "enablePreload()");
#endif
	    } else {
		apps.remove(exe);
#ifndef QT_NO_COP
		QCopEnvelope e("QPE/Application/"+exe.local8Bit(),
			       "quitIfInvisible()");
#endif
	    }
	    cfg.writeEntry("Apps",apps,',');
	}
    }
}


void DocPropertiesWidget::duplicateLnk()
{
    // The duplicate takes the new properties.
    DocLnk newdoc( *((DocLnk *)lnk) );
    if ( d->docname->text() == lnk->name() )
	newdoc.setName(tr("Copy of ")+d->docname->text());
    else
	newdoc.setName(d->docname->text());

    if ( !copyFile( newdoc ) ) {
	QMessageBox::warning( this, tr("Duplicate"), tr("File copy failed.") );
	return;
    }
    emit done();
}

bool DocPropertiesWidget::moveLnk()
{
    DocLnk newdoc( *((DocLnk *)lnk) );
    newdoc.setName(d->docname->text());

    if ( !copyFile( newdoc ) ) {
	QMessageBox::warning( this, tr("Details"), tr("Moving Document failed.") );
	return FALSE;
    }
    // remove old lnk
    lnk->removeFiles();

    return TRUE;
}

void DocPropertiesWidget::beamLnk()
{
    Ir ir;
    DocLnk doc( *((DocLnk *)lnk) );
    doc.setName(d->docname->text());
    emit done();
    ir.send( doc, tr("\"%1\"").arg(doc.name()) );
}

bool DocPropertiesWidget::copyFile( DocLnk &newdoc )
{
    const char *linkExtn = ".desktop";
    QString fileExtn;
    int extnPos = lnk->file().findRev( '.' );
    if ( extnPos > 0 )
	fileExtn = lnk->file().mid( extnPos );

    QString safename = newdoc.name();
    safename.replace(QRegExp("/"),"_");

    QString fn = d->locationCombo->pathName() + "/Documents/" + newdoc.type() + "/" + safename;
    if ( QFile::exists(fn + fileExtn) || QFile::exists(fn + linkExtn) ) {
	int n=1;
	QString nn = fn + "_" + QString::number(n);
	while ( QFile::exists(nn+fileExtn) || QFile::exists(nn+linkExtn) ) {
	    n++;
	    nn = fn + "_" + QString::number(n);
	}
	fn = nn;
    }
    newdoc.setFile( fn + fileExtn );
    newdoc.setLinkFile( fn + linkExtn );

    // Copy file
    FileManager fm;
    if ( !fm.copyFile( *lnk, newdoc ) )
	return FALSE;
    return TRUE;
}




void DocPropertiesWidget::unlinkLnk()
{
    if ( QPEMessageBox::confirmDelete( this, tr("Delete"), lnk->name() ) ) {
	lnk->removeFiles();
	if ( QFile::exists(lnk->file()) ) {
	    QMessageBox::warning( this, tr("Delete"), tr("File deletion failed.") );
	} else {
	    emit done();
	}
    }
}



DocPropertiesDialog::DocPropertiesDialog( AppLnk* l, QWidget* parent )
    : QDialog( parent, 0, TRUE )
{
    setCaption( tr("Properties") );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    d = new DocPropertiesWidget( l, this );
    vbox->add( d );
    connect( d, SIGNAL(done()), this, SLOT(reject()) );
}


DocPropertiesDialog::~DocPropertiesDialog()
{
}



void DocPropertiesDialog::done(int ok)
{
    if ( ok ) 
	d->applyChanges();
    QDialog::done( ok );
}
