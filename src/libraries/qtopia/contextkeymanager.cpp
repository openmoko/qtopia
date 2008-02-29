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

#include "contextkeymanager_p.h"
#include <qvalidator.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qaccel.h>

#ifdef Q_WS_QWS
# ifndef QT_NO_COP
#  include "qcopenvelope_qws.h"
# endif
#endif

#include "global.h"
#include "config.h"
#include "contextmenu.h"

#ifdef QTOPIA_PHONE
extern bool mousePreferred; // can't call Global::mousePreferred in libqtopia2 from libqtopia
#endif

//#define DEBUG_CONTEXT_KEYS

QString qtopia_internal_defaultButtonsFile();

static ContextKeyManager *contextKeyMgr = 0;

ContextKeyManager::ContextKeyManager()
{
    Config cfg(qtopia_internal_defaultButtonsFile(), Config::File);
    cfg.setGroup("SoftKeys");
    int buttonCount = cfg.readNumEntry("Count", 0);
    if (buttonCount) {
	buttons.resize(buttonCount);
	for (int i=0; i<buttonCount; i++) {
	    QString is = QString::number(i);
	    buttons[i] = QAccel::stringToKey(cfg.readEntry("Key"+is));
	}

	// Setup non-standard class context labels.
	ContextBar::StandardLabel lbl = ContextBar::NoLabel;
#ifdef QTOPIA_PHONE
	if( !mousePreferred )
	    lbl = ContextBar::Ok;
#endif

	setClassStandardLabel("QLineEdit", Qt::Key_Select, lbl, ContextBar::Modal);
	setClassStandardLabel("QMultiLineEdit", Qt::Key_Select, lbl, ContextBar::Modal);
	setClassStandardLabel("QSlider", Qt::Key_Select, lbl, ContextBar::Modal);
	setClassStandardLabel("QSpinBox", Qt::Key_Select, lbl, ContextBar::Modal);
	setClassStandardLabel("QButton", Qt::Key_Select, ContextBar::Select, ContextBar::ModalAndNonModal);

#ifdef QTOPIA_PHONE
	if( !mousePreferred ) {
	    setClassStandardLabel("QLineEdit", Qt::Key_Back, ContextBar::BackSpace, ContextBar::Modal);
	    setClassStandardLabel("QMultiLineEdit", Qt::Key_Back, ContextBar::BackSpace, ContextBar::Modal);
	    setClassStandardLabel("QSlider", Qt::Key_Back, ContextBar::Cancel, ContextBar::Modal);
	}
#endif

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateLabelsForFocused()));
    }
}

// Setup the appropriate context labels for widget w.
void ContextKeyManager::updateContextLabels()
{
    if (buttons.count())
	timer->start(0, TRUE);
}

void ContextKeyManager::updateLabelsForFocused()
{
    if (!qApp->activeWindow())
	return;

    QWidget *w = qApp->focusWidget();
    if (!w)
	w = qApp->activeWindow();

    bool modal = TRUE;
#ifdef QTOPIA_PHONE
    if( !mousePreferred )
	modal = w->isModalEditing();
#endif

#ifdef DEBUG_CONTEXT_KEYS
    qDebug("***Update labels for: %s(%p), %d", w->className(), w, modal);
#endif
    // Set top-level defaults
    bool fromDialog = FALSE;
    if ( w->topLevelWidget() ) {
	fromDialog = w->topLevelWidget()->inherits("QDialog");
    }

    int menuKey = ContextMenu::key();
    bool editMenu = FALSE;
    bool overrideBack = FALSE;
#ifdef QTOPIA_PHONE
    if( !mousePreferred ) {
	if (modal) {
	    if (w->inherits("QLineEdit")) {
		QLineEdit* l = (QLineEdit*)w;
		editMenu = !haveLabelForWidget(w, menuKey, modal);
		if (l->text().length() == 0 || l->isReadOnly())
		    overrideBack = TRUE;
	    } else if (w->inherits("QMultiLineEdit") ) {
		QMultiLineEdit* l = (QMultiLineEdit*)w;
		editMenu = !haveLabelForWidget(w, menuKey, modal);
		if (l->length() == 0 || l->isReadOnly())
		    overrideBack = TRUE;
	    } else if (w->testWFlags(WType_Popup)) {
		if (!haveLabelForWidget(w, Key_Back, modal))
		    overrideBack = TRUE;
	    }
	}
    }
#endif

    for (int i = 0; i < (int)buttons.count(); i++) {
	if (buttons[i] == menuKey) {
	    if (editMenu) {
		setStandard(menuKey, ContextBar::Options);
	    } else if (!updateContextLabel(w, modal, menuKey)) {
#ifdef DEBUG_CONTEXT_KEYS
		qDebug("Default: Menu");
#endif
		clearLabel(menuKey);
	    }
	} else {
	    switch (buttons[i]) {
		case Qt::Key_Select:
		    if (!updateContextLabel(w, modal, Qt::Key_Select)) {
#ifdef DEBUG_CONTEXT_KEYS
			qDebug("default: Select");
#endif
			if (w->isFocusEnabled())
			    setStandard(Qt::Key_Select, ContextBar::Select);
			else
			    clearLabel(Qt::Key_Select);
		    }
		    break;
		case Qt::Key_Back:
#ifdef QTOPIA_PHONE
		    if (!mousePreferred && overrideBack)
			setStandard(Qt::Key_Back, ContextBar::Cancel);
		    else 
#endif
		    if (!updateContextLabel(w, modal, Qt::Key_Back))
			setStandard(Qt::Key_Back, ContextBar::Back);
		    break;
		default:
		    if (!updateContextLabel(w, modal, buttons[i]))
			clearLabel(buttons[i]);
	    }
	}
    }
}

bool ContextKeyManager::updateContextLabel(QWidget *w, bool modal, int key)
{
    // First see if label is set for this widget explicitly
    QMap<QWidget*,KeyMap>::Iterator wit = contextWidget.find(w);
    if (wit != contextWidget.end()) {
	KeyMap::Iterator it = (*wit).find(key);
	if (it != (*wit).end()) {
	    switch ((*it).type(modal)) {
		case ModalState::Custom: {
#ifdef DEBUG_CONTEXT_KEYS
			qDebug( "key: %d, text: %s, pixmap: %s, widget: %s", key, (*it).text(modal).latin1(), (*it).pixmap(modal).latin1(), w->className());
#endif
			QString text((*it).text(modal));
			QString pixmap((*it).pixmap(modal));
			if (pixmap.isEmpty())
			    setText(key, text);
			else
			    setPixmap(key, pixmap);
		    }
		    return TRUE;
		case ModalState::Standard:
#ifdef DEBUG_CONTEXT_KEYS
		    qDebug( "key: %d, standard: %d, widget: %s", key, (*it).label(modal), w->className());
#endif
		    setStandard(key, (*it).label(modal));
		    return TRUE;
		default:
		    break;
	    }
	}
    }

    // Next see if label is set for the class (must accept focus)
    if (w->isFocusEnabled()) {
	QValueList<ClassModalState>::Iterator cit;
	for (cit = contextClass.begin(); cit != contextClass.end(); ++cit) {
	    if (w->inherits((*cit).className)) {
#ifdef DEBUG_CONTEXT_KEYS
		qDebug( "Inherits: %s", (*cit).className.data());
#endif
		KeyMap::Iterator it = (*cit).keyMap.find(key);
		if (it != (*cit).keyMap.end()) {
#ifdef DEBUG_CONTEXT_KEYS
		    qDebug( "found class key");
#endif
		    switch ((*it).type(modal)) {
			case ModalState::Custom: {
#ifdef DEBUG_CONTEXT_KEYS
				qDebug( "key: %d, text: %s, pixmap: %s, widget: %s", key, (*it).text(modal).latin1(), (*it).pixmap(modal).latin1(), w->className());
#endif
				QString text((*it).text(modal));
				QString pixmap((*it).pixmap(modal));
				if (pixmap.isEmpty())
				    setText(key, text);
				else
				    setPixmap(key, pixmap);
			    }
			    return TRUE;
			case ModalState::Standard:
#ifdef DEBUG_CONTEXT_KEYS
			    qDebug( "key: %d, standard: %d, widget: %s", key, (*it).label(modal), w->className());
#endif
			    setStandard(key, (*it).label(modal));
			    return TRUE;
			default:
			    break;
		    }
		}
	    }
	}
    }

    // Otherwise try our parent
    if (w->parentWidget() && !w->isTopLevel())
	return updateContextLabel(w->parentWidget(), FALSE, key);

    return FALSE;
}

QWidget *ContextKeyManager::findTargetWidget(QWidget *w, int key, bool modal)
{
    // First see if key is set for this widget explicitly
    QMap<QWidget*,KeyMap>::Iterator wit = contextWidget.find(w);
    if (wit != contextWidget.end()) {
	KeyMap::Iterator it = (*wit).find(key);
	if (it != (*wit).end()) {
	    if ((*it).type(modal) != ModalState::NoLabel)
		return w;
	}
    }

    // Next see if key is set for the class (must accept focus)
    if (w->focusPolicy() != QWidget::NoFocus) {
	QValueList<ClassModalState>::Iterator cit;
	for (cit = contextClass.begin(); cit != contextClass.end(); ++cit) {
	    if (w->inherits((*cit).className)) {
		KeyMap::Iterator it = (*cit).keyMap.find(key);
		if (it != (*cit).keyMap.end()) {
		    if ((*it).type(modal) != ModalState::NoLabel)
			return w;
		}
	    }
	}
    }

    // otherwise try our parent
    if (w->parentWidget() && !w->isTopLevel())
	return findTargetWidget(w->parentWidget(), key, FALSE);

    // No one has claimed this key.
    return 0;
}

bool ContextKeyManager::haveLabelForWidget(QWidget *w, int key, bool modal)
{
    if (buttons.count()) {
	QMap<QWidget*,KeyMap>::Iterator wit = contextWidget.find(w);
	if (wit != contextWidget.end()) {
	    KeyMap::Iterator it = (*wit).find(key);
	    if (it != (*wit).end()) {
		if ((*it).type(modal) != ModalState::NoLabel)
		    return TRUE;
	    }
	}
    }

    return FALSE;
}

void ContextKeyManager::setContextText(QWidget *w, int key, const QString &t, ContextBar::EditingState state)
{
    if (!contextWidget.contains(w))
	connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & ContextBar::Modal) {
	ms.mType = ModalState::Custom;
	ms.mText = t;
    }
    if (state & ContextBar::NonModal) {
	ms.nmType = ModalState::Custom;
	ms.nmText = t;
    }
    updateContextLabels();
}

void ContextKeyManager::setContextPixmap(QWidget *w, int key, const QString &t, ContextBar::EditingState state)
{
    if (!contextWidget.contains(w))
	connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & ContextBar::Modal) {
	ms.mType = ModalState::Custom;
	ms.mPixmap = t;
    }
    if (state & ContextBar::NonModal) {
	ms.nmType = ModalState::Custom;
	ms.nmPixmap = t;
    }
    updateContextLabels();
}

void ContextKeyManager::setContextStandardLabel(QWidget *w, int key, ContextBar::StandardLabel label, ContextBar::EditingState state)
{
    if (!contextWidget.contains(w))
	connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & ContextBar::Modal) {
	ms.mType = ModalState::Standard;
	ms.mStandard = label;
    }
    if (state & ContextBar::NonModal) {
	ms.nmType = ModalState::Standard;
	ms.nmStandard = label;
    }
    updateContextLabels();
}


void ContextKeyManager::clearContextLabel(QWidget *w, int key, ContextBar::EditingState state)
{
    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & ContextBar::Modal) {
	ms.mType = ModalState::NoLabel;
    }
    if (state & ContextBar::NonModal) {
	ms.nmType = ModalState::NoLabel;
    }
    updateContextLabels();
}

void ContextKeyManager::setClassText(const QCString &className, int key, const QString &t, ContextBar::EditingState state)
{
    ClassModalState cms(className);

    QValueList<ClassModalState>::Iterator it = contextClass.find(cms);
    if (it == contextClass.end())
	it = contextClass.prepend(cms);

    ModalState &ms = (*it).keyMap[key];
    if (state & ContextBar::Modal) {
	ms.mType = ModalState::Custom;
	ms.mText = t;
    }
    if (state & ContextBar::NonModal) {
	ms.nmType = ModalState::Custom;
	ms.nmText = t;
    }
}

void ContextKeyManager::setClassPixmap(const QCString &className, int key, const QString &t, ContextBar::EditingState state)
{
    ClassModalState cms(className);

    QValueList<ClassModalState>::Iterator it = contextClass.find(cms);
    if (it == contextClass.end())
	it = contextClass.prepend(cms);

    ModalState &ms = (*it).keyMap[key];
    if (state & ContextBar::Modal) {
	ms.mType = ModalState::Custom;
	ms.mPixmap = t;
    }
    if (state & ContextBar::NonModal) {
	ms.nmType = ModalState::Custom;
	ms.nmPixmap = t;
    }
}

void ContextKeyManager::setClassStandardLabel(const QCString &className, int key, ContextBar::StandardLabel label, ContextBar::EditingState state)
{
    ClassModalState cms(className);

    QValueList<ClassModalState>::Iterator it = contextClass.find(cms);
    if (it == contextClass.end())
	it = contextClass.prepend(cms);

    ModalState &ms = (*it).keyMap[key];
    if (state & ContextBar::Modal) {
	ms.mType = ModalState::Standard;
	ms.mStandard = label;
    }
    if (state & ContextBar::NonModal) {
	ms.nmType = ModalState::Standard;
	ms.nmStandard = label;
    }
}

void ContextKeyManager::removeSenderFromWidgetContext()
{
    if ( contextWidget.contains((QWidget*)sender() )) {
	QWidget *w = (QWidget*)sender();
#ifdef DEBUG_CONTEXT_KEYS
	qDebug("Removing context widget: %s(%p)", w->className(), w);
#endif
	contextWidget.remove(w);
	updateContextLabels();
    }
}

ContextKeyManager *ContextKeyManager::instance()
{
    if (!contextKeyMgr)
	contextKeyMgr = new ContextKeyManager();

    return contextKeyMgr;
}

void ContextKeyManager::setStandard(int key, ContextBar::StandardLabel label)
{
    QString l = standardPixmap(label);
    if (!!l)
	setPixmap(key, l);
    else
	setText(key, standardText(label));
}

void ContextKeyManager::setText(int key, const QString &text)
{
    QCopEnvelope e( "QPE/ContextBar", "setLabelText(int,QString)");
    e << key;
    e << text;
}

void ContextKeyManager::setPixmap(int key, const QString &pm)
{
    QCopEnvelope e( "QPE/ContextBar", "setLabelPixmap(int,QString)");
    e << key;
    e << pm;
}

void ContextKeyManager::clearLabel(int key)
{
    QCopEnvelope e( "QPE/ContextBar", "clearLabel(int)");
    e << key;
}

QString ContextKeyManager::standardText(ContextBar::StandardLabel l)
{
    switch (l) {
	case ContextBar::NoLabel:
	    return "";
	case ContextBar::Options:
	    return tr("Options");
	case ContextBar::Ok:
	    return tr("OK");
	case ContextBar::Edit:
	    return tr("Edit");
	case ContextBar::Select:
	    return tr("Select");
	case ContextBar::View:
	    return tr("View");
	case ContextBar::Cancel:
	    return tr("Cancel");
	case ContextBar::Back:
	    return tr("Back");
	case ContextBar::BackSpace:
	    return tr("Delete");
	case ContextBar::Next:
	    return tr("Next");
	case ContextBar::Previous:
	    return tr("Prev");
	default:
	    return QString::null;
    }
}

QString ContextKeyManager::standardPixmap(ContextBar::StandardLabel l)
{
    switch (l) {
	case ContextBar::NoLabel:
	    return "";
	case ContextBar::Options:
	    return "contextbar/options";
	case ContextBar::Back:
	    return "contextbar/back";
	case ContextBar::Cancel:
	    return "contextbar/cancel";
	case ContextBar::Ok:
	    return "contextbar/ok";
	case ContextBar::Edit:
	    return "contextbar/edit";
	case ContextBar::Select:
	    return "contextbar/select";
	case ContextBar::View:
	    return "contextbar/view";
	case ContextBar::BackSpace:
	    return "contextbar/backspace";
	case ContextBar::Next:
	    return "contextbar/next";
	case ContextBar::Previous:
	    return "contextbar/previous";
	default:
	    return QString::null;
    }
}

