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

#include "simicons.h"
#include <QWaitWidget>
#include <QFile>
#include <qtopianamespace.h>

SimIcons::SimIcons( QSimIconReader *reader, QWidget *parent )
    : QObject( parent )
{
    this->reader = reader;
    this->parent = parent;
    this->waitWidget = 0;

    if ( reader ) {
        connect( reader, SIGNAL(iconAvailable(int)), this, SLOT(iconDone(int)) );
        connect( reader, SIGNAL(iconNotFound(int)), this, SLOT(iconDone(int)) );
    }
}

SimIcons::~SimIcons()
{
    QMap<int, QString>::ConstIterator it;
    for ( it = files.begin(); it != files.end(); ++it )
        QFile::remove( it.value() );
    if ( waitWidget )
        delete waitWidget;
}

QIcon SimIcons::icon( int iconId )
{
    if ( !reader || !reader->haveIcon( iconId ) )
        return QIcon();

    QImage image = reader->icon( iconId );

    if ( image.format() == QImage::Format_Mono ) {
        // Use the monochrome image as a mask to create a transparent icon
        // in the current text color.
        QPixmap pixmap( image.size() );
        pixmap.fill( parent->palette().color(QPalette::Text) );
        pixmap.setAlphaChannel( QPixmap::fromImage( image ) );
        return QIcon( pixmap );
    } else {
        return QIcon( QPixmap::fromImage( image ) );
    }
}

// Get the filename associated with a loaded icon.
QString SimIcons::iconFile( int iconId )
{
    if ( files.contains( iconId ) )
        return files[iconId];
    else
        return QString();
}

// Add an icon to the list of icons that we need for the current command.
void SimIcons::needIcon( int iconId )
{
    if ( iconId != 0 && reader != 0 &&
         !reader->haveIcon( iconId ) &&
         !pendingIcons.contains( iconId ) &&
         !loadedIcons.contains( iconId ) ) {
        pendingIcons.append( iconId );
    }
}

// Add an icon to the list of icons that we need for the current command,
// and arrange for it to be written to an external file in /tmp.
void SimIcons::needIconInFile( int iconId )
{
    if ( iconId != 0 && reader != 0 ) {
        if ( !reader->haveIcon( iconId ) &&
            !pendingIcons.contains( iconId ) &&
            !loadedIcons.contains( iconId ) ) {
            pendingIcons.append( iconId );
        }
        fileIcons.append( iconId );
    }
}

// Request all pending icons, and emit iconsReady() once they are all loaded.
// Show the wait widget while the icons are being loaded.
void SimIcons::requestIcons()
{
    // If there are no pending icons, then emit "iconsReady()" immediately.
    if ( pendingIcons.isEmpty() ) {
        copyIconsToFiles();
        emit iconsReady();
        return;
    }

    // Show the wait widget while we fetch the icons.
    if ( !waitWidget )
        waitWidget = new QWaitWidget( parent );
    waitWidget->show();

    // Request the icons that we are missing.
    foreach ( int icon, pendingIcons )
        reader->requestIcon( icon );
}

void SimIcons::iconDone( int iconId )
{
    // We have finished loading one of the requested icons, or it
    // could not be found.  Add to the loaded list, remove from the
    // pending list, and then emit iconsReady() if all are now done.
    loadedIcons.append( iconId );
    pendingIcons.removeAll( iconId );
    if ( pendingIcons.isEmpty() ) {
        if ( waitWidget )
            waitWidget->hide();
        if ( !fileIcons.isEmpty() )
            copyIconsToFiles();
        emit iconsReady();
    }
}

void SimIcons::copyIconsToFiles()
{
    foreach ( int iconId, fileIcons ) {
        if ( files.contains( iconId ) )
            continue;
        QString filename = Qtopia::tempDir() + "simicon" + QString::number(iconId) + ".png";
        QImage image = reader->icon( iconId );
        if ( !image.isNull() && image.save( filename ) )
            files.insert( iconId, filename );
    }
    fileIcons.clear();
}
