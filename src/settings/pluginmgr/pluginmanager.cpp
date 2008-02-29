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
    QString path = QPEApplication::qpeDir() + "plugins/";
    QDir tdir(path);

    QStringList tlist = tdir.entryList( QDir::Dirs );
    QStringList::Iterator it;
    for ( it = tlist.begin(); it != tlist.end(); ++it ) {
	if ( (*it)[0] == '.' )
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
	PluginTypeItem *titem = new PluginTypeItem( pluginListView, tname );
	titem->pluginDir = *it;
	titem->apply = apply.simplifyWhiteSpace();
	titem->comment = comment;
	if ( *it == "applets" ) {
	    titem->setOpen(TRUE);
	    pluginListView->setCurrentItem( titem );
	    selectionChanged( titem );
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
	    tr("<p>Qtopia must be restarted for the changes to take affect."),
	    QMessageBox::Ok, QMessageBox::NoButton );
	Global::restart();
    }
}

void PluginManager::restart()
{
    save();
    Global::restart();
}

