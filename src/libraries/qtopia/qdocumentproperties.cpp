/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdocumentproperties.h"
#include <qstoragedeviceselector.h>

#include <qtopiasendvia.h>
#include <qtopiaservices.h>
#include <qcontent.h>
#include <qcategoryselector.h>
#include <qtopiaipcenvelope.h>
#include <qstorage.h>
#include <qtopianamespace.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qsettings.h>
#include <qdesktopwidget.h>
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
#include <QScrollArea>
#include <QDir>
#include <QPushButton>
#include <QDebug>

#include <stdlib.h>


class QDocumentPropertiesWidgetPrivate
{
public:
    QDocumentPropertiesWidgetPrivate()
    {
        categoryEdit = 0;
    }
    QString humanReadable(quint64 size);

    QStorageDeviceSelector *locationCombo;
    QCategorySelector *categoryEdit;
    QLineEdit *docname;
    QLabel *doctype;
    QLabel *comment;
    QLabel *fileSize;
    QCheckBox *fastLoad;
    const QContent *doc;
};

QString QDocumentPropertiesWidgetPrivate::humanReadable(quint64 size)
{
    if(size == 1)
        return QObject::tr("1 byte");
    else if(size < 1024)
        return QObject::tr("%1 bytes").arg(size);
    else if(size < (1024 * 1024))
        return QObject::tr("%1 KB").arg(((float)size)/1024.0, 0, 'f', 1);
    else if(size < (1024 * 1024 * 1024))
        return QObject::tr("%1 MB").arg(((float)size)/(1024.0 * 1024.0), 0, 'f', 1);
    else
        return QObject::tr("%1 GB").arg(((float)size)/(1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}


/*!
  \class QDocumentPropertiesWidget
  \ingroup content
  \mainclass
  \brief The QDocumentPropertiesWidget class provides controls for viewing and modifying the
   properties of a document.

  The QDocumentPropertiesWidget allows modification of the name, location,
  and category of a document.  
  
  In addition, the following operations are available: 
    \table
    \header
        \o Operation
        \o Slot
    \row
        \o Beaming
        \o beamLnk()
    \row
        \o Duplicating or copying
        \o duplicateLnk()
    \row
        \o Deleting
        \o unlinkLnk()
    \endtable 

  On the phone edition of Qtopia, the slots would typically be invoked through the context
  menu. 
    
  If phone edition is not used, buttons to facilitate these operations will appear on the 
  QDocumentPropertiesWidget, however the slots are still available for use if required.

  \sa QDocumentPropertiesDialog
*/

/*!
  Constructs a QDocumentPropertiesWidget with a QContent,\a doc, representing the
  document, and a \a parent widget.

  Ensure that \a doc refers to a document as opposed to an application.
 */
QDocumentPropertiesWidget::QDocumentPropertiesWidget( const QContent &doc, QWidget* parent )
    : QWidget( parent ), lnk( doc )
{
    bool isDocument = lnk.isDocument();
    d = new QDocumentPropertiesWidgetPrivate;

    d->doc = &doc;
    QVBoxLayout *tll = new QVBoxLayout( this );
    tll->setMargin(0);

    QWidget * main = new QWidget( this );
    tll->addWidget( main );
    tll->addStretch();

    QGridLayout *grid = new QGridLayout( main );
    grid->setMargin( 6 );
    grid->setSpacing( 6 );
    grid->setColumnStretch( 1, 1 );

    int row = 0;

    QLabel *label = new QLabel( tr("Name"), main );
    grid->addWidget( label, row, 0, Qt::AlignLeft | Qt::AlignTop );
    d->docname = new QLineEdit( main );
    label->setBuddy(d->docname);
    grid->addWidget( d->docname, row, 1 );
    if ( isDocument ) {
        d->docname->setText(lnk.name());
    } else {
        d->docname->setText(Qtopia::dehyphenate(lnk.name()));
        d->docname->setEnabled( false );
    }
    row++;

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect(desktop->availableGeometry(desktop->screenNumber(this)));

    if ( isDocument ) {
        if (desktopRect.width() < 240)
            label = new QLabel( tr("Loc"), main );
        else
            label = new QLabel( tr("Location"), main );
        grid->addWidget( label, row, 0, Qt::AlignLeft | Qt::AlignTop );
        d->locationCombo = new QStorageDeviceSelector( lnk, main );
        label->setBuddy(d->locationCombo);
        QFileSystemFilter *fsf = new QFileSystemFilter;
        fsf->documents = QFileSystemFilter::Set;
        if( !(doc.permissions() & QDrmRights::Distribute) && !QFileSystem::fromFileName( doc.fileName() ).isRemovable() )
            fsf->removable = QFileSystemFilter::NotSet;
        d->locationCombo->setFilter( fsf );
        grid->addWidget( d->locationCombo, row, 1 );
        row++;
    } else {
        d->locationCombo = 0;
    }

    if ( isDocument ) {
        QString cl;
        if (desktopRect.width() < 240)
            cl = tr("Cat", "short for category");
        else
            cl = tr("Category");
        label = new QLabel( cl, main );
        grid->addWidget( label, row, 0, Qt::AlignLeft | Qt::AlignTop );
        d->categoryEdit = new QCategorySelector("Documents", QCategorySelector::Editor | QCategorySelector::DialogView);
        label->setBuddy(d->categoryEdit);
        grid->addWidget( d->categoryEdit, row, 1 );
        d->categoryEdit->selectCategories( lnk.categories() );
        row++;
    }

    if ( !lnk.type().isEmpty() ) {
        grid->addWidget( new QLabel( tr("Type"), main ), row, 0, Qt::AlignLeft | Qt::AlignTop );
        d->doctype = new QLabel( main );
        grid->addWidget( d->doctype, row, 1 );
        if(lnk.type() == "application/octet-stream")
            d->doctype->setText( tr("Unknown", "Unknown document type") );
        else
            d->doctype->setText( lnk.type().replace("/", " / ") );
        d->doctype->setWordWrap( true );
        row++;
    }

    if ( !lnk.comment().isEmpty() ) {
        grid->addWidget( new QLabel( tr("Comment"), main ), row, 0, Qt::AlignLeft | Qt::AlignTop );
        d->comment = new QLabel( main );
        d->comment->setWordWrap( true );
        grid->addWidget( d->comment, row, 1 );
        d->comment->setText( "<qt>" + lnk.comment() + "</qt>" );
        d->comment->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
        row++;
    }

    grid->addWidget( new QLabel( tr("File Size"), main ), row, 0, Qt::AlignLeft | Qt::AlignTop );
    d->fileSize = new QLabel( main );
    grid->addWidget( d->fileSize, row, 1 );
    d->fileSize->setText( "<qt>" + d->humanReadable(lnk.size()) + "</qt>" );
    row++;

    if ( !isDocument
         && lnk.property("CanFastload") != "0" )  {
        d->fastLoad = new QCheckBox( tr( "Fast load (consumes memory)" ), main );
        grid->addWidget( d->fastLoad, row, 0, 1, 2 );

        QSettings cfg("Trolltech","Launcher");
        cfg.beginGroup("AppLoading");
        QStringList apps = cfg.value("PreloadApps").toString().split(',');
        d->fastLoad->setChecked( apps.contains(lnk.executableName()) );
        row++;
    } else {
        d->fastLoad = 0;
    }

    if( doc.drmState() == QContent::Protected )
    {
        QPushButton *licensesButton = new QPushButton(tr("Show Licenses"), main);
        grid->addWidget(licensesButton, row, 0, 1, 2);
        connect(licensesButton, SIGNAL(clicked()), this, SLOT(showLicenses()));
    }
}

/*!
  Destroys the widget.
 */
QDocumentPropertiesWidget::~QDocumentPropertiesWidget()
{
    delete d;
}

/*!
  Applies any changes made on the QDocumentPropertiesWidget 
 */
void QDocumentPropertiesWidget::applyChanges()
{
    bool changed=false;
    bool isDocument = lnk.isDocument();
    if ( isDocument && lnk.name() != d->docname->text() ) {
        lnk.setName(d->docname->text());
        changed=true;
    }
    if ( d->categoryEdit ) {
        QList<QString> tmp = d->categoryEdit->selectedCategories();
        if ( lnk.categories() != tmp ) {
            lnk.setCategories( tmp );
            changed = true;
        }
    }
    if ( !d->fastLoad && d->locationCombo && d->locationCombo->isChanged() ) {
        moveLnk();
    } else if ( changed ) {
        lnk.commit();
    }

    if ( d->fastLoad ) {
        QSettings cfg("Trolltech","Launcher");
        cfg.beginGroup("AppLoading");
        QStringList apps = cfg.value("PreloadApps").toString().split(',');
        QString exe = lnk.executableName();
        if ( (apps.contains(exe) > 0) != d->fastLoad->isChecked() ) {
            if ( d->fastLoad->isChecked() ) {
                apps.append(exe);
                QtopiaIpcEnvelope e("QPE/Application/"+exe,
                               "enablePreload()");
            } else {
                apps.removeAll(exe);
                QtopiaIpcEnvelope("QPE/Application/"+exe,
                               "disablePreload()");
                QtopiaIpcEnvelope("QPE/Application/"+exe,
                               "quitIfInvisible()");
            }
            cfg.setValue("Apps", apps.join(QString(',')));
        }
    }
}

// QTOPIA_DOCAPI_TODO: does this need to be moved to QContent?
/*!
  Create a duplicate of the document in the document system.
 */
void QDocumentPropertiesWidget::duplicateLnk()
{
    // The duplicate takes the new properties.
    if( !lnk.copyTo( safePath( d->docname->text(), d->locationCombo->documentPath(), lnk.type(), lnk.fileName() ) ) )
    {
        QMessageBox::warning( this, tr("Duplicate"), tr("<qt>File copy failed.</qt>") );
        return;
    }
    emit done();
}

/*!
    \internal
*/
QString QDocumentPropertiesWidget::safePath( const QString &name, const QString &location, const QString &type, const QString &oldPath ) const
{
    static const char SAFE_SPACE = '_';

    QString safename;
    // Remove all special ASCII characters and ensure that name does not start with a number
    QByteArray ascii = name.toAscii();
    for ( int i = 0; i < ascii.length(); i++ ){
        QChar c = ascii.at(i);
        if ( c.isLetterOrNumber() )
            safename += c;
        else if ( c.isSpace() )
            safename += SAFE_SPACE;
    }
    if ( safename.isEmpty() )
        safename = SAFE_SPACE;
    else if ( safename.at(0).isNumber() )
        safename.prepend( SAFE_SPACE );

    int pos = oldPath.lastIndexOf( '/' );

    pos = oldPath.indexOf( '.', pos != -1 ? pos : 0 );
    QString fileExtn;

    if ( pos > 0 )
        fileExtn = oldPath.mid( pos );

    QString dir = location + type;

    QDir root;

    if( !root.exists( dir ) )
        root.mkpath( dir );

    QString fn = location + type + "/" + safename;
    if (QFile::exists(fn + fileExtn)) {
        int n=1;
        QString nn = fn + "_" + QString::number(n);
        while (QFile::exists(nn+fileExtn)) {
            n++;
            nn = fn + "_" + QString::number(n);
        }
        fn = nn;
    }

    return fn + fileExtn;
}

// QTOPIA_DOCAPI_TODO: does this need to be moved to QContent?
/*!
  \internal
 */
bool QDocumentPropertiesWidget::moveLnk()
{
    if( !lnk.moveTo( safePath( d->docname->text(), d->locationCombo->documentPath(), lnk.type(), lnk.fileName() ) ) )
    {
        QMessageBox::warning( this, tr("Details"), tr("<qt>Moving Document failed.</qt>") );
        return false;
    }
    else
        return true;
}

/*!
  Beams the document. 
 */
void QDocumentPropertiesWidget::beamLnk()
{
    QtopiaSendVia::sendFile(this, lnk);
    emit done();
}

// QTOPIA_DOCAPI_TODO: does this need to be moved to QContent?
/*!
  Deletes the document.  
 */
void QDocumentPropertiesWidget::unlinkLnk()
{
    if ( Qtopia::confirmDelete( this, tr("Delete"), lnk.name() ) ) {
        lnk.removeFiles();
        if ( QFile::exists(lnk.fileName()) ) {
            QMessageBox::warning( this, tr("Delete"), tr("<qt>File deletion failed.</qt>") );
        } else {
            emit done();
        }
    }
}

/*!
  Show licenses in a new QDialog.
*/
void QDocumentPropertiesWidget::showLicenses()
{
    QDialog *licensesDialog = new QDialog(this);
    licensesDialog->setWindowTitle(tr("Licenses"));
    licensesDialog->setModal(true);
    licensesDialog->showMaximized();

    QVBoxLayout *layout = new QVBoxLayout(licensesDialog);

    QWidget *main = new QWidget;

    QScrollArea *sa = new QScrollArea(licensesDialog);
    sa->setFrameStyle(QFrame::NoFrame);
    sa->setWidget(main);
    sa->setWidgetResizable(true);
    layout->addWidget(sa);

    QGridLayout *grid = new QGridLayout(main);
    main->setLayout(grid);
    grid->setMargin(6);
    grid->setSpacing(6);
    grid->setColumnStretch(1, 1);

    int row = 0;
    bool hasLicense = false;

    if( d->doc->permissions() & QDrmRights::Play )
    {
        addRights( d->doc->rights( QDrmRights::Play ), grid, main, &row );
        hasLicense = true;
    }
    if( d->doc->permissions() & QDrmRights::Display )
    {
        addRights( d->doc->rights( QDrmRights::Display ), grid, main, &row );
        hasLicense = true;
    }
    if( d->doc->permissions() & QDrmRights::Execute )
    {
        addRights( d->doc->rights( QDrmRights::Execute ), grid, main, &row );
        hasLicense = true;
    }

    if( d->doc->permissions() & QDrmRights::Print )
    {
        addRights( d->doc->rights( QDrmRights::Print ), grid, main, &row );
        hasLicense = true;
    }

    if( d->doc->permissions() & QDrmRights::Export )
    {
        addRights( d->doc->rights( QDrmRights::Export ), grid, main, &row );
        hasLicense = true;
    }
    if( !hasLicense )
    {
        QLabel *licenses = new QLabel( tr( "<qt><u>No licenses</u></qt>" ), main );
        grid->addWidget( licenses, row++, 0, Qt::AlignLeft | Qt::AlignTop );
    }
}

void QDocumentPropertiesWidget::addRights( const QDrmRights &rights, QGridLayout *layout, QWidget *parent, int *row )
{
    QLabel *permission = new QLabel( QString( "<qt><u>%1</u></qt>" )
            .arg( QDrmRights::toString( rights.permission(), rights.status() ) ), parent );

    layout->addWidget( permission,
            (*row)++, 0, 1, 2, Qt::AlignLeft | Qt::AlignTop );

    foreach( QDrmRights::Constraint c, rights.constraints() )
    {
        layout->addWidget( new QLabel( c.name(), parent ), *row, 0, Qt::AlignLeft | Qt::AlignTop );

        QLabel *constraint = new QLabel( c.value().toString(), parent );
        constraint->setWordWrap(false);
        layout->addWidget( constraint, (*row)++, 1, Qt::AlignLeft | Qt::AlignTop );

        for( int i = 0; i < c.attributeCount(); i++ )
        {
            layout->addWidget( new QLabel( QString( "<qt><em>%1</em></qt>" ).arg( c.attributeName( i ) ), parent ),
                               *row, 0, Qt::AlignLeft | Qt::AlignTop );
            QLabel *attribute = new QLabel( c.attributeValue( i ).toString(), parent );
            attribute->setWordWrap( true );
            layout->addWidget( attribute, (*row)++, 1, Qt::AlignLeft | Qt::AlignTop );
        }
    }
}

/*!
  \fn void QDocumentPropertiesWidget::done()

  This signal is emitted when a file is deleted, duplicated or beamed from the
  QDocumentPropertiesWidget.
 */

/*!
  \class QDocumentPropertiesDialog
  \ingroup content
  \mainclass
  \brief The QDocumentPropertiesDialog class allows the user to examine and 
         modify properties of a document. 

  The QDocumentPropertiesDialog is a convenience class which is built around 
  a \l QDocumentPropertiesWidget, which provides controls for modifying properties 
  associated with a document via it's associated \l QContent.
  
  \sa QDocumentPropertiesWidget
*/


/*!
  Constructs a QDocumentPropertiesDialog with a QContent, \a doc, representing the 
  document, and a \a parent widget.   

  Ensure that \a doc refers to a document as opposed to an application. 
 */
QDocumentPropertiesDialog::QDocumentPropertiesDialog( const QContent &doc, QWidget* parent )
    : QDialog( parent )
{
    setModal(true);
    setWindowTitle( tr("Properties") );
    setObjectName("properties");

    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin(0);

    QScrollArea *scrollArea = new QScrollArea( this );
    scrollArea->setFrameStyle( QFrame::NoFrame );
    scrollArea->setWidgetResizable( true );
    scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    vbox->addWidget( scrollArea );
    d = new QDocumentPropertiesWidget( doc, this );
    scrollArea->setWidget( d );
    scrollArea->setFocusProxy( d );
    connect( d, SIGNAL(done()), this, SLOT(reject()) );
}

/*!
  Destroys the dialog. 
 */
QDocumentPropertiesDialog::~QDocumentPropertiesDialog()
{
}

/*!
  \reimp
 */
void QDocumentPropertiesDialog::done(int ok)
{
    if ( ok )
        d->applyChanges();
    QDialog::done( ok );
}

