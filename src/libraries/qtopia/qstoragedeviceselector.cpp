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

#include "qstoragedeviceselector.h"

#include <qcontent.h>
#include <qcategoryselector.h>
#include <qtopiaipcenvelope.h>
#include <qstorage.h>
#include <qtopianamespace.h>

#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfile.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qsize.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <stdlib.h>

/*!
  \class QStorageDeviceSelector
  \brief The QStorageDeviceSelector class displays a list of available storage
         locations.

  \ingroup qtopiaemb
  \sa QDocumentPropertiesDialog
 */


class QStorageDeviceSelectorPrivate
{
public:
    QStorageDeviceSelectorPrivate() : fileSize(0), listEmpty(true) {}
    QString originalPath;
    int fileSize;
    bool listEmpty;
};

/*!
  Constructs a QStorageDeviceSelector with parent \a parent.
 */
QStorageDeviceSelector::QStorageDeviceSelector( QWidget *parent )
    : QComboBox( parent ), filter( 0 )
{
    storage = new QStorageMetaInfo;
    d = new QStorageDeviceSelectorPrivate;
    setLocation( QContent() );
    connect( this, SIGNAL(activated(int)), this, SIGNAL(newPath()) );
    connect( storage, SIGNAL(disksChanged()), this, SLOT(updatePaths()) );
}

/*!
  Constructs a QStorageDeviceSelector with parent \a parent.
  \a lnk is pointer to an existing QContent.
 */
QStorageDeviceSelector::QStorageDeviceSelector( const QContent & lnk, QWidget *parent )
    : QComboBox( parent ), filter( 0 )
{
    storage = new QStorageMetaInfo;
    d = new QStorageDeviceSelectorPrivate;
    setLocation(lnk);
    connect( this, SIGNAL(activated(int)), this, SIGNAL(newPath()) );
    connect( storage, SIGNAL(disksChanged()), this, SLOT(updatePaths()) );
}

/*!
  Destroys the widget.
 */
QStorageDeviceSelector::~QStorageDeviceSelector()
{
    if ( filter ) {
        delete filter;
        filter = 0;
    }
    delete storage;
    delete d;
}

/*!
  Sets the display of the QStorageDeviceSelector to the location associated with the
  QContent \a lnk.
 */
void QStorageDeviceSelector::setLocation( const QContent &lnk )
{
    // NB: setLocation(const QString) assumes only lnk->file() is used.

    if ( lnk.isValid() ) {
        QFileInfo fi( lnk.file() );
        d->fileSize = fi.size();
        const QFileSystem *fs =  storage->fileSystemOf( lnk.file() );
        d->originalPath = fs ? fs->path() : QString();
    } else {
        d->fileSize = 0;
        d->originalPath = QString();
    }

    setupCombo();

    int currentLocation = -1;
    if ( lnk.isValid()) {
        int n = locations.count();
        for ( int i = 0; i < n; i++ ) {
            if ( lnk.file().contains( locations[i] ) )
                currentLocation = i;
        }
    }
    if ( currentLocation == -1 )
        currentLocation = 0; //default to the first one

    setCurrentIndex( currentLocation );
}

/*!
  \internal
*/
void QStorageDeviceSelector::setupCombo()
{
    clear();
    locations.clear();

    QString s;
    int index = 0;
    foreach ( QFileSystem *fs, storage->fileSystems(filter) ) {
        // we add 10k to the file size so we are sure we can also save the desktop file
        if ( (!d->fileSize || fs->path() == d->originalPath || fs->availBlocks() * fs->blockSize() > d->fileSize + 10000 )
                      && fs->documents())
        {
            insertItem( index, fs->name() );
            locations.append( fs->path() );
            index++;
        }
    }

    d->listEmpty = locations.count() == 0;
    if ( d->listEmpty ) {
        insertItem( 0, tr("No filesystem available!") );
        locations.append( "" );
    }
}

/*!
  \internal
 */
void QStorageDeviceSelector::updatePaths()
{
    QString oldPath = locations[currentIndex()];

    setupCombo();

    int currentLocation = 0;
    int n = locations.count();
    for ( int i = 0; i < n; i++ ) {
        if ( oldPath == locations[i] ) {
            currentLocation = i;
        }
    }
    setCurrentIndex( currentLocation );
    if ( locations[currentIndex()] != oldPath )
        emit newPath();
}

/*!
  Returns true indicating the location displayed has been changed via
  the QStorageDeviceSelector.  It is most useful when the QStorageDeviceSelector is part of a
  dialog. That is, when the dialog is accepted with accept(), QStorageDeviceSelector::isChanged() can be
  examined to check for a change of location.
 */
bool QStorageDeviceSelector::isChanged() const
{
    if ( const QFileSystem *fs = storage->fileSystemOf(locations[currentIndex()]) )
        return fs->path() != d->originalPath;

    return true;
}

/*!
  Returns the default (home) location for the file associated with this
  QStorageDeviceSelector.
 */
QString QStorageDeviceSelector::installationPath() const
{
    return locations[ currentIndex() ]+"/";
}

/*!
  Returns the document path, \i{\<path\>/Documents} associated with this QStorageDeviceSelector.
 */
QString QStorageDeviceSelector::documentPath() const
{
    // FIXME Storage.conf should be made a little more flexible so that we don't have to have magic like this
    const QFileSystem *fs = storage->fileSystemOf(locations[currentIndex()]);
    if ( fs && fs->disk() == "HOME" )
        return locations[ currentIndex() ]+"/Documents/";
    else
        return locations[ currentIndex() ]+"/";
}

/*!
  Returns a pointer to QFileSystem object, associated with the current
  selection of the QStorageDeviceSelector.
 */
const QFileSystem *QStorageDeviceSelector::fileSystem() const
{
    if ( d->listEmpty )
        return 0;
    return storage->fileSystemOf( locations[ currentIndex() ] );
}

/*!
  \fn QStorageDeviceSelector::newPath()
  Emitted when the QStorageDeviceSelector changes to a new location.
 */


/*!
  QStorageDeviceSelector only shows those filesystems which pass the conditions of \a fsf.
  */
void QStorageDeviceSelector::setFilter( QFileSystemFilter *fsf )
{
    if ( filter )
        delete filter;
    filter = fsf;
    updatePaths();
}

/*!
  Sets the display of the QStorageDeviceSelector to the location containing
  \a path.
 */
void QStorageDeviceSelector::setLocation( const QString& path )
{
    QContent lnk;
    lnk.setFile(path); // Since that's all setLocation(AppLnk) uses...
    setLocation(lnk);
}

