/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qtopia/pim/private/todoxmlio_p.h>
#include <qtopia/pim/task.h>
#include <qtopia/fieldmapimpl.h>

#include <qtextview.h>
#include <qmainwindow.h>
#include <qdialog.h>
#include <qvbox.h>

class TodoTable;
class QAction;
class QPopupMenu;
class Ir;
class CategorySelect;
class QLineEdit;
class QPEToolBar;

class TodoSettings: public QDialog
{
public:
    TodoSettings(QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags fl = 0);
    
    void setCurrentFields(const QValueList<int> &);
    QValueList<int> fields() { return map->fields(); }

private:
    FieldMap *map;
};

class TodoView : public QTextView
{
public:
    TodoView( QWidget *parent = 0, const char *name = 0)
    	: QTextView(parent, name) {}

    void init( PimTask task)
    {
    	setText( task.toRichText() );
    }
    
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
    void slotNew();
    void slotDelete();
    void slotEdit();
    void slotListView();
    void slotDetailView();
    void setShowCompleted( int );
    void currentEntryChanged( int r, int c );
    void slotFind( bool s );
    void search();
    void findFound();
    void findNotFound();
    void findWrapped();
    void setDocument( const QString & );
    void slotBeam();
    void beamDone( Ir * );
    void catSelected(int);
    void configure();

protected:
    void closeEvent( QCloseEvent *e );

private slots:
    void addEntry( const PimTask &todo );
    void removeEntry(const PimTask &todo );
    void updateEntry(const PimTask &todo );
    void selectAll();

private:
    bool receiveFile( const QString &filename );
    void showView();
    TodoView* todoView();
    void deleteTasks(const QValueList<QUuid> &);
    
    TodoXmlIO tasks;
    TodoTable *table;
    TodoView *tView;
    QAction *editAction;
    QAction *deleteAction;
    QAction *findAction;
    QAction *beamAction;
    QPEToolBar *searchBar;
    QLineEdit *searchEdit;
    QPopupMenu *contextMenu, *catMenu;
    CategorySelect *catSelect;
    QHBox *vb;
};

#endif
