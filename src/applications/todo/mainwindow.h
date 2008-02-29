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

#ifndef TODOMAINWINDOW_H
#define TODOMAINWINDOW_H

#include <qtopia/pim/task.h>
#include <qtopia/fieldmapimpl.h>

#include <qtextbrowser.h>
#include <qmainwindow.h>
#include <qdialog.h>
#include <qvbox.h>

class TodoTable;
class QAction;
class QPopupMenu;
class Ir;
class CategorySelect;
class QLineEdit;
class QLabel;
class QPEToolBar;
class ContextMenu;
class CategorySelectDialog;
#ifdef QTOPIA_DATA_LINKING
class QDLClient;
#endif

class TodoSettings: public QDialog
{
    Q_OBJECT
public:
    TodoSettings(QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags fl = 0);
    
    void setCurrentFields(const QValueList<int> &);
    QValueList<int> fields() { return map->fields(); }

private:
    FieldMap *map;
};

class TodoView : public QTextBrowser
{
    Q_OBJECT
public:
    TodoView( QWidget *parent = 0, const char *name = 0);

    void init( const PimTask &task );

    void setSource( const QString &name );
signals:
    void done();
    void previous();
    void next();
private:
#ifdef QTOPIA_DATA_LINKING
    QDLClient *mNotesQC; 
#endif
};

class TodoWindow : public QMainWindow
{
    Q_OBJECT

public:
    TodoWindow( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~TodoWindow();

public slots:
    void appMessage(const QCString &, const QByteArray &);

    void reload();
    void flush();

protected slots:
    void createNewEntry();
    void deleteCurrentEntry();
    void editCurrentEntry();
    void showListView();
    void showDetailView();
    void viewPrevious();
    void viewNext();
    void setShowCompleted( int );
    void currentEntryChanged( );
    void showFindWidget( bool s );
    void search();
    void findFound();
    void findNotFound();
    void findWrapped();
    void setDocument( const QString & );
    void beamCurrentEntry();
    void beamDone( Ir * );
    void catSelected(int);
    void catChanged();
    void configure();

protected:
    void closeEvent( QCloseEvent *e );
#ifdef QTOPIA_PHONE
    void keyPressEvent(QKeyEvent *);
#endif
    void createUI();

private slots:
    void selectAll();
    void selectCategory();

private:
    bool receiveFile( const QString &filename );
    TodoView* todoView();

#ifdef Q_WS_QWS
    QString beamfile;
#endif
    TodoTable *table;
    TodoView *tView;
    QAction *newAction;
    QAction *backAction;
    QAction *editAction;
    QAction *deleteAction;
    QAction *findAction;
    QAction *beamAction;
    QPEToolBar *searchBar;
    QLineEdit *searchEdit;
    QPopupMenu *catMenu;
    CategorySelect *catSelect;
    QWidget *listView;
#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu;
    QAction *actionCategory;
    CategorySelectDialog *categoryDlg;
    QLabel *categoryLbl;
#endif
};

#endif
