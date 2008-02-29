/**********************************************************************
** Copyright (C) 2000-2004 Trolltech AS.  All rights reserved.
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
#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <qlistview.h>
#include <qmainwindow.h>
#include <qfileinfo.h>
#include <qaction.h>
#include <qtimer.h>
#include <qstringlist.h>

class InlineEdit;

class FileItem : public QListViewItem
{
public:
    FileItem( QListView * parent, const QFileInfo & fi );

    QString key( int column, bool ascending = TRUE ) const;
    QString getFilePath(){ return fileInfo.filePath(); }
    QString getFileName(){ return fileInfo.fileName(); }
    bool    isDir(){ return fileInfo.isDir(); }
    bool    isExecutable(){ return fileInfo.isExecutable(); }
    bool    isLib();
    int     launch();
    bool    rename( const QString & name );
private:
    QString sizeString( unsigned int size );
    QFileInfo fileInfo;
};


class FileView : public QListView
{
    Q_OBJECT

public:
    FileView( const QString & dir, QWidget * parent = 0,
			  const char * name = 0 );
    ~FileView(void);
    void    setDir( const QString & dir );
    QString cd(){ return currentDir; }
    QStringList history() const { return dirHistory; }
    void    scaleIcons(void);

public slots:
    void updateDir();
    void parentDir();
    void lastDir();

    void rename();
    void copy();
    void paste();
    void del();
    void cut();
    void newFolder();
    void viewAsText();

protected:
    void generateDir( const QString & dir );
    void resizeEvent( QResizeEvent* );
    void contentsMousePressEvent( QMouseEvent * e );
    void contentsMouseReleaseEvent( QMouseEvent * e );

protected slots:
    void itemClicked( QListViewItem * i );
    void itemDblClicked( QListViewItem * i );
    void showFileMenu();
    void cancelMenuTimer();
    void selectAll(){ QListView::selectAll( TRUE ); }
    void deselectAll(){ QListView::selectAll( FALSE ); }
    void addToDocuments();
    void run();
    void endRenaming();

private:
    QString     currentDir;
    QStringList dirHistory, flist;
    QTimer      menuTimer;
    InlineEdit * le;
    FileItem   * itemToRename;
    bool         selected;

    bool copyFile( const QString & dest, const QString & src );

signals:
    void dirChanged();
    void textViewActivated( QWidget * w );
    void textViewDeactivated();
};

class FileBrowser : public QMainWindow
{
    Q_OBJECT

public:
    FileBrowser( QWidget * parent = 0,
				 const char * name = 0, WFlags f = 0 );
    FileBrowser( const QString & dir, QWidget * parent = 0,
				 const char * name = 0, WFlags f = 0 );
private:
    void init(const QString & dir);
    QString      fileToCopy;
    QPopupMenu * dirMenu, * sortMenu;
    FileView   * fileView;
    QAction    * pasteAction;
    QAction	*lastAction;
    QAction	*upAction;

    bool         copyFile( const QString & dest, const QString & src );

private slots:
    void pcmciaMessage( const QCString &msg, const QByteArray &);

    void sortName();
    void sortDate();
    void sortSize();
    void sortType();
    void updateSorting();

    void updateDirMenu();
    void dirSelected( int id );
};

#endif
