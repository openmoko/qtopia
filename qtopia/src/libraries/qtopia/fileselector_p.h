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
#ifndef FILESELECTOR_P_H
#define FILESELECTOR_P_H

#include <qtopia/qpeglobal.h>
#include <qtopia/qpeapplication.h>
#include <qlistview.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qtopia/applnk.h>
#include <qregexp.h>
#include "fileselector.h"

class TypeFilter;
class CategorySelect;
class NewDocItem;
class DocLnkSet;
class QHBox;
class StorageInfo;
class QAction;
class CategorySelectDialog;

class QTOPIA_EXPORT FileSelectorView : public QListView
{
    Q_OBJECT

public:
    FileSelectorView( QWidget *parent, const char *name );
    ~FileSelectorView();

    QBrush altBrush() const;

protected:
    void keyPressEvent( QKeyEvent *e );
    void paletteChange( const QPalette &);

private:
    QBrush stripebrush;
};

class FileSelectorPrivate
{
public:
    FileSelectorPrivate()
    {
	typeFilter = 0;
	catSelect = 0;
	newDocItem = 0;
	files = 0;
	sortMode = FileSelector::Alphabetical;
#ifdef QTOPIA_PHONE
	filterAction = 0;
	filterDlg = 0;
#endif
    }

    ~FileSelectorPrivate()
    {
	delete files;
	// everything else is a subobject
    }

    void initReread( QListView *view, int timeout ) {
	view->clear();
#ifdef QTOPIA_PHONE
        message->setText( qApp->translate("FileSelector", "<qt><center>Finding documents...</center></qt>" ) );
        widgetStack->raiseWidget( message );
#else
	new QListViewItem( view, qApp->translate("FileSelector", "Finding Documents...") );
#endif
	rereadTimer->start( timeout, TRUE );
    }

    QWidgetStack *widgetStack;
    QLabel *message;
    TypeFilter *typeFilter;
    CategorySelect *catSelect;
    QValueList<QRegExp> mimeFilters;
    int catId;
    bool showNew;
    NewDocItem *newDocItem; // child of listview
    DocLnkSet* files;
    QHBox *toolbar;
    StorageInfo *storage;
    bool needReread;
    QTimer *rereadTimer;
    FileSelector::SortMode sortMode;
#ifdef QTOPIA_PHONE
    QAction *newAction;
    QAction *deleteAction;
    QAction *filterAction;
    CategorySelectDialog *filterDlg;
    QLabel *categoryLabel;
    bool haveContextMenu;
#endif
};

#endif
