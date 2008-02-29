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

#include "inputmethods.h"

#include <qsettings.h>
#include <qtopiaapplication.h>
#include <qpluginmanager.h>

#include <qstyle.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qwidget.h>
#include <qlayout.h>
#include <qdir.h>
#include <stdlib.h>
#include <qtranslator.h>
#include <qdesktopwidget.h>
#include <qtopialog.h>

#include "windowmanagement.h"

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif
#include <qtopiachannel.h>

#ifdef QT_NO_COMPONENT
#include "../plugins/inputmethods/handwriting/handwritingimpl.h"
#include "../plugins/inputmethods/keyboard/keyboardimpl.h"
#include "../3rdparty/plugins/inputmethods/pickboard/pickboardimpl.h"
#endif

#include <QSoftMenuBar>
#include <QMenu>
#include <QMap>
#include <QStackedWidget>
#include <QDebug>
#include <QTranslatableSettings>
#include <QtopiaInputMethod>
#include <QList>

/*
  Slightly hacky: We use WStyle_Tool as a flag to say "this widget
  belongs to the IM system, so clicking it should not cause a reset".
 */
class IMToolButton : public QToolButton
{
public:
    IMToolButton( QWidget *parent = 0 ) : QToolButton( parent )
    {
    }

    QSize sizeHint() const {
        int w = style()->pixelMetric(QStyle::PM_SmallIconSize);
        return QSize(w,w);
    };
};

InputMethodSelector::InputMethodSelector(QWidget *parent)
    : QWidget(parent), mCurrent(0), m_IMMenuActionAdded(false), m_menuVS("UI/InputMethod"), defaultIM("")
{
    QHBoxLayout *hb = new QHBoxLayout;
    hb->setMargin(0);
    hb->setSpacing(0);

    pop = new QMenu( 0 );
    pop->setFocusPolicy( Qt::NoFocus ); //don't reset IM

    mButtonStack = new QStackedWidget;

    mButton = new IMToolButton;
    mButtonStack->addWidget(mButton);

    mStatus = 0;

    hb->addWidget(mButtonStack);

    mButton->setFocusPolicy(Qt::NoFocus);
    mButton->setCheckable( true );
    mButton->setAutoRaise( true );
    int sz = style()->pixelMetric(QStyle::PM_SmallIconSize);
    mButton->setIconSize(QSize(sz,sz));
    mButton->setBackgroundRole(QPalette::Button);

    connect(mButton, SIGNAL(toggled(bool)), this, SLOT(activateCurrent(bool)));

    mButton->setBackgroundRole(QPalette::Button);

    mChoice = new QToolButton;
    hb->addWidget(mChoice);

    mChoice->setBackgroundRole(QPalette::Button);
    mChoice->setIcon( QPixmap(":image/qpe/tri") );
    mChoice->setFixedWidth( 13 );
    mChoice->setAutoRaise( true );
    connect( mChoice, SIGNAL(clicked()), this, SLOT(showList()) );
    mChoice->hide();// until more than one.

    setLayout(hb);

    QTranslatableSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr
    cfg.beginGroup("InputMethods");
    defaultIM = cfg.value("DefaultIM").toString(); // No tr
}

InputMethodSelector::~InputMethodSelector()
{
    // doesn't own anything other than children... do nothing.
}

// before, need to set libName, iface, style.
void InputMethodSelector::add(QtopiaInputMethod *im)
{
    /* should check if im has a statusWidget()  */
    list.append(im);

    if (mCurrent == 0 || im->name() == defaultIM) {
        mCurrent = list[list.count()-1];
        emit activated(mCurrent);

        updateStatusIcon();
    }
    if (count() > 1)
        mChoice->show();
}

void InputMethodSelector::updateStatusIcon()
{
    if (mCurrent) {
        if (mCurrent->statusWidget(this)) {
            if (mStatus)
                mButtonStack->removeWidget(mStatus);
            mStatus = mCurrent->statusWidget(this);
            mButtonStack->addWidget(mStatus);
            mButtonStack->setCurrentWidget(mStatus);
        } else {
            QIcon i = mCurrent->icon();
            mButton->setIcon(i);
            mButtonStack->setCurrentWidget(mButton);
        }
    } else {
        mButton->setIcon(QIcon());
        mButtonStack->setCurrentWidget(mButton);
    }
}

void InputMethodSelector::focusChanged(QWidget* old, QWidget* now)
{
    Q_UNUSED(old);
    Q_UNUSED(now);
    qLog(Input) << "void InputMethodSelector::focusChanged(QWidget* old, QWidget* now)";
    updateIMMenuAction(m_IMMenuActionAdded);
}

void InputMethodSelector::showList()
{
    pop->clear();
    QMap<QAction *, QtopiaInputMethod *> map;
    foreach(QtopiaInputMethod *method, list) {
        QAction *a = pop->addAction(method->icon(), method->name());
        if ( mCurrent == method )
            a->setChecked( true );
        map.insert(a, method);
    }

    bool rtl = QApplication::isRightToLeft();
    QPoint pt;
    if ( !rtl )
        pt = mapToGlobal(mChoice->geometry().topRight());
    else
        pt = mapToGlobal(mChoice->geometry().topLeft());
    QSize s = pop->sizeHint();
    pt.ry() -= s.height();
    if ( !rtl )
        pt.rx() -= s.width();

    QAction *selected = pop->exec( pt );
    if ( selected == 0 )
        return;
    setInputMethod(map[selected]);

}

void InputMethodSelector::setInputMethod(QtopiaInputMethod *method)
{
    if (mCurrent != method) {
        activateCurrent(false);
        mCurrent = method;
        emit activated(mCurrent);
        updateStatusIcon();
    }
    activateCurrent(true);
}

void InputMethodSelector::activateCurrent( bool on )
{
    if (mCurrent) {
        QWidget *w = mCurrent->inputWidget();
        qLog(Input) << (on?"activating":"deactivating") << "input method" << (uint)mCurrent << ", with widget" << w;
        if (w) {
            if ( on ) {
                mCurrent->reset();
                // HACK... Make the texteditor fit with all input methods
                // Input methods should also never use more than about 40% of the screen
                int height = qMin( w->sizeHint().height(), 134 );

                QDesktopWidget *desktop = QApplication::desktop();
                w->resize(desktop->screenGeometry(desktop->primaryScreen()).width(), height );

                mButton->setChecked(true);
                w->show();

                //Add menu item:
                updateIMMenuAction(true);
            } else {
                mButton->setChecked(false);
                w->hide();
                updateIMMenuAction(false);
            }
            // should be emitted if the screen is changing sizes.
            emit inputWidgetShown( on );
        }
    }
}

void InputMethodSelector::clear()
{
    pop->clear();
    mChoice->hide();
    mCurrent = 0;
    emit activated(mCurrent);
    updateStatusIcon();
    list.clear();
}

class QtopiaInputMethodSorter
{
    public:
        bool operator()(const QtopiaInputMethod *first, const QtopiaInputMethod *second) {
            if (!first && second)
                return true;
            if (first && second)
                return (first->name() < second->name());
            return false;
        }
};

void InputMethodSelector::sort()
{
    if (list.count() > 1) {
        qSort( list.begin(), list.end(), QtopiaInputMethodSorter() );
        updateStatusIcon();
    }
}

QtopiaInputMethod *InputMethodSelector::current() const
{
    return mCurrent;
}

void InputMethodSelector::setInputMethod(const QString &s)
{
    foreach(QtopiaInputMethod *method, list) {
        if (method->identifier() == s) {
            setInputMethod(method);
            break;
        }
    }
}

/*
    A simple helper function for supporting inputmethod menu QActions.
    Removes the current IM's QAction from all menus.
    If the argument is true, adds the QAction to the current focus widget's
    Menu
*/

void InputMethodSelector::updateIMMenuAction(bool addToMenu)
{
    qLog(Input) << "Updating IM Menu actions";
    // Could have gotten here because input method changed menu options,
    // so set them again whether or not they're already shown
    if (m_IMMenuActionAdded) {
        qLog(Input) << "Removing valuespace Entry";
        m_menuVS.removeAttribute("MenuItem");
        m_IMMenuActionAdded = false;
    };

    if( addToMenu ){
        qLog(Input) << "Adding valuespace Entry";
        const QList<QIMActionDescription*> &actionDescriptionList  = mCurrent->menuDescription();

        if(!actionDescriptionList.isEmpty()){
            QList<QVariant> IMMenu;
            qLog(Input) << "Building IMMenu";
            for(QList<QIMActionDescription*>::const_iterator i = actionDescriptionList.begin(); i != actionDescriptionList.end(); ++i) {
//                qDebug() << "Appending an action that looks like: QIMActionDescription("<< (*i)->id() << ","<< (*i)->label() << ","<< (*i)->iconFileName() <<")";
                IMMenu.append(QVariant::fromValue(**i));
            };

            m_menuVS.setAttribute("MenuItem", QVariant(IMMenu));
            m_IMMenuActionAdded = true;
        }
    };
};


void InputMethodSelector::showChoice( bool on)
{
    if(on){
        mChoice->show();
    }
    else {
        mChoice->hide();
    };
};


InputMethods::InputMethods( QWidget *parent, IMType t ) :
    QWidget( parent ),
    loader(0), type(t), currentIM(0), lastActiveWindow(0), m_IMVisibleVS("/UI/IMVisible"), m_IMVisible(false)
{
    // Start up the input method service via QCop.
    new InputMethodService( this );

    //overrideWindowFlags(Qt::Tool);
    setObjectName("InputMethods");
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(0);
    setLayout(hbox);

    selector = new InputMethodSelector;
    hbox->addWidget(selector);


    connect(selector, SIGNAL(activated(QtopiaInputMethod *)),
            this, SLOT(choose(QtopiaInputMethod *)));

    connect(selector, SIGNAL(inputWidgetShown(bool)),
            this, SIGNAL(inputToggled(bool)));

    loadInputMethods();

    connect( qwsServer, SIGNAL(windowEvent(QWSWindow *, QWSServer::WindowEvent)),
            this, SLOT(updateHintMap(QWSWindow *, QWSServer::WindowEvent)));

    // might also add own win id since wouldn't have been added at start up.
}

InputMethods::~InputMethods()
{
    unloadInputMethods();
}

void InputMethods::hideInputMethod()
{
    selector->activateCurrent(false);
}

void InputMethods::showInputMethod()
{
    selector->activateCurrent(true);
}

void InputMethods::showInputMethod(const QString& name)
{
    selector->setInputMethod(name);
    selector->activateCurrent(true);
}

void InputMethods::activateMenuItem(int v)
{
    selector->current()->menuActionActivated(v);
}

void InputMethods::resetStates()
{
    // just the current ones.
    if (selector->current())
            selector->current()->reset();
}

QRect InputMethods::inputRect() const
{
    if (selector->current()) {
        QWidget *w = selector->current()->inputWidget();
        if (w && w->isVisible())
            return w->geometry();
    }
    return QRect();
}

void InputMethods::unloadInputMethods()
{
    if (currentIM) {
        QWSServer::setCurrentInputMethod( 0 );
        currentIM = 0;
    }
    if ( loader ) {
        selector->clear();
        ifaceList.clear();
        delete loader;
        loader = 0;
    }
}

void InputMethods::loadInputMethods()
{
    selector->blockSignals(true);
#ifndef QT_NO_COMPONENT
    hideInputMethod();
    unloadInputMethods();

    loader = new QPluginManager( "inputmethods" );

    foreach ( QString name, loader->list() ) {
        qLog(Input) << "Loading IM: "<<name;
        QObject *instance = loader->instance(name);
        QtopiaInputMethod *plugin = qobject_cast<QtopiaInputMethod*>(instance);
        if ( plugin ) {
            bool require_keypad = plugin->testProperty(QtopiaInputMethod::RequireKeypad);
            bool require_mouse = plugin->testProperty(QtopiaInputMethod::RequireMouse);
            if (type == Keypad && require_mouse || type == Mouse && require_keypad) {
                delete instance;
            } else {
                QWidget *w = plugin->inputWidget();
                if (w) {
                    w->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint );
                    w->setAttribute( Qt::WA_GroupLeader );
                    if(plugin->testProperty(QtopiaInputMethod::InputWidget) &&
                    plugin->testProperty(QtopiaInputMethod::DockedInputWidget))
                    {
                        qLog(Input) << "Docking input widget for "<<plugin->name();
                        WindowManagement::dockWindow(w, WindowManagement::Bottom);
                    }
                }
                selector->add(plugin);
                ifaceList.append(instance);
            }
            connect(plugin, SIGNAL(stateChanged()), this, SLOT(updateIMVisibility()));
        } else {
            delete instance;
        }
    }

    selector->sort();
#else

    if (type == Mouse) {
        // some check for which are avail?
        selector.add(new HandwritingInputMethod());
        selector.add(new KeyboardInputMethod());
        selector.add(new UnicodeInputMethod());
    }
    // } else if (type == Keypad) {
    // and the any?
#endif




    if ( selector->current()) {
        //keyBased->show?
        QtopiaInputMethod *imethod = selector->current();
        currentIM = imethod->inputModifier();
        QWSServer::setCurrentInputMethod( currentIM );
    } else {
        currentIM = 0;
    }

    updateIMVisibility();
    selector->blockSignals(false);
}

void InputMethods::updateIMVisibility()
{
    // hide... negates chance to interact with non input widget input methods...
    // Should never be true? True for only certain states?
    bool imvisible = false;

    // Only show the selector if there are at least 2 input methods,
    // and the current window has some kind of input hint.
    // Unless the current input method has an interactive widget, in which
    // case show it anyway. (Is this valid?)

    //the logic is:
    //bool shouldHideSelector=false;
    //if(selector->count()<2) shouldHideSelector = true;
    //if(hintMap[lastActiveWindow] == "") shouldHideSelector = true;
    //if(c && c->testProperty(QtopiaInputMethod::InteractiveIcon))  shouldHideSelector = false;  //this one might be wrong
    if (((selector->count() < 2)
                || !lastActiveWindow
                || !hintMap.contains(lastActiveWindow)
                || hintMap[lastActiveWindow] == "")
            && !selector->current()->testProperty(QtopiaInputMethod::InteractiveIcon)) {
        selector->showChoice( false );

    }
    else {
        selector->showChoice( true );
        imvisible = true;
   }

    if( imvisible != m_IMVisible ) {
        m_IMVisibleVS.setAttribute( "", QVariant(imvisible));
        m_IMVisible = imvisible;
        selector->activateCurrent(imvisible);
    }
    selector->refreshIMMenuAction();
    emit visibilityChanged();

}

void InputMethods::choose(QtopiaInputMethod* imethod)
{
    if ( imethod ) {
        currentIM = imethod->inputModifier();
        QWSServer::setCurrentInputMethod( currentIM );
    } else if (currentIM) {
        QWSServer::setCurrentInputMethod( 0 );
        currentIM = 0;
    }
}

void InputMethods::inputMethodHint( int h, int wid, bool password)
{

    if (hintMap.contains(wid)) {
        switch (h) {
            case (int)QtopiaApplication::Number:
                hintMap[wid] = "int";
                break;
            case (int)QtopiaApplication::PhoneNumber:
                hintMap[wid] = "phone";
                break;
            case (int)QtopiaApplication::Words:
                hintMap[wid] = "words";
                break;
            case (int)QtopiaApplication::Text:
                hintMap[wid] = "text";
                break;
            default:
                hintMap[wid] = QString();
                break;
        }
    }
    if(password)
        hintMap[wid] += " password";
    if (wid && wid == lastActiveWindow)
    {
        updateHint(wid);
    }
}

void InputMethods::inputMethodHint( const QString& h, int wid )
{
    bool r;
    if (h.contains("only")) {
        r = true;
    } else {
        r = false;
    }
    if (hintMap.contains(wid))
        hintMap[wid] = h;
    if (restrictMap.contains(wid))
        restrictMap[wid] = r;
    if (wid && wid == lastActiveWindow)
        updateHint(wid);
}

void InputMethods::inputMethodPasswordHint(bool passwordFlag, int wid)
{
    if(!hintMap.contains(wid))
    {
        // TODO: find default hint.
        // In the meantime, a normal sethint must be called before
        // setting password
        return;
    };

    int stringindex =  hintMap[wid].indexOf("password");
    if(!passwordFlag && stringindex!=-1)
    {
        hintMap[wid].remove("password");
        if(hintMap[wid].at(stringindex-1)==' ')
            hintMap[wid].remove(stringindex-1,1);

    } else if (passwordFlag && stringindex == -1)
    {
         hintMap[wid].append(" password");
    }
    if (wid && wid == lastActiveWindow)
        updateHint(wid);
}

void InputMethods::updateHintMap(QWSWindow *w, QWSServer::WindowEvent e)
{
    if (!w)
        return;
    // one signal can be multiple events.
    if ((e & QWSServer::Create) == QWSServer::Create) {
        if (!hintMap.contains(w->winId()))
            hintMap.insert(w->winId(), QString());
        if (!restrictMap.contains(w->winId()))
            restrictMap.insert(w->winId(), false);
    } else if ((e & QWSServer::Destroy) == QWSServer::Destroy) {
        if (hintMap.contains(w->winId()))
            hintMap.remove(w->winId());
        if (restrictMap.contains(w->winId()))
            restrictMap.remove(w->winId());
    }

    if ( (e & QWSServer::Active) == QWSServer::Active
         && w->winId() != lastActiveWindow)  {
        lastActiveWindow = w->winId();
        updateHint(lastActiveWindow);
    }
}

/* TODO: Also... if hint null, don't just set the hint, remove the IM/GM
   Not a problem now, (well behaved plugins) but should be done for
   misbehaved plugins.
 */
void InputMethods::updateHint(int wid)
{
    QtopiaInputMethod *imethod = selector->current();
    if ( imethod )
        imethod->setHint(hintMap[wid], restrictMap[wid]);
    updateIMVisibility();
}

bool InputMethods::shown() const
{
    return selector->current() && selector->current()->inputWidget() && selector->current()->inputWidget()->isVisible();
}

QString InputMethods::currentShown() const
{
    return shown() ? selector->current()->name() : QString();
}

bool InputMethods::selectorShown() const
{
    return selector->isVisible();
}

/*!
    \service InputMethodService InputMethod
    \brief Provides the Qtopia InputMethod service.

    The \i InputMethod service enables applications to adjust the input
    method that is being used on text entry fields.

    The messages in this service are sent as ordinary QCop messages
    on the \c QPE/InputMethod channel.  The service is provided by
    the Qtopia server.

    Normally applications won't need to send these messages directly,
    as they are handled by methods in the QtopiaApplication class.

    \sa QtopiaApplication
*/

/*!
    \internal
*/
InputMethodService::InputMethodService( InputMethods *parent )
    : QtopiaIpcAdaptor( "QPE/InputMethod", parent )
{
    this->parent = parent;
    publishAll(Slots);
}

/*!
    \internal
*/
InputMethodService::~InputMethodService()
{
    // Nothing to do here.
}

/*!
    Set the input method \a hint for \a windowId.  The valid values
    for \a hint are specified in QtopiaApplication::InputMethodHint.
    This message should not be used if the \c Named hint is requested.

    This slot corresponds to the QCop message
    \c{inputMethodHint(int,int)} on the \c QPE/InputMethod channel.
*/
void InputMethodService::inputMethodHint( int hint, int windowId )
{
    parent->inputMethodHint( hint, windowId );
}

/*!
    Set the input method \a hint for \a windowId.  This message should
    be used for \c Named hints.

    This slot corresponds to the QCop message
    \c{inputMethodHint(QString,int)} on the \c QPE/InputMethod channel.
*/
void InputMethodService::inputMethodHint( const QString& hint, int windowId )
{
    parent->inputMethodHint( hint, windowId );
}

/*!
    Set the input method \a passwordFlag for \a windowId.
*/

void InputMethodService::inputMethodPasswordHint(bool passwordFlag, int windowId)
{
    parent->inputMethodPasswordHint(passwordFlag, windowId);
};

/*!
    Explicitly hide the current input method.

    The current input method may still indicated in the taskbar, but no
    longer takes up screen space, and can no longer be interacted with.

    This slot corresponds to the QCop message
    \c{hideInputMethod()} on the \c QPE/InputMethod channel.

    \sa showInputMethod()
*/
void InputMethodService::hideInputMethod()
{
    parent->hideInputMethod();
}

/*!
    Explicitly show the current input method.

    Input methods may be indicated in the taskbar by a small icon. If the
    input method is activated (shown) then it takes up some proportion
    of the bottom of the screen, to allow the user to interact (input
    characters) with it.

    This slot corresponds to the QCop message
    \c{showInputMethod()} on the \c QPE/InputMethod channel.

    \sa hideInputMethod()
*/
void InputMethodService::showInputMethod()
{
    parent->showInputMethod();
}

/*!
    \internal
*/
void InputMethodService::activateMenuItem(int v)
{
    parent->activateMenuItem(v);
}

/*!
    If actions have been added to the softmenu on behalf of an IM, make
    sure they are up to date.

    see also: \a stateChanged()
*/
void InputMethodSelector::refreshIMMenuAction()
{
    updateIMMenuAction(m_IMMenuActionAdded);

};

