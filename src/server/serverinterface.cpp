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
#include "serverinterface.h"
#include "server.h"
#include "documentlist.h"

#include <qtopia/qpeapplication.h>
#include <qwindowsystem_qws.h>
#include <qgfx_qws.h>

class ServerLayoutManager : public QObject
{
    Q_OBJECT
public:
    ServerLayoutManager();

    void addDocked( QWidget *w, ServerInterface::DockArea placement, const QSize &s );

private slots:
    void removeWidget();

private:
    struct Item {
	QWidget *w;
	ServerInterface::DockArea p;
	QSize fixed;
    };

    bool eventFilter( QObject *object, QEvent *event );
    void layout();
    Item *findWidget( const QWidget *w ) const;

    QList<Item> docked;
};

ServerLayoutManager::ServerLayoutManager()
{
    docked.setAutoDelete( TRUE );
    qApp->desktop()->installEventFilter( this );
}

void ServerLayoutManager::addDocked( QWidget *w, ServerInterface::DockArea placement, const QSize &s )
{
    Item *oldItem = findWidget(w);
    if (oldItem)
	docked.removeRef(oldItem);
    
    Item *i = new Item;
    i->w = w;
    i->p = placement;
    i->fixed = s;
    if (!oldItem) {
	w->installEventFilter( this );
	connect(w, SIGNAL(destroyed()), this, SLOT(removeWidget()));
    }
    docked.append(i);
    layout();
}

void ServerLayoutManager::removeWidget()
{
    QWidget *w = (QWidget*)sender();
    Item *item = findWidget(w);
    if ( item ) {
	docked.removeRef( item );
	layout();
    }
}

bool ServerLayoutManager::eventFilter( QObject *object, QEvent *event )
{
    if ( object == qApp->desktop() ) {
	if ( event->type() == QEvent::Resize )
	    layout();
	return QObject::eventFilter( object, event );
    }

    Item *item;

    switch ( event->type() ) {
	case QEvent::Hide:
	case QEvent::Show:
	    item = findWidget( (QWidget *)object );
	    if ( item )
		layout();
	    break;
	
	default:
	    break;
    }

    return QObject::eventFilter( object, event );
}

void ServerLayoutManager::layout()
{
    QRect mwr( qApp->desktop()->geometry() );
    QListIterator<Item> it( docked );
    Item *item;
    for ( ; (item = it.current()); ++it ) {
	QWidget *w = item->w;
	if ( !w->isVisible() )
	    continue;
	QSize sh = w->sizeHint();
	QSize fs = item->fixed.isValid() ? item->fixed : sh;
	switch ( item->p ) {
	    case ServerInterface::Top:
		w->setGeometry(mwr.left(), mwr.top(),
		    mwr.width(), sh.height() );
		mwr.setTop( mwr.top() + fs.height() - 1 );
		break;
	    case ServerInterface::Bottom:
		w->setGeometry( mwr.left(), mwr.bottom()-sh.height()+1,
		    mwr.width(), sh.height() );
		mwr.setBottom( mwr.bottom()-fs.height() );
		break;
	    case ServerInterface::Left:
		w->setGeometry( mwr.left(), mwr.top(),
		    sh.width(), mwr.height() );
		mwr.setLeft( w->geometry().right() + 1 );
		break;
	    case ServerInterface::Right:
		w->setGeometry( mwr.right()-sh.width()+1, mwr.top(),
		    sh.width(), mwr.height() );
		mwr.setRight( w->geometry().left() - 1 );
		break;
	}
    }

#ifdef Q_WS_QWS
# if QT_VERSION < 0x030000
    QWSServer::setMaxWindowRect( qt_screen->mapToDevice(mwr,
	QSize(qt_screen->width(),qt_screen->height())) );
# else
    QWSServer::setMaxWindowRect( mwr );
# endif
#endif
}

ServerLayoutManager::Item *ServerLayoutManager::findWidget( const QWidget *w ) const
{
    QListIterator<Item> it( docked );
    Item *item;
    for ( ; (item = it.current()); ++it ) {
	if ( item->w == w )
	    return item;
    }

    return 0;
}

//---------------------------------------------------------------------------

/*! \class ServerInterface serverinterface.h
  \brief The ServerInterface class provides an interface for Qtopia
  custom launchers.

  The ServerInterface allows the user interface of the launcher to be
  customized to suit the device.  For a PDA style device, the default
  launcher is strongly recommended, however specialist devices may
  choose to provide a launcher better suited to the application.

  The launcher is fixed for any particular device and is not loaded
  as a plugin.  Launcher interfaces must
  be compilied into the server by editing server.pro and server.cpp.
*/

/*!
  \fn ServerInterface::createGUI()

  Implement this function to create the custom launcher UI.

  \sa showGUI()
*/

/*!
  \fn ServerInterface::showGUI()

  Implement this function to show the custom launcher UI for
  the first time. Will be called after createGUI().
*/

/*!
  \fn ServerInterface::destroyGUI()

  Implement this function to destroy the custom launcher UI.  All resources
  allocated by createGUI() must be released.
*/

/*!
  \enum ServerInterface::ApplicationState

  The ApplicationState enum type specifies the state of an application.
  The possible values are: <ul>

  <li> \c Launching - the application has been requested, but is not yet running.
  <li> \c Running - the application is running.
  <li> \c Terminated - the application has been terminated.
</ul>
*/

/*!
  \fn ServerInterface::applicationStateChanged(const QString& name, ApplicationState state)

  The application \a name has changed state to \a state.  This can be used
  to show launch notification and update a list of running applications.
*/

/*!
  \fn ServerInterface::typeAdded(const QString& type, const QString& name, const QPixmap& pixmap, const QPixmap& bgPixmap)

  An application category \a type has been added, for example "Games".
  \a name is the translated name of the category.  \a pixmap and
  \a bgPixmap are small and large icons for the new type.

  Types can be added or removed at any time, for example, when a storage
  card containing a non-standard category is inserted or removed.

  \sa typeRemoved()
*/

/*!
  \fn ServerInterface::typeRemoved(const QString& type)

  An application category \a type has been removed.

  Types can be added or removed at any time, for example, when a storage
  card containing a non-standard category is inserted or removed.

  \sa typeAdded()
*/

/*!
  \fn ServerInterface::applicationAdded(const QString& type, const AppLnk& app)

  Add an application \a app of type \a type to the launcher.

  \sa applicationRemoved()
*/

/*!
  \fn ServerInterface::applicationRemoved(const QString& type, const AppLnk& app)

  Remove an application \a app of type \a type from the launcher.

  \sa applicationAdded()
*/

/*!
  \fn ServerInterface::allApplicationsRemoved()

  Remove all applications from the launcher.
*/

/*!
  \fn const AppLnkSet &ServerInterface::appLnks()

  Returns the current set of available applications.
*/

/*!
  \fn ServerInterface::documentAdded(const DocLnk& doc)

  Add a document \a doc to the launcher.

  \sa documentRemoved()
*/

/*!
  \fn ServerInterface::documentRemoved(const DocLnk& doc)

  Remove a document \a doc to the launcher.

  \sa documentAdded()
*/

/*!
  \fn ServerInterface::documentChanged(const DocLnk& oldDoc, const DocLnk& newDoc)

  Change document properties of existing document \a oldDoc to \a newDoc.
*/

/*!
  \fn ServerInterface::allDocumentsRemoved()

  Remove all documents from the launcher.
*/

/*!
  \fn ServerInterface::storageChanged(const QList<FileSystem> &)

  A filesystem has been added or removed.
*/

/*!
  \fn ServerInterface::applicationScanningProgress(int percent)

  The system is scanning for installed applications.  \a percent
  provides the percentage of the filesystems scanned.  This function will
  always be called with \a percent equal to zero when scanning starts,
  and with \a percent equal to 100 when scanning is complete.
*/

/*!
  \fn ServerInterface::documentScanningProgress(int percent)

  The system is scanning for documents.  \a percent
  provides the percentage of the filesystems scanned.  This function will
  always be called with \a percent equal to zero when scanning starts,
  and with \a percent equal to 100 when scanning is complete.
*/

/*!
  \fn bool ServerInterface::requiresApplications() const

  Return TRUE if the custom launcher requires the server to scan for
  applications.
*/

/*!
  \fn bool ServerInterface::requiresDocuments() const

  Return TRUE if the custom launcher requires the server to scan for
  documents.
*/

/*!
  \enum ServerInterface::DockArea

  The DockArea enum type defines the areas where widgets can be docked:
  <ul>
  <li> \c Top - the top of the screen.
  <li> \c Bottom - the Bottom of the screen.
  <li> \c Left - the Left of the screen.
  <li> \c Right - the Right of the screen.
  </ul>
*/

/*!
  \fn ServerInterface::dockWidget(QWidget *w, DockArea placement)

  Docks a top-level widget \a w on a side of the screen specified by
  \a placement.  The widget is placed
  according to the order that it was docked, its sizeHint() and whether
  previously docked widgets are visible.  The desktop area available to
  QWidget::showMaximized() will exclude any visible docked widgets.
  
  For example, if a widget is
  docked at the bottom of the screen, its sizeHint() will define its
  height and it will use the full width of the screen.  If a widget is
  then docked to the right, its sizeHint() will define its width and
  it will be as high as possible without covering
  the widget docked at the bottom.

  This function is useful for reserving system areas such as taskbars
  and input methods that should not be covered by applications.
*/


ServerInterface::~ServerInterface()
{
}

void ServerInterface::dockWidget( QWidget *w, DockArea placement )
{
    dockWidget(w, placement, QSize());
}

void ServerInterface::dockWidget( QWidget *w, DockArea placement, const QSize &s )
{
    static ServerLayoutManager *lm = 0;

    if ( !lm )
	lm = new ServerLayoutManager;

    lm->addDocked( w, placement, s );
}

const AppLnkSet& ServerInterface::appLnks()
{
    return *DocumentList::appLnkSet;
}

#include "serverinterface.moc"

