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

#include "qsoftmenubar.h"
#include "contextkeymanager_p.h"
#include <QMenu>
#include <QKeyEvent>
#include <QDebug>
#include <QFileInfo>
#include <QDesktopWidget>
#include <QTextEdit>
#include <QLineEdit>

#include <QtopiaInputMethod>

#include <qtopialog.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <QValueSpaceItem>

//#define DEBUG_QSOFTMENUBAR_IM_ACTIONS

/*!
  \class QSoftMenuBar
  \brief The QSoftMenuBar class allows the label in the phone
  soft menu bar to be set.

  QSoftMenuBar is only available in the Qtopia phone edition.  It is
  used to set the icon/text in the soft menu bar to describe the action
  performed when pressing the associated soft button.  Keep in
  mind that not all phones have soft keys and therefore may not have
  a visible soft menu bar.  The application should be useable without
  soft key shortcuts.

  The labels are updated whenever a widget gains or loses focus
  or their navigation focus state changes.  Therefore, the labels should
  be set for each widget that may gain focus, and will respond to a
  soft key.  The standard Qt and Qtopia widgets
  set the labels appropriately, so this is usually only necessary for
  custom widgets.

  The rule for determining what label is displayed on the soft menu bar and
  where the key events are delivered is:

\list
  \o If the current focus widget has claimed the key, then the corresponding
  context label is shown.
  \o Otherwise, the widget's parent, grandparent and so on may set the label.
  \o If the focus widget has not claimed the key and an ancestor has, then
  the key event will be sent directly to the ancestor.
\endlist

  QSoftMenuBar labels specify both a pixmap and a text label.  Currently,
  Qtopia Phone Edition will always use the pixmap rather than text.  The
  text label must still be provided because future versions of Qtopia
  Phone Edition may allow the user to choose to view text labels.

  QSoftMenuBar is only available in the Qtopia Phone Edition.

  \ingroup qtopiaemb
*/

/*!
  \enum QSoftMenuBar::FocusState

  \value EditFocus apply the label setting when the widget has edit focus.
  \value NavigationFocus apply the label setting when the widget does not have edit focus.
  \value AnyFocus apply the label setting regardless of focus state.
*/

/*!
  \enum QSoftMenuBar::StandardLabel

  \value NoLabel
  \value Options
  \value Ok
  \value Edit
  \value Select
  \value View
  \value Cancel
  \value Back
  \value BackSpace
  \value Next
  \value Previous
*/

/*!
  \enum QSoftMenuBar::LabelType
  \value IconLabel
  \value TextLabel
*/


QSoftMenuBar::QSoftMenuBar()
{
}


/*!
  When widget \a w gains focus, sets the label in the soft menu bar for key
  \a key to text \a t and pixmap \a pm when in state \a state.

  The pixmaps used in the soft menu bar may be no larger than 22x16 pixels.
*/
void QSoftMenuBar::setLabel(QWidget *w, int key, const QString &pm, const QString &t, FocusState state)
{
    ContextKeyManager::instance()->setContextText(w, key, t, state);
    ContextKeyManager::instance()->setContextPixmap(w, key, pm, state);
}

/*!
  When widget \a w gains focus, sets the label in the soft menu bar for key
  \a key to the standard label \a label when in state \a state.
*/
void QSoftMenuBar::setLabel(QWidget *w, int key, StandardLabel label, FocusState state)
{
    ContextKeyManager::instance()->setContextStandardLabel(w, key, label, state);
}

/*!
  Clears any label set for widget \a w, key \a key in state \a state.

  \sa setLabel()
*/
void QSoftMenuBar::clearLabel(QWidget *w, int key, FocusState state)
{
    ContextKeyManager::instance()->clearContextLabel(w, key, state);
}

/*!
  Returns the list of soft keys (i.e. keys with a corresponding label
  in the soft menu bar).
*/
const QList<int> &QSoftMenuBar::keys()
{
    return ContextKeyManager::instance()->keys();
}

class EditMenu : public QMenu
{
    Q_OBJECT
public:
    EditMenu(QWidget *parent)
        : QMenu(parent)
    {
        copyAction = addAction(QIcon(":icon/selecttext"), tr("Select..."));
        cutAction = addAction(QIcon(":icon/cut"), tr("Cut"));
        cutAction->setVisible(false);
        pasteAction = addAction(QIcon(":icon/paste"), tr("Paste"));
        if ( isReadOnly(parent) )
            pasteAction->setVisible(false);
        connect(this, SIGNAL(triggered(QAction*)),
                this, SLOT(editMenuActivated(QAction*)));
    }

    static bool keyEventFilter( QObject *o, QEvent *e )
    {
        if (highlighting) {
            if ( e->type() == QEvent::KeyPress /*|| e->type() == QEvent::KeyRelease*/ ) {
                QKeyEvent *ke = (QKeyEvent *)e;
                if ( ke->key() >= Qt::Key_Left && ke->key() <= Qt::Key_Down ) {
                    if ( !(ke->modifiers()&Qt::ShiftModifier) ) {
                        if ( highlighting ) {
                            qApp->postEvent(o,new QKeyEvent(ke->type(),ke->key(),ke->modifiers()|Qt::ShiftModifier,ke->text(),ke->isAutoRepeat(),ke->count()));
                            return true;
                        }
                    }
                } else {
                    setHighlighting(0);
                }
            }
        }
        return false;
    }

    static bool isHighlighting()
    {
        return highlighting;
    }

    static void setHighlighting(EditMenu* m)
    {
        if ( highlighting != m ) {
            if ( highlighting ) {
                highlighting->copyAction->setText(tr("Select..."));
                highlighting->copyAction->setIcon(QIcon(":icon/selecttext"));
                highlighting->cutAction->setVisible(false);
            }
            highlighting = m;
        }
    }

private slots:
    void editMenuActivated(QAction *action)
    {
        QString text;
        int scan=0;
        QWidget* w = qApp->focusWidget();
        bool reset_sel = true;

        if (w && action == copyAction) {
            bool has_sel = false;
            QTextEdit* te = qobject_cast<QTextEdit*>(w);
            QLineEdit* le = qobject_cast<QLineEdit*>(w);
            if ( te ) {
                has_sel = te->textCursor().hasSelection();
            } else if ( le ) {
                has_sel = le->hasSelectedText();
            }
            if ( has_sel ) {
                text="c"; scan=Qt::Key_C; // Copy
            } else {
                reset_sel = false;
                setHighlighting(this);
                copyAction->setText(tr("Copy"));
                copyAction->setIcon(QIcon(":icon/copy"));
                if ( !isReadOnly(w) )
                    cutAction->setVisible(true);
            }
        } else if (action == cutAction) {
            text="x"; scan=Qt::Key_X;
        } else if (action == pasteAction) {
            text="v"; scan=Qt::Key_V;
        }
        if ( reset_sel ) {
            setHighlighting(0);
        }

        if (scan && qApp->focusWidget()) {
            QKeyEvent *ke = new QKeyEvent(QEvent::KeyPress, scan, Qt::KeyboardModifiers(Qt::ControlModifier), text);
            QApplication::postEvent(qApp->focusWidget(), ke);
            ke = new QKeyEvent(QEvent::KeyRelease, scan, Qt::KeyboardModifiers(Qt::ControlModifier), text);
            QApplication::postEvent(qApp->focusWidget(), ke);
        }
    }

private:
    bool isReadOnly(QWidget* w) const
    {
        bool ro = false;
        QTextEdit* te = qobject_cast<QTextEdit*>(w);
        QLineEdit* le = qobject_cast<QLineEdit*>(w);
        if ( te ) {
            ro = te->isReadOnly();
        } else if ( le ) {
            ro = le->isReadOnly();
        }
        return ro;
    }

    static QPointer<EditMenu> highlighting;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
};
QPointer<EditMenu> EditMenu::highlighting;

class MenuManager : public QObject
{
    Q_OBJECT
public:
    MenuManager();

    QMenu *menuFor(QWidget *w, QSoftMenuBar::FocusState state);
    bool hasMenu(QWidget *w, QSoftMenuBar::FocusState state);
    void addMenuTo(QWidget *w, QMenu *menu, QSoftMenuBar::FocusState state);
    void removeMenuFrom(QWidget *w, QMenu *menu, QSoftMenuBar::FocusState state);
    void setHelpEnabled(QWidget *widget, bool enable);
    void setCancelEnabled(QWidget *widget, bool enable);
    void setInputMethodEnabled(QWidget *widget, bool enable);
    int key();

    static MenuManager *instance() {
        if (!mmgr)
            mmgr = new MenuManager;
        return mmgr;
    }

#ifdef QTOPIA_TEST
    QMenu *getActiveMenu() const;
#endif

protected:
    bool eventFilter(QObject *, QEvent *e);

private:
    void popup(QWidget *w, QMenu *menu);
    QMenu *internalMenuFor(QWidget *w, QSoftMenuBar::FocusState state);
    bool helpExists(QWidget *w);
    QList<QWidget*> widgetsWithMenu(QMenu *menu, QSoftMenuBar::FocusState state);
    QString findHelp(const QWidget* w);
    bool triggerMenuItem( QMenu *menu, int keyNum );

private slots:
    void widgetDestroyed();
    void menuDestroyed();
    void help();
    void inputMethod();

private:
    struct WidgetData {
        QString helpFile;
        int shown : 1;
        int helpexists : 1;
        int helpEnabled : 1;
        int cancelEnabled : 1;
        int hasHelp : 1;
        int inputMethodEnabled : 1;
    };
    QWidget *focusWidget;
    QMap<QWidget*, QWidget*> focusWidgetMap;
    QMap<QWidget*, QMenu*> modalMenuMap;
    QMap<QWidget*, QMenu*> nonModalMenuMap;
    QMap<QWidget*, WidgetData> widgetDataMap;
    QPointer<QAction> sepAction;
    QPointer<QAction> helpAction;
    QPointer<QAction> inputMethodAction;
    QPointer<QAction> cancelAction;
    static QPointer<MenuManager> mmgr;
#ifdef QTOPIA_TEST
    QMenu *activeMenu;
#endif
};

QPointer<MenuManager> MenuManager::mmgr = 0;

/*!
  Returns the QMenu assigned to widget \a widget in focus
  state \a state.  If a QMenu does not yet exist for the widget,
  an empty menu will be created with \a widget as its parent.

  \sa addMenuTo()
*/
QMenu *QSoftMenuBar::menuFor(QWidget *widget, FocusState state)
{
    return MenuManager::instance()->menuFor(widget, state);
}

/*!
  Returns true if the widget \a widget has a menu assigned to it
  in focus state \a state.

  \sa menuFor()
*/
bool QSoftMenuBar::hasMenu(QWidget *widget, FocusState state)
{
    return MenuManager::instance()->hasMenu(widget, state);
}

/*!
  Adds QMenu \a menu to widget \a widget for focus state \a state.

  \sa removeMenuFrom()
*/
void QSoftMenuBar::addMenuTo(QWidget *widget, QMenu *menu, FocusState state)
{
    MenuManager::instance()->addMenuTo(widget, menu, state);
}

/*!
  Removes this QMenu \a menu from widget \a widget for focus state \a state.

  \sa addMenuTo()
*/
void QSoftMenuBar::removeMenuFrom(QWidget *widget, QMenu *menu, FocusState state)
{
    MenuManager::instance()->removeMenuFrom(widget, menu, state);
}

/*!
  Sets whether help is available for widget \a widget when a menu
  attached to it is shown.  If \a enable is true help will be available
  for the widget.
*/
void QSoftMenuBar::setHelpEnabled(QWidget *widget, bool enable)
{
    MenuManager::instance()->setHelpEnabled(widget, enable);
}

/*!
  Sets whether inputMethod is available for widget \a widget when a menu
  attached to it is shown.  If \a enable is true inputMethod's menu action will be available
  for the widget.
*/
void QSoftMenuBar::setInputMethodEnabled(QWidget *widget, bool enable)
{
    MenuManager::instance()->setInputMethodEnabled(widget, enable);
}

/*!
  Sets whether cancel is available for widget \a widget when a menu
  attached to it is shown and the widget's top level window is a QDialog.
  If \a enable is true cancel will be available for the dialog.
*/
void QSoftMenuBar::setCancelEnabled(QWidget *widget, bool enable)
{
    MenuManager::instance()->setCancelEnabled(widget, enable);
}

/*!
  Returns the key that activates menus managed by the soft menu bar.
*/
int QSoftMenuBar::menuKey()
{
    return MenuManager::instance()->key();
}

/*!
  Creates and returns a standard "Edit" menu used for QLineEdit and QTextEdit.
*/
QMenu *QSoftMenuBar::createEditMenu()
{
    return new EditMenu(0);
}

#ifdef QTOPIA_TEST
/*!
  \internal
*/
QMenu* QSoftMenuBar::activeMenu()
{
    return MenuManager::instance()->getActiveMenu();
}
#endif

MenuManager::MenuManager()
    : QObject(qApp)
    , focusWidget(0)
    , sepAction(0)
    , helpAction(0)
    , inputMethodAction(0)
    , cancelAction(0)
#ifdef QTOPIA_TEST
    , activeMenu(0)
#endif
{
}

QMenu *MenuManager::menuFor(QWidget *w, QSoftMenuBar::FocusState state)
{
    if (!state)
        state = QSoftMenuBar::AnyFocus;

    QMenu *m = internalMenuFor(w, state);
    if (!m && (w->inherits("QLineEdit") ||
            w->inherits("QTextEdit") && !w->inherits("QTextBrowser"))) {
        m = new EditMenu(w);
        addMenuTo(w, m, state);
    } else if (!m) {
        m = new QMenu(w);
        addMenuTo(w, m, state);
    }
    return m;
}

bool MenuManager::hasMenu(QWidget *w, QSoftMenuBar::FocusState state)
{
    if (!state)
        state = QSoftMenuBar::AnyFocus;

    QMenu *m = internalMenuFor(w, state);
    return ( m != 0 );
}

void MenuManager::addMenuTo(QWidget *w, QMenu *menu, QSoftMenuBar::FocusState state)
{
    if (!w || !menu)
        return;

    if (!state)
        state = QSoftMenuBar::AnyFocus;

    QWidget *fw = w;
    while (fw->focusProxy())
        fw = fw->focusProxy();

    bool haveModal = internalMenuFor(w, QSoftMenuBar::EditFocus) == menu;
    bool haveNonModal = internalMenuFor(w, QSoftMenuBar::NavigationFocus) == menu;

    if (!focusWidgetMap.contains(fw)) {
        fw->installEventFilter(this);
        w->installEventFilter(this);
        focusWidgetMap[fw] = w;
    }
    if (widgetsWithMenu(menu, QSoftMenuBar::AnyFocus).isEmpty()) {
        menu->installEventFilter(this);
        connect(menu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));
    }
    QSoftMenuBar::setLabel(fw, key(), QSoftMenuBar::Options, state);
    QSoftMenuBar::setLabel(w, key(), QSoftMenuBar::Options, state);
    QSoftMenuBar::setLabel(menu, key(), QSoftMenuBar::Options, QSoftMenuBar::AnyFocus);

    if (state & QSoftMenuBar::EditFocus)
        modalMenuMap.insert(w, menu);
    if (state & QSoftMenuBar::NavigationFocus)
        nonModalMenuMap.insert(w, menu);

    WidgetData d;
    d.shown = 0;
    d.helpexists = 0;
    d.helpEnabled = 1;
    d.inputMethodEnabled = 1;
    d.cancelEnabled = 1;
    d.hasHelp = 0;
    widgetDataMap.insert(w, d);

    if (!haveModal && !haveNonModal)
        connect(w, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
}

void MenuManager::removeMenuFrom(QWidget *w, QMenu *menu, QSoftMenuBar::FocusState state)
{
    if (!state)
        state = QSoftMenuBar::AnyFocus;

    QWidget *fw = w;
    while (fw->focusProxy())
        fw = fw->focusProxy();

    bool haveModal = internalMenuFor(w, QSoftMenuBar::EditFocus) == menu;
    bool haveNonModal = internalMenuFor(w, QSoftMenuBar::NavigationFocus) == menu;

    if (state & QSoftMenuBar::EditFocus && haveModal) {
        QSoftMenuBar::clearLabel(fw, key(), QSoftMenuBar::EditFocus);
        QSoftMenuBar::clearLabel(w, key(), QSoftMenuBar::EditFocus);
        modalMenuMap.remove(w);
        haveModal = false;
    }
    if (state & QSoftMenuBar::NavigationFocus && haveNonModal) {
        QSoftMenuBar::clearLabel(fw, key(), QSoftMenuBar::NavigationFocus);
        QSoftMenuBar::clearLabel(w, key(), QSoftMenuBar::NavigationFocus);
        nonModalMenuMap.remove(w);
        haveNonModal = false;
    }

    if (!haveModal && !haveNonModal) {
        fw->removeEventFilter(fw);
        w->removeEventFilter(fw);
        focusWidgetMap.remove(fw);
        disconnect(w, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
    }
    if (widgetsWithMenu(menu, QSoftMenuBar::AnyFocus).isEmpty()) {
        menu->removeEventFilter(this);
        disconnect(menu, SIGNAL(destroyed()), this, SLOT(menuDestroyed()));
    }
    widgetDataMap.remove(w);
}

void MenuManager::setHelpEnabled(QWidget *widget, bool enable)
{
    if (widgetDataMap.contains(widget))
        widgetDataMap[widget].helpEnabled = enable;
}

void MenuManager::setInputMethodEnabled(QWidget *widget, bool enable)
{
    if (widgetDataMap.contains(widget))
        widgetDataMap[widget].inputMethodEnabled = enable;
}

void MenuManager::setCancelEnabled(QWidget *widget, bool enable)
{
    if (widgetDataMap.contains(widget))
        widgetDataMap[widget].cancelEnabled = enable;
}

int MenuManager::key()
{
    static int k = -1;
    if (k < 0) {
        if (!Qtopia::hasKey(Qt::Key_Menu)) {
            if (Qtopia::hasKey(Qt::Key_Context1))
                k = Qt::Key_Context1;
            else
                qWarning("Cannot map key to QMenu");
        } else {
            k = Qt::Key_Menu;
        }
    }

    return k;
}

#ifdef QTOPIA_TEST
QMenu *MenuManager::getActiveMenu() const
{
    return activeMenu;
}
#endif

bool MenuManager::triggerMenuItem( QMenu *menu, int keyNum )
{
    QList<QAction *> list = menu->actions();
    int count = list.count();
    keyNum = keyNum - 48;
    int index = 1;
    if (count) {
        for (int i = 0 ; i < count ; i++) {
            if ( !list.at(i)->isSeparator() && list.at(i)->isVisible() ) {
                if (keyNum == index && list.at(i)->isEnabled()) {
                    emit list.at(i)->trigger();
                    return true;
                } else {
                    index++;
                    if ( index == 10 )
                        index = 0;
                }
            }
        }
    }
    return false;
}

bool MenuManager::eventFilter(QObject *o, QEvent *e)
{
    if (!o || !e || e->type() != QEvent::KeyPress )
        return false;

    if (!o->isWidgetType())
        return false;

    QKeyEvent *k = (QKeyEvent *) e;
    int keyNum = k->key();

    QMenu *menu = qobject_cast<QMenu*>(o);
    if (menu && keyNum >= Qt::Key_0 && keyNum <= Qt::Key_9 ) {
        // trigger menuitem and close menu
        if (triggerMenuItem(menu, keyNum)) {
#ifdef QTOPIA_TEST
            activeMenu = 0;
#endif
            menu->close();
        }
    }

    if (k->key() != key()) {
        return EditMenu::keyEventFilter(o,e);
    }

    focusWidget = (QWidget*)o;

    if (e->type() == QEvent::KeyPress) {
        if (menu) {
            menu->close();
        } else {
            QMap<QWidget*, QWidget*>::ConstIterator it;
            QWidget *w = 0;
            it = focusWidgetMap.find(focusWidget);
            if (it == focusWidgetMap.end()) {
                w = qobject_cast<QWidget*>(o);
            } else {
                w = *it;
            }
            if (w) {
                QMenu *menu = internalMenuFor(w, w->hasEditFocus() ? QSoftMenuBar::EditFocus : QSoftMenuBar::NavigationFocus);
                if (menu) {
#ifdef QTOPIA_TEST
                    activeMenu = menu;
#endif
                    popup(w, menu);
                    return true;
                }
            }
        }
    }
    return false;
}

QMenu *MenuManager::internalMenuFor(QWidget *w, QSoftMenuBar::FocusState state)
{
    QMap<QWidget*, QMenu*>::ConstIterator it;
    if (state & QSoftMenuBar::EditFocus) {
        it = modalMenuMap.find(w);
        if (it != modalMenuMap.end())
            return *it;
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        it = nonModalMenuMap.find(w);
        if (it != nonModalMenuMap.end())
            return *it;
    }

    return 0;
}

QList<QWidget*> MenuManager::widgetsWithMenu(QMenu *menu, QSoftMenuBar::FocusState state)
{
    QList<QWidget*> list;
    QMap<QWidget*, QMenu*>::ConstIterator it;
    if (state & QSoftMenuBar::EditFocus) {
        for (it = modalMenuMap.begin(); it != modalMenuMap.end(); ++it) {
            if (*it == menu)
                list += it.key();
        }
    }
    if (state & QSoftMenuBar::NavigationFocus) {
        for (it = nonModalMenuMap.begin(); it != nonModalMenuMap.end(); ++it) {
            if (*it == menu && !list.contains(it.key()))
                list += it.key();
        }
    }

    return list;
}

void MenuManager::widgetDestroyed()
{
    QWidget *w = (QWidget*)sender();
    QMenu *menu = internalMenuFor(w, QSoftMenuBar::AnyFocus);
    if (menu) {
        removeMenuFrom(w, menu, QSoftMenuBar::AnyFocus);
    }
}

void MenuManager::menuDestroyed()
{
    QMenu *menu = (QMenu*)sender();
    QList<QWidget*> widgets = widgetsWithMenu(menu, QSoftMenuBar::AnyFocus);
    QList<QWidget*>::Iterator it;
    for (it = widgets.begin(); it != widgets.end(); ++it)
        removeMenuFrom(*it, menu, QSoftMenuBar::AnyFocus);
}

void MenuManager::help()
{
    QWidget* w = focusWidget;
    if (!w)
        w = qApp->activeWindow();
    if (!w)
        return;
    QWidget* focus = w;
    w = w->topLevelWidget();
    if (helpExists(w)) {
        QString hf = findHelp(focus);
        if ( hf.isEmpty() )
            hf = findHelp( w );
        if (hf.isEmpty()) {
            WidgetData &d = widgetDataMap[w];
            hf = d.helpFile;
        }
        qLog(Help) << ">>> Using help " << hf << "<<<";
        QtopiaServiceRequest env("Help", "setDocument(QString)");
        env << hf;
        env.send();
    }
}

void MenuManager::inputMethod()
{
    // Input method is likely to be in different process than server
    // so send notification via ipc
    QtopiaIpcEnvelope envelope("QPE/InputMethod", "activateMenuItem(int)");
    QAction *action =qobject_cast<QAction *>(sender());
    if(action){
        qLog(Input) << "Notifying IM that menu id " << action->data().toInt() << " has been activated";
        envelope << action->data().toInt();
    }
    else
    {
        envelope << 0;
        qLog(Input) << "MenuManager not able to determine which input method menu action was selected";
    };
}

void MenuManager::popup(QWidget *w, QMenu *menu)
{
    if (!widgetDataMap.contains(w))
        return;

    WidgetData &d = widgetDataMap[w];

    if (!d.shown ) {
        d.hasHelp = helpExists(w);
        d.shown = true;
    }

    if (sepAction) {
        delete sepAction;
        sepAction = 0;
    }

    if (helpAction) {
        delete helpAction;
        helpAction = 0;
    }

    if (cancelAction ) {
        delete cancelAction;
        cancelAction = 0;
    }
    // for now, refresh every time, in case input method has changed it's action
    // TODO: InputMethods should let us know when the IMMenu changes
    if(inputMethodAction)
    {
        delete inputMethodAction;
        inputMethodAction=0;
    }

    int visibleActionCount = 0;
    foreach ( QAction *action, menu->actions() )
        if ( action->isVisible() )
            visibleActionCount++;

    if (visibleActionCount)
        sepAction = menu->addSeparator();

    QList<QVariant> descriptionList = (QValueSpaceItem("UI/InputMethod").value( "MenuItem" )).toList();

    if(d.inputMethodEnabled && !descriptionList.isEmpty()){
        qLog(Input) << "Adding IM action to QSoftMenuBar menu";
        QIMActionDescription desc = descriptionList.first().value<QIMActionDescription>();
#ifdef DEBUG_QSOFTMENUBAR_IM_ACTIONS
        qDebug() << "desc.label is "<<desc.label();
        qDebug() << "desc.iconFileName is "<<desc.iconFileName();
        qDebug() << "inputMethodAction->icon() is "<<inputMethodAction->icon();
#endif
        inputMethodAction = menu->addAction(QIcon(desc.iconFileName()),desc.label());
        inputMethodAction->setData(desc.id());

        if(descriptionList.size() > 1);
        {
            QMenu* IMMenu = new QMenu();
            inputMethodAction->setMenu(IMMenu);
            connect(IMMenu, SIGNAL(triggered(QAction*)), this, SLOT(inputMethod()));
            qLog(Input) << "QSoftMenuBar is building IM menu";
            QList<QVariant>::iterator i = descriptionList.begin();
            ++i;
            for(; i != descriptionList.end(); ++i) {
                qLog(Input) << "Appending QAction from QIMActionDescription("<< i->value<QIMActionDescription>().id() << ","<< i->value<QIMActionDescription>().label() << ","<< i->value<QIMActionDescription>().iconFileName() <<")";

                QAction* action = IMMenu->addAction(QIcon(i->value<QIMActionDescription>().iconFileName()), i->value<QIMActionDescription>().label());
                action->setData(i->value<QIMActionDescription>().id());
                connect(action, SIGNAL(triggered()), this, SLOT(inputMethod()));
            }
        }
    }

    if (d.helpEnabled && d.hasHelp )
        helpAction = menu->addAction(QIcon( ":icon/help" ),
            tr("Help"), this, SLOT(help()));


    // qwizard check first?
    if (w && d.cancelEnabled) {
        QWidget* tlw = w->topLevelWidget();
        if (tlw->inherits("QDialog") && !Qtopia::hasKey(Qt::Key_No)) {
            if (!QtopiaApplication::isMenuLike((QDialog*)tlw))
                cancelAction = menu->addAction(QIcon(":icon/cancel"), tr("Cancel"), tlw, SLOT(reject()));
        }
    }

    if( !helpAction && !cancelAction && !inputMethodAction && sepAction ) {
        delete sepAction;
        sepAction = 0;
    }

    if (menu->actions().count()) {
        QDesktopWidget *desktop = QApplication::desktop();
        QRect r = desktop->availableGeometry(desktop->primaryScreen());
        QPoint pos;
        if ( QApplication::layoutDirection() == Qt::LeftToRight )
            pos = QPoint(r.left(), r.bottom() - menu->sizeHint().height() + 1);
        else {
            int x = r.right() - menu->sizeHint().width()+1;
            if ( x < 0 ) x=1;
            pos = QPoint(x, r.bottom() - menu->sizeHint().height() + 1);
        }
        menu->popup(pos);
        foreach (QAction *a, menu->actions()) {
            if (a->isEnabled() && a->isVisible()) {
                if ( a->menu() == 0 )
                    menu->setActiveAction(a);
                break;
            }
        }
    }
}

QString MenuManager::findHelp(const QWidget* w)
{
    // seems like we sometimes start an app with just the name and sometimes with a full path.
    // So make sure we only use the basename here
    QFileInfo fi( QString(qApp->argv()[0]) );
    QString hf;
    if( w==NULL ) {
        hf = fi.baseName() + ".html";
        QStringList helpPath = Qtopia::helpPaths();
        for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end(); ++it) {
            if ( QFile::exists( *it + "/" + hf ) ) {
                qLog(Help) << "Using help " << hf;
                return hf;
                break;
            }
        }
    }
    const QObject *widget=w;
    while(widget) {
        qLog(Help) << "checking object with widget->metaObject()->className():" << widget->metaObject()->className() << "widget->objectName():" << widget->objectName().toLower();
        hf = fi.baseName();
        hf += "-" + widget->objectName().toLower() + ".html";
        QStringList helpPath = Qtopia::helpPaths();
        for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end(); ++it) {
            if ( QFile::exists( *it + "/" + hf ) ) {
                qLog(Help) << "Using help " << hf;
                return hf;
                break;
            }
        }
        widget=widget->parent();
    }

    hf = fi.baseName();
    if ( w && w->objectName().isEmpty() ) {
        QString parents;
        parents = QString("(%1)").arg(w->metaObject()->className());
        QObject *p = w->parent();
        while (p) {
            parents = QString("(%1)%2").arg(p->metaObject()->className()).arg(parents);
            p = p->parent();
        }
        qLog(Help) << "No help for" << hf << parents;
    } else {
        qLog(Help) << "No help for " << hf;
    }
    return "";
}

bool MenuManager::helpExists(QWidget *w)
{
    WidgetData &d = widgetDataMap[w];
    if (d.helpFile.isNull()) {
        QString hf = findHelp(0);
        bool he = !hf.isEmpty();
        d.helpFile = hf;
        d.helpexists = he;
        return he;
    }
    if (!d.helpexists)
        qLog(UI) << ">>> NO help exists for" << d.helpFile;
    return d.helpexists;
}

#include "qsoftmenubar.moc"
