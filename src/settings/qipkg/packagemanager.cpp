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
#include "packagemanager.h"
#include "pkdesc.h"
#include "pkfind.h"
#include "pksettings.h"

#include <qtopia/process.h>
#include <qtopia/resource.h>
#include <qtopia/stringutil.h>
#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/applnk.h>

#include <qtopia/storage.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qlayout.h>



#include <qprogressbar.h>
#include <qcombobox.h>
#include <qdict.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qtextview.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#include <stdlib.h>
#include <unistd.h> //symlink() 
#include <sys/stat.h> // mkdir()

static QPixmap *pm_uninstalled=0;
static QPixmap *pm_installed=0;
static QPixmap *pm_uninstall=0;
static QPixmap *pm_install=0;


class PackageItem : public QCheckListItem {
    bool installed;
public:
    PackageItem(QListView* lv, const QString& name, const QString& desc, const QString& size, bool inst ) :
	QCheckListItem(lv,name,CheckBox), installed(inst)
    {
	setText(1,desc);
	setText(2,size);
    }

    void paintCell( QPainter *p,  const QColorGroup & cg,
                    int column, int width, int alignment )
    {
	if ( !p )
	    return;

	p->fillRect( 0, 0, width, height(),
	    isSelected()? cg.highlight() : cg.base() );

	if ( column != 0 ) {
	    // The rest is text
	    QListViewItem::paintCell( p, cg, column, width, alignment );
	    return;
	}

	QListView *lv = listView();
	if ( !lv )
	    return;
	int marg = lv->itemMargin();
	int r = marg;

	QPixmap pm = statePixmap();
	p->drawPixmap(marg,(height()-pm.height())/2,pm);
	r += pm.width()+1;

	p->translate( r, 0 );
	QListViewItem::paintCell( p, cg, column, width - r, alignment );
    }

    void paintFocus( QPainter *p, const QColorGroup & cg,
                             const QRect & r )
    {
	// Skip QCheckListItem
	// (makes you wonder what we're getting from QCheckListItem)
	QListViewItem::paintFocus(p,cg,r);
    }

    QPixmap statePixmap() const
    {
	if ( !isOn() ) {
	    if ( !installed )
		return *pm_uninstalled;
	    else
		return *pm_installed;
	} else {
	    if ( !installed )
		return *pm_install;
	    else
		return *pm_uninstall;
	}
    }

    QString name() const { return text(0); }
    QString description() const { return text(1); }
    bool isInstalled() const { return installed; }

    QString key( int column, bool ascending ) const
    {
	if ( column == 2 ) {
	    QString t = text(2);
	    double bytes=t.toDouble();
	    if ( t.contains('M') ) bytes*=1024*1024;
	    else if ( t.contains('K') || t.contains('k') ) bytes*=1024;
	    if ( !ascending ) bytes=999999999-bytes;
	    return QString().sprintf("%09d",(int)bytes);
	} else {
	    return QListViewItem::key(column,ascending);
	}
    }
};

/* 
 *  Constructs a PackageManager which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
PackageManager::PackageManager( QWidget* parent,  const char* name, WFlags fl )
    : PackageManagerBase( parent, name, fl )
{
    settings = new PackageManagerSettings(this,0,TRUE);
    connect( settings->newserver, SIGNAL(clicked()), this, SLOT(newServer()) );
    connect( settings->removeserver, SIGNAL(clicked()), this, SLOT(removeServer()) );
    connect( settings->servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );
    connect( doit, SIGNAL(clicked()), this, SLOT(doIt()) );
    settings->servername->setEnabled(FALSE);
    settings->serverurl->setEnabled(FALSE);

    if (!pm_uninstalled) {
	pm_uninstalled = new QPixmap(Resource::loadPixmap("uninstalled"));
	pm_installed = new QPixmap(Resource::loadPixmap("installed"));
	pm_install = new QPixmap(Resource::loadPixmap("install"));
	pm_uninstall = new QPixmap(Resource::loadPixmap("uninstall"));
    }

    QFontMetrics fm = fontMetrics();
    int w0 = fm.width(PackageManagerBase::tr("Package"))+30;
    int w2 = fm.width("00000")+4;
    list->setColumnWidth(0,w0);
    list->setColumnWidth(1,228-w2-w0); // ### screen-biased
    list->setColumnWidth(2,w2);
    list->setColumnWidthMode(0,QListView::Manual);
    list->setColumnWidthMode(1,QListView::Manual);
    list->setColumnWidthMode(2,QListView::Manual);
    list->setSelectionMode( QListView::Multi );
    details = 0;
    ipkg_old = 0;
    readSettings();
    updatePackageList();
    progress->hide();

    if ( settings->servers->childCount() == 0 ) {
	QTimer::singleShot(0, this, SLOT(doSettings()));
    }
}


/*  
 *  Destroys the object and frees any allocated resources
 */
PackageManager::~PackageManager()
{
    // no need to delete child widgets, Qt does it all for us
}

void PackageManager::newServer()
{
    QCheckListItem* item;
    if ( settings->servername->isEnabled() || settings->serverurl->text().isEmpty() ) {
	item = new QCheckListItem(settings->servers,tr("Package Feed"),QCheckListItem::CheckBox);
	item->setText(1, "http://");
    } else {
	// allows one-level undo
	item = new QCheckListItem(settings->servers,settings->servername->text(),QCheckListItem::CheckBox);
	item->setText(1, settings->serverurl->text());
    }
    item->setOn(TRUE);
    editServer(item);
}

void PackageManager::editServer(QListViewItem* i)
{
    QCheckListItem* item = (QCheckListItem*)i;
    if ( settings->servername->isEnabled() ) {
	disconnect( settings->servername, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)) );
	disconnect( settings->serverurl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)) );
    } else {
	settings->servername->setEnabled(TRUE);
	settings->serverurl->setEnabled(TRUE);
    }

    settings->servername->setText( item->text(0) );
    settings->serverurl->setText( item->text(1) );

    editedserver = item;

    connect( settings->servername, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)) );
    connect( settings->serverurl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)) );
}

void PackageManager::removeServer()
{
    if (settings->servers->childCount() > 0) {
	disconnect( settings->servername, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)) );
	disconnect( settings->serverurl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)) );
	settings->servername->setText(editedserver->text(0));
	settings->serverurl->setText(editedserver->text(1));
	disconnect( settings->servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );
	delete editedserver;
	connect( settings->servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );
	settings->servername->setEnabled(FALSE);
	settings->serverurl->setEnabled(FALSE);
    }
}

void PackageManager::nameChanged(const QString& t)
{
    disconnect( settings->servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );
    editedserver->setText(0,t);
    connect( settings->servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );
}

void PackageManager::urlChanged(const QString& t)
{
    editedserver->setText(1,t);
}

static void selectComboItem(QComboBox *cb, const QString s)
{
    for (int i=0; i<cb->count(); i++) {
	if ( cb->text(i) == s ) {
	    cb->setCurrentItem(i);
	    return;
	}
    }
}

void PackageManager::updatePackageList()
{
    disconnect(section,SIGNAL(activated(int)),this,SLOT(updatePackageList()));
    disconnect(subsection,SIGNAL(activated(int)),this,SLOT(updatePackageList()));

    list->clear();
    QString cursection = section->currentText();
    QString cursubsection = subsection->currentText();
    QString all=tr("All");
    if ( cursection == all ) cursection=QString::null;
    if ( cursubsection == all ) cursubsection=QString::null;
    section->clear();
    subsection->clear();

    QDict<void> sections;
    QDict<void> subsections;
    QDict<void> installed;

    QRegExp separatorRegExp( ":[\t ]+" );

    QString status = ipkgStatusOutput();
    if ( !status.isEmpty() ) {
	QStringList lines = QStringList::split('\n',status,TRUE);
	QString name;
	QString status;
	for (QStringList::Iterator it = lines.begin(); it!=lines.end(); ++it) {
	    QString line = *it;
	    if ( line.length()<=1 ) {
		// EOR
		if ( !name.isEmpty() ) {
		    if ( status.contains(" installed") ) // No tr
			installed.replace(name,(void*)1);
		    name="";
		}
		status="";
	    } else if ( line[0] == ' ' || line[0] == '\t' ) {
		// continuation
	    } else {
		int sep = line.find(separatorRegExp);
		if ( sep >= 0 ) {
		    QString tag = line.left(sep);
		    if ( tag == "Package" ) { // No tr
			name = line.mid(sep+2).simplifyWhiteSpace();
		    } else if ( tag == "Status" ) { // No tr
			status = line.mid(sep+1);
		    }
		}
	    }
	}
    }

    QString info = ipkgInfoOutput();
    if ( !info.isEmpty() ) {
	QStringList lines = QStringList::split('\n',info,TRUE);
	QString description_short;
	QString name;
	QString size;
	QString sec;
	for (QStringList::Iterator it = lines.begin(); it!=lines.end(); ++it) {
	    QString line = *it;
	    if ( line.length()<=1 ) {
		// EOR
		if ( !name.isEmpty() ) {
		    int sl = sec.find('/');
		    QString s = sl < 0 ? sec : sec.left(sl);
		    QString ss = sl < 0 ? QString::null : sec.mid(sl+1);
		    sections.replace(s,(void*)1);
		    if ( cursection.isNull()
			|| cursection == s
			    && (cursubsection.isNull() || cursubsection == ss) )
		    {
			if ( !cursection.isNull() && !ss.isNull() )
			    subsections.replace(ss,(void*)1);
			description_short[0] = description_short[0].upper();
			if ( description_short.left(4) == "The " ) // No tr
			    description_short = description_short.mid(4);
			if ( description_short.left(2) == "A " ) // No tr
			    description_short = description_short.mid(2);
			description_short[0] = description_short[0].upper();
			new PackageItem(list,name,description_short,size,
			       installed.find(name));
		    }
		    installed.remove( name );
		}
		name="";
		size="";
		sec="main";
	    } else if ( line[0] == ' ' || line[0] == '\t' ) {
		// continuation
	    } else {
		int sep = line.find(separatorRegExp);
		if ( sep >= 0 ) {
		    QString tag = line.left(sep);
		    if ( tag == "Package" ) { // No tr
			name = line.mid(sep+2).simplifyWhiteSpace();
		    } else if ( tag == "Description" ) { // No tr
			description_short = line.mid(sep+2).simplifyWhiteSpace();
		    } else if ( tag == "Installed-Size" ) {
			size = line.mid(sep+2).simplifyWhiteSpace();
		    } else if ( tag == "Section" ) { // No tr
			sec = line.mid(sep+2).simplifyWhiteSpace();
		    }
		}
	    }
	}
    }
    if ( installed.count() && cursection.isNull() ) {
	// we have some packages without description
	QDictIterator<void> it( installed );
	for( ; it.current(); ++it )
	    new PackageItem( list, it.currentKey(), "", "?" , TRUE );
    }

    QStringList s;
    QDictIterator<void> it( sections );
    for( ; it.current(); ++it )
	s.append(it.currentKey());
    s.sort();
    section->insertItem(all);
    section->insertStringList(s);
    selectComboItem(section,cursection.isNull()?all:cursection);
    if ( cursection.isNull() ) {
	subsection->setEnabled(FALSE);
    } else {
	subsection->setEnabled(TRUE);
	QStringList s;
	QDictIterator<void> it( subsections );
	for( ; it.current(); ++it )
	    s.append(it.currentKey());
	s.sort();
	subsection->insertItem(all);
	subsection->insertStringList(s);
	selectComboItem(subsection,cursubsection.isNull()?all:cursubsection);
    }

    connect(section,SIGNAL(activated(int)),SLOT(updatePackageList()));
    connect(subsection,SIGNAL(activated(int)),this,SLOT(updatePackageList()));
}

PackageItem* PackageManager::current() const
{
    return (PackageItem*)list->currentItem();
}

/* 
 * public slot
 */
void PackageManager::doCurrentDetails(bool multi)
{
    PackageItem* pit = current();
    if ( pit ) {
	if ( !details ) {
	    details = new PackageDetails;
	    connect( details->install, SIGNAL(clicked()),
		    this, SLOT(installCurrent()));
	    connect( details->remove, SIGNAL(clicked()),
		    this, SLOT(removeCurrent()));
	    details->description->setTextFormat(RichText);
	}
	if ( multi ) {
	    disconnect( details->ignore, SIGNAL(clicked()),
		    details, SLOT(close()));
	    connect( details->ignore, SIGNAL(clicked()),
		    this, SLOT(doNextDetails()));
	} else {
	    disconnect( details->ignore, SIGNAL(clicked()),
		    this, SLOT(doNextDetails()));
	    connect( details->ignore, SIGNAL(clicked()),
		    details, SLOT(close()));
	}
	pit->setSelected(FALSE);
	details->setCaption("Package: " + pit->name()); // No tr
	details->description->setText(fullDetails(pit->name()));
	details->install->setEnabled(!pit->isInstalled());
	details->remove->setEnabled(pit->isInstalled());
	details->showMaximized();
    }
}

void PackageManager::doDetails()
{
    doCurrentDetails(FALSE);
}

void PackageManager::doNextDetails()
{
    QListViewItem* i = list->firstChild();
    for ( ; i; i = i->nextSibling() ) {
	if ( i->isSelected() )
	    break;
    }
    list->setCurrentItem(i);
    if ( i ) {
	doCurrentDetails(TRUE);
    } else if ( details )
	details->close();
}

QString PackageManager::fullDetails(const QString& pk)
{
    QString status;
    Process ipkg_status(QStringList() << "ipkg" << "info" << pk);
    if ( ipkg_status.exec("",status) ) {
	QStringList lines = QStringList::split('\n',status,TRUE);
	QString description;
	for (QStringList::Iterator it = lines.begin(); it!=lines.end(); ++it) {
	    QString line = *it;
	    if ( line == " ." ) {
		description.append("<p>");
	    } else if ( line[0] == ' ' || line[0] == '\t' ) {
		// continuation
		description.append(" ");
		description.append(Qtopia::escapeString(line));
	    } else {
		int sep = line.find(QRegExp(":[\t ]+"));
		if ( sep >= 0 ) {
		    QString tag = line.left(sep);
		    description.append("<br>");
		    description.append("<b>");
		    description.append(Qtopia::escapeString(tag));
		    description.append(":</b> ");
		    description.append(Qtopia::escapeString(line.mid(sep+2)));
		} else {
		    description.append(" ");
		    description.append(Qtopia::escapeString(line));
		}
	    }
	}
	return description;
    }

    return QString::null;
}

void PackageManager::installCurrent()
{
    current()->setOn(TRUE);
    details->close();
}

void PackageManager::removeCurrent()
{
    current()->setOn(TRUE);
    details->close();
}

bool PackageManager::readIpkgConfig(const QString& conffile)
{
    QFile conf(conffile);
    if ( conf.open(IO_ReadOnly) ) {
	QTextStream s(&conf);
	settings->servers->clear();
	ipkg_old=0;
	QCheckListItem* currentserver=0;
	while ( !s.atEnd() ) {
	    QString l = s.readLine();
	    QStringList token = QStringList::split(' ', l);
	    if ( token[0] == "src" || token[0] == "#src" ) {
		// script-based ipkg cannot handles spaces
		QString name = token[1];
		for (int i=0; i<(int)name.length(); ++i)
		    if ( name[i]=='_' ) name[i] = ' ';
		QCheckListItem* item = new QCheckListItem(settings->servers,name,QCheckListItem::CheckBox);
		currentserver=item;
		item->setText(1, token[2]);
		item->setOn( token[0] == "src" );
	    } else if ( token[0] == "dest" ) {
		// needs UI
	    } else if ( token[0] == "option" ) { // No tr
		// ### somehow need to use the settings from netsetup
// 		if ( token[1] == "http_proxy" )
// 		    settings->http->setText(token[2]);
// 		else if ( token[1] == "ftp_proxy" )
// 		    settings->ftp->setText(token[2]);
// 		else if ( token[1] == "proxy_username" )
// 		    settings->username->setText(token[2]);
// 		else if ( token[1] == "proxy_password" )
// 		    settings->password->setText(token[2]);
	    } else {
		// Old style?
		int eq = l.find('=');
		if ( eq >= 0 ) {
		    QString v = l.mid(eq+1).stripWhiteSpace();
		    if ( v[0] == '"' || v[0] == '\'' ) {
			int cl=v.find(v[0],1);
			if ( cl >= 0 )
			    v = v.mid(1,cl-1);
		    }
		    if ( l.left(12) == "IPKG_SOURCE=" ) {
			ipkg_old=1;
			QCheckListItem* item = new QCheckListItem(settings->servers,v,QCheckListItem::CheckBox);
			currentserver=item;
			item->setText(1, v);
		    } else if ( l.left(13) == "#IPKG_SOURCE=" ) {
			QCheckListItem* item = new QCheckListItem(settings->servers,v,QCheckListItem::CheckBox);
			item->setText(1, v);
		    } else if ( l.left(10) == "IPKG_ROOT=" ) {
			// ### no UI
// 		    } else if ( l.left(20) == "IPKG_PROXY_USERNAME=" ) {
// 			settings->username->setText(v);
// 		    } else if ( l.left(20) == "IPKG_PROXY_PASSWORD=" ) {
// 			settings->password->setText(v);
// 		    } else if ( l.left(16) == "IPKG_PROXY_HTTP=" ) {
// 			settings->http->setText(v);
// 		    } else if ( l.left(16) == "IPKG_PROXY_FTP=" ) {
// 			settings->ftp->setText(v);
		    }
		}
	    }
	}
	if ( ipkg_old ) {
	    // Should be radiobuttons
	    if ( currentserver )
		currentserver->setOn(TRUE);
	}
	return TRUE;
    } else {
	return FALSE;
    }
}

/* 
 * public slot
 */
void PackageManager::doSettings()
{
    settings->showMaximized();
    if ( settings->exec() ) {
	writeSettings();
	startRun();
	runIpkg("update");
	endRun();
	updatePackageList();
    } else {
	readSettings();
    }
}

void PackageManager::readSettings()
{
    // read from config file(s)
    readIpkgConfig("/etc/ipkg.conf");
}

void PackageManager::writeSettings()
{
    QFile conf("/etc/ipkg.conf");
    if ( conf.open(IO_WriteOnly) ) {
	QTextStream s(&conf);
	s << "# Written by Qtopia Package Manager\n";
	if ( !ipkg_old ) {
	    for (QListViewItem* i = settings->servers->firstChild(); i; i = i->nextSibling()) {
		QCheckListItem* item = (QCheckListItem*)i;
		if ( !item->isOn() )
		    s << "#";
		QString name = item->text(0);
		for (int i=0; i<(int)name.length(); ++i)
		    if ( name[i]==' ' ) name[i] = '_';
		s << "src " << name << " " << item->text(1) << "\n";
	    }
	    s << "dest root /\n"; // ### need UI
// 	    if ( !settings->username->text().isEmpty() )
// 		s << "option proxy_username " << settings->username->text() << "\n";
// 	    if ( !settings->password->text().isEmpty() )
// 		s << "option proxy_password " << settings->password->text() << "\n";
// 	    if ( !settings->http->text().isEmpty() )
// 		s << "option http_proxy " << settings->http->text() << "\n";
// 	    if ( !settings->ftp->text().isEmpty() )
// 		s << "option ftp_proxy " << settings->ftp->text() << "\n";
	} else {
	    // Old style
	    bool src_selected=FALSE;
	    for (QListViewItem* i = settings->servers->firstChild(); i; i = i->nextSibling()) {
		QCheckListItem* item = (QCheckListItem*)i;
		if ( item->isOn() )
		    src_selected=TRUE;
		else
		    s << "#";
		s << "IPKG_SOURCE=\"" << item->text(1) << "\"\n";
	    }
	    s << "IPKG_ROOT=/\n"
// 	      << "IPKG_PROXY_USERNAME=\"" << settings->username->text() << "\"\n"
// 	      << "IPKG_PROXY_PASSWORD=\"" << settings->password->text() << "\"\n"
// 	      << "IPKG_PROXY_HTTP=\"" << settings->http->text() << "\"\n"
// 	      << "IPKG_PROXY_FTP=\"" << settings->ftp->text() << "\"\n"
	    ;
	}
	conf.close();
    }
}

/* 
 * public slot
 */
void PackageManager::doFind()
{
    Search s(this, 0, TRUE);
    s.setMaximumWidth(QApplication::desktop()->width());
    if ( s.exec() ) {
	QString p = s.pattern->text();
	if ( p.isEmpty() ) {
	    list->selectAll(FALSE);
	} else {
	    selectPackages(findPackages(p));
	    doNextDetails();
	}
    }
}

void PackageManager::selectPackages( const QStringList& l )
{
    QDict<void> d;
    for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
	d.replace(*it,(void*)1);
    QListViewItem* i;
    for ( i = list->firstChild(); i; i = i->nextSibling() ) {
	PackageItem* pit = (PackageItem*)i;
	i->setSelected( d[pit->name()] );
    }
}

QStringList PackageManager::findPackages( const QRegExp& r )
{
    QStringList matches;

    QString info = ipkgInfoOutput();
    if ( !info.isEmpty() ) {
	QStringList lines = QStringList::split('\n',info,TRUE);
	QRegExp re = r;
	QString description="";
	QString name;
	for (QStringList::Iterator it = lines.begin(); it!=lines.end(); ++it) {
	    QString line = *it;
	    if ( line.length()<=1 ) {
		// EOR
		if ( re.match(description) >= 0 )
		    matches.append(name);
		description="";
		name="";
	    } else if ( line[0] == ' ' || line[0] == '\t' ) {
		// continuation
		description.append(" ");
		description.append(Qtopia::escapeString(line));
	    } else {
		int sep = line.find(QRegExp(":[\t ]+"));
		if ( sep >= 0 ) {
		    QString tag = line.left(sep);
		    if ( tag == "Package" )
			name = line.mid(sep+2).simplifyWhiteSpace();
		    if ( !description.isEmpty() )
			description.append("<br>");
		    description.append("<b>");
		    description.append(Qtopia::escapeString(tag));
		    description.append(":</b> ");
		    description.append(Qtopia::escapeString(line.mid(sep+2)));
		}
	    }
	}
    }

    return matches;
}

QString PackageManager::getDestination( bool *ok, bool alwaysprompt )
{
    StorageInfo storage;
    const QList<FileSystem> &fs = storage.fileSystems();
    QListIterator<FileSystem> it ( fs );

    
    QDialog *dialog = new QDialog( this, "dest", TRUE );
    QVBoxLayout *topl = new QVBoxLayout( dialog );
    QVButtonGroup *bg = new QVButtonGroup( tr("Install to"), dialog );
    topl->addWidget( bg );
    QRadioButton *b1 = new QRadioButton( tr( "Internal storage" ), bg );
    b1->setChecked( TRUE );
    
    bool prompt = alwaysprompt;
    for ( ;it.current(); ++it ) {
	if ( (*it)->isRemovable() ) {
	    (void)new QRadioButton( (*it)->name(), bg );
	    prompt = TRUE;
	}
    }
	
    if ( !prompt || dialog->exec() ) {
	QButton *b = bg->selected();
	if ( b ) {
	    if ( ok ) 
		*ok = TRUE;
	    if ( b == b1 )
		return QString::null;
	    it.toFirst();
	    for ( ;it.current(); ++it ) {
		if ( (*it)->name() == b->text() )
		    return (*it)->path() + "/Qtopia";		
	    }
	}
    }
	
    if ( ok ) 
	*ok = FALSE;
    return QString::null;
}

/*!
  Install \a ipk to \a location. Returns FALSE on error, TRUE otherwise.
  */

bool PackageManager::installIpkg( const QString &ipk, const QString &location )
{
    //qDebug( "installing %s to '%s'", ipk.ascii(), location.ascii() );
    QString cmd;
    QStringList orig_packages;
    if ( !location.isEmpty() ) {
        mkdir( location.ascii(), 0777 );
	cmd += "-d " + location + " ";
	// get list of *.list in location/usr/lib/ipkg/info/*.list
	QDir dir(location + "/usr/lib/ipkg/info", "*.list", 
		QDir::Name, QDir::Files);

	orig_packages = dir.entryList();
    }
    cmd += "install " + ipk;
    qDebug( "running '%s'", cmd.ascii() );
    int r = runIpkg( cmd );
    if ( !location.isEmpty() ) {
	//symlink like crazy:
	
	QDir dir(location + "/usr/lib/ipkg/info", "*.list", 
		QDir::Name, QDir::Files);

	QStringList new_packages = dir.entryList();

	// for each file in new_packages but not in orig_packages, 

	for ( QStringList::Iterator it = orig_packages.begin()
		; it != orig_packages.end(); ++it ) {
	    if (new_packages.contains(*it)) {
		new_packages.remove(*it);
	    }
	}

	// for each of the new ones:
	//    for each filename in the .list file:
	//   	if filename ends with '/'
	//	    mkdir -p filename
	//	else
	//	    ln -s location+filename filename
	for ( QStringList::Iterator it = new_packages.begin()
		; it != new_packages.end(); ++it ) {
	    QFile f(location + "/usr/lib/ipkg/info/" + (*it));
	    //make a copy so we can remove the symlinks later
	    mkdir( (" /usr/lib/ipkg/info/"+location).ascii(), 0777 );
	    system(("cp " + f.name() + " /usr/lib/ipkg/info/"+location).ascii());
	    
	    if ( f.open(IO_ReadOnly) ) {   
		QTextStream ts(&f);

		QString s;
		while ( !ts.eof() ) {        // until end of file...
		    s = ts.readLine();       // line of text excluding '\n'
		    // for s, do link/mkdir.
		    if ( s.right(1) == "/" ) {
			qDebug("do mkdir for %s", s.ascii());
			mkdir( s.ascii(), 0777 );
			//possible optimization: symlink directories
			//that don't exist already. -- Risky.
		    } else {
			qDebug("do symlink for %s", s.ascii());
			symlink( (location+s).ascii(), s.ascii() );
		    }
		}
		f.close();
	    }
	}
    }
    return !r;
}

/* 
 * public slot
 */
void PackageManager::doUpgrade()
{
    startMultiRun(2);
    runIpkg("update");
    runIpkg("upgrade");
    updatePackageList();
    endRun();
}


void PackageManager::doIt()
{
    bool ok = commitWithIpkg();
    updatePackageList(); // things may have changed
    if (!ok) qApp->beep();
}

bool PackageManager::commitWithIpkg()
{
    // A full implementation would do the following, but we'll just do
    // it simply and non-interactively for now.
    //
    // setenv IPKG_CONF_DIR for a null $IPKG_CONF_DIR/ipkg.conf
    // setenv IPKG_SOURCE, IPKG_ROOT, etc.
    // run ipkg, processing interactivity as dialogs
    //   - "... (Y/I/N/O/D) [default=N] ?"  -> ...
    //   - "[Press ENTER to continue]" (if D chosen above)
    //   - "The following packages are marked `Essential'... Install them now [Y/n] ?"
    //   - "The following packages...ready to be installed:... Install them now [Y/n] ?"
    // return FALSE cancelled

    QStringList to_remove, to_install;

    for ( QListViewItem* i = list->firstChild(); i; i = i->nextSibling() ) {
	PackageItem* pit = (PackageItem*)i;
	if ( pit->isOn() ) {
	    if ( pit->isInstalled() )
		to_remove.append(pit->name());
	    else
		to_install.append(pit->name());
	}
    }

    bool ok=TRUE;

    int jobs = to_remove.count()+to_install.count();
    if ( jobs ) {
	startMultiRun(jobs);

	if ( to_remove.count() ) {
	    for (QStringList::ConstIterator it=to_remove.begin(); it!=to_remove.end(); ++it) {
		if ( runIpkg("remove " + *it) != 0 ) {
		    ok = FALSE;
		}
	    }
	}
	if ( to_install.count() ) {
	    bool doInstall;
	    QString location = getDestination( &doInstall, FALSE );
	    if ( doInstall ) {
		for (QStringList::ConstIterator it=to_install.begin(); it!=to_install.end(); ++it) {
		    if ( !installIpkg( *it, location ) )
			ok = FALSE;
		}
	    }
	}

	// ##### If we looked in the list of files, we could send out accurate
	// ##### messages. But we don't bother yet, and just do an "all".
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/System", "linkChanged(QString)");
	QString lf = QString::null;
	e << lf;
#endif

#if QT_VERSION > 230 // a bug breaks this otherwise
	if ( !ok )
	    QMessageBox::warning(this, "Error", "<p><tt>ipkg</tt> says something went wrong. Sorry.");
#endif

	endRun();
    }

    return ok;
}

QString PackageManager::ipkgStatusOutput()
{
    if ( cachedIpkgStatusOutput.isEmpty() ) {
	Process ipkg_status( QStringList() << "ipkg" << "status" );
	cachedIpkgStatusOutput.detach();
	ipkg_status.exec( 0, cachedIpkgStatusOutput );
    }
    return QString::fromLocal8Bit( cachedIpkgStatusOutput );
}

QString PackageManager::ipkgInfoOutput()
{
    if ( cachedIpkgInfoOutput.isEmpty() ) {
	Process ipkg_info( QStringList() << "ipkg" << "info" );
	cachedIpkgInfoOutput.detach();
	ipkg_info.exec( 0, cachedIpkgInfoOutput );
    }
    return QString::fromLocal8Bit( cachedIpkgInfoOutput );
}

void PackageManager::setCachedIpkgOutputDirty()
{
    cachedIpkgStatusOutput = cachedIpkgInfoOutput = "";
}

void PackageManager::startMultiRun(int jobs)
{
    startRun();
    progress->setTotalSteps(jobs);
    progress->setProgress(0);
}

void PackageManager::startRun()
{
    progress->show();
    doit->hide();
}

void PackageManager::endRun()
{
    doit->show();
    progress->hide();
}

int PackageManager::runIpkg(const QString& args)
{
    if ( progress->progress() == -1 )
	startMultiRun(1);
    else
	startRun();
    QString cmd = "ipkg ";
    if ( ipkg_old )
	cmd += "</dev/null ";
    else
	cmd += "-force-defaults ";
    int r = system((cmd+args).latin1());
    progress->setProgress(progress->progress()+1);
    setCachedIpkgOutputDirty();
    endRun();
    return r;
}

// simple hack to get support for ipkg mimetype
void PackageManager::maybeInstall( const QString &ipk )
{
    int pos = ipk.findRev( "/" );
    QString package = ipk.mid( pos + 1 );
    pos = package.find ( "_" );
    if ( pos != -1 )
	package = package.left( pos );
    
    bool ok;
    QString dest = getDestination( &ok, TRUE );
    if ( ok ) {
	startRun();
	installIpkg( ipk, dest );
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/System", "linkChanged(QString)");
	QString lf = QString::null;
	e << lf;
#endif
	endRun();
    }
}

void PackageManager::setDocument(const QString& fileref)
{
    if ( fileref.isNull() )
	return;
    DocLnk doc( fileref );
    if ( doc.file().isEmpty() )
	return;
    maybeInstall( doc.file() );
    updatePackageList();
}
