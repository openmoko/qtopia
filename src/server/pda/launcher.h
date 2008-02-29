/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <qsettings.h>
#include <qstorage.h>
#include <qcontent.h>
#include <qcontentset.h>

#include <qstringlist.h>
#include <qprogressbar.h>
#include <qlist.h>
#include <qmap.h>
#include <qdatetime.h>

#include <qsoftmenubar.h>

#include "launcherview.h"

class QCategoryManager;
class QStackedWidget;
class LauncherTabBar;
class TaskBar;
class Launcher;
class LauncherTab;

struct TypeView {
  TypeView() : view(0)/*, contextMenu(0), holdMenu(0)*/ {}
  LauncherView *view;
/*  QMenu *contextMenu;
  QMenu *holdMenu;
  */
  QString name;
  QIcon icon;
};

class LauncherTabWidget : public QWidget {
    // can't use a QTabWidget, since it won't let us set the frame style.
    Q_OBJECT
public:
    LauncherTabWidget( Launcher* parent );

    void updateDocs(QContentSet* docFolder);
    void setBusy(bool on);
    LauncherView *currentView(void);

    LauncherView* newView( const QString&, const QIcon& pm, const QString& label );
    void deleteView( const QString& );
    void setTabViewAppearance( LauncherView *v, QSettings &cfg );
    void setTabAppearance( LauncherTab *, const QString &type, QSettings &cfg );

    LauncherView *view( const QString & );
    LauncherView *docView();
    void showTab(const QString& id);


    void layoutTabs();

signals:
    void selected(const QString&);
    void clicked(const QContent*);
    void rightPressed(const QContent*);

protected slots:
    void raiseTabWidget();
    void tabProperties();
    void initLayout();

private slots:
    void launcherMessage( const QString &, const QByteArray &);
    void appMessage( const QString &, const QByteArray &);

protected:
    void changeEvent(QEvent *e);

private:
    LauncherTabBar* categoryBar;
    Launcher *launcher;
    LauncherView *docview;

    QStackedWidget *stack;
};

class Launcher : public QWidget
{
    Q_OBJECT
public:
    Launcher();
    ~Launcher();

    void showDocTab();

    QStringList idList() const { return ids; }

public slots:
    void viewSelected(const QString&);
    void showTab(const QString&);
    void select( const QContent * );
    void properties( const QContent * );
    void delayedSelect();
    void delayedProperties();
    void makeVisible();

signals:
    void executing( const QContent * );

private slots:
    void systemMessage( const QString &, const QByteArray &);
    void toggleSymbolInput();
    void toggleNumLockState();
    void toggleCapsLockState();
    void askForPin(bool);
    void categoriesChanged();

protected:
    void showEvent(QShowEvent *e);
    bool eventFilter( QObject *o, QEvent *ev );
    void timerEvent( QTimerEvent *e );

private:
    void addType(const QString& type, const QString& name,
                 const QIcon &iconconst);
    void removeType(const QString& type);

    void updateApps();
    void loadDocs();
    void updateDocs();
    void updateTabs();
    QMap<QString, TypeView> map;
    LauncherTabWidget *tabs;
    QStringList ids;
    TaskBar *tb;

    QContent *delayedAppLnk;
    QDate last_today_show;
    int tid_today;

    QCategoryManager *categories;
    QStringList currentCategories;
};

#endif // LAUNCHERVIEW_H
