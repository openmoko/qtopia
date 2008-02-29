/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
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

/*  Browser (impl. to catch setSource)	*/
class Browser: public QTextBrowser
{
    Q_OBJECT

public:
    Browser(QWidget *parent = 0, const char *name = 0);

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
    Browser *todayView;
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
