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
#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <qtopia/config.h>
#include <qtopia/storage.h>
#include <qtopia/applnk.h>
#include <qmainwindow.h>
#include <qstringlist.h>
#include <qprogressbar.h>
#include <qvbox.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qdict.h>
#include "launcherview.h"
#include "launchertab.h"
#include "loadingwidget.h"
#include "serverinterface.h"

class QWidgetStack;
class TaskBar;
class Launcher;

class LauncherTabWidget : public QVBox {
    // can't use a QTabWidget, since it won't let us set the frame style.
    Q_OBJECT
public:
    LauncherTabWidget( Launcher* parent );

    void updateDocs(AppLnkSet* docFolder);
    void setBusy(bool on);
    LauncherView *currentView(void);

    LauncherView* newView( const QString&, const QPixmap& pm, const QString& label );
    void deleteView( const QString& );
    void setTabViewAppearance( LauncherView *v, Config &cfg );
    void setTabAppearance( LauncherTab *, Config &cfg );

    LauncherView *view( const QString & );
    LauncherView *docView();

    void createDocLoadingWidget();
    void setLoadingWidgetEnabled( bool v );
    void setLoadingProgress( int percent );

    LauncherTabBar* categoryBar;

signals:
    void selected(const QString&);
    void clicked(const AppLnk*);
    void rightPressed(const AppLnk*);

protected slots:
    void raiseTabWidget();
    void tabProperties();
    void initLayout();

private slots:
    void launcherMessage( const QCString &, const QByteArray &);
    void appMessage( const QCString &, const QByteArray &);
    void setProgressStyle();

protected:
    void paletteChange( const QPalette &p );
    void styleChange( QStyle & );

private:
    Launcher *launcher;
    LauncherView *docview;

    QWidgetStack *stack;
    LoadingWidget *docLoadingWidget;
};

class Launcher : public QMainWindow, public ServerInterface
{
    Q_OBJECT
public:
    Launcher();
    ~Launcher();

    // implementing ServerInterface
    void createGUI();
    void showGUI();
    void destroyGUI();
    void typeAdded( const QString& type, const QString& name, const QPixmap& pixmap, const QPixmap& bgPixmap );
    void typeRemoved( const QString& type );
    void applicationAdded( const QString& type, const AppLnk& doc );
    void applicationRemoved( const QString& type, const AppLnk& doc );
    void allApplicationsRemoved();
    void applicationStateChanged( const QString& name, ApplicationState state );
    void documentAdded( const DocLnk& doc );
    void documentRemoved( const DocLnk& doc );
    void allDocumentsRemoved();
    void documentChanged( const DocLnk& oldDoc, const DocLnk& newDoc );
    void storageChanged( const QList<FileSystem> & );
    void applicationScanningProgress( int percent );
    void documentScanningProgress( int percent );
    bool requiresApplications() const { return TRUE; }
    bool requiresDocuments() const { return TRUE; }
    void showLoadingDocs();
    void showDocTab();

    QStringList idList() const { return ids; }

public slots:
    void viewSelected(const QString&);
    void showTab(const QString&);
    void select( const AppLnk * );
    void delayedSelect();
    void properties( const AppLnk * );
    void delayedProperties();
    void makeVisible();

signals:
    void executing( const AppLnk * );

private slots:
    void systemMessage( const QCString &, const QByteArray &);
    void toggleSymbolInput();
    void toggleNumLockState();
    void toggleCapsLockState();
    void askForPin(bool);

protected:
    bool eventFilter( QObject *o, QEvent *ev );
    void timerEvent( QTimerEvent *e );

private:
    void updateApps();
    void loadDocs();
    void updateDocs();
    void updateTabs();

    LauncherTabWidget *tabs;
    QStringList ids;
    TaskBar *tb;

    AppLnk *delayedAppLnk;
    QDate last_today_show;
    int tid_today;
};

#endif // LAUNCHERVIEW_H
