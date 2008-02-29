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

#include "speeddial.h"

#include <qtopia/config.h>
#include <qtopia/resource.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>

#include <qintdict.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qdialog.h>

#include <time.h>

class SpeedDialListPrivate {
public:
    SpeedDialListPrivate() : note(0) {}
    QLabel* note; // JUST FOR BETA
};

// SpeedDial is very similar to DeviceButtonManager

static QDict<SpeedDialRec>* recs=0;
static time_t recs_ts = (time_t)-1;
static void updateReqs()
{
    time_t ts = Config::timeStamp("SpeedDial");
    if ( ts != recs_ts ) {
	delete recs;
	recs = new QDict<SpeedDialRec>;
	recs->setAutoDelete(TRUE);
	recs_ts = ts;
	Config cfg("SpeedDial");
	cfg.setGroup("Dial");
	QStringList d = cfg.readListEntry("Defined",',');
	for (QStringList::ConstIterator it=d.begin(); it!=d.end(); ++it) {
	    cfg.setGroup("Dial"+*it);
	    QString s = cfg.readEntry("Service");
	    QString m = cfg.readEntry("Message");
	    QByteArray a = cfg.readByteArrayEntry("Args");
	    QString l = cfg.readEntry("Label");
	    QString ic = cfg.readEntry("Icon");
	    ServiceRequest req(s,m.latin1());
	    if (!a.isEmpty())
		req.writeRawBytes(a.data(), a.size());
	    recs->insert((*it),new SpeedDialRec(req,l,ic));
	}
    }
}
static void writeReqs(const QString& changed)
{
    {
	Config cfg("SpeedDial");
	QStringList d;
	QDictIterator<SpeedDialRec> it(*recs);
	SpeedDialRec* rec;
	bool found = FALSE;
	while ((rec=it.current())) {
	    d.append(it.currentKey());
	    if ( changed.isNull() || changed == it.currentKey() ) {
		cfg.setGroup("Dial"+it.currentKey());
		cfg.writeEntry("Service",rec->request.service());
		cfg.writeEntry("Message",QString(rec->request.message()));
		cfg.writeEntry("Args",rec->request.data());
		cfg.writeEntry("Label",rec->label);
		cfg.writeEntry("Icon",rec->icon);
		found = TRUE;
	    }
	    ++it;
	}
	if ( !found ) {
	    // removed it
	    cfg.removeGroup("Dial"+changed);
	}
	cfg.setGroup("Dial");
	cfg.writeEntry("Defined",d,',');
    }
    recs_ts = Config::timeStamp("SpeedDial");
}
static int firstFree()
{
    for (int i=1; i<100; i++) {
	if ( !recs->find(QString::number(i)) )
	    return i;
    }
    return -1;
}

class SpeedDialItem : public QListBoxText {
public:
    SpeedDialItem(const QString& i, const SpeedDialRec& rec, SpeedDialList* parent) :
	QListBoxText(parent),
	input(i),
	record(rec)
    {
    }

    QString input;
    SpeedDialRec record;

    int width( const QListBox* lb ) const
    {
	return lb->width();
    }

    void paint( QPainter *p )
    {
	QFontMetrics fm(listBox()->fontMetrics());
	QPixmap pm,pm0;
	QString txt;

	if ( !record.request.isNull() )
	    pm = record.pixmap();
	SpeedDialList* sdl = (SpeedDialList*)listBox();
	if ( selected() && !sdl->seltext.isNull() ) {
	    pm0 = pm;
	    pm = SpeedDialRec::pixmap(sdl->selicon);
	    txt += sdl->seltext;
	} else {
	    if ( !record.request.isNull() ) {
		pm = record.pixmap();
		txt = record.label;
	    }
	}

	int x = 0;
	p->drawText(x,fm.height()-1,input+":");
	x += fm.width(input)+fm.width(":")+fm.width(" ");
	if ( !pm0.isNull() ) {
	    p->drawPixmap(x,0,pm0);
	    x += pm0.width()+2;
	}
	if ( !pm.isNull() ) {
	    p->drawPixmap(x,0,pm);
	    x += pm.width()+2;
	}
	p->drawText(x,fm.height()-1,txt);
    }
};

SpeedDialList::SpeedDialList(QWidget* parent) :
    QListBox(parent)
{
    updateReqs();
    init(QString::null);
}

SpeedDialList::~SpeedDialList()
{
    delete d;
}

SpeedDialList::SpeedDialList(const QString& label, const QString& icon, QWidget* parent) :
    QListBox(parent),
    seltext(label),
    selicon(icon)
{
    updateReqs();
    init("");
}

void SpeedDialList::resizeEvent(QResizeEvent* e)
{
qDebug("resize %dx%d",width(),height());
    QListBox::resizeEvent(e);
}

void SpeedDialList::init(const QString& f)
{
    d = new SpeedDialListPrivate;
    d->note = 0;
    sel = 0;
    sel_tid = 0;
    setCaption(tr("Speed Dial"));
    int fn = 0;
    if (Global::mousePreferred()) {
	if ( f.isNull() ) {
	    fn = 1;
	} else if (f.isEmpty()) {
	    fn = firstFree();
	    if ( fn < 0 )
		fn = 99;
	}
    }
    for (int i=1; i<100; i++) {
	QString inp = QString::number(i);
	SpeedDialRec* rec = recs->find(inp);
	if ( !Global::mousePreferred() || rec ) {
	    SpeedDialItem *it = new SpeedDialItem(QString::number(i),rec ? *rec : SpeedDialRec(),this);
	    if ( f == inp || i == fn )
		setCurrentItem(it);
	}
    }
    QDictIterator<SpeedDialRec> it(*recs);
    SpeedDialRec* rec;
    while ((rec=it.current())) {
	QString inp = it.currentKey();
	bool ok;
	if ( inp.toInt(&ok) > 99 || !ok ) { // skip numbers 0..99, done above
	    SpeedDialItem *i = new SpeedDialItem(inp,*rec,this);
	    if ( f == inp )
		setCurrentItem(i);
	}
	++it;
    }

    connect(this,SIGNAL(selected(QListBoxItem*)),
	    this,SLOT(select(QListBoxItem*)));
    connect(this,SIGNAL(clicked(QListBoxItem*)),
	    this,SLOT(select(QListBoxItem*)));
}

void SpeedDialList::reload(const QString& sd)
{
    SpeedDialRec *r = recs->find(sd);
    for (int i=0; i<(int)count(); ++i) {
	SpeedDialItem *it = (SpeedDialItem*)item(i);
	if ( it->input == sd ) {
	    if ( r ) {
		it->record = *r;
	    } else if ( Global::mousePreferred() ) {
		removeItem(i);
		setCurrentItem(i);
		return;
	    } else {
		it->record = SpeedDialRec();
	    }
	    updateItem(i);
	    return;
	}
    }
    if ( r ) {
	SpeedDialItem *i = new SpeedDialItem(sd,*r,this);
	setCurrentItem(i);
    }
}

void SpeedDialList::select(QListBoxItem* i)
{
    SpeedDialItem *sdi = (SpeedDialItem*)i;
    if ( sdi )
	emit speedDialSelected(sdi->input);
}

QString SpeedDialList::currentSpeedDial() const
{
    SpeedDialItem *it = (SpeedDialItem*)item(currentItem());
    return it ? it->input : QString::null;
}

void SpeedDialList::setCurrentSpeedDial(const QString& sd)
{
    for (int i=0; i<(int)count(); ++i) {
	if ( ((SpeedDialItem*)item(i))->input == sd ) {
	    setCurrentItem(i);
	    return;
	}
    }
}

void SpeedDialList::keyPressEvent(QKeyEvent* e)
{
    int k = e->key();
    if ( k >= Key_0 && k <= Key_9 ) {
	sel = sel * 10 + k - Key_0;
	if ( sel_tid ) {
	    killTimer(sel_tid);
	    sel_tid = 0;
	}
	if ( sel ) {
	    setCurrentSpeedDial(QString::number(sel));
	    if ( sel < 10 )
		sel_tid = startTimer(800);
	    else
		sel = 0;
	}
    } else {
	QListBox::keyPressEvent(e);
    }
}

void SpeedDialList::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == sel_tid ) {
	killTimer(sel_tid);
	sel = 0;
	sel_tid = 0;
    }
}


class SpeedDialDialog : public QDialog {
    Q_OBJECT
public:
    SpeedDialDialog(const ServiceRequest& a, const QString& l, const QString& ic, QWidget* parent) :
	QDialog(parent,0,TRUE),
	action(a),
	label(l),
	icon(ic)
    {
	if ( Global::mousePreferred() ) {
	    QVBoxLayout *vb = new QVBoxLayout(this,6);
	    setCaption(tr("Enter Speed Dial Letters"));
	    QHBoxLayout *hb;

	    hb = new QHBoxLayout(vb);
	    QLabel *i = new QLabel(this);
	    i->setPixmap(SpeedDialRec::pixmap(ic));
	    hb->addStretch();
	    hb->addWidget(i);
	    hb->addWidget(new QLabel(l,this));
	    hb->addStretch();

	    hb = new QHBoxLayout(vb);
	    inputle = new QLineEdit(this);
	    QPEApplication::setInputMethodHint(inputle,QPEApplication::Text);
	    hb->addWidget(inputle);
	    ok = new QPushButton(tr("OK"),this);
	    hb->addWidget(ok);
	    ok->setSizePolicy(inputle->sizePolicy());
	    inputle->setMaxLength(3);

	    hb = new QHBoxLayout(vb);
	    hb->addStretch();
	    currinfo = new QHBox(this);
	    curricon = new QLabel(currinfo);
	    currlabel = new QLabel(currinfo);
	    curricon->setMargin(1);
	    currlabel->setMargin(1);
	    hb->addWidget(currinfo);
	    hb->addStretch();

	    connect(inputle,SIGNAL(textChanged(const QString&)),this,SLOT(lookup(const QString&)));
	    connect(ok,SIGNAL(clicked()),this,SLOT(storeInput()));
	} else {
	    QVBoxLayout *vb = new QVBoxLayout(this);
	    SpeedDialList *list = new SpeedDialList(label,icon,this);
            list->setCurrentItem(0);
	    vb->addWidget(list);
	    setCaption(list->caption());
	    connect(list,SIGNAL(speedDialSelected(const QString&)),this,SLOT(store(const QString&)));
	}
#ifdef QTOPIA_PHONE
	QPEApplication::setMenuLike(this,TRUE);
#endif
    }

    QString choice;

private slots:
    void storeInput()
    {
	store(inputle->text());
    }

    void lookup(const QString& f)
    {
	updateReqs();
	SpeedDialRec* rec = recs->find(f);
	QPalette warn = palette();
	if ( rec ) {
	    warn.setColor(QColorGroup::Background,red);
	    warn.setColor(QColorGroup::Text,white);
	    curricon->setPixmap(SpeedDialRec::pixmap(rec->icon));
	    currlabel->setText(rec->label);
	    ok->setText(tr("Change"));
	} else {
	    curricon->setText(" ");
	    currlabel->setText(" ");
	    ok->setText(tr("OK"));
	}
	curricon->setPalette(warn);
	currlabel->setPalette(warn);
    }

    void store(const QString& f)
    {
	if ( !f.isEmpty() ) {
	    choice = f;
	    SpeedDial::set(f,SpeedDialRec(action,label,icon));
	}
	accept();
    }

private:
    ServiceRequest action;
    QString label, icon;
    QLineEdit *inputle;
    QHBox *currinfo;
    QLabel *curricon;
    QLabel *currlabel;
    QPushButton *ok;
};


QString SpeedDial::addWithDialog(const QString& label, const QString& icon,
    const ServiceRequest& action, QWidget* parent)
{
    SpeedDialDialog dlg(action,label,icon,parent);
    if ( QPEApplication::execDialog(&dlg) ) {
	return dlg.choice;
    } else {
	return QString::null;
    }
}

SpeedDialRec SpeedDial::find(const QString& speeddial)
{
    updateReqs();
    SpeedDialRec* rec = recs->find(speeddial);
    return rec ? *rec : SpeedDialRec(ServiceRequest(),QString::null,QString::null);
}

void SpeedDial::remove(const QString& speeddial)
{
    updateReqs();
    recs->remove(speeddial);
    writeReqs(speeddial); // NB. must do this otherwise won't work
}

void SpeedDial::set(const QString& speeddial, const SpeedDialRec& r)
{
    updateReqs();
    recs->replace(speeddial,new SpeedDialRec(r));
    writeReqs(speeddial);
}

QPixmap SpeedDialRec::pixmap() const
{
    return pixmap(icon);
}

QPixmap SpeedDialRec::pixmap(const QString& nm)
{
    QPixmap r = Resource::loadIconSet(nm).pixmap(QIconSet::Small, TRUE);
    if ( r.isNull() ) {
	// Inefficient! SpeedDial icons really should be icons.
	QImage i = Resource::loadImage(nm);
	if ( !i.isNull() ) {
	    Config cfg("qpe");
	    cfg.setGroup("Appearance");
	    int size = cfg.readNumEntry("IconSize",16);
	    r.convertFromImage(i.smoothScale(size,size));
	}
    }
    return r;
}

#include "speeddial.moc"
