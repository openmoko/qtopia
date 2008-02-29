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

#ifndef TODOMAINWINDOW_H
#define TODOMAINWINDOW_H

#include <qtopia/pim/qtask.h>
#include <qtopiaabstractservice.h>

#include <qtextbrowser.h>
#include <qmainwindow.h>
#include <qdialog.h>
#include <qlayout.h>

#include <QDSData>
#include <QDLBrowserClient>
#include <QStack>

class TodoTable;
class QAction;
class QPopupMenu;
class Ir;
class QCategorySelector;
class QCategoryFilter;
class QLineEdit;
class QLabel;
class QMenu;
class QCategoryDialog;
class QStackedWidget;
class QTaskModel;
class QDSActionRequest;

class TodoView : public QDLBrowserClient
{
    Q_OBJECT
public:
    TodoView( QWidget *parent = 0 );

    void init( const QTask &task );

    QTask task() const { return mTask; }

signals:
    void done();
    void previous();
    void next();

protected:
    void keyPressEvent( QKeyEvent *e );

private:
    QDLBrowserClient *mNotesQC;
    QTask   mTask;
};

class TodoWindow : public QMainWindow
{
    Q_OBJECT
    friend class TasksService;

public:
    TodoWindow( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~TodoWindow();

public slots:
    void appMessage(const QString &, const QByteArray &);

    void reload();
    void flush();

protected slots:
    void createNewEntry();
    void deleteCurrentEntry();
    void editCurrentEntry();
    void showListView();
    void showDetailView(const QTask &);
    void viewPrevious();
    void viewNext();
    void setShowCompleted( int );
    void currentEntryChanged( );

    void showFindWidget( bool s );
    void startNewSearch(const QString &);
    void nextSearchItem();

    void findFound();
    void findNotFound();
    void findWrapped();
    void setDocument( const QString & );
    void beamCurrentEntry();
    void catSelected(const QCategoryFilter &);

    void qdlActivateLink( const QDSActionRequest& request );
    void qdlRequestLinks( const QDSActionRequest& request );

private slots:
    void doneDetailView();

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

    QDSData taskQDLLink( QTask& task );
    void removeTaskQDLLink( QTask& task );
    void removeTasksQDLLink( QList<QUniqueId>& taskIds );

    QString beamfile;

    QTaskModel *model;

    TodoTable *table;
    TodoView *tView;
    QAction *newAction;
    QAction *backAction;
    QAction *editAction;
    QAction *deleteAction;
    QAction *findAction;
    QAction *beamAction;
    QToolBar *searchBar;
    QLineEdit *searchEdit;
    QPopupMenu *catMenu;
    QCategorySelector *catSelect;
    QWidget *listView;
    QStackedWidget *centralView;
    QStack<QUniqueId> prevTasks;
#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
    QAction *actionCategory;
    QCategoryDialog *categoryDlg;
    QLabel *categoryLbl;
#endif
    bool closeAfterDetailView;
};

class TasksService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class TodoWindow;
private:
    TasksService( TodoWindow *parent )
        : QtopiaAbstractService( "Tasks", parent )
        { todo = parent; publishAll(); }

public:
    ~TasksService();

public slots:
    void newTask();
    void addTask( const QTask& task );
    void updateTask( const QTask& task );
    void removeTask( const QTask& task );
    void showTask( const QUniqueId& uid );
    void activateLink( const QDSActionRequest& request );
    void requestLinks( const QDSActionRequest& request );

private:
    TodoWindow *todo;
};

#endif
