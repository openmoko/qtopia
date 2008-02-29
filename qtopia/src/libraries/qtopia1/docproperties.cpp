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

#include "docproperties.h"
#include "locationcombo.h"

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

#include <qapplication.h>
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


class DocPropertiesWidgetPrivate 
{
public:
    DocPropertiesWidgetPrivate()
    {
	categoryEdit = 0;
    }
    
    LocationCombo *locationCombo;
    CategorySelect *categoryEdit;
    QLineEdit *docname;
    QLabel *doctype;
    QLabel *comment;
    QCheckBox *fastLoad;
    QString warning;
};


/*!
  \class DocPropertiesWidget docproperties.h
  \brief The DocPropertiesWidget class provides controls for modifying an
         AppLnks properties.

  The DocPropertiesWidget and allows modification of the name, location,
  and category of a file associated with a particular AppLnk.  The file can
  also be deleted, copied and beamed from the DocPropertiesWidget.

  First availability: Qtopia 1.6

  \ingroup qtopiaemb
  \sa DocPropertiesDialog
*/

#if QT_VERSION >= 0x40000
#error "use dehyphenate from stringutil.h"
#endif
static QString dehyphenate(const QString& s)
{
    QChar shy(0x00ad);
    int i=0;
    while (i<(int)s.length() && s[i]!=shy)
	i++;
    if ( i==(int)s.length() )
	return s;
    QString r = s.left(i);
    for (; i<(int)s.length(); ++i) {
	if ( s[i]!=shy )
	    r += s[i];
    }
    return r;
}

/*!
  Constructs a DocPropertiesWidget with parent \a parent and name \a name.
  \a l is a pointer to an existing AppLnk.
 */
DocPropertiesWidget::DocPropertiesWidget( AppLnk* l, QWidget* parent, const char *name )
    : QWidget( parent, name ), lnk( l )
{
    bool isDocument = lnk->isDocLnk();
    d = new DocPropertiesWidgetPrivate;

    QVBoxLayout *tll = new QVBoxLayout( this, 0, 0 );


    
    QWidget * main = new QWidget( this );
    tll->addWidget( main );
    
    QGridLayout *grid = new QGridLayout( main, 1, 2, 6, 2 );
    grid->setColStretch( 1, 1 );

    int row = 0;
    
    grid->addWidget( new QLabel( tr("Name"), main ), row, 0 );
    d->docname = new QLineEdit( main );
    grid->addWidget( d->docname, row, 1 );
    if ( isDocument ) {
	d->docname->setText(lnk->name());
    } else {
	d->docname->setText(dehyphenate(lnk->name()));
	d->docname->setEnabled( FALSE );
    }
    row++;
    
    
    if ( isDocument ) {
	if (qApp->desktop()->width() < 240)
	    grid->addWidget( new QLabel( tr("Loc"), main ), row, 0 );
	else
	    grid->addWidget( new QLabel( tr("Location"), main ), row, 0 );
	d->locationCombo = new LocationCombo( lnk, main );
	grid->addWidget( d->locationCombo, row, 1 );
	row++;
    } else {
	d->locationCombo = 0;
    }

    if ( isDocument ) {
	QString cl;
	if (qApp->desktop()->width() < 240)
	    cl = tr("Cat");
	else
	    cl = tr("Category");
	grid->addWidget( new QLabel( cl, main ), row, 0 );
	d->categoryEdit = new CategorySelect( main );
	grid->addWidget( d->categoryEdit, row, 1 );
	d->categoryEdit->setCategories( lnk->categories(),
					"Document View", // No tr
					tr("Document View") );
	row++;
    }
    
    if ( !lnk->type().isEmpty() ) {
	grid->addWidget( new QLabel( tr("Type"), main ), row, 0 );
	d->doctype = new QLabel( main );
	grid->addWidget( d->doctype, row, 1 );
	d->doctype->setText( lnk->type() );    
	row++;
    }
    
    if ( !lnk->comment().isEmpty() ) {
	grid->addWidget( new QLabel( tr("Comment"), main ), row, 0 );
	d->comment = new QLabel( main );
	grid->addWidget( d->comment, row, 1 );
	d->comment->setText( "<qt>" + lnk->comment() + "</qt>" );
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

#ifndef QTOPIA_PHONE	//XXX confusing due to context buttons.
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
#endif
}

/*!
  Destroys the widget.
 */
DocPropertiesWidget::~DocPropertiesWidget()
{
    delete d;
}

void DocPropertiesWidget::setWarning( const QString& w )
{
    d->warning = w;
}

/*!
  Applys any changes made on the DocPropertiesWidget.
 */
void DocPropertiesWidget::applyChanges()
{
    bool changed=FALSE;
    bool isDocument = lnk->isDocLnk();
    if ( isDocument && lnk->name() != d->docname->text() ) {
	lnk->setName(d->docname->text());
	changed=TRUE;
    }
    if ( d->categoryEdit ) {
	QArray<int> tmp = d->categoryEdit->currentCategories();
	if ( lnk->categories() != tmp ) {
	    lnk->setCategories( tmp );
	    changed = TRUE;
	}
    }
    if ( !d->fastLoad && d->locationCombo && d->locationCombo->isChanged() ) {
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
		QCopEnvelope("QPE/Application/"+exe.local8Bit(),
			       "disablePreload()");
		QCopEnvelope("QPE/Application/"+exe.local8Bit(),
			       "quitIfInvisible()");
#endif
	    }
	    cfg.writeEntry("Apps",apps,',');
	}
    }
}

/*!
  Performs a copy operation.
 */
void DocPropertiesWidget::duplicateLnk()
{
    // The duplicate takes the new properties.
    DocLnk newdoc( *((DocLnk *)lnk) );
    if ( d->docname->text() == lnk->name() )
	newdoc.setName(tr("Copy of ")+d->docname->text());
    else
	newdoc.setName(d->docname->text());

    if ( !copyFile( newdoc ) ) {
	QMessageBox::warning( this, tr("Duplicate"), tr("<qt>File copy failed.</qt>") );
	return;
    }
    emit done();
}

/*!
  \internal
 */
bool DocPropertiesWidget::moveLnk()
{
    DocLnk newdoc( *((DocLnk *)lnk) );
    newdoc.setName(d->docname->text());

    if ( !copyFile( newdoc ) ) {
	QMessageBox::warning( this, tr("Details"), tr("<qt>Moving Document failed.</qt>") );
	return FALSE;
    }
    // remove old lnk
    lnk->removeFiles();

    return TRUE;
}

/*!
  Beams the associated document.
 */
void DocPropertiesWidget::beamLnk()
{
    Ir ir;
    DocLnk doc( *((DocLnk *)lnk) );
    doc.setName(d->docname->text());
    emit done();
    ir.send( doc, tr("\"%1\"").arg(doc.name()) );
}

/*!
  \internal
 */
bool DocPropertiesWidget::copyFile( DocLnk &newdoc )
{
    const char *linkExtn = ".desktop";
    QString fileExtn;
    int extnPos = lnk->file().findRev( '.' );
    if ( extnPos > 0 )
	fileExtn = lnk->file().mid( extnPos );

    QString safename = newdoc.name();
    safename.replace(QRegExp("/"),"_");

    QString fn = d->locationCombo->documentPath() + newdoc.type() + "/" + safename;
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

/*!
  Deletes the file associated with this link.
 */
void DocPropertiesWidget::unlinkLnk()
{
    QString object = d->warning.isNull() ?
        lnk->name() : lnk->name() + d->warning;
    if ( QPEMessageBox::confirmDelete( this, tr("Delete"), object ) ) {
	lnk->removeFiles();
	if ( QFile::exists(lnk->file()) ) {
	    QMessageBox::warning( this, tr("Delete"), tr("<qt>File deletion failed.</qt>") );
	} else {
	    emit done();
	}
    }
}

/*!
  \fn void DocPropertiesWidget::done()

  This signal is emitted when a file is deleted, copied or beamed from the
  DocPropertiesWidget.
 */

/*!
  \fn void DocPropertiesWidget::deleted()
  \internal
*/

/*!
  \class DocPropertiesDialog docproperties.h
  \brief The DocPropertiesDialog class allows the user to examine attributes
         associated with a AppLnk object.

  The DocPropertiesDialog uses a DocPropertiesWidget to allow the user to
  examine and modify attributes associated with a file.

  First availability: Qtopia 1.6

  \ingroup qtopiaemb
  \sa DocPropertiesWidget
*/


/*!
  Constructs a DocPropertiesDialog with parent \a parent and name \a name.
  \a l is a pointer to an existing AppLnk.
 */
DocPropertiesDialog::DocPropertiesDialog( AppLnk* l, QWidget* parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    setCaption( tr("Properties") );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    d = new DocPropertiesWidget( l, this );
    vbox->add( d );
    connect( d, SIGNAL(done()), this, SLOT(reject()) );
}

/*!
  Destroys the dialog.
 */
DocPropertiesDialog::~DocPropertiesDialog()
{
}

/*!
  If \a ok is TRUE, any modifications done in the dialog are applied to the
  associated AppLnk.
 */
void DocPropertiesDialog::done(int ok)
{
    if ( ok ) 
	d->applyChanges();
    QDialog::done( ok );
}

