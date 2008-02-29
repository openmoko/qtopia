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

#ifndef _MSC_VER
# define private public
# define protected public
#endif
#include <qtopia/qpeapplication.h>
#undef private
#undef protected

#include "firstuse.h"
#include "serverapp.h"
#include "inputmethods.h"

#include "../settings/language/languagesettings.h"
#include "../settings/systemtime/settime.h"


#include <qtopia/custom.h>

#if defined(QPE_NEED_CALIBRATION)
#include "../settings/calibrate/calibrate.h"
#endif

#include <qtopia/resource.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/config.h>
#include <qtopia/fontmanager.h>
#include <qtopia/timezone.h>
#include <qtopia/global.h>

#include <qfile.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qheader.h>
#include <qpainter.h>
#include <qsimplerichtext.h>
#include <qtimer.h>
#include <qlayout.h>
#if defined( Q_WS_QWS )
# include <qgfx_qws.h>
#endif

#include <stdlib.h>

//#define USE_INPUTMETHODS

static FirstUse *firstUse = 0;

class TzListItem : public QListViewItem
{
public:
    TzListItem(QListViewItem *li, const QString &text, const QCString &id)
	: QListViewItem(li, text), tzId(id) {}

    const QCString &id() const { return tzId; }

public:
    QCString tzId;
};

class TzAreaListItem : public QListViewItem
{
public:
    TzAreaListItem(QListView *li, const QString &text)
	: QListViewItem(li, text), childSel(FALSE) {}

    void setChildSelected(bool s) { childSel = s; repaint(); }

protected:
    void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int align) {
	QColorGroup mycg(cg);
	if (childSel) {
	    mycg.setColor(QColorGroup::Text, cg.highlightedText());
	    mycg.setBrush(QColorGroup::Base, cg.brush(QColorGroup::Highlight));
	}
	QListViewItem::paintCell(p, mycg, column, width, align);
    }

public:
    bool childSel;
};

class TimeZoneDialog : public QDialog
{
    Q_OBJECT
public:
    TimeZoneDialog(QWidget *parent=0, const char *name=0, bool modal=TRUE);

protected:
    void accept();

protected slots:
    void tzChanged(QListViewItem *);
    void tzClicked(QListViewItem *, const QPoint &pos, int);

private:
    QListView *tzListView;
    TzListItem *currItem;
};

TimeZoneDialog::TimeZoneDialog(QWidget *parent, const char *name, bool modal)
    : QDialog(parent, name, modal), currItem(0)
{
    QVBoxLayout *vb = new QVBoxLayout(this);
    tzListView = new QListView(this);
    tzListView->addColumn("");
    tzListView->header()->hide();
    tzListView->setRootIsDecorated(TRUE);
    connect(tzListView, SIGNAL(currentChanged(QListViewItem*)),
	    this, SLOT(tzChanged(QListViewItem*)));
    connect(tzListView, SIGNAL(clicked(QListViewItem*,const QPoint&,int)),
	    this, SLOT(tzClicked(QListViewItem*,const QPoint&,int)));
    vb->addWidget(tzListView);

    QString currTz = getenv("TZ");
    Config config("locale");
    config.setGroup("Location");
    currTz = config.readEntry("Timezone", currTz);

    QMap<QString,QListViewItem *> areaMap;

    QStrList ids = TimeZone::ids();
    QStrListIterator it(ids);
    for (; it.current(); ++it) {
	QCString tzId = it.current();
        //suppress timezone id /RC and /OK
        if (tzId.contains(QRegExp("/RC")) || tzId.contains(QRegExp("/OK"))) {
            continue;
        }
	TimeZone tz(tzId);
      
        QString area = tz.area();
        int pos = area.find('/');
        if (pos > -1) {
            area.truncate(pos);
            area = qApp->translate("TimeZone", area);
        }
	QListViewItem *rparent = 0;
	QMap<QString,QListViewItem *>::ConstIterator ait = areaMap.find(area);
	if (ait == areaMap.end()) {
	    rparent = new TzAreaListItem(tzListView, area);
	    rparent->setSelectable(FALSE);
	    areaMap[area] = rparent;
	} else {
	    rparent = *ait;
	}
        //qDebug("%s-%s-%s-", QString(tzId).latin1(), tz.city().latin1(), tz.area().latin1());
	TzListItem *citem = new TzListItem(rparent, tz.city(), tzId);
	if (tzId == currTz.latin1()) {
	    TzAreaListItem *ai;
	    if (currItem) {
		ai = (TzAreaListItem *)currItem->parent();
		ai->setChildSelected(FALSE);
	    }
	    currItem = citem;
	    tzListView->setCurrentItem(currItem);
	    ai = (TzAreaListItem *)currItem->parent();
	    ai->setChildSelected(TRUE);
	    citem->parent()->setOpen(TRUE);
	} else if (!currItem) {
	    currItem = citem;
	    tzListView->setCurrentItem(currItem);
	    TzAreaListItem *ai = (TzAreaListItem *)currItem->parent();
	    ai->setChildSelected(TRUE);
	}
    }
    tzListView->sort();
}

void TimeZoneDialog::accept()
{
    if (currItem) {
	qDebug("TimeZone: %s", currItem->id().data());
	setenv("TZ", currItem->id().data(), 1);
	Config config("locale");
	config.setGroup("Location");
	config.writeEntry("Timezone", QString(currItem->id().data()));
    }
    QDialog::accept();
}

void TimeZoneDialog::tzChanged(QListViewItem *item)
{
    TzAreaListItem *ai;
    if (item) {
	if (item->parent()) {
	    tzListView->setSelected(currItem, FALSE);
	    ai = (TzAreaListItem *)currItem->parent();
	    ai->setChildSelected(FALSE);
	    TzListItem *tzitem = (TzListItem*)item;
	    currItem = tzitem;
	}
    }
    tzListView->setSelected(currItem, TRUE);
    ai = (TzAreaListItem *)currItem->parent();
    ai->setChildSelected(TRUE);
}

void TimeZoneDialog::tzClicked(QListViewItem *item, const QPoint &, int)
{
    if (item && !item->parent())
	item->setOpen(!item->isOpen());
}

static QDialog *createTimeZone(QWidget *parent) {
    return new TimeZoneDialog(parent, 0, TRUE);
}

//===========================================================================

static QDialog *createLanguage(QWidget *parent) {
    LanguageSettings *dlg = new LanguageSettings(parent, 0, 0);
    dlg->setConfirm(FALSE);
    return dlg;
}

static void acceptLanguage(QDialog *dlg)
{
    dlg->accept();
    if (firstUse)
	firstUse->reloadLanguages();
}

static QDialog *createDateTime(QWidget *parent) {
    SetDateTime *dlg = new SetDateTime(parent, 0, TRUE);
    dlg->setTimezoneEditable(FALSE);
    return dlg;
}

static void acceptDialog(QDialog *dlg)
{
    dlg->accept();
}


struct {
    bool enabled;
    QDialog *(*createFunc)(QWidget *parent);
    void (*acceptFunc)(QDialog *dlg);
    const char *trans;
    const char *desc;
    bool needIM;
}
settingsTable[] =
{
    { TRUE, createLanguage, acceptLanguage, "language.qm", QT_TRANSLATE_NOOP("FirstUse", "Language"), FALSE },
    { TRUE, createTimeZone, acceptDialog, "timezone.qm", QT_TRANSLATE_NOOP("FirstUse", "Timezone"), FALSE },
    { TRUE, createDateTime, acceptDialog, "systemtime.qm", QT_TRANSLATE_NOOP("FirstUse", "Date/Time"), TRUE },
    { FALSE, 0, 0, "", "", FALSE }
};


FirstUse::FirstUse(QWidget *parent, const char *name, WFlags f)
    : QDialog(parent, name, TRUE, f), currDlgIdx(-1), currDlg(0),
	needCalibrate(FALSE), needRestart(TRUE)
{
    ServerApplication::allowRestart = FALSE;
    // we force our height beyound the maximum (which we set anyway)
    QRect desk = qApp->desktop()->geometry();
    setGeometry( 0, 0, desk.width(), desk.height() );

    // more hackery
    // It will be run as either the main server or as part of the main server
    QWSServer::setScreenSaverIntervals(0);
    loadPixmaps();

    setFocusPolicy(NoFocus);

    taskBar = new QWidget(0, 0, WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop | WGroupLeader);

#ifdef USE_INPUTMETHODS
    inputMethods = new InputMethods(taskBar);
    connect(inputMethods, SIGNAL(inputToggled(bool)),
	    this, SLOT(calcMaxWindowRect()));
#endif

    back = new QPushButton(tr("<< Back"), taskBar);
    back->setFocusPolicy(NoFocus);
    connect(back, SIGNAL(clicked()), this, SLOT(previousDialog()) );

    next = new QPushButton(tr("Next >>"), taskBar);
    next->setFocusPolicy(NoFocus);
    connect(next, SIGNAL(clicked()), this, SLOT(nextDialog()) );

    // need to set the geom to lower corner
    int x = 0;
    controlHeight = back->sizeHint().height();
    QSize sz(0,0);
#ifdef USE_INPUTMETHODS
    sz = inputMethods->sizeHint();
    inputMethods->setGeometry(0,0, sz.width(), controlHeight );
    x += sz.width();
#endif
    int buttonWidth = (width() - sz.width()) / 2;
    back->setGeometry(x, 0, buttonWidth, controlHeight);
    x += buttonWidth;
    next->setGeometry(x, 0, buttonWidth, controlHeight);

    taskBar->setGeometry( 0, height() - controlHeight, desk.width(), controlHeight);
    taskBar->hide();

    QWidget *w = new QWidget(0);
    w->showMaximized();
    int titleHeight = w->geometry().y() - w->frameGeometry().y();
    delete w;

    titleBar = new QLabel(0, 0, WStyle_Tool | WStyle_Customize | WStyle_StaysOnTop | WGroupLeader);
    QPalette pal = titleBar->palette();
    pal.setBrush(QColorGroup::Background, pal.brush(QPalette::Normal, QColorGroup::Highlight));
    pal.setColor(QColorGroup::Text, pal.color(QPalette::Normal, QColorGroup::HighlightedText));
    titleBar->setPalette(pal);
    titleBar->setAlignment(AlignCenter);
    titleBar->setGeometry(0, 0, desk.width(), titleHeight);
    titleBar->hide();

    calcMaxWindowRect();

#if defined(QPE_NEED_CALIBRATION)
    if ( !QFile::exists("/etc/pointercal") ) {
	needCalibrate = TRUE;
	grabMouse();
    }
#endif
    defaultFont = font();

    reloadLanguages();

    firstUse = this;
}


FirstUse::~FirstUse()
{
    delete taskBar;
    delete titleBar;
    ServerApplication::allowRestart = TRUE;
    firstUse = 0;
}

void FirstUse::calcMaxWindowRect()
{
#ifdef Q_WS_QWS
    QRect wr;
    int displayWidth  = qApp->desktop()->width();
    QRect ir;
# ifdef USE_INPUTMETHODS
    ir = inputMethods->inputRect();
# endif
    if ( ir.isValid() ) {
	wr.setCoords( 0, 0, displayWidth-1, ir.top()-1 );
    } else {
	wr.setCoords( 0, 0, displayWidth-1,
		qApp->desktop()->height() - controlHeight-1);
    }

#if QT_VERSION < 0x030000
    QWSServer::setMaxWindowRect( qt_screen->mapToDevice(wr,
		QSize(qt_screen->width(),qt_screen->height()))
			       );
#else
    QWSServer::setMaxWindowRect( wr );
#endif
#endif
}

/* accept current dialog, and bring up next */
void FirstUse::nextDialog()
{
    if (currDlg)
	settingsTable[currDlgIdx].acceptFunc(currDlg);
    currDlgIdx = findNextDialog(TRUE);
}

/* accept current dialog and bring up previous */
void FirstUse::previousDialog()
{
    if (currDlgIdx != 0) {
	if (currDlg)
	    settingsTable[currDlgIdx].acceptFunc(currDlg);
	currDlgIdx = findNextDialog(FALSE);
    }
}

void FirstUse::switchDialog()
{
    if (currDlgIdx == -1) {
	qDebug( "Done!" );
	{
	    Config config( "qpe" );
	    config.setGroup( "Startup" );
	    config.writeEntry( "FirstUse", FALSE );
	    config.write();
        } 
        Config cfg = Config( "WorldTime" );
        cfg.setGroup( "TimeZones" );
       
        // translate the existing list of TimeZone names
        // This is usually enforced during the startup of qpe 
        // (main.cpp::refreshTimeZoneConfig). However when we use
        // First use we have to do it here again to ensure a
        // translation
        int zoneIndex = 0;
        TimeZone curZone;
        QString zoneID;
 
        while (cfg.hasKey( "Zone"+ QString::number( zoneIndex ))){
            zoneID = cfg.readEntry( "Zone" + QString::number( zoneIndex ));
            curZone = TimeZone( zoneID );
            if ( !curZone.isValid() ){
                qDebug( "initEnvironment() Invalid TimeZone %s", zoneID.latin1() );
                break;
            }
            cfg.writeEntry( "ZoneName" + QString::number( zoneIndex ), curZone.city() );
            zoneIndex++;
    }

#ifndef QTOPIA_PHONE
	QPixmap pix = Resource::loadPixmap("bigwait");
	QLabel *lblWait = new QLabel(0, "wait hack!", // No tr
		QWidget::WStyle_Customize | QWidget::WDestructiveClose |
		QWidget::WStyle_NoBorder | QWidget::WStyle_Tool |
		QWidget::WStyle_StaysOnTop);
	lblWait->setPixmap( pix );
	lblWait->setAlignment( QWidget::AlignCenter );
	lblWait->setGeometry( qApp->desktop()->geometry() );
	lblWait->show();
	qApp->processEvents();
	QTimer::singleShot( 1000, lblWait, SLOT(close()) );
#endif
	accept();
	ServerApplication::allowRestart = TRUE;
    } else {
	updateButtons();
	currDlg = settingsTable[currDlgIdx].createFunc(this);
	currDlg->showMaximized();
	currDlg->exec();
	delete currDlg;
	currDlg = 0;
	QTimer::singleShot(0, this, SLOT(switchDialog()));
    }
}

void FirstUse::reloadLanguages()
{
    // read language from config file.  Waiting on QCop takes too long.
    Config config("locale");
    config.setGroup( "Language");
    QString l = config.readEntry( "Language", "en_US");
    QString cl = getenv("LANG");
    qWarning("language message - " + l);
    // setting anyway...
    if (l.isNull() )
	unsetenv( "LANG" );
    else {
	qWarning("and its not null");
	setenv( "LANG", l.latin1(), 1 );
    }
#ifndef QT_NO_TRANSLATION
    // clear old translators
#ifndef _MSC_VER
  //### revise to allow removal of translators under MSVC
  if(qApp->translators) {
        qApp->translators->setAutoDelete(TRUE);
        delete (qApp->translators);
        qApp->translators = 0;
    }
#endif

    const char *qmFiles[] = { "qt.qm", "qpe.qm", "libqpe.qm", "libqtopia.qm" , 0 };

    // qpe/library translation files.
    int i = 0;
    QTranslator *trans;
    while (qmFiles[i]) {
	trans = new QTranslator(qApp);
	QString atf = qmFiles[i];
	QString tfn = QPEApplication::qpeDir() + "i18n/"+l+"/"+atf;
	qWarning("loading " + tfn);
	if ( trans->load(tfn) ) {
	    qWarning(" installing translator");
	    qApp->installTranslator( trans );
	} else  {
	    delete trans;
	}
	i++;
    }

    // first use dialog translation files.
    i = 0;
    while (settingsTable[i].createFunc) {
	if (settingsTable[i].enabled && settingsTable[i].trans) {
	    trans = new QTranslator(qApp);
	    QString atf = settingsTable[i].trans;
	    QString tfn = QPEApplication::qpeDir() + "i18n/"+l+"/"+atf;
	    qWarning("loading " + tfn);
	    if ( trans->load(tfn) ) {
		qWarning(" installing translator");
		qApp->installTranslator( trans );
	    } else  {
		delete trans;
	    }
	}
	i++;
    }

    loadPixmaps();

    QStringList qpepaths = Global::qtopiaPaths();
    for (QStringList::Iterator qit=qpepaths.begin(); 
            qit != qpepaths.end(); ++qit ) {
        QTranslator t(0);
        QString tfn = *qit+"i18n/";
        if (t.load(tfn+l+"/QtopiaDefaults.qm")) {
            Config fmcfg("FontMap");
            fmcfg.setGroup("Map");
            //we assume the user wants normal font size
            QString bf=fmcfg.readEntry("Font1[]"); 
            QStringList fs = QStringList::split(',',t.findMessage("FontMap",bf,0).translation());
            if ( fs.count() == 3 ) {
                //qDebug("%s %s", fs[1].latin1(), fs[2].latin1());
                QFont f(fs[1]);
                f.setPointSize(fs[2].toInt());
                qApp->setFont(f, TRUE);
            }
        } else{ //drop back to default font
            qApp->setFont(QFont("helvetica"), TRUE);
        }
            
    }
#endif
    updateButtons();
}

void FirstUse::paintEvent( QPaintEvent * )
{
    QPainter p( this );

    p.drawPixmap(0,0, splash);

    QFont f = p.font();
    f.setPointSize(15);
    f.setItalic(FALSE);
    f.setBold(FALSE);
    p.setFont(f);

    if ( currDlgIdx < 0 ) {
	drawText(p, tr( "Tap anywhere on the screen to continue." ));
    } else if ( settingsTable[currDlgIdx].createFunc ) {
	drawText(p, tr("Please wait, loading %1 settings.").arg(tr(settingsTable[currDlgIdx].desc)) );
    } else {
	drawText(p, tr("Please wait..."));
    }
}

void FirstUse::loadPixmaps()
{
    /* create background, tr so can change image with language.
       images will likely contain text. */
    splash.convertFromImage( Resource::loadImage("FirstUseBackground") //No tr
	    .smoothScale( width(), height() ) );

    setBackgroundPixmap(splash);
}

void FirstUse::drawText(QPainter &p, const QString &text)
{
    QString altered = "<CENTER>" + text + "</CENTER>";

    QSimpleRichText rt(altered, p.font());
    rt.setWidth(width() - 20);

    int h = (height() * 3) / 10; // start at 30%
    if (rt.height() < height() / 2)
	h += ((height() / 2) - rt.height()) / 2;
    rt.draw(&p, 10, h, QRegion(0,0, width()-20, height()), palette());
}

int FirstUse::findNextDialog(bool forwards)
{
    int i;
    if (forwards) {
	i = currDlgIdx+1;
	while ( settingsTable[i].createFunc && !settingsTable[i].enabled )
	    i++;
	if ( !settingsTable[i].createFunc )
	    i = -1;
    } else {
	i = currDlgIdx-1;
	while ( i >= 0 && !settingsTable[i].enabled )
	    i--;
    }

    return i;
}

void FirstUse::updateButtons()
{
    if ( currDlgIdx >= 0 ) {
#ifdef USE_INPUTMETHODS
	inputMethods->setEnabled(settingsTable[currDlgIdx].needIM);
#endif
	taskBar->show();
	titleBar->setText("<b>"+tr(settingsTable[currDlgIdx].desc)+"</b>");
	titleBar->show();
    }

    int i = findNextDialog(FALSE);
    back->setText(tr("<< Back"));
    back->setEnabled( i >= 0 );

    i = findNextDialog(TRUE);
    if ( i < 0)
	next->setText(tr("Finish"));
    else
	next->setText(tr("Next >>"));
    next->setEnabled( TRUE );
}

void FirstUse::keyPressEvent( QKeyEvent *e )
{
    // Allow cancelling at first dialog, in case display is broken.
    if ( e->key() == Key_Escape && currDlgIdx < 0 )
	QDialog::keyPressEvent(e);
}

void FirstUse::mouseReleaseEvent( QMouseEvent * )
{
    if ( currDlgIdx < 0 ) {
#if defined(QPE_NEED_CALIBRATION)
	if ( needCalibrate ) {
	    releaseMouse();
	    Calibrate *cal = new Calibrate;
	    cal->exec();
	    delete cal;
	}
#endif
	currDlgIdx = 0;
	currDlg = 0;
	QTimer::singleShot(0, this, SLOT(switchDialog()));
    }
}

#include "firstuse.moc"
