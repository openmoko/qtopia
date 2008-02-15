/****************************************************************************
 **
 ** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "appviewer.h"
#include <qdocumentselector.h>
#include <qsoftmenubar.h>
#include <qtopianamespace.h>

#include <QFile>
#include <QTextStream>
#include <QMenu>
#include <QDebug>
#include <QKeyEvent>

/*
 *  Constructs a AppViewer which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
AppViewer::AppViewer( QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f )
{
    setWindowTitle( tr( "App Viewer" ));

    textArea = new QTextEdit(this);
    textArea->setReadOnly(true);
    appSelector = new QDocumentSelector();

    QAction *actionOpen = new QAction(tr("Open Document"), this );
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(openDocument()));
    QMenu* menu = QSoftMenuBar::menuFor(textArea);
    menu->addAction(actionOpen);

    connect( appSelector, SIGNAL(documentSelected(QContent)),
            this, SLOT(documentSelected(QContent)) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AppViewer::~AppViewer()
{
    delete appSelector;
    // no need to delete child widgets, Qt does it all for us
}

/*
 * Call the appSelector widget to allow the user to choose an application
 * to display information about
 */
void AppViewer::openApplicationInfo()
{
    QContentFilter appFilter = QContentFilter( QContent::Application );
    appSelector->setFilter( appFilter );
    appSelector->showMaximized();
}

/*
 * Respond to the documentSelected signal from the appSelector, by displaying
 * information about the selected application.
 */
void AppViewer::documentSelected( const QContent &appContent )
{
    appSelector->hide();
    if ( appContent.isValid() )
    {
        textArea->setHtml( getInformation( appContent ));
    }
    else
    {
        textArea->setHtml(
                tr( "<font color=\"#CC0000\">Could not find information about %1</font>" )
                .arg( appContent.name() ));
        qWarning() << "Application " << appContent.file() << " not found";
    }
}

/*
 * Find information about an Application, including what other installed applications
 * there are which have binaries bigger or smaller than this one.
 * Pre-requisite - the appContent is valid, ie has a backing file.
 */
QString AppViewer::getInformation( const QContent &appContent )
{
    QFileInfo fi( appContent.file() );
    QString info = tr( "Binary is: <b>%1</b><br>" ).arg( fi.fileName() );

    qint64 chosenAppSize = fi.size();
    info += tr( "Size is: <b>%1 bytes</b><br>" ).arg( chosenAppSize );

    enum Count { SMALL, LARGE };
    int qtopiaCounts[2] = { 0, 0 };
    int packageCounts[2] = { 0, 0 };
    int *currentCount;
    QStringList paths = Qtopia::installPaths();
    foreach ( QString p, paths )
    {
        QDir qDir( p + "bin" );
        qDebug( "Checking %s\n", qPrintable( qDir.path() ));
        if ( qDir.exists() )
        {
            QFileInfoList binaries = qDir.entryInfoList( QDir::Executable );
            currentCount = ( p == Qtopia::packagePath() ) ? packageCounts : qtopiaCounts;
            foreach ( QFileInfo f, binaries )
                if ( f.size() > chosenAppSize )
                    ++currentCount[LARGE];
                else
                    ++currentCount[SMALL];
        }
    }

    info += tr( ", bigger than <font color=\"#CC0000\">%1 percent</font> of Qtopia binaries" )
        .arg( (int)( (float)qtopiaCounts[SMALL] * 100.0 / (float)qtopiaCounts[LARGE] ));

    if ( packageCounts[SMALL] > 0 || packageCounts[LARGE] > 0 )
    {
        info += tr( ", and bigger than <font color=\"#CC0000\">%1 percent</font> of package binaries" )
            .arg( (int)( (float)packageCounts[SMALL] * 100.0 / (float)packageCounts[LARGE] ));
    }

    return info;
}
