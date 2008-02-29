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

#include "archiveviewer.h"
#include <qdocumentproperties.h>
#include <qtopiaapplication.h>
#include <QSoftMenuBar>
#include <QtDebug>
#include <QAction>
#include <QMenu>
#include <QHideEvent>
#include <QShowEvent>
#include "applicationlauncher.h"

/*!
    \class ArchiveViewer
    Archives displays the contents of an archive file format.
*/

/*!
    Constructs a new archive viewer with parent \a parent and flags \a flags.
*/
ArchiveViewer::ArchiveViewer( QWidget* parent, Qt::WFlags flags )
    : LauncherView( parent, flags )
    , propDlg( 0 )
{
    setObjectName( "archives" );

    QMenu *softMenu = QSoftMenuBar::menuFor( this );

    setFocusPolicy( Qt::StrongFocus );

    propertiesAction = new QAction( QIcon(":icon/info"), tr("Properties..."), this );

    QObject::connect( propertiesAction, SIGNAL(triggered()), this, SLOT(showProperties()) );
    QObject::connect( this, SIGNAL(clicked(const QContent&)), this, SLOT(executeContent(const QContent&)) );

    softMenu->addAction( propertiesAction );
}

/*!
    Displays all files in the directory \a document.
*/
void ArchiveViewer::setDocument( const QString &document )
{
    QContentSet::scan( document );

    QContent content( document );

    setWindowTitle( content.name() );
    setFilter( QContentFilter( QContentFilter::Location, document ) );
}

void ArchiveViewer::executeContent( const QContent &content )
{
    if( content.type() == "application/vnd.oma.drm.dcf" )
    {
        filterStack.push( contentSet->filter() );

        setFilter( QContentFilter( QContentFilter::Location, content.file() ) );
    }
    else if( !content.executableName().isEmpty() )
    {
        content.execute();
    }
    else
    {
        if( !warningBox )
        {
#ifdef QTOPIA_PHONE
            warningBox = QAbstractMessageBox::messageBox(
                    this,
                    tr("No application"),
                    tr("<qt>No application is defined for this document.</qt>"),
                    QAbstractMessageBox::Information );
#else
            warningBox = new QMessageBox(
                    tr("No application"),
                    tr("<qt>No application is defined for this document.</qt>"),
                    QMessageBox::NoIcon
                            QMessageBox::Ok,
                    QMessageBox::NoButton,
                    QMessageBox::NoButton,
                    this,);
#endif
            warningBox->setAttribute(Qt::WA_DeleteOnClose); // It's a QPointer<> so safe.
        }

        QtopiaApplication::showDialog( warningBox );
    }
}

void ArchiveViewer::showProperties()
{
    const QContent &doc = currentItem();
    if (doc.id() != QContent::InvalidId && doc.isValid()) {
        propLnk = doc;
        if (propDlg)
            delete propDlg;
        propDlg = new QDocumentPropertiesDialog(propLnk, this);
        propDlg->setObjectName("document-properties");
        QtopiaApplication::showDialog(propDlg);
        propDlg->setWindowState(propDlg->windowState() | Qt::WindowMaximized);
    }
}

/*!
    \reimp
 */
void ArchiveViewer::showEvent( QShowEvent *event )
{
    if( !contentSet->filter().isValid() )
        setFilter(
                QContentFilter( QContent::Document ) &
                QContentFilter( QContentFilter::MimeType, "application/vnd.oma.drm.dcf" ) );

    LauncherView::showEvent( event );
}

#ifdef QTOPIA_KEYPAD_NAVIGATION
/*!
    \reimp
*/
void ArchiveViewer::keyPressEvent( QKeyEvent *event )
{
    if( event->key() == Qt::Key_Back && !filterStack.isEmpty() )
    {
        setFilter( filterStack.top() );

        filterStack.pop();

        event->accept();
    }
    else
        LauncherView::keyPressEvent( event );
}
#endif

/*!
    \reimp
*/
void ArchiveViewer::hideEvent( QHideEvent *event )
{
    setFilter( QContentFilter() );

    LauncherView::hideEvent( event );
}

static QWidget *archives()
{
    return new ArchiveViewer;
}
QTOPIA_SIMPLE_BUILTIN(archives,archives);

