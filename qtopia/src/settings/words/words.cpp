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

#define QTOPIA_INTERNAL_DICTOPERATIONS

#include "words.h"
#include "../../3rdparty/plugins/inputmethods/pkim/pkimmatcher.h"

#include <qtopia/global.h>
#include <qtopia/contextmenu.h>
#include <qtopia/resource.h>
#include <qtopia/contextbar.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/qpetoolbar.h>
#include <qtopia/config.h>

#include <qaction.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qdict.h>
#include <qvbox.h>
#include <qlistbox.h>
#include <qsizepolicy.h>

#include <stdlib.h>

class WordListItem : public QListBoxText {
    int maxlen;
    QString lang;
    const QFont *lfont;
public:
    WordListItem(QListBox* parent, const QString& str, int len, const QString& language, const QFont* f) :
	QListBoxText(parent,str),
	maxlen(len),
	lang(language),
	lfont(f ? new QFont(*f) : 0)
    {
    }

    ~WordListItem()
    {
	delete lfont;
    }

    void paint( QPainter *painter )
    {
	QFontMetrics fm = painter->fontMetrics();
	QString txt = text();
	int x=3;
	int y = fm.ascent() + fm.leading()/2;
	QFont fnt = painter->font();
	fnt.setBold(TRUE);
	painter->setFont(fnt);
	painter->drawText( x, y, txt, maxlen );
	x += fm.width(txt.left(maxlen)); // before setting font unbold
	if ( maxlen >= 0 && (int)txt.length() > maxlen ) {
	    fnt.setBold(FALSE);
	    painter->setFont(fnt);
	    painter->drawText( x, y, txt.mid(maxlen) );
	}
	if ( !lang.isEmpty() ) {
	    x += fm.width(txt.mid(maxlen)) + fm.width(' ');
	    if ( lfont ) fnt = *lfont;
	    fnt.setItalic(TRUE);
	    painter->setFont(fnt);
	    painter->drawText( x, y, "("+lang+")" );
	}
    }
};

Words::Words( QWidget* parent,  const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    setCaption(tr("Words"));

    QVBox *central = new QVBox(this);
    setCentralWidget(central);

    line = new QLineEdit(central);

    box = new QListBox(central);
    box->setFocusPolicy(NoFocus);
    QPalette pal = box->palette();
    pal.setBrush(QPalette::Normal, QColorGroup::Background, pal.normal().brush(QColorGroup::Base));
    box->setPalette(pal);

    a_add = new QAction( tr("Add"), Resource::loadIconSet("add"), QString::null, 0, this );
    connect( a_add, SIGNAL(activated()), this, SLOT(addWord()) );
    a_del = new QAction( QString::null, Resource::loadIconSet("trash"), QString::null, 0, this );
    connect( a_del, SIGNAL(activated()), this, SLOT(deleteWord()) );
    a_prefer = new QAction( tr("Prefer"), Resource::loadIconSet("prefer"), QString::null, 0, this );
    connect( a_prefer, SIGNAL(activated()), this, SLOT(preferWord()) );

    QActionGroup *mode = new QActionGroup(this);
    mode->setExclusive(TRUE);

    a_alllocal = new QAction( tr("All Added"), Resource::loadIconSet("addedwords"), QString::null, 0, mode );
    connect( a_alllocal, SIGNAL(activated()), this, SLOT(showAddedDict()) );
    a_alllocal->setToggleAction(TRUE);
    a_allpref = new QAction( tr("All Preferred"), Resource::loadIconSet("preferredwords"), QString::null, 0, mode );
    connect( a_allpref, SIGNAL(activated()), this, SLOT(showPreferredDict()) );
    a_allpref->setToggleAction(TRUE);
    a_alldel = new QAction( tr("All Deleted"), Resource::loadIconSet("deletedwords"), QString::null, 0, mode );
    connect( a_alldel, SIGNAL(activated()), this, SLOT(showDeletedDict()) );
    a_alldel->setToggleAction(TRUE);

    a_pkim = new QAction( tr("Number Match"), QString::null, 0, mode );
    a_word = new QAction( tr("Word Lookup/Add"), Resource::loadIconSet("find"), QString::null, 0, mode );
    a_pkim->setToggleAction(TRUE);
    a_word->setToggleAction(TRUE);
    connect( mode, SIGNAL(selected(QAction*)), this, SLOT(modeChanged()) );

    tooltip = new QLabel(this, "TooltipLabel");
    tooltip->setFrameStyle(QFrame::Panel|QFrame::Raised);
    tooltip->setAlignment(Qt::AlignCenter | Qt::WordBreak);
    tooltip->setBackgroundMode(QWidget::PaletteBackground);
    tooltip->setAutoResize(TRUE);
    tooltip->setMaximumWidth(150);
    tooltip->hide();

#ifdef QTOPIA_PHONE
    ContextMenu *menu = new ContextMenu(this);

    // XXX Need to turn off cut/paste menu for line,
    // XXX so the above is used. For now, duplicated it...
    ContextMenu *linemenu = new ContextMenu(line,0);
    for (int i=0; i<=1; i++) {
	// XXX see linemenu above
	ContextMenu *m = i ? menu : linemenu;
	a_add->addTo(m);
	a_del->addTo(m);
	a_prefer->addTo(m);
	m->insertSeparator();
	if ( !Global::mousePreferred() )
	    a_pkim->addTo(m);
	a_word->addTo(m);
	a_alllocal->addTo(m);
	if ( !Global::mousePreferred() )
	    a_allpref->addTo(m);
	a_alldel->addTo(m);
    }
    if ( Global::mousePreferred() )
	a_word->setOn(TRUE);
    else
	a_pkim->setOn(TRUE);
#else
    // Create Toolbars
    QPEToolBar *bar = new QPEToolBar( this );
    bar->setHorizontalStretchable( TRUE );
    setToolBarsMovable( FALSE );

    a_add->addTo(bar);
    a_del->addTo(bar);
    //a_prefer->addTo(bar);
    bar->addSeparator();
    //a_pkim->addTo(bar);
    a_word->addTo(bar);
    a_alllocal->addTo(bar);
    //a_allpref->addTo(bar);
    a_alldel->addTo(bar);

    a_word->setOn(TRUE);
#endif

    connect(line,SIGNAL(textChanged(const QString&)), this, SLOT(lookup()));
    connect(box,SIGNAL(currentChanged(QListBoxItem*)), this, SLOT(updateActions()));

    modeChanged();
    updateActions();
}

Words::~Words()
{
}

void Words::updateActions()
{
    a_add->setEnabled(
	    a_word->isOn()
	    && line->text().length() > 0
	    && box->text(0) != line->text()
	 || a_alldel->isOn()
	    && box->currentItem()>=0
	);
    a_del->setEnabled( !a_alldel->isOn() &&
	    !dummy && box->currentItem()>=0 );
    a_del->setText(
	    a_allpref->isOn() ? tr("Unprefer")
	    : a_alldel->isOn() ? tr("Undelete")
	    : tr("Delete"));
    a_prefer->setEnabled(
	    a_pkim->isOn()
	    && box->currentItem()>=1 // ie. not already preferred
	    && box->text(box->currentItem()).length() == line->text().length() // not a prefix
	    && box->count()>1
	    && box->text(1).length() == line->text().length()); // ie. more than one choice
}

#include "../language/langname.h"

void Words::search()
{
    QString pattern = line->text();
    QStringList langs = PkIMMatcher::chosenLanguages();
    QDict<void> seen;
    for (int prefix=0; prefix<=(pattern.length()>3?1:0); prefix++) {
	bool firstlang=TRUE;
	QString fl = languageName(*langs.begin(),0);
	for (QStringList::ConstIterator lit = langs.begin(); lit!=langs.end(); ++lit) {
	    // could pre-construct dictionarys.
	    PkIMMatcher matcher("dict-"+*lit);
	    if (matcher.isValid()) {
		bool ok=TRUE;
		if (a_word->isOn())
		    matcher.matchSet(line->text());
		else 
		    ok=matcher.match(line->text());
		if ( ok ) {
		    QStringList r = matcher.choices(FALSE, prefix);

		    QFont f = font();
		    QString lname;
		    if ( !firstlang ) {
			lname = languageName(*lit,&f);
			// Make English (U.S.) look better when base lang is English (British)
			int paren = lname.find('(');
			if ( paren >= 0 && lname.left(paren)==fl.left(paren) )
			    lname = lname.mid(paren+1,lname.length()-paren-2);
		    }
		    QFont *lnfont = f==font() ? 0 : &f;

		    for (QStringList::ConstIterator it = r.begin(); it!=r.end(); ++it)
			if ( !prefix || (*it).length() != pattern.length() ) {
			    if ( !seen.find(*it) ) {
				seen.insert(*it,(void*)1);
				new WordListItem(box,*it,pattern.length(),lname,lnfont);
			    }
			}
		}
	    }
	    firstlang=FALSE;
	}
    }
}

void Words::lookup()
{
    QString pattern = line->text();
    for (int i=0; i<(int)pattern.length(); ++i) {
	if ( pattern[i] <= ' ' )
	    pattern.remove(i,1);
    }
    if ( pattern != line->text() ) {
	line->setText(pattern);
	return; // already been here
    }
    int cur = 0;
    dummy = FALSE;
    if (a_word->isOn()) {
	/* user has type a specific word, and wants to know if it
	   is in the dictinary */
	box->clear();
	if ( pattern.length() > 0 ) {
	    search();
            tooltip->hide();
	}
	if ( pattern.length() <= 3 && box->count() == 0 ) {
            tooltip->setText(tr("Type letters to match."));
            tooltip->resize(tooltip->maximumWidth(), tooltip->heightForWidth(tooltip->maximumWidth()));
            tooltip->raise();
            tooltip->show();
	    dummy = TRUE;
	}
    } else if ( a_pkim->isOn() ) {
	/* user has done what would have been dict lookup, 
	   find out what the word was */
	box->clear();
	if ( pattern.length() > 0 ) {
	    search();
            tooltip->hide();
	}
	if ( pattern.length() <= 3 && box->count() == 0 ) {
            tooltip->setText(tr("Type numbers to match."));
            tooltip->resize(tooltip->maximumWidth(), tooltip->heightForWidth(tooltip->maximumWidth()));
            tooltip->raise();
            tooltip->show();
	    dummy = TRUE;
	}
    } else {
	/* the user has selected a list (like added words) and wants
	   to jump to a point in it */
	/* note the lack of call to search? its because no list is made */
	cur = -1;
	for (int i=0; i<(int)box->count(); i++) {
	    QString t = box->text(i);
	    // is this dictionary depended?
	    // no, just set dependent.
	    PkIMMatcher matcher("ext");
	    if ( matcher.match(t,0,pattern,pattern.length()) == (int)pattern.length() ) {
		cur = i;
		break;
	    }
	}
    }
    if ( !dummy && box->count() ) {
	box->setCurrentItem(cur);
	if ( cur >= 0 )
	    box->setSelected(cur,TRUE);
	else
	    box->clearSelection();
    }
    updateActions();
}

void Words::modeChanged()
{
    lookup();
#ifdef QTOPIA_PHONE
    if ( a_word->isOn()) {
	QPEApplication::setInputMethodHint(line,QPEApplication::Text);
    } else {
	QPEApplication::setInputMethodHint(line,QPEApplication::Number);
    }
#endif
    line->clear(); // Old text not useful.
}

QSize Words::sizeHint() const
{
    return QSize(-1,400);
}

void Words::resizeEvent(QResizeEvent *resizeEvent)
{
    QWidget::resizeEvent(resizeEvent);
    tooltip->move((width()-tooltip->width())/2, (height()-tooltip->height())/3*2);
}

void Words::addWord()
{
    int i = box->currentItem();
    if ( a_alldel->isOn() && i>=0 ) {
	Global::addWords(box->currentText()); // add it back
	showDeletedDict();
    } else {
	Global::addWords(line->text());
    }
    lookup();
}

void Words::deleteWord()
{
    QString w = box->currentText();
    int i = box->currentItem();
    box->removeItem(box->currentItem());
    Global::removeWords("preferred",w); // No tr
    if ( !a_allpref->isOn() )
	Global::removeWords(w);
    if ( i == (int)box->count() )
	i--;
    box->setCurrentItem(i);
    box->setSelected(i,TRUE);
}

void Words::preferWord()
{
    QStringList nopref;
    for (int i=0; i<(int)box->count(); i++) {
	if ( i!=box->currentItem() )
	    nopref.append(box->text(i));
    }
    Global::removeWords("preferred",nopref);
    Global::addWords("preferred",box->text(box->currentItem()));
    lookup();
}

void Words::showDict(const char* name)
{
    box->clear();
    dummy = FALSE;
    tooltip->hide();
    box->insertStringList(Global::dawg(name).allWords());
    box->setCurrentItem(0);
    box->setSelected(0,TRUE);
    updateActions();
}

void Words::showAddedDict()
{
    showDict("local"); // No tr
}

void Words::showPreferredDict()
{
    showDict("preferred"); // No tr
}

void Words::showDeletedDict()
{
    showDict("deleted"); // No tr
}

void Words::keyPressEvent(QKeyEvent* e)
{
    if ( e->type()==QEvent::KeyPress ) {
	int ch=0;
	switch ( e->key() ) {
	    case Key_Up:
		ch = -1;
		break;
	    case Key_Down:
		ch = 1;
		break;
	}
	if ( box->currentItem() >= 0 && ch ) {
            int i = (box->currentItem()+ch+box->count())%box->count();
            box->setCurrentItem(i);
            box->setSelected(i,TRUE);
	    e->accept();
	    return;
        }
    }
    QMainWindow::keyPressEvent(e);
}
