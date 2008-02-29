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
#ifndef FILESELECTOR_H
#define FILESELECTOR_H

#include <qtopia/qpeglobal.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtoolbutton.h>
#include <qlistview.h>

#include <qtopia/filemanager.h>
#include <qtopia/applnk.h>

class QPopupMenu;
class QPushButton;
class FileSelectorView;

class QTOPIA_EXPORT FileSelectorItem : public QListViewItem
{
public:
    FileSelectorItem( QListView *parent, const DocLnk& f );
    ~FileSelectorItem();

    DocLnk file() const { return fl; }
    void paintCell ( QPainter *, const QColorGroup & cg, int column, int width, int alignment );

private:
    friend class FileSelector;
    DocLnk fl;
};

class FileSelectorPrivate;
class QTOPIA_EXPORT FileSelector : public QVBox
{
    Q_OBJECT

public:
    FileSelector( const QString &mimefilter, QWidget *parent, const char *name=0, bool newVisible = TRUE, bool closeVisible = TRUE );
    ~FileSelector();
    void setNewVisible( bool b );
    void setCloseVisible( bool b );
    int fileCount();
    DocLnk selectedDocument() const;	    // libqtopia
    QValueList<DocLnk> fileList() const;    // libqtopia
#ifdef QTOPIA_INTERNAL_FS_SEL
    // Caller must delete return value
    const DocLnk *selected(); // use selectedDocument() instead
#endif
    void setCurrentCategory( int newCatUid );	// libqtopia2

    void reread();

    enum SortMode	// libqtopia2
    {
	Alphabetical,
	ReverseAlphabetical,
	Chronological,
	ReverseChronological
    };

    void setSortMode( SortMode m );	// libqtopia2

#ifdef QTOPIA_PHONE
    void addOptions(QPopupMenu *m);
#endif

signals:
    void fileSelected( const DocLnk & );
    void newSelected( const DocLnk & );
    void closeMe();
    void typeChanged(void); // not available in 1.5.0
    void categoryChanged(void); // not available in 1.5.0

private:
    void showEvent( QShowEvent * );

private slots:
    void createNew();
    void fileClicked( int, QListViewItem *, const QPoint &, int );
    // pressed to get 'right down'
    void filePressed( int, QListViewItem *, const QPoint &, int );
    void fileClicked( QListViewItem *);
    void typeSelected( const QString &type );
    void catSelected( int );
    void cardChanged();
    void linkChanged( const QString & );
    void slotReread();
    void deleteFile();
    void showFilterDlg();
    void currentChanged(QListViewItem *);

private:
    void updateView();
    void updateWhatsThis();

private:
    FileSelectorView *view;
    QString filter;
    QToolButton *buttonNew, *buttonClose;
    FileSelectorPrivate *d;
};

#endif
