/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include <qtopia/pim/qappointment.h>
#include <qtopiaabstractservice.h>
#include <qmainwindow.h>
#include <QDSData>
#include <QDLBrowserClient>
#include <QStack>

class TodoTable;
class QAction;
class QPopupMenu;
class QCategorySelector;
class QCategoryFilter;
class QLineEdit;
class QLabel;
class QMenu;
class QCategoryDialog;
class QStackedWidget;
class QTaskModel;
class QDSActionRequest;

class ListPositionBar;

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

    QString createTaskText(const QTask& task);
    static QString formatDate(const QDate& date, const QDate& today);

private:
    QDLBrowserClient *mNotesQC;
    QTask   mTask;
    bool mIconsLoaded;
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
    void createNewEntry(bool useCurrentCategory=true);
    void deleteCurrentEntry();
    void editCurrentEntry();
    void showListView();
    void showDetailView(const QTask &);
    void viewPrevious();
    void viewNext();
    void showCompletedTasks();
    void hideCompletedTasks();
    void taskModelReset( );
    void markTaskDone();
    void markTaskNotDone();
    void saveNewTask(const QTask&, const QAppointment&);

    void setDocument( const QString & );
    void beamCurrentEntry();
    void catSelected(const QCategoryFilter &);
    void markMenuDirty();

    void qdlActivateLink( const QDSActionRequest& request );
    void qdlRequestLinks( const QDSActionRequest& request );

private slots:
    void doneDetailView();

protected:
    void closeEvent( QCloseEvent *e );
    void keyPressEvent(QKeyEvent *);
    bool eventFilter(QObject *o, QEvent *e);
    void createUI();

private slots:
    void selectAll();
    void selectCategory();
    void delayedInit();
    void updateContextMenu();

private:
    bool receiveFile( const QString &filename );
    TodoView* todoView();

    QDSData taskQDLLink( QTask& task );
    void removeTaskQDLLink( QTask& task );
    void removeTasksQDLLink( QList<QUniqueId>& taskIds );

    void updateDependentAppointment(const QTask &src, const QAppointment& appt);

    QString beamfile;

    QTaskModel *model;

    TodoTable *table;
    TodoView *tView;
    ListPositionBar *listPositionBar;
    QAction *newAction;
    QAction *backAction;
    QAction *editAction;
    QAction *deleteAction;
    QAction *beamAction;
    QAction *markDoneAction;
    QAction *markNotDoneAction;
    QToolBar *searchBar;
    QLineEdit *searchEdit;
    QPopupMenu *catMenu;
    QCategorySelector *catSelect;
    QWidget *listView;
    QStackedWidget *centralView;
    QStack<QUniqueId> prevTasks;
    QAction *actionShowCompleted;
    QAction *actionHideCompleted;
    QAction *actionCategory;
    QCategoryDialog *categoryDlg;
    QLabel *categoryLbl;
    bool closeAfterDetailView;
    bool contextMenuActionsDirty;
    bool showCompleted;
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
    void updateRecurringTasks(const QDateTime& datetime, int data);

private:
    TodoWindow *todo;
};

#endif
