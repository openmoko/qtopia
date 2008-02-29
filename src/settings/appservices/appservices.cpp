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

#define QTOPIA_INTERNAL_MIMEEXT

#include "appservices.h"

#include <qtopia/resource.h>
#include <qtopia/applnk.h>
#include <qtopia/mimetype.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/services.h>

#include <qtimer.h>
#include <qlistview.h>
#include <qdir.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

class ASCheckListItem : public QCheckListItem {
public:
    ASCheckListItem(QListView *parent, const QString &sv, const AppLnk* lnk) :
	QCheckListItem(parent,QString::null,Controller), link(lnk), svr(sv)
    {
	QString svdir=QPEApplication::qpeDir()+"services/"+sv;
	Config service(svdir+".service",Config::File);
	QString icon;
	if ( service.isValid() ) {
	    QString nm = service.readEntry("Name");
	    if ( nm.isEmpty() || service.readNumEntry("Multiple",0) ) {
		// Not presented to users
		svr = QString::null;
	    } else {
		setText(0,service.readEntry("Name"));
		icon = service.readEntry("Icon");
	    }
	} else if ( lnk ) {
	    QString name=sv.mid(5); // Strip "Open/"
	    if ( name.left(12)=="application/" ) {
		QString ap = name.mid(12);
		if ( ap.left(2) == "x-" )
		    ap = ap.mid(2);
		name = ap;//AppServices::tr("%1 (%2)").arg(lnk->name()).arg(ap);
	    } else if ( name.find('/')>0 ) {
		MimeType mt(name);
		name = mt.extension().upper();
	    } // else not translated
	    setText(0,AppServices::tr("Open %1").arg(name));
	    icon = lnk->icon();
	} else {
	    svr = QString::null;
	}
	int s = AppLnk::bigIconSize();
	QImage img;
	if ( !icon.isNull() )
	    img = Resource::loadImage(icon);
	if ( img.isNull() )
	    img = Resource::loadImage("Generic");
	img = img.smoothScale(s,s);
	QPixmap pm; pm.convertFromImage(img);
	setPixmap(0,pm);
    }

    ASCheckListItem(QCheckListItem *parent, const AppLnk* lnk) :
	QCheckListItem(parent,lnk->name(),RadioButton), link(lnk)
    {
    }

    bool isValid() const
    {
	return !svr.isNull();
    }

    int width( const QFontMetrics&,
                       const QListView* lv, int column) const
    {
	QFont f=chFont(lv->font());
	return QCheckListItem::width(QFontMetrics(f),lv,column)+5;
    }

    void paintCell( QPainter *p,  const QColorGroup & cg,
                    int column, int width, int alignment )
    {
	QFont f=p->font();
	if ( !svr.isEmpty() ) {
	    QFont bf=chFont(f);
	    p->setFont(bf);
	}
	QCheckListItem::paintCell(p,cg,column,width,alignment);
	p->setFont(f);
    }

    QString service() const
    {
	return svr;
    }

    QString application() const
    {
	return link ? link->exec() : QString("XXXX");
    }

private:
    QFont chFont(const QFont& f) const
    {
        QFont r = f;
	r.setPointSize(f.pointSize()*4/3);
	r.setBold(TRUE);
	return r;
    }
    const AppLnk* link;
    QString svr;
};

/* 
 *  Constructs a AppServices which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
AppServices::AppServices( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : AppServicesBase( parent, name, modal, fl )
{
    allapps=0;
    connect(lv,SIGNAL(clicked(QListViewItem*)),this,SLOT(check(QListViewItem*)));
    QTimer::singleShot(1,this,SLOT(loadState()));
}

void AppServices::check(QListViewItem* i)
{
    if ( i && i->parent() ) {
	// must be checkbox
	((QCheckListItem*)i)->setOn(TRUE);
    }
}

void AppServices::drawWait(bool on)
{
    QWidget* waitwi = lv->viewport();
    QPainter waitpaint(waitwi);
    QPixmap pm = Resource::loadPixmap("bigwait");
    int x = (waitwi->width()-pm.width())/2;
    int y = (waitwi->height()-pm.height())/2;
    if ( on ) {
	waitpaint.drawPixmap(x,y,pm);
    } else {
	waitpaint.eraseRect(x,y,pm.width(),pm.height());
    }
    waitpaint.end();
}

void AppServices::loadState()
{
    lv->clear();
    drawWait(TRUE);
    delete allapps;
    allapps=new AppLnkSet(MimeType::appsFolderName());

    int max=-1;
    QDict<ASCheckListItem> servicedict;
    QDict<ASCheckListItem> appdict;

#define ADDAPP(si,lnk,ver) \
    ASCheckListItem* ai = new ASCheckListItem(si,lnk); \
    appdict.insert(lnk->exec(),ai); \
    /*ai->setText(1,ver);*/ \
    ai->setPixmap(0,lnk->pixmap()); \
    if ( def == lnk->exec() ) { \
	ai->setOn(TRUE); \
    }

    // All services
    QStringList s = Service::list();
    for (QStringList::ConstIterator it = s.begin(); it!=s.end(); ++it) {
	QString sv=QPEApplication::qpeDir()+"services/"+(*it);
	QDir dir(sv,QString::null,QDir::Name | QDir::IgnoreCase,QDir::Files);
	QStringList apps = dir.entryList();
	Config binding(Service::config(*it), Config::File);
	binding.setGroup("Service");
	ASCheckListItem* si = new ASCheckListItem(lv,(*it),0);
	if ( si->isValid() ) {
	    servicedict.insert(*it,si);
	    QString def = binding.readEntry("default");
	    for (QStringList::ConstIterator ait = apps.begin(); ait!=apps.end(); ++ait) {
		const AppLnk* lnk = allapps->findExec(*ait);
		if ( lnk ) {
		    QFile f(sv+"/"+*ait);
		    if ( f.open(IO_ReadOnly) ) {
			//int vern=QString(f.readAll()).stripWhiteSpace().toInt();
			//QString ver; ver.sprintf("%d.%02d",vern/100,vern%100);
			ADDAPP(si,lnk,ver);
		    }
		}
	    }
	    si->setOpen(TRUE);
	} else {
	    delete si;
	}
    }

    // Now include all MIME types
    const QList<AppLnk> &c = allapps->children();
    const AppLnk* lnk;
    for (QListIterator<AppLnk> ai(c); (lnk=ai.current()); ++ai) {
	if ( !appdict.find(lnk->exec()) ) {
	    // Not a service-aware application
	    QStringList types = lnk->mimeTypes();
	    for (QStringList::ConstIterator ti=types.begin(); ti!=types.end(); ++ti) {
		QString mt = *ti;
		if ( mt.right(2)=="/*" )
		    mt.truncate(mt.length()-2);
		QString sv = "Open/"+mt;
		ASCheckListItem* s = servicedict.find(sv);
		if ( !s ) {
		    s = new ASCheckListItem(lv,sv,lnk);
		    servicedict.insert(sv,s);
		    s->setOpen(TRUE);
		}
		Config binding(Service::config(sv), Config::File);
		binding.setGroup("Service");
		QString def = binding.readEntry("default");
		ADDAPP(s,lnk,"1.00");
	    }
	}
    }

    bool some = FALSE;
    QDictIterator<ASCheckListItem> sdit(servicedict);
    ASCheckListItem* si;
    for (; (si=sdit.current()); ++sdit) {
	int n = si->childCount();
	max = QMAX(max,n);
	if ( n==0 && show0->isOn()
	  || n==1 && show1->isOn()
	  || n>1 && showmulti->isOn())
	{
	    some = TRUE;
	    if ( n == 1 ) {
		((ASCheckListItem*)si->firstChild())->setOn(TRUE);
	    }
	} else {
	    delete si;
	}
    }
    if ( !some ) {
	switch ( max ) {
	    case -1:
		// there are no services
		choices->setEnabled(FALSE);
		break;
	    case 1:
		// there are no service choices
		showmulti->setEnabled(FALSE);
		if ( showmulti->isChecked() ) {
		    show1->setChecked(TRUE);
		    loadState();
		}
		break;
	    case 0:
		// there are no services provided
		showmulti->setEnabled(FALSE);
		show1->setEnabled(FALSE);
		if (!show0->isChecked()) {
		    show0->setChecked(TRUE);
		    loadState();
		}
		break;
	}
    }
    drawWait(FALSE);
}

/*  
 *  Destroys the object and frees any allocated resources
 */
AppServices::~AppServices()
{
    delete allapps;
}


void AppServices::done(int y)
{
    if ( y ) {
	QListViewItem* s = lv->firstChild();
	while ( s ) {
	    ASCheckListItem* sv=(ASCheckListItem*)s;
	    Config binding("Service-"+sv->service());
	    binding.setGroup("Service");
	    for (QListViewItem* a = s->firstChild(); a; a = a->nextSibling()) {
		ASCheckListItem* ap=(ASCheckListItem*)a;
		if ( ap->isOn() ) {
		    binding.writeEntry("default",ap->application());
		    break;
		}
	    }
	    s = s->nextSibling();
	}
    }
    AppServicesBase::done(y);
    close();
}
