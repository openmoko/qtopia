/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "documentview.h"
#include <QtopiaSendVia>
#include <QDialog>
#include <QSoftMenuBar>
#include <QDocumentPropertiesWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMimeType>
#include <QDrmContent>
#include <QtopiaApplication>
#include <QMenu>
#include <QLabel>
#include <QTimer>
#include <QCategoryDialog>
#include <QProgressBar>
#include <QValueSpaceItem>
#include "qabstractmessagebox.h"
#include <qtopiaservices.h>
#include <QContentFilterDialog>

////////////////////////////////////////////////////////////////
//
// DocumentLauncherView implementation

class MyDocPropertiesDialog : public QDialog
{
    Q_OBJECT
    public:
        MyDocPropertiesDialog( const QContent &lnk, QWidget* parent = 0 );
        ~MyDocPropertiesDialog();

        void done(int);

    private:
        QDocumentPropertiesWidget *d;
};

MyDocPropertiesDialog::MyDocPropertiesDialog( const QContent &l, QWidget* parent )
    : QDialog( parent )
{
    setWindowTitle( tr("Properties") );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    d = new QDocumentPropertiesWidget( l, this );
    vbox->addWidget( d );
    connect( d, SIGNAL(done()), this, SLOT(reject()) );
}

MyDocPropertiesDialog::~MyDocPropertiesDialog()
{
}

void MyDocPropertiesDialog::done(int ok)
{
    if ( ok )
        d->applyChanges();
    QDialog::done( ok );
}

class QLabeledProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    QLabeledProgressBar(QWidget *parent=0) : QProgressBar(parent) {};
    virtual ~QLabeledProgressBar() {};
    virtual QString text () const {return labelText;};
    virtual void setText(const QString &label) {labelText=label;};
private:
    QString labelText;
};

////////////////////////////////////////////////////////////////
//
// DocumentLauncherView implementation

DocumentLauncherView::DocumentLauncherView(QWidget* parent, Qt::WFlags fl)
    : LauncherView(parent, fl), typeLbl(0), actionDelete(0), actionProps(0),
    deleteMsg(0), propDlg(0), rightMenu(0), actionBeam(0), actionRightsIssuer(0),
    typeDlg(0), categoryLbl(0), categoryDlg(0)
{
    QMenu * softMenu = QSoftMenuBar::menuFor(this);
    rightMenu = new QMenu(this);

    actionProps = new QAction( QIcon(":icon/info"), tr("Properties..."), this );
    QObject::connect(actionProps, SIGNAL(triggered()),
                     this, SLOT(propertiesDoc()));
    softMenu->addAction(actionProps);
    rightMenu->addAction(actionProps);

    if (QtopiaSendVia::isFileSupported()) {
        actionBeam = new QAction( QIcon(":icon/beam"), tr("Send"), this );
        QObject::connect(actionBeam, SIGNAL(triggered()), this, SLOT(beamDoc()));
        softMenu->addAction(actionBeam);
        rightMenu->addAction(actionBeam);
    }

    actionDelete = new QAction( QIcon(":icon/trash"), tr("Delete..."), this );
    connect(actionDelete, SIGNAL(triggered()), this, SLOT(deleteDoc()));
    softMenu->addAction(actionDelete);
    rightMenu->addAction(actionDelete);

    actionRightsIssuer = new QAction( QIcon( ":image/drm/Drm" ), "Get license", this );
    QObject::connect(actionRightsIssuer, SIGNAL(triggered()),
                     this, SLOT(openRightsIssuerURL()) );
    actionRightsIssuer->setVisible( false );
    softMenu->addAction(actionRightsIssuer);
    rightMenu->addAction(actionRightsIssuer);

    separatorAction = softMenu->addSeparator();

    QAction *a = new QAction( tr("View Type..."), this );
    connect(a, SIGNAL(triggered()), this, SLOT(selectDocsType()));
    softMenu->addAction(a);

    a = new QAction( QIcon(":icon/viewcategory"), tr("View Category..."), this );
    connect(a, SIGNAL(triggered()), this, SLOT(selectDocsCategory()));
    softMenu->addAction(a);

    typeLbl = new QLabel(this);
    layout()->addWidget(typeLbl);
    typeLbl->hide();

    categoryLbl = new QLabel(this);
    layout()->addWidget(categoryLbl);
    categoryLbl->hide();

    scanningBar = new QLabeledProgressBar(this);
    layout()->addWidget(scanningBar);
    scanningBar->setText(tr("Scanning", "Scanner is searching for documents"));
    scanningBar->setMinimum(0);
    scanningBar->setMaximum(10);
    scanningBarUpdateTimer = new QTimer(this);
    scanningBarUpdateTimer->setInterval(1500);
    scanningBarUpdateTimer->setSingleShot(false);
    scanningVSItem=new QValueSpaceItem("/Documents/Scanning", this);
    updatingVSItem=new QValueSpaceItem("/Documents/Updating", this);
    installingVSItem=new QValueSpaceItem("/Documents/Installing", this);
    connect(scanningVSItem, SIGNAL(contentsChanged()), this, SLOT(updateScanningStatus()));
    connect(updatingVSItem, SIGNAL(contentsChanged()), this, SLOT(updateScanningStatus()));
    connect(installingVSItem, SIGNAL(contentsChanged()), this, SLOT(updateScanningStatus()));
    connect(scanningBarUpdateTimer, SIGNAL(timeout()), this, SLOT(updateScanningStatus()));
    updateScanningStatus();

    connect(this, SIGNAL(rightPressed(QContent)),
            this, SLOT(launcherRightPressed(QContent)));

    connect( icons, SIGNAL(currentIndexChanged(const QModelIndex&,const QModelIndex&)),
             this, SLOT(currentChanged(const QModelIndex&,const QModelIndex&)) );

    QContentFilter filter( QContent::Document );

    setFilter(filter);
    setViewMode(QListView::ListMode);
    contentSetChanged();
    connect(contentSet, SIGNAL(changed()), this, SLOT(contentSetChanged()));
    connect(contentSet, SIGNAL(changed(const QContentIdList &, QContent::ChangeType)), this, SLOT(contentSetChanged()));
}

DocumentLauncherView::~DocumentLauncherView()
{
}

void DocumentLauncherView::setFilter( const QContentFilter &filter )
{
    LauncherView::setFilter( filter );

    if( typeDlg )
        typeDlg->setFilter( filter );

    if( categoryDlg )
        categoryDlg->setFilter( filter );
}

void DocumentLauncherView::launcherRightPressed(QContent lnk)
{
    if(lnk.id() != QContent::InvalidId && lnk.isValid())
        rightMenu->popup(QCursor::pos());
}

void DocumentLauncherView::beamDoc()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        QtopiaSendVia::sendFile(0, doc);
    }
}

void DocumentLauncherView::deleteDocWorker()
{
    deleteLnk.removeFiles();
    if (QFile::exists(deleteLnk.file())) {
        if(deleteMsg)
            delete deleteMsg;
        deleteMsg = QAbstractMessageBox::messageBox( this, tr("Delete"),
                "<qt>" + tr("File deletion failed.") + "</qt>",
                QAbstractMessageBox::Warning, QAbstractMessageBox::Ok );
        QtopiaApplication::showDialog(deleteMsg);
    }
}

void DocumentLauncherView::deleteDoc(int r)
{
    if (r == QAbstractMessageBox::Yes) {
        // We can't delete the deleteMsg object directly in the deleteDoc(int) function
        // because it is in response to the done() signal emitted by the deleteMsg object
        // which is still in use. This happens when trying to delete a read only file.
        QTimer::singleShot(10,this,SLOT(deleteDocWorker()));
    }
}

void DocumentLauncherView::deleteDoc()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        deleteLnk = doc;
        if(deleteMsg)
            delete deleteMsg;
        deleteMsg = QAbstractMessageBox::messageBox( this, tr("Delete"),
                "<qt>" + tr("Are you sure you want to delete %1?").arg(deleteLnk.name()) + "</qt>",
                QAbstractMessageBox::Warning, QAbstractMessageBox::Yes, QAbstractMessageBox::No );
        connect(deleteMsg, SIGNAL(finished(int)), this, SLOT(deleteDoc(int)));
        QtopiaApplication::showDialog(deleteMsg);
    }
}

void DocumentLauncherView::propertiesDoc()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        propLnk = doc;
        if (propDlg)
            delete propDlg;
        propDlg = new MyDocPropertiesDialog(propLnk, this);
        propDlg->setObjectName("document-properties");
        QtopiaApplication::showDialog(propDlg);
        propDlg->setWindowState(propDlg->windowState() | Qt::WindowMaximized);
    }
}

void DocumentLauncherView::openRightsIssuerURL()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid())
        QDrmContent::activate( doc );
}

void DocumentLauncherView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    QContent currentContent = model->content( current );
    QContent previousContent = model->content( previous );

    bool pDistribute = previousContent.permissions() & QDrmRights::Distribute;
    bool cDistribute = currentContent.permissions() & QDrmRights::Distribute;

    bool pActivate = QDrmContent::canActivate( previousContent );
    bool cActivate = QDrmContent::canActivate( currentContent );

    if( actionBeam && pDistribute != cDistribute )
        actionBeam->setEnabled( cDistribute );
    if( actionRightsIssuer && pActivate != cActivate )
        actionRightsIssuer->setVisible( cActivate );
}

void DocumentLauncherView::selectDocsType()
{
    if( !typeDlg )
    {
        QContentFilterModel::Template subTypePage(
                QContentFilter::MimeType,
                QString(),
                QContentFilterModel::CheckList );

        QContentFilterModel::Template typePage;

        typePage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        typePage.addLabel( subTypePage, tr( "Audio" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "audio/*" ) ) );
        typePage.addLabel( subTypePage, tr( "Image" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "image/*" ) ) );
        typePage.addLabel( subTypePage, tr( "Text"  ), QContentFilter( QContentFilter::MimeType, QLatin1String( "text/*"  ) ) );
        typePage.addLabel( subTypePage, tr( "Video" ), QContentFilter( QContentFilter::MimeType, QLatin1String( "video/*" ) ) );
        typePage.addList( ~( QContentFilter( QContentFilter::MimeType, QLatin1String( "audio/*" ) )
                           | QContentFilter( QContentFilter::MimeType, QLatin1String( "image/*" ) )
                           | QContentFilter( QContentFilter::MimeType, QLatin1String( "text/*"  ) )
                           | QContentFilter( QContentFilter::MimeType, QLatin1String( "video/*" ) ) ),
                          QContentFilter::MimeType );

        typeDlg = new QContentFilterDialog( typePage, this );

        typeDlg->setFilter( mainFilter );
    }

    QtopiaApplication::execDialog( typeDlg );

    showType( typeDlg->checkedFilter() );

    QString label = typeDlg->checkedLabel();

    if( !typeFilter.isValid() || label.isEmpty() )
        typeLbl->hide();
    else
    {
        typeLbl->setText( tr("Type: %1").arg( label ) );
        typeLbl->show();
    }
}

void DocumentLauncherView::selectDocsCategory()
{
    if( !categoryDlg )
    {
        QContentFilterModel::Template categoryPage;

        categoryPage.setOptions( QContentFilterModel::CheckList | QContentFilterModel::SelectAll );

        categoryPage.addList( QContentFilter::Category );
        categoryPage.addList( QContentFilter::Category, QLatin1String( "Documents" ) );

        categoryDlg = new QContentFilterDialog( categoryPage, this );

        categoryDlg->setFilter( mainFilter );
    }

    QtopiaApplication::execDialog( categoryDlg );

    showCategory( categoryDlg->checkedFilter() );

    QString label = categoryDlg->checkedLabel();

    if( !categoryFilter.isValid() || label.isEmpty() )
        categoryLbl->hide();
    else
    {
        categoryLbl->setText( tr("Category: %1").arg( label ) );
        categoryLbl->show();
    }
}

void DocumentLauncherView::contentSetChanged()
{
    if (contentSet->count()) {
        if (!actionProps->isEnabled())
            actionProps->setEnabled(true);
        if (!actionDelete->isEnabled())
            actionDelete->setEnabled(true);
        if (actionBeam && !actionBeam->isEnabled())
            actionBeam->setEnabled(true);
        separatorAction->setEnabled(true);
        if (!actionRightsIssuer->isEnabled())
            actionRightsIssuer->setEnabled(true);
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Select);
    }
    else {
        actionProps->setEnabled(false);
        actionDelete->setEnabled(false);
        if (actionBeam)
            actionBeam->setEnabled(false);
        separatorAction->setEnabled(false);
        actionRightsIssuer->setEnabled(false);
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    }
}

void DocumentLauncherView::updateScanningStatus()
{
    if(sender() == scanningBarUpdateTimer)
    {
        if((scanningVSItem && scanningVSItem->value().toBool() == true)
        || (updatingVSItem && updatingVSItem->value().toBool() == true)
        || (installingVSItem && installingVSItem->value().toBool() == true))
        {
            scanningBar->show();
            if(!scanningBarUpdateTimer->isActive())
                QMetaObject::invokeMethod(scanningBarUpdateTimer, "start", Qt::AutoConnection);
            // update the progress. todo: when doing invert, also change the colours over too.
            if(scanningBar->value() == scanningBar->maximum())
            {
                scanningBar->setInvertedAppearance(!scanningBar->invertedAppearance());
                scanningBar->setValue(scanningBar->minimum());
            }
            else
                scanningBar->setValue(scanningBar->value()+1);
        }
        else
        {
            if(scanningBarUpdateTimer->isActive())
                QMetaObject::invokeMethod(scanningBarUpdateTimer, "stop", Qt::AutoConnection);
            scanningBar->hide();
        }
    }
    else
    {
        if(!scanningBarUpdateTimer->isActive())
            QMetaObject::invokeMethod(scanningBarUpdateTimer, "start", Qt::AutoConnection);
    }
}

#include "documentview.moc"
