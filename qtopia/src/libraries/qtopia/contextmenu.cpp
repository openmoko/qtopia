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
#define INCLUDE_MENUITEM_DEF
#define QTOPIA_INTERNAL_LANGLIST
#include <qpopupmenu.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <qfile.h>
#include <qaction.h>
#include <qgfx_qws.h>

#include <qtopia/qpeapplication.h>
#include <qtopia/global.h>
#include <qtopia/resource.h>
#include <qtopia/contextbar.h>
#include "contextmenu.h"

#ifdef QTOPIA_PHONE
extern bool mousePreferred; // can't call Global::mousePreferred in libqtopia2 from libqtopia
#endif

extern const QArray<int> qpe_systemButtons();

QIconSet qtopia_internal_loadIconSet( const QString &pix );

class ContextMenuData
{
public:
    ContextMenuData()
    {
	adjustHeight = FALSE;
	shown = FALSE;
	focusWidget = 0;
	helpexists = FALSE;
	cancelId = -1;
	helpId = -1;
	helpEnabled = TRUE;
        hasHelp = FALSE;
	sepId = -1;
    }
    
    ~ContextMenuData()
    {
    }

    bool adjustHeight;
    QWidget *focusWidget;
    bool shown;
    QString helpFile;
    bool helpexists;
    int cancelId;
    int helpId;
    bool helpEnabled;
    bool hasHelp;
    int sepId;
    QMap<QWidget*, QWidget*> focusWidgetMap;
};

static QMap<QWidget*, ContextMenu *> *modalMenuMap=0;
static QMap<QWidget*, ContextMenu *> *nonModalMenuMap=0;

/*!
  \class ContextMenu
  \brief The ContextMenu class provides a QPopupMenu that is bound to a
  context button.

  The Context Bar will be updated when the ContextMenu is available.  Pressing
  the context button will popup the ContextMenu.

  Note that the rules for context button assignment are identical as
  those for ContextBar, i.e. if the current focus widget does not claim
  the context key or have a context menu, then the context menu or label of
  an ancestor will be active.

  ContextMenu is only available in the Qtopia Phone Edition.

  \sa ContextBar

  \ingroup qtopiaphone
*/

/*!
  Constructs a ContextMenu which will popup when Key_Menu is pressed and
  widget \a parent has focus and is in modal editing state \a state.
    The \a parent and \a name parameters are the standard Qt parent parameters.

  \sa ContextBar
*/
ContextMenu::ContextMenu(QWidget *parent, const char *name, int state)
    : QPopupMenu(parent, name)
{
    d = new ContextMenuData();
    init();
    if (parent)
	addTo(parent, state);
}

void ContextMenu::init()
{
    if (!modalMenuMap) {
	modalMenuMap = new QMap<QWidget*, ContextMenu*>;
	nonModalMenuMap = new QMap<QWidget*, ContextMenu*>;
    }
    hide();
    ContextBar::setLabel(this, key(), ContextBar::Options, ContextBar::ModalAndNonModal);
}

/*!
    Destructs the ContextMenu
*/
ContextMenu::~ContextMenu()
{
    QMap<QWidget*, QWidget*>::Iterator it = d->focusWidgetMap.begin();
    while (it != d->focusWidgetMap.end()) {
	QWidget *w = *it;
	++it;
	removeFrom(w);
    }
    delete d;
}

/*!
  Sets whether help is enabled in the context menu.
*/
void ContextMenu::setEnableHelp(bool b)
{
    d->helpEnabled = b;
    // if already shown?
    if (d->helpId != -1) {
	setItemEnabled(d->helpId, b);
    }
}

/*!
  Adds this ContextMenu to widget \a w for modal editing state \a state.

  \sa removeFrom()
*/
void ContextMenu::addTo(QWidget *w, int state)
{
    if (!w)
	return;

    if (!state)
	state = Modal | NonModal;

    QWidget *fw = w;
    while (fw->focusProxy())
	fw = fw->focusProxy();

    if (!d->focusWidgetMap.contains(fw)) {
	fw->installEventFilter(this);
	d->focusWidgetMap[fw] = w;
    }
    ContextBar::setLabel(fw, key(), ContextBar::Options, (ContextBar::EditingState)state);

    if (state & Modal)
	modalMenuMap->insert(w, this);
    if (state & NonModal)
	nonModalMenuMap->insert(w, this);

    connect(w, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
}

/*!
  Removes this ContextMenu from widget \a w for modal editing state \a state.

  \sa addTo()
*/
void ContextMenu::removeFrom(QWidget *w, int state)
{
    if (!state)
	state = Modal | NonModal;

    QWidget *fw = w;
    while (fw->focusProxy())
	fw = fw->focusProxy();

    bool haveModal = internalMenuFor(w, Modal) == this;
    bool haveNonModal = internalMenuFor(w, NonModal) == this;

    if (state & Modal && haveModal) {
	ContextBar::clearLabel(fw, key(), ContextBar::Modal);
	modalMenuMap->remove(w);
	haveModal = FALSE;
    }
    if (state & NonModal && haveNonModal) {
	ContextBar::clearLabel(fw, key(), ContextBar::NonModal);
	nonModalMenuMap->remove(w);
	haveNonModal = FALSE;
    }

    if (!haveModal && !haveNonModal) {
	fw->removeEventFilter(fw);
	d->focusWidgetMap.remove(fw);
    }
    disconnect(w, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
}

/*!
  Inserts action \a a at position \a idx in the menu.

  \sa addAction()
*/
void ContextMenu::insertAction(QAction *a, int idx)
{
    if (!a)
	return;

    int curCount = (int)count();
    a->addTo( this );
    if (idx >= 0 && idx < curCount-1 && (int)count() > curCount) {
	// a could be an action group. need to insert the group at idx
	// have to add all of them using addTo, then move them to the
	// desired position 
	// can't copy from a separate list because ids need to be preserved
	while( curCount < (int)count() )
	{
	    QMenuItem *mi = mitems->take( curCount );
	    if( !mi )
	    {
		qDebug("BUG : ContextMenu::insertAction can't take null item");
		return;
	    }
	    mitems->insert(idx++, mi);

	    // Make sure menuContentsChanged() is called
	    int acc = accel(mi->id());
	    setAccel(acc, mi->id());
	    ++curCount;
	}
    }
}

/*!
  Adds action \a a to the end of the menu.

  \sa insertAction()
*/
void ContextMenu::addAction(QAction *a)
{
    if (a)
	a->addTo(this);
}

/*!
  Returns the ContextMenu assigned to widget \a w in modal editing
  state \a state.  If a ContextMenu does not yet exist for the widget,
  an empty menu will be created.

*/
ContextMenu *ContextMenu::menuFor(QWidget *w, int state)
{
    if (!state)
	state = Modal | NonModal;

    ContextMenu *m = internalMenuFor(w, state);
    if (!m && (w->inherits("QLineEdit") || w->inherits("QMultiLineEdit"))) {
	m = createEditMenu(w, state);
    } else if (!m) {
	m = new ContextMenu(w, 0, state);
    }

    return m;
}

ContextMenu *ContextMenu::createEditMenu(QWidget *w, int state)
{
    ContextMenu *editMenu = new ContextMenu(w, "EditMenu", state); 
    editMenu->insertItem(qtopia_internal_loadIconSet("copy"), tr("Copy"),1);
    editMenu->insertItem(qtopia_internal_loadIconSet("paste"), tr("Paste"),2);
    connect(editMenu, SIGNAL(activated(int)),
	    editMenu, SLOT(editMenuActivated(int)));
    return editMenu;
}

void ContextMenu::editMenuActivated(int i)
{
    int ascii=0;
    int scan=0;

    switch (i) {
	case 0:	
	    ascii='x'; scan=Key_X; // Cut
	    break;
	case 1:
	    ascii='c'; scan=Key_C; // Copy
	    break;
	case 2:
	    ascii='v'; scan=Key_V; // Paste
    }

    if (scan && qApp->focusWidget()) {
	QKeyEvent *ke = new QKeyEvent(QEvent::KeyPress, scan, ascii, ControlButton);
	QApplication::postEvent(qApp->focusWidget(), ke);
	ke = new QKeyEvent(QEvent::KeyRelease, scan, ascii, ControlButton);
	QApplication::postEvent(qApp->focusWidget(), ke);
    }
}

/*!
  Returns the context menu key.
*/
int ContextMenu::key()
{
    static int k = -1;
    if (k < 0) {
	if (!qpe_systemButtons().contains(Key_Menu)) {
	    if (qpe_systemButtons().contains(Key_Context1))
		k = Key_Context1;
	    else
		qWarning("Cannot map key to ContextMenu");
	} else {
	    k = Key_Menu;
	}
    }

    return k;
}

extern QRect qt_maxWindowRect;

#include <qfileinfo.h>
static QString findHelp(const QWidget* w)
{
    // seems like we sometimes start an app with just the name and sometimes with a full path.
    // So make sure we only use the basename here
    QFileInfo fi( QString(qApp->argv()[0]) );
    QString hf = fi.baseName();
    if ( w ) {
        hf += "-";
        hf += w->name();
    }
    hf += ".html";
    QStringList helpPath = Global::helpPath();
    for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end(); ++it) {
        if ( QFile::exists( *it + "/" + hf ) ) {
//            qDebug(">>> Using help \"%s\" <<<",hf.latin1());
            return hf;
            break;
        }
    }

    if ( w && QString(w->name()) == "unnamed" ) {
        QString parents;
        parents = QString("(%1)").arg(w->className());
        QObject *p = w->parent();
        while (p) {
            parents = QString("(%1)%2").arg(p->className()).arg(parents);
            p = p->parent();
        }
        qDebug(QString(">>> No help for \"%1 %2\" <<<").arg(hf).arg(parents));
    } else {
        qDebug(">>> No help for \"%s\" <<<",hf.latin1());
    }
    return "";
}

/*!
  Returns whether help exists for the parent widget.
*/
bool ContextMenu::helpExists() const
{
    if ( d->helpFile.isNull() ) {
        QString hf = findHelp(0);
        bool he = !hf.isEmpty();
        d->helpFile = hf;
        d->helpexists = he;
        return he;
    }
    if (!d->helpexists) {
        qDebug(">>> NO help exists for \"%s\" <<<",d->helpFile.latin1());
    }
    return d->helpexists;
}

/*!
  Invokes end-user help for the parent widget.

  This slot is automatically connected to a 'Help' item
  if helpExists().
*/
void ContextMenu::help()
{
    QWidget* w = d->focusWidget;
    if ( !w )
        w = qApp->activeWindow();
    if ( !w )
        return;
    w = w->topLevelWidget();
    if ( helpExists() ) {
        QString hf = findHelp(w);
        if ( hf.isEmpty() ) {
            hf = d->helpFile;
        }
        qDebug(">>> Using help \"%s\" <<<",hf.latin1());
        Global::execute( "helpbrowser", hf );
    }
}

/*!
  Opens the popup menu in a standard position.
*/
void ContextMenu::popup()
{
    QRect r = qt_maxWindowRect;
    d->adjustHeight = TRUE;
    popup( r.bottomLeft() );
    d->adjustHeight = FALSE;
}

/*!
  Opens the popup menu at \a pos.
*/
void ContextMenu::popup(const QPoint &pos)
{
    if( !d->shown ) {
        d->hasHelp = helpExists();
        d->shown = TRUE;
    }
    
    if( d->sepId != -1 ) {
        removeItem( d->sepId );
        d->sepId = -1;
    }
    
    if( d->helpId != -1 ) {
        removeItem( d->helpId );
        d->helpId = -1;
    }
    
    if( d->cancelId != -1 ) {
        removeItem( d->cancelId );
        d->cancelId = -1;
    }
   
    if( count() ) d->sepId = insertSeparator();
    
    if( d->helpEnabled && d->hasHelp )
        d->helpId = insertItem( qtopia_internal_loadIconSet( "help_icon" ),
            tr( "Help" ), this, SLOT( help() ) );
            
    // qwizard check first?
    if ( d->focusWidget ) {
	QWidget* tlw = d->focusWidget->topLevelWidget();
	if ( tlw->inherits("QDialog")
		&& !qpe_systemButtons().contains(Key_No))
	{
	    if (!QPEApplication::isMenuLike((QDialog*)tlw))
		d->cancelId = insertItem(qtopia_internal_loadIconSet("close"), tr("Cancel"), tlw, SLOT(reject()));
	}
    }
    
    if( d->helpId == -1 && d->cancelId == -1 && d->sepId != -1 ) {
        removeItem( d->sepId );
        d->sepId = -1;
    }

    if (count()) {
        emit aboutToShow();
        blockSignals(TRUE);
	if (d->adjustHeight)
	    QPopupMenu::popup( QPoint(pos.x(), pos.y() - sizeHint().height() + 1) );
	else
	    QPopupMenu::popup(pos);
        blockSignals(FALSE);
	focusNextPrevChild(TRUE); // ie. setFirstItemActive();
    }
}

/*!
  \reimp
*/
bool ContextMenu::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || e->type() != QEvent::KeyPress )
	return FALSE;

    if (!o->isWidgetType())
	return FALSE;

    QKeyEvent *k = (QKeyEvent *) e;
    if (k->key() != key())
	return FALSE;

    d->focusWidget = (QWidget*)o;

    if( e->type() == QEvent::KeyPress ) {
	QMap<QWidget*, QWidget*>::ConstIterator it;
	it = d->focusWidgetMap.find(d->focusWidget);
	if (it != d->focusWidgetMap.end()) {
	    QWidget *w = *it;
	    if (((mousePreferred || w->isModalEditing()) && 
		    internalMenuFor(w, Modal) == this)
		|| ((mousePreferred || !w->isModalEditing()) && 
		    internalMenuFor(w, NonModal) == this)) {
		popup();
		return TRUE;
	    }
	}
    } 
    return FALSE;
}

/*!
  \reimp
*/
void ContextMenu::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == key())
	close();
    else
	QPopupMenu::keyPressEvent(e);
}

/*!
  \reimp
*/
void ContextMenu::keyReleaseEvent(QKeyEvent *e)
{
    QPopupMenu::keyReleaseEvent(e);
}

/*!
  \reimp
  */
void ContextMenu::mouseReleaseEvent(QMouseEvent *e)
{
    int item = idAt( e->pos() );
    if (item == -1) {
	if (!rect().contains(e->pos()))
	    return;
    }
    QPopupMenu::mouseReleaseEvent(e);
}

ContextMenu *ContextMenu::internalMenuFor(QWidget *w, int state)
{
    QMap<QWidget*, ContextMenu*>::ConstIterator it;
    if (state & Modal && modalMenuMap) {
	it = modalMenuMap->find(w);
	if (it != modalMenuMap->end())
	    return *it;
    }
    if (state & NonModal && modalMenuMap) {
	it = nonModalMenuMap->find(w);
	if (it != nonModalMenuMap->end())
	    return *it;
    }

    return 0;
}

void ContextMenu::widgetDestroyed()
{
    QWidget *w = (QWidget*)sender();
    removeFrom(w);
}

