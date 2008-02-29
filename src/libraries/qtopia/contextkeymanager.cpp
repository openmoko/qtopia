/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "contextkeymanager_p.h"
#include <qvalidator.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qspinbox.h>
#include <qsettings.h>
#include <qcombobox.h>
#include <QKeySequence>
#include <QtGui/qtextcursor.h>
#include <QTextBlock>
#include <QTextLayout>

#include "qtopiaipcenvelope.h"
#include "qsoftmenubar.h"
#include "qtopialog.h"

#ifdef QTOPIA_PHONE
extern bool mousePreferred; // can't call Qtopia::mousePreferred in libqtopia2 from libqtopia
#endif

class ContextKeyManagerQSpinBoxLineEditAccessor : public QSpinBox
{
public:
    QLineEdit *getLineEdit() { return lineEdit(); }
};

static ContextKeyManager *contextKeyMgr = 0;

ContextKeyManager::ContextKeyManager()
{
    QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
    cfg.beginGroup("SoftKeys");
    int buttonCount = cfg.value("Count", 0).toInt();
    if (buttonCount) {
        for (int i=0; i<buttonCount; i++) {
            QString is = QString::number(i);
            int btn = QKeySequence(cfg.value("Key"+is).toString())[0];
            qLog(UI) << "Read key:" << cfg.value("Key"+is).toString() << "id:" << btn;
            buttons.append(btn);
        }

        // Setup non-standard class context labels.
        QSoftMenuBar::StandardLabel lbl = QSoftMenuBar::NoLabel;
#ifdef QTOPIA_PHONE
        if( !mousePreferred )
            lbl = QSoftMenuBar::Ok;
#endif

        setClassStandardLabel("QLineEdit", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QTextEdit", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QSlider", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QSpinBox", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QComboBox", Qt::Key_Select, lbl, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QButton", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);
        setClassStandardLabel("QMenu", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);
        setClassStandardLabel("QTextBrowser", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::EditFocus);
        setClassStandardLabel("QComboBoxPrivateContainer", Qt::Key_Select, QSoftMenuBar::Select, QSoftMenuBar::AnyFocus);

        setClassStandardLabel("QMenu", Qt::Key_Back, QSoftMenuBar::Cancel, QSoftMenuBar::AnyFocus);

#ifdef QTOPIA_PHONE
        if( !mousePreferred ) {
            setClassStandardLabel("QLineEdit", Qt::Key_Back, QSoftMenuBar::BackSpace, QSoftMenuBar::EditFocus);
            setClassStandardLabel("QTextEdit", Qt::Key_Back, QSoftMenuBar::BackSpace, QSoftMenuBar::EditFocus);
            setClassStandardLabel("QSlider", Qt::Key_Back, QSoftMenuBar::Cancel, QSoftMenuBar::EditFocus);
            setClassStandardLabel("QComboBoxPrivateContainer", Qt::Key_Back, QSoftMenuBar::Cancel, QSoftMenuBar::AnyFocus);
            setClassStandardLabel("QTextBrowser", Qt::Key_Back, QSoftMenuBar::Back, QSoftMenuBar::EditFocus);
        }
#endif

        timer = new QTimer(this);
        timer->setSingleShot(true);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateLabelsForFocused()));
    }
#ifdef QTOPIA_PHONE
    QSettings config("Trolltech","qpe");
    config.beginGroup( "ContextMenu" );
    labelType = (QSoftMenuBar::LabelType)config.value( "LabelType", QSoftMenuBar::IconLabel).toInt();
#endif
}

// Setup the appropriate context labels for widget w.
void ContextKeyManager::updateContextLabels()
{
    if (buttons.count())
        timer->start(0);
}

void ContextKeyManager::updateLabelsForFocused()
{
    if (!qApp->activeWindow())
        return;

    QWidget *w = QApplication::activePopupWidget();
    if (!w)
        w = qApp->focusWidget();
    if (!w)
        w = qApp->activeWindow();

    bool modal = true;
#ifdef QTOPIA_PHONE
    if( !mousePreferred )
        modal = w->hasEditFocus();
#endif

    // Set top-level defaults
    bool fromDialog = false;
    if ( w->topLevelWidget() ) {
        fromDialog = w->topLevelWidget()->inherits("QDialog");
    }

    int menuKey = QSoftMenuBar::menuKey();
    bool editMenu = false;
    QSoftMenuBar::StandardLabel backLabel = QSoftMenuBar::Back;
    bool overrideBack = false;
#ifdef QTOPIA_PHONE
    if( !mousePreferred ) {
        if (modal) {
            QLineEdit *l = qobject_cast<QLineEdit*>(w);
            if (!l && w->inherits("QSpinBox"))
                l = ((ContextKeyManagerQSpinBoxLineEditAccessor*)w)->getLineEdit();
            if (!l && w->inherits("QComboBox"))
                l = ((QComboBox*)w)->lineEdit();
            if (l) {
                editMenu = !haveLabelForWidget(w, menuKey, modal);
                if (l->text().length() == 0 || l->isReadOnly())
                    backLabel = QSoftMenuBar::Cancel;
                else if (l->cursorPosition() == 0)
                    backLabel = QSoftMenuBar::NoLabel;
                else
                    backLabel = QSoftMenuBar::BackSpace;
                overrideBack = true;
            } else if (w->inherits("QTextEdit") && !w->inherits("QTextBrowser")) {
                QTextEdit *e = qobject_cast<QTextEdit*>(w);
                editMenu = !haveLabelForWidget(w, menuKey, modal);
                if ((e->document()->isEmpty() && e->textCursor().block().layout()->preeditAreaText().isEmpty()) || e->isReadOnly()) {
                    backLabel = QSoftMenuBar::Cancel;
                    overrideBack = true;
                }
            } else if ( (w->windowFlags() & Qt::Popup) ) {
                if (!haveLabelForWidget(w, Qt::Key_Back, modal)) {
                    backLabel = QSoftMenuBar::Cancel;
                    overrideBack = true;
                }
            }
        }
    }
#endif

    for (int i = 0; i < (int)buttons.count(); i++) {
        if (buttons[i] == menuKey) {
            if (editMenu) {
                setStandard(w, menuKey, QSoftMenuBar::Options);
            } else if (!updateContextLabel(w, modal, menuKey)) {
                clearLabel(w, menuKey);
            }
        } else {
            switch (buttons[i]) {
                case Qt::Key_Select:
                    if (!updateContextLabel(w, modal, Qt::Key_Select)) {
                        if (w->focusPolicy() != Qt::NoFocus)
                            setStandard(w, Qt::Key_Select, QSoftMenuBar::Select);
                        else
                            clearLabel(w, Qt::Key_Select);
                    }
                    break;
                case Qt::Key_Back:
#ifdef QTOPIA_PHONE
                    if (!mousePreferred && overrideBack)
                        setStandard(w, Qt::Key_Back, backLabel);
                    else
#endif
                    if (!updateContextLabel(w, modal, Qt::Key_Back))
                        setStandard(w, Qt::Key_Back, QSoftMenuBar::Back);
                    break;
                default:
                    if (!updateContextLabel(w, modal, buttons[i]))
                        clearLabel(w, buttons[i]);
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
                        QString text((*it).text(modal));
                        QString pixmap((*it).pixmap(modal));
                        if (pixmap.isEmpty() || labelType == QSoftMenuBar::TextLabel)
                            setText(w, key, text);
                        else
                            setPixmap(w, key, pixmap);
                    }
                    return true;
                case ModalState::Standard:
                    setStandard(w, key, (*it).label(modal));
                    return true;
                default:
                    break;
            }
        }
    }

    // Next see if label is set for the class (must accept focus)
    if (w->focusPolicy() != Qt::NoFocus
        || (w->windowFlags() & Qt::Popup)) {
        QList<ClassModalState>::Iterator cit;
        for (cit = contextClass.begin(); cit != contextClass.end(); ++cit) {
            if (w->inherits((*cit).className.toAscii().constData())) {
                KeyMap::Iterator it = (*cit).keyMap.find(key);
                if (it != (*cit).keyMap.end()) {
                    switch ((*it).type(modal)) {
                        case ModalState::Custom: {
                                QString text((*it).text(modal));
                                QString pixmap((*it).pixmap(modal));
                                if (pixmap.isEmpty() || labelType == QSoftMenuBar::TextLabel)
                                    setText(w, key, text);
                                else
                                    setPixmap(w, key, pixmap);
                            }
                            return true;
                        case ModalState::Standard:
                            setStandard(w, key, (*it).label(modal));
                            return true;
                        default:
                            break;
                    }
                }
            }
        }
    }

    // Otherwise try our parent
    if (w->parentWidget() && !w->isTopLevel()) {
        return updateContextLabel(w->parentWidget(), false, key);
    }

    return false;
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
    if (w->focusPolicy() != Qt::NoFocus) {
        QList<ClassModalState>::Iterator cit;
        for (cit = contextClass.begin(); cit != contextClass.end(); ++cit) {
            if (w->inherits((*cit).className.toAscii().constData())) {
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
        return findTargetWidget(w->parentWidget(), key, false);

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
                    return true;
            }
        }
    }

    return false;
}

void ContextKeyManager::setContextText(QWidget *w, int key, const QString &t, QSoftMenuBar::FocusState state)
{
    if (!contextWidget.contains(w))
        connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Custom;
        ms.mText = t;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Custom;
        ms.nmText = t;
    }
    updateContextLabels();
}

void ContextKeyManager::setContextPixmap(QWidget *w, int key, const QString &t, QSoftMenuBar::FocusState state)
{
    if (!contextWidget.contains(w))
        connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Custom;
        ms.mPixmap = t;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Custom;
        ms.nmPixmap = t;
    }
    updateContextLabels();
}

void ContextKeyManager::setContextStandardLabel(QWidget *w, int key, QSoftMenuBar::StandardLabel label, QSoftMenuBar::FocusState state)
{
    if (!contextWidget.contains(w))
        connect(w, SIGNAL(destroyed()), this, SLOT(removeSenderFromWidgetContext()));

    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Standard;
        ms.mStandard = label;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Standard;
        ms.nmStandard = label;
    }
    updateContextLabels();
}


void ContextKeyManager::clearContextLabel(QWidget *w, int key, QSoftMenuBar::FocusState state)
{
    KeyMap &keyMap = contextWidget[w];
    ModalState &ms = keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::NoLabel;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::NoLabel;
    }
    updateContextLabels();
}

void ContextKeyManager::setClassText(const QString &className, int key, const QString &t, QSoftMenuBar::FocusState state)
{
    ClassModalState cms(className);

    int idx = contextClass.indexOf(cms);
    if (idx == -1) {
        contextClass.prepend(cms);
        idx = 0;
    }

    ModalState &ms = contextClass[idx].keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Custom;
        ms.mText = t;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Custom;
        ms.nmText = t;
    }
}

void ContextKeyManager::setClassPixmap(const QString &className, int key, const QString &t, QSoftMenuBar::FocusState state)
{
    ClassModalState cms(className);

    int idx = contextClass.indexOf(cms);
    if (idx == -1) {
        contextClass.prepend(cms);
        idx = 0;
    }

    ModalState &ms = contextClass[idx].keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Custom;
        ms.mPixmap = t;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Custom;
        ms.nmPixmap = t;
    }
}

void ContextKeyManager::setClassStandardLabel(const QString &className, int key, QSoftMenuBar::StandardLabel label, QSoftMenuBar::FocusState state)
{
    ClassModalState cms(className);

    int idx = contextClass.indexOf(cms);
    if (idx == -1) {
        contextClass.prepend(cms);
        idx = 0;
    }

    ModalState &ms = contextClass[idx].keyMap[key];
    if (state & QSoftMenuBar::EditFocus) {
        ms.mType = ModalState::Standard;
        ms.mStandard = label;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        ms.nmType = ModalState::Standard;
        ms.nmStandard = label;
    }
}

void ContextKeyManager::removeSenderFromWidgetContext()
{
    if ( contextWidget.contains((QWidget*)sender() )) {
        QWidget *w = (QWidget*)sender();
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

void ContextKeyManager::setStandard(QWidget *w, int key, QSoftMenuBar::StandardLabel label)
{
    QString l = standardPixmap(label);
    if (labelType == QSoftMenuBar::IconLabel && !l.isEmpty())
        setPixmap(w, key, l);
    else
        setText(w, key, standardText(label));
}

void ContextKeyManager::setText(QWidget *w, int key, const QString &text)
{
    w = w->topLevelWidget();
    if (w->windowFlags() & Qt::Popup)
        w = QApplication::activeWindow();
    int win = w->winId();
    QtopiaIpcEnvelope e( "QPE/QSoftMenuBar", "setLabelText(int,int,QString)");
    e << win;
    e << key;
    e << text;
}

void ContextKeyManager::setPixmap(QWidget *w, int key, const QString &pm)
{
    w = w->topLevelWidget();
    if (w->windowFlags() & Qt::Popup)
        w = QApplication::activeWindow();
    int win = w->winId();
    QtopiaIpcEnvelope e( "QPE/QSoftMenuBar", "setLabelPixmap(int,int,QString)");
    e << win;
    e << key;
    e << pm;
}

void ContextKeyManager::clearLabel(QWidget *w, int key)
{
    w = w->topLevelWidget();
    if (w->windowFlags() & Qt::Popup)
        w = QApplication::activeWindow();
    int win = w->winId();
    QtopiaIpcEnvelope e( "QPE/QSoftMenuBar", "clearLabel(int,int)");
    e << win;
    e << key;
}

void ContextKeyManager::setLabelType(QSoftMenuBar::LabelType type)
{
    labelType = type;
}

QString ContextKeyManager::standardText(QSoftMenuBar::StandardLabel l)
{
    switch (l) {
        case QSoftMenuBar::NoLabel:
            return "";
        case QSoftMenuBar::Options:
            return tr("Options");
        case QSoftMenuBar::Ok:
            return tr("OK");
        case QSoftMenuBar::Edit:
            return tr("Edit");
        case QSoftMenuBar::Select:
            return tr("Select");
        case QSoftMenuBar::View:
            return tr("View");
        case QSoftMenuBar::Cancel:
            return tr("Cancel");
        case QSoftMenuBar::Back:
            return tr("Back");
        case QSoftMenuBar::BackSpace:
            return tr("Delete");
        case QSoftMenuBar::Next:
            return tr("Next");
        case QSoftMenuBar::Previous:
            return tr("Prev");
        default:
            return QString();
    }
}

QString ContextKeyManager::standardPixmap(QSoftMenuBar::StandardLabel l)
{
    switch (l) {
        case QSoftMenuBar::NoLabel:
            return "";
        case QSoftMenuBar::Options:
            return "options";
        case QSoftMenuBar::Back:
            return "i18n/back";
        case QSoftMenuBar::Cancel:
            return "cancel";
        case QSoftMenuBar::Ok:
            return "ok";
        case QSoftMenuBar::Edit:
            return "edit";
        case QSoftMenuBar::Select:
            return "select";
        case QSoftMenuBar::View:
            return "view";
        case QSoftMenuBar::BackSpace:
            return "i18n/backspace";
        case QSoftMenuBar::Next:
            return "i18n/next";
        case QSoftMenuBar::Previous:
            return "i18n/previous";
        default:
            return QString();
    }
}

