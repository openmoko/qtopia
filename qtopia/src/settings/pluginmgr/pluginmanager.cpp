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

#include "pluginmanager.h"
#include <qtopia/qpeapplication.h>
#include <qtopia/pluginloader.h>
#include <qtopia/global.h>
#include <qtopia/config.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/resource.h>
#include <qdir.h>
#include <qlistview.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtimer.h>

static QString stripSystem( const QString &libName )
{
#ifndef Q_OS_WIN32
    QString name = (libName).mid(3);
    int sep = name.find( ".so" );
#else
    QString name = libName;
    int sep = name.find( ".dll" );
#endif
    if ( sep > 0 )
	name.truncate( sep );

    return name;
}

class PluginTypeItem : public QListViewItem
{
public:
    PluginTypeItem( QListView *v, const QString &text )
	: QListViewItem( v, text ) {}

    QString pluginDir;
    QString apply;
    QString comment;
};

class PluginItem : public QCheckListItem
{
public:
    PluginItem( QListViewItem *parent, const QString &text, Type t = CheckBox )
	: QCheckListItem( parent, text, t ) {}

    QString pluginType;
    QString pluginName;
    QString comment;
};

class CommentLabel : public QLabel
{
public:
    CommentLabel( QWidget *parent, const char *name=0 )
	: QLabel( parent, name ) {}

    int heightForWidth( int w ) const
    {
	int h = QLabel::heightForWidth(w);
	return QMAX(h,36);
    }
};

//---------------------------------------------------------------------------

PluginManager::PluginManager( QWidget *parent, const char *name, bool modal, WFlags f )
    : PluginManagerBase( parent, name, modal, f )
{
    pluginListView->header()->hide();
    pluginListView->setRootIsDecorated( TRUE );
    connect( pluginListView, SIGNAL(selectionChanged(QListViewItem*)),
	this, SLOT(selectionChanged(QListViewItem*)) );
    connect( restartBtn, SIGNAL(clicked()), this, SLOT(restart()) );
    if ( !PluginLoader::inSafeMode() )
	restartBtn->hide();
    QVBoxLayout *vb = new QVBoxLayout(commentFrame);
    vb->setMargin(4);
    commentLabel = new CommentLabel(commentFrame);
    commentLabel->setTextFormat( RichText );
    commentLabel->setText( tr("Loading...") );
    vb->addWidget(commentLabel);
    QTimer::singleShot(0, this, SLOT(init()) );
}

void PluginManager::selectionChanged( QListViewItem *i )
{
    QString comment;
    if ( i && i->parent() )
	comment = ((PluginItem*)i)->comment;
    else if ( i )
	comment = ((PluginTypeItem*)i)->comment;

    commentLabel->setText( comment );
}

void PluginManager::drawWait(bool on)
{
    QWidget* waitwi = pluginListView->viewport();
    QPainter waitpaint(waitwi);
    QPixmap pm = Resource::loadPixmap("bigwait");
    int x = (waitwi->width()-pm.width())/2;
    int y = (waitwi->height()-pm.height())/2;
    if ( on ) {
	waitpaint.drawPixmap(x,y,pm);
    } else {
	waitpaint.eraseRect(x,y,pm.width(),pm.height());
    }
}

void PluginManager::init()
{
    drawWait( TRUE );
    QStringList qpepaths = Global::qtopiaPaths();
    QDict<PluginTypeItem> items;
    for (QStringList::Iterator qit=qpepaths.begin(); qit != qpepaths.end(); ++qit ) {
	QString path = *qit + "plugins/";
	QDir tdir(path);

	QStringList tlist = tdir.entryList( QDir::Dirs );
	QStringList::Iterator it;
	for ( it = tlist.begin(); it != tlist.end(); ++it ) {
	    if ( (*it)[0] == '.' || *it == "application" || *it == "network")
		continue;
	    QString pluginPath = path + *it + '/';
	    QStringList required;
	    QString apply, comment;
	    QString tname = *it;
	    tname[0] = tname[0].upper();
	    if ( QFile::exists(pluginPath + ".directory") ) {
		Config config( pluginPath + ".directory", Config::File );
		tname = config.readEntry( "Name", tname );
		required = config.readListEntry( "Required", ',' );
		apply = config.readEntry( "Apply" );
		comment = config.readEntry( "Comment" );
	    }
	    PluginTypeItem *titem = items.find(*it);
	    if ( !titem ) {
		titem = new PluginTypeItem( pluginListView, tname );
		items.insert(*it,titem);
		titem->pluginDir = *it;
		titem->apply = apply.simplifyWhiteSpace();
		titem->comment = comment;
		if ( *it == "applets" ) {
		    titem->setOpen(TRUE);
		    pluginListView->setCurrentItem( titem );
		    selectionChanged( titem );
		}
	    }

	    PluginLoader loader( *it );
#ifndef Q_OS_WIN32
	    QDir pdir(path + *it, "lib*.so");
#else
	    QDir pdir(path + *it, "*.dll");
#endif

	    QStringList plist = pdir.entryList();
	    QStringList::Iterator pit;
	    for ( pit = plist.begin(); pit != plist.end(); ++pit ) {
		QString base = stripSystem( *pit );
		QString name = base;
		name[0] = name[0].upper();
		if ( QFile::exists( pluginPath + base + ".desktop" ) ) {
		    Config config( pluginPath + base + ".desktop", Config::File );
                    config.setGroup("Desktop Entry");
		    name = config.readEntry( "Name", name );
		    comment = config.readEntry( "Comment" );
		    if ( !required.count() )
			required = config.readListEntry( "Required", ',' );
		}
		PluginItem *pitem = new PluginItem( titem, name );
		pitem->pluginType = *it;
		pitem->pluginName = *pit;
		pitem->comment = comment;
		if ( loader.isEnabled( *pit ) )
		    pitem->setOn( TRUE );
		if ( required.contains(base) )
		    pitem->setEnabled(FALSE);
	    }
	}
    }
    drawWait( FALSE );
}

void PluginManager::accept()
{
    commentLabel->setText( tr("Applying changes - Please wait...") );
    qApp->processEvents();
    save();
    QDialog::accept();
}

void PluginManager::done( int r )
{
    PluginManagerBase::done(r);
    close();
}

void PluginManager::save()
{
    bool restartQtopia = FALSE;
    PluginTypeItem *titem = (PluginTypeItem *)pluginListView->firstChild();
    while ( titem ) {
	PluginLoader loader( titem->pluginDir );
	PluginItem *pitem = (PluginItem *)titem->firstChild();
	bool needApply = FALSE;
	while ( pitem ) {
	    if ( pitem->isOn() != loader.isEnabled(pitem->pluginName) ) {
		qDebug( "Changed: %s", pitem->isOn() ? "On" : "Off" );
		needApply = TRUE;
		loader.setEnabled( pitem->pluginName, pitem->isOn() );
	    }
	    pitem = (PluginItem *)pitem->nextSibling();
	}
	if ( needApply && !titem->apply.isEmpty() ) {
	    QStringList p = QStringList::split( ' ', titem->apply );
	    if ( p.count() == 2 ) {
		if ( p[0] == "QPE/System" && p[1] == "restart()" )
		    restartQtopia = TRUE;
		else
		    QCopEnvelope e( p[0].latin1(), p[1].latin1() );
	    }
	}
	titem = (PluginTypeItem *)titem->nextSibling();
    }

    if ( restartQtopia ) {
	QMessageBox::warning(0, tr("Restart Qtopia"),
	    tr("<qt>Qtopia must be restarted for the changes to take affect.</qt>"),
	    QMessageBox::Ok, QMessageBox::NoButton );
	Global::restart();
    }
}

void PluginManager::restart()
{
    save();
    Global::restart();
}

