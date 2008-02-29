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



#ifndef TODAY_H
#define TODAY_H

#include <qmainwindow.h>
#include <qtextbrowser.h>
#include <qpopupmenu.h>
#include <qdialog.h>
#include <qlayout.h>

#include <qtopia/todayplugininterface.h>

class PluginLoader;
class QTimer;

/*  TodayBrowser (impl. to catch setSource)	*/
class TodayBrowser: public QTextBrowser
{
    Q_OBJECT

public:
    TodayBrowser(QWidget *parent = 0, const char *name = 0);

signals:
    void taskClicked(const QString &href);

public slots:
    virtual void setSource(const QString &name);
};


/*  Today	*/

struct TodayPlugin {
    TodayInterface *iface;
    TodayPluginInterface *viewer;
    bool active;
    int pos;
};

class ConfigDialog;

class Today: public QMainWindow
{
    Q_OBJECT
public:
    Today(QWidget *parent = 0, const char *name = 0, WFlags fl = 0);
    ~Today();

protected slots:
    virtual void resizeEvent(QResizeEvent *);

    void appMessage(const QCString &, const QByteArray &);
    void sysMessage(const QCString &, const QByteArray &);
    
    void taskSelected(const QString &id);
    void optionButtonClicked();
    void settingsMenuClicked(int id);
    void reload();
    void dayChange(void);

private:
    void init();
    void loadPlugins();
    void setupPluginMenu();
    
    void launchQCop(const QStringList params);
    void update();
    void readConfig();
    void sort();
    void updateView();

    void startDayTimer(void);

private:
    TodayBrowser *todayView;
    QPopupMenu *pluginSettings;
    QTimer  *daytimer;
    PluginLoader *loader;
};

class ConfigDialog: public QDialog
{
public:
    ConfigDialog(QWidget *parent=0, const char *name=0, WFlags fl=0);

    void setMainWidget(QWidget *w, const QString &caption);

private:
    QGridLayout *grid;
};

#endif
