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



#include "today.h"
#include "todayoptions.h"

#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/global.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/services.h>
#include <qtopia/datebookmonth.h>
#include <qtopia/pluginloader.h>

#include <qvaluelist.h>
#include <qdir.h>
#include <qpushbutton.h>
#include <qcursor.h>
#include <qwidget.h>

#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qtimer.h>

TodayBrowser::TodayBrowser(QWidget *parent, const char *name)
	: QTextBrowser(parent, name)
{
}

void TodayBrowser::setSource(const QString &name)
{
    emit taskClicked(name);
}

static QValueList<TodayPlugin> pluginList;

Today::Today(QWidget *parent, const char *name, WFlags fl)
    : QMainWindow(parent, name, fl), loader(0)
{
    setCaption( tr("Today") );

    daytimer = new QTimer();
    connect(daytimer, SIGNAL(timeout()), this, SLOT(dayChange()));

    QCopChannel *c = new QCopChannel("QPE/System", this);
    connect(c, SIGNAL(received(const QCString&,const QByteArray&)),
	this, SLOT(sysMessage(const QCString&,const QByteArray&)));

    init();

    connect( qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)),
        this, SLOT(appMessage(const QCString&,const QByteArray&)) );
}

Today::~Today()
{
    delete daytimer;
}

void Today::init()
{
    todayView = new TodayBrowser(this, "todayView");
    QWhatsThis::add(todayView, tr("Displays todays' activities.  Configure the view by tapping the Today icon."));
    setCentralWidget(todayView);
    connect(todayView, SIGNAL( taskClicked(const QString&) ),
	this, SLOT( taskSelected(const QString&) ) );

    loadPlugins();
    setupPluginMenu();

    //	Don't call updateView as resizeEvent will handle that
    readConfig();
    sort();

    startDayTimer();
}

//
// Figure out how long till the change of day, and start the day change timer.
//
void
Today::startDayTimer(void)
{
    QDateTime	now;
    QDateTime	tomorrow;

    now = QDateTime::currentDateTime();
    tomorrow = now.addDays(1);
    tomorrow.setTime(QTime(0, 0));

    daytimer->stop();
    daytimer->start(1000 * now.secsTo(tomorrow), TRUE);
}

//
// Change-of-day timer has gone off.  Updating the view will grab new
// info from the plugins.  Restart the timer so we catch the next
// change-of-day.
//
void
Today::dayChange(void)
{
    updateView();
    startDayTimer();
}

void Today::update()
{
    readConfig();
    sort();
    updateView();
    startDayTimer();
}

void Today::appMessage(const QCString &msg, const QByteArray &)
{
    /*	Reload plugin should contain plugin that has changed.  We can
	only update all plugins for now though.	*/
    if ( msg == "reloadPlugin(QString)" ) {
	update();
    }
}

//
// Respond to system messages.
//
void
Today::sysMessage(const QCString &msg, const QByteArray &)
{
    if (msg == "timeChange(QString)") {
	update();
    }
}

void Today::readConfig()
{
    Config config("today");
    config.setGroup("view");

    QValueList<TodayPlugin>::Iterator it;
    int count = 0;
    for ( it = pluginList.begin(); it != pluginList.end(); ++it ) {
	(*it).pos = config.readNumEntry( (*it).viewer->name() + "_pos", count);
	count++;

	(*it).active = ( (*it).pos != -1 );
    }  
}

/*  Don't think I'll win any prizes for this sorting algorithm, but there's
	probably a maximum of 5-6 plugins so it'll have to do */
void Today::sort()
{
    QValueList<TodayPlugin> list;

    uint current = 0;

    while ( current < pluginList.count() ) {
	for (uint x = 0; x < pluginList.count(); x++) {
	    if ((uint) pluginList[x].pos == current)
		list.append( pluginList[x] );
	}
	current++;
    }

    for (uint x = 0; x < pluginList.count(); x++) {
	if (pluginList[x].pos == -1 || (uint) pluginList[x].pos >= pluginList.count() )
	    list.append( pluginList[x] );
    }

    pluginList = list;
}

void Today::loadPlugins()
{
    //reset old plugins
    if ( loader ) {
	QValueList<TodayPlugin>::Iterator tit;
	for ( tit = pluginList.begin(); tit != pluginList.end(); ++tit ) {
	    loader->releaseInterface( (*tit).iface );
	}
	delete loader;
	loader = 0;
    }

    loader = new PluginLoader( "today" );

    QStringList list = loader->list();
    QStringList::Iterator it;

    uint count = 0;
    for ( it = list.begin(); it != list.end(); ++it ) {
	TodayInterface *iface = 0;

	if ( loader->queryInterface( *it, IID_TodayPlugin, (QUnknownInterface**)&iface ) == QS_OK ) {
	    qDebug( "loaded: %s", (*it).latin1() );

	    TodayPlugin plugin;
	    plugin.active = TRUE;
	    plugin.pos = count;

	    plugin.iface = iface;
	    plugin.viewer = plugin.iface->object(this, "todayPlugin");
	    if ( plugin.viewer ) {
		pluginList.append( plugin );
		QObject* obj = plugin.iface->objectFor(plugin.viewer);
		if ( obj ) {
		    connect( obj, SIGNAL( reload()), this, SLOT( reload() ) );
		}
		count++;
	    } else {
		qDebug("object function not implemented, cannot use plugin");
	    }
	    
	}
    }
}

void Today::setupPluginMenu()
{
    pluginSettings = new QPopupMenu(this);

    pluginSettings->insertItem(tr("Edit Today View"), 0);
    pluginSettings->insertSeparator();
    
    for (uint count = 0; count < pluginList.count(); count++) {
	if ( pluginList[count].viewer->configurable() )
	    pluginSettings->insertItem(pluginList[count].viewer->name() + "...", count + 1);
    }
    
    connect(pluginSettings, SIGNAL(activated(int)), this, SLOT(settingsMenuClicked(int)) );
}

void Today::updateView()
{
    if ( pluginList.count() == 0 ) {
	todayView->setText(tr("No plugins found"));
	return;
    }

    QString txt = "<table><tr>";
    txt += "<td><a href=\"TodayOptions\"><img src=\"TodayApp\"></a></td>";
    txt += "<td><b> <big> <font color=#0000FF><a href=\"TodayDate\">" +
	TimeString::localYMD( QDate::currentDate(), TimeString::Long ) +
	" </a> </font> </big> </b> <body></td>";
    txt += "</tr></table>";

    uint count = 0;
    TodayPlugin plugin;
    QFontMetrics fm( qApp->font() );
    int avgFontWidth = fm.width("abcdefghijklmnopqrstuvwxyz")/26;
    int fontHeight = fm.height();

    uint _width = width() / ( avgFontWidth + 2);
    uint _height = ( ( height() ) / (pluginList.count() + 1) ) / fontHeight;
    
    for (uint i = 0; i < pluginList.count(); i++) {
	plugin = pluginList[i];

	if ( plugin.active ) {
	    txt += " <hr> ";
	    txt += plugin.viewer->html(_width, _height);

	    count++;
	}
    }

    if ( count == 0 ) {
	txt += " <p> <hr> <b> "+tr("No plugins selected for display.")+"<br>";
	txt += tr("Click <a href=\"TodayPluginEdit\">here</a> to configure display.")+"</b>";
    }

    txt += "</body>";

    todayView->setText( txt );
}

void Today::resizeEvent(QResizeEvent * /* event */)
{
    updateView();
}


/*  Currently understands three commands:  raise, service and qcop	*/
void Today::taskSelected(const QString &id)
{
    if ( id == "TodayPluginEdit" ) {
	settingsMenuClicked(0);
    } else if ( id == "TodayOptions" ) {
	optionButtonClicked();
    } else if ( id == "TodayDate" ) {
/*	This is just confusing.  Disabled for now.

	QPopupMenu *pop = new QPopupMenu(this);
	DateBookMonth *calendar = new DateBookMonth(this, "calendar", TRUE );
	pop->insertItem( calendar);
	
	pop->popup( QCursor::pos() );
*/
    } else {
	int pos = id.find(":");
	if ( pos == -1 )
	    return;
	QString command = id.left(pos);
	QString param = id.mid(pos+1);

	if ( command == "raise" ) { // No tr
	    {
		QCString str = "QPE/Application/";
		str += param.latin1();
		QCopEnvelope( str, "raise()");
	    }
	} else if ( command == "qcop" ) {
	    launchQCop( QStringList::split(':', param) );
	}  else if ( command == "service" ) { // No tr
	}
    }
}

/*  params (original string) should be of format: name;linkId */
void Today::launchQCop(const QStringList params)
{
    if (params.count() != 2)
	return;

    QString pName = params[0];
    int pluginPos = -1;
    for (uint i = 0; i < pluginList.count(); i++) {
	if (pluginList[i].viewer->name() == pName) {
	    pluginPos = i;
	    break;
	}
    }
    if ( pluginPos == -1 )
	return;
	
    pluginList[pluginPos].viewer->itemSelected(params[1]);
}

void Today::settingsMenuClicked(int id)
{
    if ( id == 0 ) {
        TodayOptions to(this, "options", TRUE); // No tr
	to.setPlugins( pluginList );
	if ( QPEApplication::execDialog(&to) == QDialog::Accepted ) {
	    update();
	}
    } else {
	QString target = pluginSettings->text(id);
	target.truncate(target.length() - 3);	//to strip ...

	//Plugins changes list pos according to sorting so we need to search
	for (uint i = 0; i < pluginList.count(); i++) {
	    if (pluginList[i].viewer->name() == target) {
		
		ConfigDialog configDialog(this, "editor", TRUE); // No tr
		
		/* Destructor of configDialog deletes the plugin widget	*/
		QWidget *pWidget = pluginList[i].viewer->widget(&configDialog);
		QString str = tr("%1 settings", "eg. Calendar settings").arg(pluginList[i].viewer->name());
		configDialog.setMainWidget(pWidget, tr(str) );
		if ( QPEApplication::execDialog(&configDialog) == QDialog::Accepted ) {
		    pluginList[i].viewer->accepted(pWidget);
		    update();
		}
		
		break;
	    }
	}
    }
} 

void Today::optionButtonClicked()
{
    if  ( pluginSettings->count() > 2 ) {
	pluginSettings->popup( QCursor::pos() );
    } else {
	settingsMenuClicked( 0 );
    }
}

void Today::reload()
{
    QString str = "reload called from plugin "; // No tr
    str += sender()->name();
    qWarning( str );

    update();
}

/*  Config Dialog, which might be more advanced	*/
ConfigDialog::ConfigDialog(QWidget *parent, const char *name, WFlags fl)
    : QDialog(parent, name, fl)
{
    if ( layout() ) {
	layout()->setSpacing(0);
	layout()->setMargin(0);
    }
    
    grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(6);
}


void ConfigDialog::setMainWidget(QWidget *w, const QString &caption)
{
    setCaption( caption);
    grid->addWidget(w, 0, 0);
}

