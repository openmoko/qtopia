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
#include "packagewizard.h"
#include "pkdesc.h"

#include <qtopia/process.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>
#include <qtopia/stringutil.h>
#include <qtopia/locationcombo.h>
#include <qtopia/qpeapplication.h>
#ifdef QWS
#include <qtopia/qcopenvelope_qws.h>
#endif
#include <qtopia/applnk.h>
#include <qtopia/qprocess.h>

#include <qtopia/storage.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qmultilineedit.h>
#include <qlayout.h>

#include <qprogressbar.h>
#include <qcombobox.h>
#include <qstylesheet.h>
#include <qdict.h>
#include <qfile.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
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

//#define IPKGSIM

#ifdef IPKGSIM
static const char *ipkg_name = "ipkgsim";
#else
static const char *ipkg_name = "ipkg";
#endif

static QString packageId(const QString& name_or_file)
{
    // clean up pkg.  If we have a whole file, we want just the name of the package.
    // If we have a _VER_ARCH.ipk ending, try to slim down the name to what
    // the package name probably is.
    int start = name_or_file.find(QRegExp("[^/_]+_[^/]*_[^/_]*.ipk$"));
    int end = name_or_file.find(QRegExp("_[^/]*_[^/_]*.ipk$"));
    if (start > 0 && end > start)
	return name_or_file.mid(start, end - start);
    else
	return name_or_file;
}
/*
class DetailsPopup : public QPopupMenu {
    QLabel* label;
public:
    DetailsPopup(QWidget* parent) : QPopupMenu(parent)
    {
	label = new QLabel(this);
	label->setTextFormat(RichText);
	insertItem(label);
    }

    void setText(const QString& s)
    {
	label->setText(s);
    }
};
*/
class ServerListItem : public QCheckListItem {
    bool isdup;
public:
    ServerListItem(QListView *parent, const QString &text) :
	QCheckListItem(parent,text,CheckBox),
	isdup(FALSE)
    {
    }

    void setDup(bool y)
    {
	if ( isdup != y ) {
	    isdup = y;
	    repaint();
	}
    }

    void paintCell( QPainter *p, const QColorGroup & cg,
                            int column, int width, int alignment )
    {
	QColorGroup c = cg;
	if ( column==0 && isdup ) c.setColor(QColorGroup::Text,red);
	QCheckListItem::paintCell(p,c,column,width,alignment);
    }

    void setName(const QString& n) { setText(0,n); }
    QString name() const { return text(0); }
    static QString id(const QString& nm)
    {
	QString n = nm;
	for (int i=0; i<(int)n.length(); ++i)
	    if ( n[i]==' ' ) n[i] = '_';
	return n;
    }
    QString id() const
    {
	return id(name());
    }

    void setOn(bool y)
    {
	// Force listview's signals
	listView()->setSelected(this,FALSE);
	QCheckListItem::setOn(y);
	listView()->setSelected(this,y);
    }
};

static bool parseInfo(const QStringList& lines, QString& name, QString& description_short, QString& description, QString& size, QString& sec)
{
    QRegExp separatorRegExp( ":[\t ]+" );
    bool in_desc=FALSE;
    for (QStringList::ConstIterator it = lines.begin(); it!=lines.end(); ++it) {
	QString line = *it;
	if ( line[0] == ' ' || line[0] == '\t' ) {
	    // continuation
	    if ( in_desc )
		description+=line;
	} else {
	    int sep = line.find(separatorRegExp);
	    if ( sep >= 0 ) {
		QString tag = line.left(sep);
		in_desc=FALSE;
		if ( tag == "Package" ) { // No tr
		    name = line.mid(sep+2).simplifyWhiteSpace();
		} else if ( tag == "Description" ) { // No tr
		    description_short = line.mid(sep+2).simplifyWhiteSpace();
		    in_desc = TRUE;
		} else if ( tag == "Installed-Size" ) {
		    size = line.mid(sep+2).simplifyWhiteSpace();
		} else if ( tag == "Section" ) { // No tr
		    sec = line.mid(sep+2).simplifyWhiteSpace();
		}
		description += line + "<p>";
	    }
	}
    }
    if ( !name.isEmpty() ) {
	description_short[0] = description_short[0].upper();
	if ( description_short.left(4) == "The " ) // No tr
	    description_short = description_short.mid(4);
	if ( description_short.left(2) == "A " ) // No tr
	    description_short = description_short.mid(2);
	description_short[0] = description_short[0].upper();
	return TRUE;
    }
    return FALSE;
}


class PackageItem : public QListViewItem {
    bool installed;
    DocLnk link;
    QString full;
public:
    PackageItem(QListView* lv, const QString& name, const QString& desc, const QString& fulld, const QString& size, bool inst ) :
	QListViewItem(lv,name), installed(inst), full(fulld)
    {
	setText(1,desc);
	setText(2,size);
    }

    PackageItem(QListView* lv, const DocLnk& lnk, bool inst ) :
	QListViewItem(lv,packageId(lnk.file())), installed(inst),
	link(lnk)
    {
    }

    QString id() const
    {
	if ( link.fileKnown() ) {
	    return file();
	} else {
	    return text(0);
	}
    }

    QString file() const
    {
	return link.file();
    }

    void setInfo(const QString& c)
    {
	parseInfo(c);
    }

    bool parseInfo()
    {
	return parseInfo(link.comment());
    }

    bool parseInfo(const QString& comment)
    {
	QStringList info = QStringList::split("\n",comment);
	QString name;
	QString description_short;
	QString description;
	QString size;
	QString sec;
	if ( ::parseInfo(info, name, description_short, description, size, sec ) ) {
	    setName(name);
	    setDescription(description_short);
	    setFullDescription(description);
	    setSize(size);
	    return TRUE;
	} else {
	    setDescription("?");
	    setFullDescription("?");
	    setSize("?");
	    return FALSE;
	}
    }

    void setName(const QString& txt)
    {
	setText(0, txt);
    }

    void setDescription(const QString& txt)
    {
	setText(1, txt);
    }

    QString fullDescription() const { return full; }

    void setFullDescription(const QString& txt)
    {
	full = txt;
    }

    void setSize(const QString& txt)
    {
	setText(2, txt);
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

    QPixmap statePixmap() const
    {
	if ( !isSelected() ) {
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

// ====================================================================

class LabelMaker
{
public:
    LabelMaker(QLabel *lbl)
	: mLbl( lbl ), message(), extraPackages( 0 ), tooManyPackages( FALSE )
    {
	QWidget *gparent = mLbl->parentWidget()->parentWidget();
	int gpheight = gparent->geometry().height();
	int fheight = QFontMetrics(QFont()).height();
	maxheight = gpheight - (3 * fheight);
    }

    void addString(QString s)
    {
	message += s;
	mLbl->setText( message );
    }

    void addPackage(QString s)
    {
	if ( tooManyPackages ) {
	    oneMorePackage();
	    return;
	}
	QString adding = QString("<li>%1").arg(s);
	uint curSize = message.length();
	addString(adding);
	mLbl->adjustSize();
	if ( mLbl->geometry().height() >= maxheight ) {
	    // oops, we're too long, take off that last line
	    moreIndex = curSize;
	    oneMorePackage();
	    tooManyPackages = TRUE;
	}
    }

private:
    void oneMorePackage()
    {
	extraPackages++;
	message.truncate( moreIndex );
	addString( QString("<li>%1 more packages").arg( extraPackages ) );
    }

    QLabel *mLbl;
    QString message;
    int maxheight;
    uint extraPackages;
    uint moreIndex;
    bool tooManyPackages;
};

// ====================================================================

/* 
 *  Constructs a PackageWizard that is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f' 
 */
PackageWizard::PackageWizard( QWidget* parent,  const char* name, WFlags fl )
    : PackageWizardBase( parent, name, fl )
{
    infoProcess = 0;
    committed = FALSE;
    installedRootDict = 0;
    
    connect( newserver, SIGNAL(clicked()), this, SLOT(newServer()) );
    connect( removeserver, SIGNAL(clicked()), this, SLOT(removeServer()) );
    connect( servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );
    servername->setEnabled(FALSE);
    serverurl->setEnabled(FALSE);

    if (!pm_uninstalled) {
	pm_uninstalled = new QPixmap(Resource::loadPixmap("uninstalled"));
	pm_installed = new QPixmap(Resource::loadPixmap("installed"));
	pm_install = new QPixmap(Resource::loadPixmap("install"));
	pm_uninstall = new QPixmap(Resource::loadPixmap("uninstall"));
    }

    packagelist->setColumnWidthMode(0,QListView::Manual);
    packagelist->setColumnWidthMode(1,QListView::Manual);
    packagelist->setColumnWidthMode(2,QListView::Manual);
    packagelist->setColumnAlignment(2,AlignRight);
    packagelist->setSelectionMode( QListView::Multi );
    packagelist->setVScrollBarMode(QScrollView::AlwaysOn);
    packagelist->installEventFilter(this);
    packagelist->setShowSortIndicator(TRUE);

    ipkg_old = 0;
    readSettings();
    updateServerSelection();
    progress_net->hide();
    progress_confirm->hide();
    QPEApplication::setStylusOperation(packagelist->viewport(),QPEApplication::RightOnHold);
    //package_description = new DetailsPopup(this);
    delete mode_share; // ### Not implemented yet

    setHelpEnabled(page_mode,FALSE);
    setHelpEnabled(page_servers,FALSE);
    setHelpEnabled(page_packages,FALSE);
    setHelpEnabled(page_share,FALSE);
    setHelpEnabled(page_location,FALSE);
    setHelpEnabled(page_confirmation,FALSE);
}


/*  
 *  Destroys the object and frees any allocated resources
 */
PackageWizard::~PackageWizard()
{
    delete installedRootDict;
    // no need to delete child widgets, Qt does it all for us
}

QProgressBar* PackageWizard::progress()
{
    if ( currentPage() == page_confirmation )
	return progress_confirm;
    else
	return progress_net;
}

void PackageWizard::setAppropriates()
{
    setNextEnabled( page_mode, !!mode_choice->selected() );
    setNextEnabled( page_share, !!share_choice->selected() );
    updatePackageNext();

    setFinishEnabled( page_confirmation, TRUE );

    setAppropriate( page_mode, qcopDocument.isEmpty() );
    setAppropriate( page_servers, !committed && mode_net->isChecked() );
    setAppropriate( page_packages, !committed && qcopDocument.isEmpty() );
    bool need_loc = !mode_rm->isChecked();
    /*
    if ( mode_share->isChecked() ) {
	// If sharing to a Document, need location
	// ...
    }
    */
    setAppropriate( page_location, !committed && need_loc );
    setAppropriate( page_share, FALSE ); // ( !committed && mode_share->isChecked() );
}

void PackageWizard::updatePackageNext()
{
    bool any = FALSE;
    for (QListViewItem* i = packagelist->firstChild(); !any && i; i = i->nextSibling()) {
	PackageItem* item = (PackageItem*)i;
	if ( item->isSelected() )
	    any = TRUE;
    }
    setNextEnabled( page_packages, any );
    PackageItem* cur = (PackageItem*)packagelist->currentItem();
    details->setEnabled(!!cur);
}

void PackageWizard::showDetails()
{
    PackageItem* cur = (PackageItem*)packagelist->currentItem();
    if ( cur ) {
	QString d = cur->fullDescription();

	if ( d.isEmpty() ) d = tr("No details.");
	PackageDetails det(this,0,TRUE);
	det.setCaption(tr("Package: %1").arg(cur->name()));
	det.description->setText(d);
	det.showMaximized();
	cur->setSelected(det.exec());
	updatePackageNext();
    }
}

void PackageWizard::updateServerSelection()
{
    int nservers = 0;
    QDict<void> dict;
    bool dups=FALSE;
    for (QListViewItem* i = servers->firstChild(); i; i = i->nextSibling()) {
	ServerListItem* item = (ServerListItem*)i;
	if ( dict.find(item->id()) ) {
	    dups = TRUE;
	    item->setDup(TRUE);
	} else {
	    item->setDup(FALSE);
	}
	dict.insert(item->id(),(void*)1);
	if ( item->isOn() )
	    nservers++;
    }
    setNextEnabled( page_servers, nservers>0 && !dups );
}

void PackageWizard::showPage( QWidget* w )
{
    if ( w != currentPage() ) {
	committed = FALSE;
	if ( w == page_servers ) {
	} else if ( w == page_packages ) {
	    if ( currentPage() == page_mode
	      || currentPage() == page_servers ) {
		// Slow, show page now
		packagelist->clear();
		PackageWizardBase::showPage(w);
		qApp->processEvents();
		updatePackageList();
	    }
	} else if ( w == page_share ) {
	} else if ( w == page_location ) {
	    if ( location->fileSystem() ) {
		setNextEnabled( page_location, TRUE );
	    } else {
		setNextEnabled( page_location, FALSE );
	    }
	} else if ( w == page_confirmation ) {
	    LabelMaker lm( confirmation );
	    lm.addString( "<P>" );
	    if ( mode_doc->isChecked() || mode_net->isChecked() ) {
		if ( mode_doc->isChecked() )
		    lm.addString( tr("Installing on %1","eg. SD card").arg(location->fileSystem()->name()) );
		else
		    lm.addString( tr("Installing from network to %1","eg. SD card").arg(location->fileSystem()->name()) );
		lm.addString( "<ul>" );
		for (QListViewItem* i = packagelist->firstChild(); i; i = i->nextSibling()) {
		    PackageItem* item = (PackageItem*)i;
		    if ( item->isSelected() )
			lm.addPackage( item->name() );
		}
		lm.addString( "</ul>" );
		finishButton()->setText(tr("Install"));
	    } else if ( mode_rm->isChecked() ) {
		lm.addString( tr("Uninstall:") );
		lm.addString( "<ul>" );
		for (QListViewItem* i = packagelist->firstChild(); i; i = i->nextSibling()) {
		    PackageItem* item = (PackageItem*)i;
		    if ( item->isSelected() )
			lm.addPackage( item->name() );
		}
		lm.addString( "</ul>" );
		finishButton()->setText(tr("Remove"));
	    } else {
		// Sharing not implemented
	    }
	}

	setAppropriates();
    }
    PackageWizardBase::showPage(w);
}

bool PackageWizard::eventFilter( QObject * o, QEvent *e )
{
    if ( o == packagelist && e->type()==QEvent::Resize ) {
	QFontMetrics fm = packagelist->fontMetrics();
	int w0 = fm.width("xxxxxx")+packagelist->width()/6;
	int w2 = fm.width("0000000")+4;
	int w1 = packagelist->width()-w2-w0-packagelist->frameWidth()*2-
	    packagelist->verticalScrollBar()->width();
	packagelist->setColumnWidth(0,w0);
	packagelist->setColumnWidth(1,w1);
	packagelist->setColumnWidth(2,w2);
    }
    return FALSE;
}

void PackageWizard::newServer()
{
    ServerListItem* item;
    if ( servername->isEnabled() || serverurl->text().isEmpty() ) {
	int n=1;
	bool dup=TRUE;
	QString nm;
	while (dup) {
	    nm = tr("Package Feed");
	    if ( n>1 )
		nm += " "+QString::number(n);
	    dup = FALSE;
	    for (QListViewItem* i = servers->firstChild(); i; i = i->nextSibling()) {
		ServerListItem* oitem = (ServerListItem*)i;
		if ( ServerListItem::id(nm)==oitem->id() )
		    dup = TRUE;
	    }
	    n++;
	}
	item = new ServerListItem(servers,nm);
	item->setText(0, nm);
	item->setText(1, "http://");
    } else {
	// allows one-level undo
	item = new ServerListItem(servers,servername->text());
	item->setText(1, serverurl->text());
    }
    item->setOn(TRUE);
    editServer(item);
}

void PackageWizard::editServer(QListViewItem* i)
{
    ServerListItem* item = (ServerListItem*)i;
    if ( servername->isEnabled() ) {
	disconnect( servername, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)) );
	disconnect( serverurl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)) );
    } else {
	servername->setEnabled(TRUE);
	serverurl->setEnabled(TRUE);
    }

    servername->setText( item->text(0) );
    serverurl->setText( item->text(1) );

    editedserver = item;

    connect( servername, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)) );
    connect( serverurl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)) );
}

void PackageWizard::removeServer()
{
    if (servers->childCount() > 0) {
	disconnect( servername, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)) );
	disconnect( serverurl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)) );
	disconnect( servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );

	//
	// Point editserver at something legitimate.
	//
	delete editedserver;
	if (!servers->selectedItem()) {
	    servers->setSelected(servers->firstChild(), TRUE);
	}
	editedserver = (ServerListItem*)servers->selectedItem();

	connect( servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );

	//
	// Disable edits if there are no more servers.
	//
	if (!editedserver) {
	    servername->clear();
	    serverurl->clear();
	    servername->setEnabled(FALSE);
	    serverurl->setEnabled(FALSE);
	} else {
	    servername->setText(editedserver->text(0));
	    serverurl->setText(editedserver->text(1));
	}
    }
}

void PackageWizard::nameChanged(const QString& t)
{
    disconnect( servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );
    editedserver->setText(0,t);
    connect( servers, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(editServer(QListViewItem*)) );
    updateServerSelection();
}

void PackageWizard::urlChanged(const QString& t)
{
    editedserver->setText(1,t);
}

/*
static void selectComboItem(QComboBox *cb, const QString s)
{
    for (int i=0; i<cb->count(); i++) {
	if ( cb->text(i) == s ) {
	    cb->setCurrentItem(i);
	    return;
	}
    }
}
*/

void PackageWizard::updatePackageList()
{
    packagelist->clear();

    if ( mode_doc->isChecked() ) {
	insertLocalPackageItems();
	setNextEnabled( page_packages, FALSE );
    } else if ( mode_net->isChecked() ) {
	writeSettings();
	QString out;
	runIpkg("update",out); // No tr
	insertPackageItems(FALSE);
    } else {
	insertPackageItems(TRUE);
    }
}

void PackageWizard::insertLocalPackageItems()
{
    QDict<void> installed = installedPackages();
    DocLnkSet docs;
    Global::findDocuments(&docs,"application/ipkg");
    for (QListIterator<DocLnk> it(docs.children()); it.current(); ++it) {
	const DocLnk& lnk = **it;
	// only show un-installed packages.
	if (!installed.find(packageId(lnk.file()))) {
	    PackageItem* item = new PackageItem(packagelist,lnk,
		    FALSE);
	    if ( !item->parseInfo() ) {
		infoPending.append(item);
		if ( infoPending.count() == 1 )
		    startInfoProcess();
	    }
	}
    }
}

void PackageWizard::startInfoProcess()
{
    if ( infoPending.count() ) {
	if ( !infoProcess ) {
	    infoProcess = new QProcess(this);
	    connect(infoProcess, SIGNAL(readyReadStdout()), this, SLOT(infoReadyRead()));
	    connect(infoProcess, SIGNAL(readyReadStderr()), this, SLOT(infoReadError()));
	    connect(infoProcess, SIGNAL(processExited()), this, SLOT(infoDone()));
	}
	info="";
	infoProcess->clearArguments();
	infoProcess->addArgument("qipkg-info-file");
	PackageItem* item = (PackageItem*)infoPending.at(0);
	infoProcess->addArgument(item->file());
	infoProcess->start();
	infoProcess->setPriority(infoProcess->priority()+20); // dont hog CPU
    }
}

void PackageWizard::infoReadyRead()
{
    while ( infoProcess->canReadLineStdout() ) {
        QString f = infoProcess->readLineStdout();
	info.append("\n");
	info.append(f);
    }
}

void PackageWizard::infoReadError()
{
    // blabber from tar
}

void PackageWizard::infoDone()
{
    PackageItem* item = (PackageItem*)infoPending.take(0);
    item->setInfo(info);
    startInfoProcess();
}

QDict<void> PackageWizard::installedPackages()
{
    QDict<void> installed;
    QString status = ipkgStatusOutput();
    delete installedRootDict;
    installedRootDict = 0;
    if ( !status.isEmpty() ) {
	QRegExp separatorRegExp( ":[\t ]+" );
	QStringList lines = QStringList::split('\n',status,TRUE);
	QString name;
	QString status;
	QString root;
	for (QStringList::Iterator it = lines.begin(); it!=lines.end(); ++it) {
	    QString line = *it;
	    if ( line.length()<=1 ) {
		// EOR
		if ( !name.isEmpty() ) {
		    if ( status.contains(" installed") ) { // No tr
			installed.replace(name,(void*)1);
			if ( !root.isEmpty() && root != "/" ) {
			    if ( !installedRootDict ) {
				installedRootDict = new QDict<QString>;
				installedRootDict->setAutoDelete( TRUE );
			    }
			    installedRootDict->replace( name, new QString(root) );
			}
		    }
		    name=QString::null;
		}
		status=QString::null;
		root=QString::null;
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
		    } else if ( tag == "Root" )  { // No tr
			root = line.mid(sep+2).simplifyWhiteSpace();
		    }
		}
	    }
	}
    }
    return installed;
}

void PackageWizard::insertPackageItems(bool installed_only)
{
    QDict<void> installed = installedPackages();

    QString info = ipkgInfoOutput();
    if ( !info.isEmpty() ) {
	QStringList lines = QStringList::split('\n',info,TRUE);
	QStringList record;
	for (QStringList::Iterator it = lines.begin(); it!=lines.end(); ++it) {
	    QString line = *it;
	    if ( line.length()<=1 ) {
		// EOR
		QString description_short;
		QString description;
		QString name;
		QString size;
		QString sec;
		if ( parseInfo(record, name, description_short, description, size, sec) ) {
		    bool inst = installed.find(name);
		    if ( installed_only == inst ) {
			new PackageItem(packagelist,name,description_short,description,size,inst);
			installed.remove( name );
		    }
		}
		record.clear();
	    } else {
		record.append(line);
	    }
	}
    }
    if ( installed_only && installed.count() ) {
	// we have some packages without description
	QDictIterator<void> it( installed );
	for( ; it.current(); ++it ) {
	    new PackageItem( packagelist, it.currentKey(), "", "?" , "?", TRUE );
	}
    }
}

void PackageWizard::revertFailedInstalls(QString& out)
{
    QStringList failed;

    QString status = ipkgStatusOutput();
    if ( !status.isEmpty() ) {
	QRegExp separatorRegExp( ":[\t ]+" );
	QStringList lines = QStringList::split('\n',status,TRUE);
	QString name;
	QString status;
	for (QStringList::Iterator it = lines.begin(); it!=lines.end(); ++it) {
	    QString line = *it;
	    if ( line.length()<=1 ) {
		// EOR
		if ( !name.isEmpty() ) {
		    if ( status.contains(" not-installed") ) // No tr
			failed.append(name);
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

    for (QStringList::Iterator it = failed.begin(); it!=failed.end(); ++it) {
	QString e;
	runIpkg(QStringList() << "remove" << *it, e); // No tr
	out += e;
    }
}

PackageItem* PackageWizard::current() const
{
    return (PackageItem*)packagelist->currentItem();
}

/* 
void PackageWizard::doCurrentDetails(bool multi)
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
 */
/*
QString PackageWizard::fullDetails(const QString& pk)
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
*/
bool PackageWizard::readIpkgConfig(const QString& conffile)
{
    QFile conf(conffile);
    ServerListItem* currentserver=0;
    if ( conf.open(IO_ReadOnly) ) {
	QTextStream s(&conf);
	servers->clear();
	ipkg_old=0;
	while ( !s.atEnd() ) {
	    QString l = s.readLine();
	    QStringList token = QStringList::split(' ', l);
	    if ( token[0] == "src" || token[0] == "#src" ) {
		// script-based ipkg cannot handles spaces
		QString name = token[1];
		for (int i=0; i<(int)name.length(); ++i)
		    if ( name[i]=='_' ) name[i] = ' ';
		ServerListItem* item = new ServerListItem(servers,name);
		currentserver=item;
		item->setText(1, token[2]);
		item->setOn( token[0] == "src" );
	    } else if ( token[0] == "dest" ) {
		// needs UI
	    } else if ( token[0] == "option" ) { // No tr
		// ### somehow need to use the settings from netsetup
// 		if ( token[1] == "http_proxy" )
// 		    http->setText(token[2]);
// 		else if ( token[1] == "ftp_proxy" )
// 		    ftp->setText(token[2]);
// 		else if ( token[1] == "proxy_username" )
// 		    username->setText(token[2]);
// 		else if ( token[1] == "proxy_password" )
// 		    password->setText(token[2]);
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
			ServerListItem* item = new ServerListItem(servers,v);
			currentserver=item;
			item->setText(1, v);
		    } else if ( l.left(13) == "#IPKG_SOURCE=" ) {
			ServerListItem* item = new ServerListItem(servers,v);
			item->setText(1, v);
		    } else if ( l.left(10) == "IPKG_ROOT=" ) {
			// ### no UI
// 		    } else if ( l.left(20) == "IPKG_PROXY_USERNAME=" ) {
// 			username->setText(v);
// 		    } else if ( l.left(20) == "IPKG_PROXY_PASSWORD=" ) {
// 			password->setText(v);
// 		    } else if ( l.left(16) == "IPKG_PROXY_HTTP=" ) {
// 			http->setText(v);
// 		    } else if ( l.left(16) == "IPKG_PROXY_FTP=" ) {
// 			ftp->setText(v);
		    }
		}
	    }
	}
	if ( ipkg_old ) {
	    // Should be radiobuttons
	    if ( currentserver )
		currentserver->setOn(TRUE);
	}
    }
    if ( !currentserver ) {
	currentserver = new ServerListItem(servers,tr("Trolltech Qtopia"));
	currentserver->setText(1,"http://qtopia.net/packages/"+Global::architecture());
	currentserver->setOn(TRUE);
    }
    return TRUE;
}

void PackageWizard::readSettings()
{
    // read from config file(s)
    readIpkgConfig(QPEApplication::qpeDir()+"/etc/ipkg.conf");
}

void PackageWizard::writeSettings()
{
    QFile conf(QPEApplication::qpeDir()+"/etc/ipkg.conf");
    if ( conf.open(IO_WriteOnly) ) {
	QTextStream s(&conf);
	s << "# Written by Qtopia Package Manager\n"; // No tr
	if ( !ipkg_old ) {
	    for (QListViewItem* i = servers->firstChild(); i; i = i->nextSibling()) {
		ServerListItem* item = (ServerListItem*)i;
		if ( !item->isOn() )
		    s << "#";
		QString id = item->id();
		s << "src " << id << " " << item->text(1) << "\n";
	    }
	    // No UI for alt dests
	    s << "dest root /\n"; // No tr
// 	    if ( !username->text().isEmpty() )
// 		s << "option proxy_username " << username->text() << "\n";
// 	    if ( !password->text().isEmpty() )
// 		s << "option proxy_password " << password->text() << "\n";
// 	    if ( !http->text().isEmpty() )
// 		s << "option http_proxy " << http->text() << "\n";
// 	    if ( !ftp->text().isEmpty() )
// 		s << "option ftp_proxy " << ftp->text() << "\n";
	} else {
	    // Old style
	    bool src_selected=FALSE;
	    for (QListViewItem* i = servers->firstChild(); i; i = i->nextSibling()) {
		ServerListItem* item = (ServerListItem*)i;
		if ( item->isOn() )
		    src_selected=TRUE;
		else
		    s << "#";
		s << "IPKG_SOURCE=\"" << item->text(1) << "\"\n";
	    }
	    s << "IPKG_ROOT=/\n"
// 	      << "IPKG_PROXY_USERNAME=\"" << username->text() << "\"\n"
// 	      << "IPKG_PROXY_PASSWORD=\"" << password->text() << "\"\n"
// 	      << "IPKG_PROXY_HTTP=\"" << http->text() << "\"\n"
// 	      << "IPKG_PROXY_FTP=\"" << ftp->text() << "\"\n"
	    ;
	}
	conf.close();
    } else {
	qWarning("Cannot write to $QPEDIR/etc/ipkg.conf");
    }
}

void PackageWizard::selectPackages( const QStringList& l )
{
    QDict<void> d;
    for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
	d.replace(*it,(void*)1);
    QListViewItem* i;
    for ( i = packagelist->firstChild(); i; i = i->nextSibling() ) {
	PackageItem* pit = (PackageItem*)i;
	i->setSelected( d[pit->name()] );
    }
}

QStringList PackageWizard::findPackages( const QRegExp& r )
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
		    if ( tag == "Package" ) // No tr
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

/*!
  Install \a ipk to \a location. Returns FALSE on error, otherwise TRUE.
  */

bool PackageWizard::installIpkg( const QString &ipk, const QString &location, QString& out )
{
    //qDebug( "installing %s to '%s'", ipk.ascii(), location.ascii() );
    out = QString("installing %1 to %2").arg(ipk).arg(location);
    QStringList cmd;
    QStringList orig_packages;
    if ( !location.isEmpty() ) {
        // ??????? mkdir( location.ascii(), 0777 );
	cmd += "-d";
	cmd += location;
    }
    cmd += "install"; // No tr
    cmd += ipk;
    bool r = runIpkg( cmd, out );

    return r;
}

/* 
void PackageWizard::doUpgrade()
{
    startMultiRun(2);
    QString out;
    runIpkg("update",out);
    runIpkg("upgrade",out);
    updatePackageList();
    endRun();
}
 */


/*
void PackageWizard::doIt()
{
    bool ok = commitWithIpkg();
    updatePackageList(); // things may have changed
    if (!ok) qApp->beep();
}
*/

QStringList PackageWizard::linksInPackage(const QString& pkg, const QString & root )
{
    QString files;
    QStringList cmd;
    cmd << ipkg_name;
    if ( root != "/" )
	cmd << "-d" << root;
    cmd << "files" << pkg;  // No tr
    Process ipkg_files( cmd );
    QStringList r;
    if ( ipkg_files.exec("",files) ) {
	QStringList lines = QStringList::split('\n',files,FALSE);
	for (QStringList::Iterator it = lines.begin(); it!=lines.end(); ++it) {
	    QString fn = *it;
	    // Quickly weed out most of the files that do not end 
	    // with ".desktop" (8 chars)
	    if ( fn.length() < 8 || fn.constref(fn.length()-8) != '.' )
		continue;
	    if ( root != "/" && fn.startsWith( root ) )
		fn = fn.mid( root.length() - 1 );
	    
	    if ( fn.startsWith("//") ) // a ipkg bug
		fn = fn.mid(1);

	    // Two supported symlink conventions
	    if ( fn.left(12)=="/opt/Qtopia/" ) {
		fn.replace(0,12,QPEApplication::qpeDir());
	    } else if ( fn.left(15)=="/opt/QtPalmtop/" ) {
		fn.replace(0,15,QPEApplication::qpeDir());
	    }

	    if ( Global::isDocumentFileName(fn) || Global::isAppLnkFileName(fn) ) {
		r.append(fn);
	    }
	}
    }
    return r;
}

void PackageWizard::showError(const QString& err)
{
    QDialog dlg(this,"error",TRUE); // No tr
    (new QVBoxLayout(&dlg,4,2))->setAutoAdd(TRUE);
    QLabel lbl(tr("Error"),&dlg);
    QMultiLineEdit e(&dlg);
    e.setReadOnly(TRUE);
    e.setText(err);
    QPushButton c(tr("Close"),&dlg);
    dlg.connect(&c,SIGNAL(clicked()),SLOT(accept()));
    dlg.showMaximized();
    dlg.exec();
}

bool PackageWizard::commitWithIpkg()
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

    QStringList to_do;

    for ( QListViewItem* i = packagelist->firstChild(); i; i = i->nextSibling() ) {
	PackageItem* pit = (PackageItem*)i;
	if ( pit->isSelected() )
	    to_do.append(pit->id());
    }

    bool ok=TRUE;

    int jobs = to_do.count();
    QString errlog;
    if ( jobs ) {
	startMultiRun(jobs);
	QStringList linksChanged;

	if ( mode_rm->isChecked() ) {
	    for (QStringList::ConstIterator it=to_do.begin(); it!=to_do.end(); ++it) {
		QString out;
		QStringList cmd;
		QString *root = installedRootDict ? 
				installedRootDict->take(*it) : 0;

		QStringList l = linksInPackage(*it, root ? *root : QString("/"));

		if ( root )
		    cmd << "-d" << *root;
		delete root;
		cmd << "remove" << *it; // No tr

		if ( !runIpkg( cmd, out) ) {
		    ok = FALSE;
		    errlog += out;
		} else {
		    linksChanged += l;
		}
	    }

	}
	if ( mode_doc->isChecked() || mode_net->isChecked() ) {
	    QString loc = location->installationPath();
	    for (QStringList::ConstIterator it=to_do.begin(); it!=to_do.end(); ++it) {
		QString out;
		if ( !installIpkg( *it, loc, out ) ) {
		    errlog += getenv("PATH");
		    errlog += "\n";
		    errlog += "install ipkg returned false for " + loc + "\n";
		    ok = FALSE;
		    errlog += out;
		} else {
		    linksChanged += linksInPackage(packageId(*it), loc );
		}
	    }
	    if ( !ok ) {
		QString out;
		revertFailedInstalls(out);
		errlog += out;
	    }
	}


	// We could be smarter about when we do this, and use the
	// "create" or "remove" arguments to update only what's needed,
	// but it doesn't take that long.
	system( "qtopia-update-symlinks" );

	
	// ##### If we looked in the list of files, we could send out accurate
	// ##### messages. But we don't bother yet, and just do an "all".
#ifndef QT_NO_COP
	for (QStringList::ConstIterator it=linksChanged.begin(); it!=linksChanged.end(); ++it) {
	    QCopEnvelope e("QPE/System", "linkChanged(QString)");
	    e << *it;
	}
#endif

#if QT_VERSION > 230 // a bug breaks this otherwise
	if ( !ok )
	    showError(errlog);
#endif

	endRun();
    }

    return ok;
}

QString PackageWizard::ipkgStatusOutput()
{
    if ( cachedIpkgStatusOutput.isEmpty() ) {
	Process ipkg_status( QStringList() << ipkg_name << "status" ); // No tr
	cachedIpkgStatusOutput.detach();
	ipkg_status.exec( 0, cachedIpkgStatusOutput );
	
	StorageInfo storage;
	const QList<FileSystem>& sifs(storage.fileSystems());
	QListIterator<FileSystem> sit(sifs);
	FileSystem* fs;
	for ( ; (fs=sit.current()); ++sit ) {
	    //qDebug( "looking at %s", fs->path().latin1() );
	    if ( fs->path() != "/" && 
		 QFile::exists( fs->path() +
#ifdef IPKGSIM
		     QPEApplication::qpeDir() +
#endif
		     "/usr/lib/ipkg/status" ) ) {
		QCString subOut;
		Process ipkg2( QStringList() << ipkg_name << "-d" 
			       << fs->path() << "status" ); // No tr
		ipkg2.exec( 0, subOut );
		//qDebug( "  result %s", subOut.data() );
		cachedIpkgStatusOutput += subOut;
	    }
	}
    }
    return QString::fromLocal8Bit( cachedIpkgStatusOutput );
}

QString PackageWizard::ipkgInfoOutput()
{
    if ( cachedIpkgInfoOutput.isEmpty() ) {
	Process ipkg_info( QStringList() << ipkg_name << "info" );
	cachedIpkgInfoOutput.detach();
	ipkg_info.exec( 0, cachedIpkgInfoOutput );
    }
    return QString::fromLocal8Bit( cachedIpkgInfoOutput );
}

void PackageWizard::setCachedIpkgOutputDirty()
{
    cachedIpkgStatusOutput = cachedIpkgInfoOutput = "";
}

void PackageWizard::startMultiRun(int jobs)
{
    startRun();
    progress()->setTotalSteps(jobs);
    progress()->setProgress(0);
}

void PackageWizard::startRun()
{
    setEnabled(FALSE);
    progress()->setEnabled(TRUE);
    progress()->show();
}

void PackageWizard::endRun()
{
    QPEApplication::setTempScreenSaverMode(QPEApplication::Enable);
    progress()->hide();
    progress()->reset();
    setEnabled(TRUE);
    setAppropriates();
}

bool PackageWizard::runIpkg(const QStringList& args, QString& out)
{
    bool justone = progress()->progress() == -1;
    if ( justone ) {
	startMultiRun(1);
    } else {
	startRun();
    }
    QStringList cmd;
    cmd += ipkg_name;
    if ( !ipkg_old )
	cmd += "-force-defaults";
    cmd += args;
    qDebug("runn comand %s", (const char *)cmd.join(" ").local8Bit());
    qApp->processEvents();
    QPEApplication::setTempScreenSaverMode(QPEApplication::DisableSuspend);
    Process ipkg_status(cmd);
    bool r = ipkg_status.exec("",out);
    //qDebug( "RESULT %s", out.latin1() );
    progress()->setProgress(progress()->progress()+1);
    setCachedIpkgOutputDirty();
    if ( justone )
	endRun();
    return r;
}

/*
// simple hack to get support for ipkg mimetype
void PackageWizard::maybeInstall( const QString &ipk )
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
	QString out;
	installIpkg( ipk, dest, out );
#ifndef QT_NO_COP
	QCopEnvelope e("QPE/System", "linkChanged(QString)");
	QString lf = QString::null;
	e << lf;
#endif
	endRun();
    }
}
*/

void PackageWizard::setDocument(const QString& fileref)
{
    if ( fileref.isNull() )
	return;
    DocLnk doc( fileref );
    if ( doc.file().isEmpty() )
	return;
    qcopDocument = doc.file();
    mode_doc->setChecked(TRUE);
    setAppropriates();
    showPage(page_location);
    packagelist->clear();
    QDict<void> installed = installedPackages();
    bool inst = installed.find(packageId(doc.file()));
    PackageItem* item = new PackageItem(packagelist,doc,inst);
    item->parseInfo();
    item->setSelected(TRUE);
}

void PackageWizard::done(int y)
{
    if ( finishButton()->isEnabled() ) {
	setFinishEnabled( page_confirmation, FALSE );
	if ( y && !committed ) {
	    if ( commitWithIpkg() ) {
		finishButton()->setText(tr("Close"));
		committed = TRUE;
		setAppropriates();
	    }
            else
            {
                confirmation->setText( "FAILED OPERATION:<br>" + confirmation->text() );
		finishButton()->setText(tr("Retry"));
            }
	    setFinishEnabled( page_confirmation, TRUE );
	    return;
	} else {
	    // nothing.
	}
    }
    PackageWizardBase::done(y);
    close();
}
