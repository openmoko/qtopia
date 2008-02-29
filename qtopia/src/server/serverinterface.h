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
#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H


#include <qstring.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qtopia/applnk.h>
#include <qtopia/storage.h>


class ServerInterface {
public:
    virtual ~ServerInterface();

    virtual void createGUI() = 0;
    virtual void showGUI() = 0;
    virtual void destroyGUI() = 0;

    // These notify of the application categories, like what is used in the current launcher tabs,
    // for example "Applications", "Games" etc.
    virtual void typeAdded( const QString& type, const QString& name, const QPixmap& pixmap, const QPixmap& bgPixmap ) = 0;
    virtual void typeRemoved( const QString& type ) = 0;

    // These functions notify when an application is added or removed and to which category it belongs
    virtual void applicationAdded( const QString& type, const AppLnk& doc ) = 0;
    virtual void applicationRemoved( const QString& type, const AppLnk& doc ) = 0;
    virtual void allApplicationsRemoved() = 0;
    // Useful for wait icons, or running application lists etc
    enum ApplicationState { Launching, Running, Terminated, Raised };
    virtual void applicationStateChanged( const QString& name, ApplicationState state ) = 0;
    static const AppLnkSet& appLnks();

    // These functions notify of adding, removal and changes of document links used in the documents tab for example
    virtual void documentAdded( const DocLnk& doc ) = 0;
    virtual void documentRemoved( const DocLnk& doc ) = 0;
    virtual void allDocumentsRemoved() = 0;
    virtual void documentChanged( const DocLnk& oldDoc, const DocLnk& newDoc ) = 0;

    virtual void storageChanged( const QList<FileSystem> & ) = 0;

    virtual void applicationScanningProgress( int percent ) = 0;
    virtual void documentScanningProgress( int percent ) = 0;

    // Tells the server if the GUI needs to be updated about documents or applnks
    virtual bool requiresApplications() const = 0;
    virtual bool requiresDocuments() const = 0;

    enum DockArea { Top, Bottom, Left, Right };
    static void dockWidget( QWidget *w, DockArea placement );
    static void dockWidget( QWidget *w, DockArea placement, const QSize &s );
};


#endif // SERVER_INTERFACE_H
