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

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qtopia/contextmenu.h>

#include <qmainwindow.h>
#include <qtextbrowser.h>
#include <qvaluestack.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include <qtimer.h>
#include <qlabel.h>

class MagicTextBrowser : public QTextBrowser {
    Q_OBJECT
public:
    MagicTextBrowser( QWidget* parent );
    
    // Clear page source and history
    void clear();
    
    // Return page source
    QString source() { return current; }
    
    // Set page source
    void setSource( const QString& );
    
signals:
    // Back status changed
    void hasBack( bool );
    
    // Forward status changed
    void hasForward( bool );
    
public slots:
    // Go to previous page in history
    void backward();
    
    // Go to next page in history
    void forward();
    
private:
    // Display source and set as current
    void setCurrent( const QString& file );

    // Replace qtopia tags with help page links
    bool magic( const QString&, const QString&, const QString& );
    
    // Generate help page links 
    QString generate( const QString& );
    
    QString current;
    QValueStack< QString > backStack, forwardStack;
};

class HelpBrowser : public QMainWindow
{
    Q_OBJECT
public:
    HelpBrowser( QWidget* parent = 0, const char *name = 0, WFlags f = 0 );
    
#ifdef QTOPIA_PHONE
    bool eventFilter( QObject*, QEvent* );
#endif

public slots:
    void setDocument( const QString &doc );
    
private slots:
    void appMessage( const QCString& msg, const QByteArray& data );
    
    void goHome();
    
    void textChanged();
    
    // Only Qtopia Phone
    void setBackDisabled();
    
    // Only Qtopia Phone
    void setBack( bool );

protected:
    void closeEvent( QCloseEvent* );
    
private:
    void init();
    
#ifdef QTOPIA_PHONE
    void close();
#endif
    
    MagicTextBrowser *browser;
    QAction *backAction, *forwardAction;
#ifdef DEBUG
    QLabel *location;
#endif

#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu;
    QTimer *closeTimer;
    bool pressed, closeOk;
#endif
};

#endif
