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
#include "mainwindow.h"
#include "charsetedit.h"
#include "gprefbase.h"
#include <qtopia/qpeapplication.h>
#include <qtopia/config.h>
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/contextmenu.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/resource.h>
#include <qdir.h>
#include <qaction.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qheader.h>

class ProfileItem : public QObject, public QListViewItem
{
    Q_OBJECT
public:
    ProfileItem(ProfileItem *parent, const QString &name, QIMPenCharSet *s)
	: QListViewItem(parent, name), mSet(s), mProf(0)
	{ QListViewItem::setOpen(TRUE); }
    ProfileItem(QListView *parent, const QString &name, QIMPenProfile *p)
	: QListViewItem(parent, name), mSet(0), mProf(p)
	{ QListViewItem::setOpen(TRUE); }
    ~ProfileItem() {}


    ProfileItem *parent() {return (ProfileItem*)QListViewItem::parent();}

    void setOpen(bool)
    {
	emit selected(this);
    }

    QIMPenCharSet *set() const { return mSet; }
    QIMPenProfile *profile() const { return mProf; }

signals:
    void selected(QListViewItem *);

private:
    QIMPenCharSet *mSet;
    QIMPenProfile *mProf;
};


/*
   Edit of a single profile.  loadProfiles later moved elsewhere when
   is to handle multiple profiles? (second level of list?)
 */
QIMPenProfileEdit::QIMPenProfileEdit(QWidget *parent, const char *name, int f)
    : QMainWindow(parent, name, f), cdiag(0), gdiag(0)
{
    setCaption(tr("Handwriting"));

    lv = new QListView(this);
    lv->setFrameStyle(QFrame::NoFrame);
    lv->addColumn("");
    setCentralWidget(lv);
    lv->header()->hide();

    loadProfiles();

    connect(lv, SIGNAL(returnPressed(QListViewItem*)), this, SLOT(editItem(QListViewItem*)));
    connect(lv, SIGNAL(clicked(QListViewItem*)), this, SLOT(editItem(QListViewItem*)));
#ifdef QTOPIA_PHONE
    new ContextMenu( this );
#endif        
}

QIMPenProfileEdit::~QIMPenProfileEdit()
{
}

// of course, should not be here...
bool QIMPenProfileEdit::loadProfiles()
{
    lv->clear();
    // different profile for Fullscreen to nomral?
    profileList.clear();
    QString path = QPEApplication::qpeDir() + "etc/qimpen"; // no tr
    QDir dir( path, "*.conf" ); // no tr
    QStringList list = dir.entryList();
    QStringList::Iterator it;
    // XXX  do as tree instead.
    for ( it = list.begin(); it != list.end(); ++it ) {
	QIMPenProfile *p = new QIMPenProfile( path + "/" + *it ); // no tr
	profileList.append( p );
    }
    if (profileList.count() < 1) {
	// QMessage box and quit
	QMessageBox::information(0, tr("No profiles found"),
		tr("<qt>Unable to find any handwriting profiles."),
		QMessageBox::Ok);
	return FALSE;
    }
    // add items in list (and their parts) to view
    for ( int i = 0; i < (int)profileList.count(); i++ ) {
	QIMPenProfile *prof = profileList.at(i);

	ProfileItem *pi = new ProfileItem(lv, prof->name(), prof);

	QStringList cSets = prof->charSets();
	QStringList::Iterator it;
	for (it = cSets.begin(); it != cSets.end(); ++it) {
	    ProfileItem *si = new ProfileItem(pi, prof->title(*it), prof->charSet(*it));
	    connect(si, SIGNAL(selected(QListViewItem *)), this, SLOT(editItem(QListViewItem *)));
	}
	pi->setOpen(TRUE);
	connect(pi, SIGNAL(selected(QListViewItem *)), this, SLOT(editItem(QListViewItem *)));
    }

    if (lv->firstChild() != 0)
	lv->setCurrentItem(lv->firstChild());

    // still interesting to know the 'default'
    return TRUE;
}

bool QIMPenProfileEdit::saveProfiles()
{
    // Save charsets
    bool ok = TRUE;
    for ( int i = 0; i < (int)profileList.count(); i++ ) {
	QIMPenProfile *prof = profileList.at(i);
	QStringList cSets = prof->charSets();
	QStringList::Iterator it;
	for ( it = cSets.begin(); it != cSets.end(); ++it ) {
	    QIMPenCharSet *set = prof->charSet(*it);
	    if ( !set->save() ) {
		ok = FALSE;
		break;
	    }
	}
    }
    if ( !ok ) {
	if ( QMessageBox::critical( 0, tr( "Out of space" ),
		    tr("<qt>Unable to save information. "
			"Free up some space "
			"and try again."
			"<br>Quit anyway?</qt>"),
		    QMessageBox::Yes|QMessageBox::Escape,
		    QMessageBox::No|QMessageBox::Default )
		!= QMessageBox::No ) {
	    ok = TRUE;
	}
    } else {
	QCopEnvelope e("QPE/Handwriting", "settingsChanged()"); // or, via caching and time-stamps. // no tr
    }
    return ok;
}

void QIMPenProfileEdit::editItem(QListViewItem *i)
{
    if (!i)
	return;
    // XXX to item which might be a or be
    ProfileItem *pi = (ProfileItem *)i;
    if (pi->set()) {
	if (cdiag == 0) {
	    cdiag = new CharSetDlg(this, "chng", TRUE);
	}
	cdiag->setCharSet(pi->set());
	cdiag->setIsFS(((ProfileItem *)pi->parent())->profile()->canIgnoreStroke());

	if (QPEApplication::execDialog(cdiag)) {
	    // need to save the changes
	    pi->set()->save();
	    QCopEnvelope e("QPE/Handwriting", "settingsChanged()"); // no tr
	} else {
	    loadProfiles(); // only way to get combining data as well in set.
	}

    } else if (pi->profile()) {
	if (gdiag == 0) {
	    gdiag = new QDialog(this, 0, TRUE);
	    gdiag->setCaption(tr("Handwriting"));

	    QHBoxLayout *hl = new QHBoxLayout(gdiag);
	    gpb = new GeneralPrefBase(gdiag);
	    hl->addWidget(gpb);
	}
	QIMPenProfile *profile = pi->profile();

	gpb->multiStrokeSlider->setValue(profile->multiStrokeTimeout());
	gpb->ignoreStrokeSlider->setValue(profile->ignoreStrokeTimeout());
	gpb->inputStyle->setChecked(profile->style() == QIMPenProfile::BothCases);
	if (profile->canSelectStyle())
	    gpb->inputStyle->show();
	else
	    gpb->inputStyle->hide();

	if (profile->canIgnoreStroke()) {
	    gpb->ignoreStrokeLabel->show();
	    gpb->ignoreStrokeSlider->show();
	} else {
	    gpb->ignoreStrokeLabel->hide();
	    gpb->ignoreStrokeSlider->hide();
	}

	if (QPEApplication::execDialog(gdiag)) {
	    profile->setMultiStrokeTimeout(gpb->multiStrokeSlider->value());
	    profile->setIgnoreStrokeTimeout(gpb->ignoreStrokeSlider->value());
	    profile->setStyle(gpb->inputStyle->isChecked() ? QIMPenProfile::BothCases : QIMPenProfile::ToggleCases);
	    profile->save();
	    QCopEnvelope e("QPE/Handwriting", "settingsChanged()"); // no tr
	}
    }
}

#include "mainwindow.moc"
