/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qtuitestnamespace.h"
#include "qtuitestwidgets_p.h"
#include "qalternatestack_p.h"
#include "qelapsedtimer_p.h"
#include "qeventwatcher_p.h"
#include <QAction>
#include <QDebug>
#include <QEventLoop>
#include <QKeySequence>
#include <QPointer>
#include <QTimer>

#ifdef QTOPIA_TARGET
# include <Qtopia>
#endif

/*
    A simple auto pointer class which deletes the pointed-to QObject
    later via deleteLater()
*/
template <typename T>
class QDelayedAutoPointer
{
public:
    inline QDelayedAutoPointer(T* thing)
        : raw(thing)
    {}
    inline ~QDelayedAutoPointer()
    { raw->deleteLater(); }

    inline T* operator->()
    { return raw; }

    inline operator T*()
    { return raw; }

private:
    T* raw;
};

/*
    A helper class to encapsulate generation of input events
*/
struct QtUiTestInput
{
    virtual ~QtUiTestInput() {}
    virtual void post() const =0;
    virtual QString toString() const =0;
};


namespace QtUiTest {
    QString objectName(QObject* obj)
    {
        QString ret = obj->objectName();
        QAction* act(0);
        if (ret.isEmpty() && (act = qobject_cast<QAction*>(obj))) {
            ret = act->text();
            if (ret.isEmpty()) ret = act->iconText();
        }
        return ret;
    }
    QString toString(QObject* obj)
    {
        if (!obj) return "QObject(0x0)";
        return QString("%1(0x%2 \"%3\")")
            .arg(obj->metaObject()->className())
            .arg(QString::number(qptrdiff(obj), 16))
            .arg(objectName(obj))
        ;
    }

    QString toString(QEvent::Type type)
    {
#define DO(A) if (type == QEvent::A) return #A
        DO(KeyPress);
        DO(KeyRelease);
        DO(MouseButtonPress);
        DO(MouseButtonRelease);
        DO(Show);
        DO(Hide);
        DO(FocusIn);
        DO(FocusOut);
#ifdef QT_KEYPAD_NAVIGATION
        DO(EnterEditFocus);
        DO(LeaveEditFocus);
#endif
        DO(WindowBlocked);
#undef  DO
        return QString::number(int(type));
    }

    QString toString(QList<QEvent::Type> const& types)
    {
        QString ret;
        QString sep;
        foreach (QEvent::Type type, types) {
            ret += sep + toString(type);
            sep = ",";
        }
        return ret;
    }

    QString toString(Qt::Key key)
    { return QKeySequence(key).toString(); }

    QString toString(QPoint const& pos)
    { return QString("(%1,%2)").arg(pos.x()).arg(pos.y()); }

    QString toString(Qt::MouseButtons const& buttons)
    {
        QStringList ret;
#define DO(A) if (buttons & Qt::A) ret << #A
        DO(LeftButton);
        DO(RightButton);
        DO(MidButton);
        DO(XButton1);
        DO(XButton2);
#undef  DO
        return ret.join(",");
    }

    bool keyClick(QObject*, QList<QEvent::Type> const&, int, Qt::KeyboardModifiers,
        QtUiTest::InputOption);
    bool mouseClick(QObject*, QList<QEvent::Type> const&, QPoint const&, Qt::MouseButtons,
        QtUiTest::InputOption);
    bool inputWithEvent(QObject*, QList<QEvent::Type> const&, QtUiTestInput const&);
    bool inputWithSignal(QObject*, QByteArray const&, QtUiTestInput const&);
};

struct QtUiTestKeyClick : public QtUiTestInput
{
    QtUiTestKeyClick(Qt::Key key, Qt::KeyboardModifiers modifiers, QtUiTest::InputOption options)
        : m_key(key), m_modifiers(modifiers), m_options(options)
    {}

    virtual void post() const
    { QtUiTest::keyClick(m_key, m_modifiers, m_options); }

    virtual QString toString() const
    { return QString("Key click \"%1\"").arg(QtUiTest::toString(m_key)); }

    Qt::Key                m_key;
    Qt::KeyboardModifiers  m_modifiers;
    QtUiTest::InputOption  m_options;
};

struct QtUiTestMouseClick : public QtUiTestInput
{
    QtUiTestMouseClick(QPoint const& pos, Qt::MouseButtons buttons, QtUiTest::InputOption options)
        : m_pos(pos), m_buttons(buttons), m_options(options)
    {}

    virtual void post() const
    { QtUiTest::mouseClick(m_pos, m_buttons, m_options); }

    virtual QString toString() const
    {
        return QString("Mouse click \"%1\" at %2")
            .arg(QtUiTest::toString(m_buttons))
            .arg(QtUiTest::toString(m_pos))
        ;
    }

    QPoint const&          m_pos;
    Qt::MouseButtons       m_buttons;
    QtUiTest::InputOption  m_options;
};


/*!
    \preliminary
    \namespace QtUiTest
    \inpublicgroup QtUiTestModule

    \brief The QtUiTest namespace provides the plugin interfaces used for
    customizing the behaviour of QtUiTest.

    When running a \l{QSystemTest}{Qt Extended system test}, functions such as
    \l{QSystemTest::}{select()} and \l{QSystemTest::}{getText()} are used to
    perform actions and retrieve information from widgets in Qtopia.
    This is implemented by wrapping each conceptual widget (which is not
    necessarily a QWidget) with a test widget class.

    These test widgets each implement one or more of the widget interfaces
    in the QtUiTest namespace.  The interfaces are used to determine
    what actions can be taken on a particular widget, and how to perform
    them.

    For example, when the following system test code executes:
    \code
        select("Dog", "Favorite Animal");
    \endcode

    QtUiTest will first look up the QWidget which is labelled by the
    text "Favorite Animal".  It will then use qtuitest_cast to cast
    this widget to a QtUiTest::SelectWidget.  If this is successful, it
    will then call \l{QtUiTest::SelectWidget::select()}{select("Dog")}
    on the list widget.

    It is possible to customize the behavior of QtUiTest for particular
    widgets by creating custom test widget classes and a
    QtUiTest::WidgetFactory factory class to wrap QObject instances in
    test widgets.
*/

/*!
    \fn T QtUiTest::qtuitest_cast_helper(QObject* object,T dummy)
    \internal
*/

/*!
    \relates QtUiTest
    \fn T qtuitest_cast(const QObject *object)

    Casts \a object to the specified Qt Extended test widget interface \c{T}.

    If \a object already implements \c{T}, it is simply casted and returned.
    Otherwise, QtUiTest will attempt to find or create a test widget to
    wrap \a object, using all loaded QtUiTest::WidgetFactory plugins.
    If a test widget cannot be created to wrap \a object, 0 is returned.

    In either case, the returned value must not be deleted by the caller.

    Example:
    \code
    // Attempt to select the item "Foo" from the given widget
    bool selectFoo(QWidget *widget) {
        QtUiTest::SelectWidget* sw
            = qtuitest_cast<QtUiTest::SelectWidget*>(widget);
        if (!sw || !sw->canSelect("Foo")) {
            return false;
        }
        return sw->select("Foo");
    }
    \endcode
*/


/*!
    \enum QtUiTest::InputOption

    This enum type specifies the options to be used when simulating key
    and mouse events.

    \value NoOptions no options.
    \value DemoMode  when simulating, force artificial delays between key
                     and mouse events, and animate some events.
    \value KeyRepeat when simulating key press events, simulate auto-repeat
                     key press events. The default is to simulate regular key
                     press events.
*/

/*!
    \enum QtUiTest::WidgetType

    This enum type specifies different types of widgets which can be retrieved
    via QtUiTest::findWidget().

    \value Focus The widget which currently has keyboard focus.  Note that this
                 need not be located in the current application.
    \value InputMethod A currently active
                     \l{QtopiaInputMethod::inputWidget()}{input method widget}.
    \value SoftMenu  A currently displayed \l{QSoftMenuBar}{soft menu bar}.
    \value OptionsMenu The context/options menu which is currently shown, or
                       would be shown if the user attempted to raise a context
                       menu (typically by pressing Qt::Key_Context1).
    \value TabBar    The \l{QTabBar}{tab bar} for the currently active
                     \l{QTabWidget}{tab widget}, if one exists.
                     QtUiTest is not designed to handle multiple nested
                     tab widgets.
    \value HomeScreen The home screen widget.
    \value Launcher The widget (typically a grid-like menu) in the server
                    process used for launching applications.
    \value CallManager An object which implements SelectWidget and knows how to
                       select "accept" and "hangup" to manage calls.
                       This need not map to a single actual widget.
                       This is used to implement
                       \l{QtopiaSystemTest::}{callAccept()} and
                       \l{QtopiaSystemTest::}{callHangup()}.
*/

/*!
    \enum QtUiTest::ConnectionType

    This enum specifies the behavior of QtUiTest::connect().

    \value Before The connection will be activated before all connections
                  established by QObject::connect().
    \value After  The connection will be activated after all connections
                  established by QObject::connect().
*/

/*!
    \enum QtUiTest::Key

    This enum provides mappings for high-level conceptual keys to platform-specific
    values of Qt::Key.

    \value Key_Activate         Key used to activate generic UI elements.
    \value Key_ActivateButton   Key used to activate buttons.
    \value Key_Select           Key used to select an item from lists.
*/

/*!
    Set or clear the specified \a option for subsequent simulated input
    events.  The option is set if \a on is true, otherwise it is cleared.
*/
void QtUiTest::setInputOption(QtUiTest::InputOption option, bool on)
{ QtUiTestWidgets::instance()->setInputOption(option, on); }

/*!
    Returns true if \a option is currently set.
*/
bool QtUiTest::testInputOption(QtUiTest::InputOption option)
{ return QtUiTestWidgets::instance()->testInputOption(option); }

/*!
    Simulate a mouse press event at the global co-ordinates given by \a pos,
    for the specified \a buttons.  \a options are applied to the simulated
    event.
*/
void QtUiTest::mousePress(QPoint const& pos, Qt::MouseButtons buttons,
            QtUiTest::InputOption options)
{ QtUiTestWidgets::instance()->mousePress(pos, buttons, options); }


/*!
    Simulate a mouse release event at the global co-ordinates given by \a pos,
    for the specified \a buttons.  \a options are applied to the simulated
    event.
*/
void QtUiTest::mouseRelease(QPoint const& pos, Qt::MouseButtons buttons,
            QtUiTest::InputOption options)
{ QtUiTestWidgets::instance()->mouseRelease(pos, buttons, options); }

/*!
    Simulate a mouse click event at the global co-ordinates given by \a pos,
    for the specified \a buttons.  \a options are applied to the simulated
    event.
*/
void QtUiTest::mouseClick(QPoint const& pos, Qt::MouseButtons buttons,
            QtUiTest::InputOption options)
{ QtUiTestWidgets::instance()->mouseClick(pos, buttons, options); }

/*!
    \overload
    Simulate a mouse click event.
    Returns true if the event appears to be delivered to \a object within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::mouseClick(QObject* object, QPoint const& pos, Qt::MouseButtons buttons,
        QtUiTest::InputOption options)
{
    return mouseClick(object,
        QList<QEvent::Type>() << QEvent::MouseButtonRelease << QEvent::Hide << QEvent::WindowBlocked,
        pos, buttons, options
    );
}

/*!
    \internal
*/
bool QtUiTest::mouseClick(QObject* object, QList<QEvent::Type> const& types, QPoint const& pos,
        Qt::MouseButtons buttons, QtUiTest::InputOption options)
{ return inputWithEvent(object, types, QtUiTestMouseClick(pos, buttons, options)); }

/*!
    \overload
    Simulate a mouse click event.
    Returns true if the event causes \a object to emit \a signal within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::mouseClick(QObject* object, QByteArray const& signal, QPoint const& pos,
        Qt::MouseButtons buttons, QtUiTest::InputOption options)
{ return inputWithSignal(object, signal, QtUiTestMouseClick(pos, buttons, options)); }

/*!
    Simulate a key press event, using the given \a key and \a modifiers.
    \a key must be a valid Qt::Key or QtUiTest::Key.
    \a options are applied to the simulated event.
*/
void QtUiTest::keyPress(int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{ QtUiTestWidgets::instance()->keyPress(static_cast<Qt::Key>(key), modifiers, options); }

/*!
    Simulate a key release event, using the given \a key and \a modifiers.
    \a key must be a valid Qt::Key or QtUiTest::Key.
    \a options are applied to the simulated event.
*/
void QtUiTest::keyRelease(int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{ QtUiTestWidgets::instance()->keyRelease(static_cast<Qt::Key>(key), modifiers, options); }

/*!
    Simulate a key click event, using the given \a key and \a modifiers.
    \a key must be a valid Qt::Key or QtUiTest::Key.
    \a options are applied to the simulated event.
*/
void QtUiTest::keyClick(int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{ QtUiTestWidgets::instance()->keyClick(static_cast<Qt::Key>(key), modifiers, options); }

/*!
    \overload
    Simulate a key click event.
    Returns true if the event appears to be delivered to \a object within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::keyClick(QObject* object, int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{
    return keyClick(object,
        QList<QEvent::Type>() << QEvent::KeyRelease << QEvent::Hide << QEvent::WindowBlocked << QEvent::Show,
        key, modifiers, options
    );
}

/*!
    \internal
*/
bool QtUiTest::inputWithEvent(QObject* object, QList<QEvent::Type> const& types,
        QtUiTestInput const& event)
{
    QPointer<QObject> sender = object;
    QDelayedAutoPointer<QEventWatcher> w = new QEventWatcher;
    w->addObject(sender);
    foreach (QEvent::Type type, types)
        w->addType(type);

    event.post();

    if (!w->count() && !QtUiTest::waitForSignal(w, SIGNAL(event(QObject*,int)))) {
        setErrorString(QString(
            "%1 was expected to result in %2 receiving an event of type(s) %3, "
            "but it didn't.")
            .arg(event.toString())
            .arg(toString(sender))
            .arg(toString(types)));
        return false;
    }
    return true;
}

/*!
    \internal
*/
bool QtUiTest::inputWithSignal(QObject* object, QByteArray const& signal,
        QtUiTestInput const& event)
{
    if (signal.isEmpty()) return false;
    QPointer<QObject> sender = object;

    QTimer dummy;
    dummy.setInterval(1000);
    if (!QObject::connect(sender, signal, &dummy, SLOT(start()))) {
        setErrorString(QString("Object %1 has no signal %2").arg(toString(sender)).arg(&signal.constData()[1]));
        return false;
    }

    event.post();

    if (!dummy.isActive() && !QtUiTest::waitForSignal(sender, signal)) {
        setErrorString(QString(
            "%1 was expected to result in %2 emitting the signal %3, "
            "but it didn't.")
            .arg(event.toString())
            .arg(toString(sender))
            .arg(&signal.constData()[1]));
        return false;
    }
    return true;
}

/*!
    \internal
    \overload
    Simulate a key click event.
    Returns true if \a object receives any event of the given \a types within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::keyClick(QObject* object, QList<QEvent::Type> const& types, int key,
        Qt::KeyboardModifiers modifiers, QtUiTest::InputOption options)
{ return inputWithEvent(object, types, QtUiTestKeyClick(Qt::Key(key), modifiers, options)); }

/*!
    \overload
    Simulate a key click event.
    Returns true if \a object emits \a signal within maximumUiTimeout()
    milliseconds.
    If it does not, the errorString() will be set accordingly.
*/
bool QtUiTest::keyClick(QObject* object, QByteArray const& signal, int key, Qt::KeyboardModifiers modifiers,
        QtUiTest::InputOption options)
{ return inputWithSignal(object, signal, QtUiTestKeyClick(Qt::Key(key), modifiers, options)); }

/*!
    Returns true if widget interaction should prefer mouse events over
    key events.

    For example, when Qt Extended is used on a touchscreen-only device, this
    function will return true.
*/
bool QtUiTest::mousePreferred()
{
#ifdef QTOPIA_TARGET
    return Qtopia::mousePreferred();
#else
    static bool ret = !qgetenv("QTUITEST_MOUSEPREFERRED").isEmpty();
    return ret;
#endif
}

/*!
    Returns the maximum amount of time, in milliseconds, the user interface is allowed to take
    to generate some response to a user's action.

    This value is useful to determine how long test widgets should wait for certain events to occur
    after simulating key/mouse events.  The value may be device-specific.
*/
int QtUiTest::maximumUiTimeout()
{ return 2000; }

/*!
    Returns the Qt::Key corresponding to \a c.

    This function is commonly used in conjunction with keyClick() to enter
    a string of characters using the keypad.

    Example:
    \code
    using namespace QtUiTest;
    QString text = "hello world";
    // ...
    foreach (QChar c, text) {
        keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
    }
    \endcode
*/
Qt::Key QtUiTest::asciiToKey(char c)
{ return static_cast<Qt::Key>(QKeySequence(QString(QChar(c)))[0]); }

/*!
    Returns any Qt::KeyboardModifiers which would be required to input \a c.

    This function is commonly used in conjunction with keyClick() to enter
    a string of characters using the keypad.

    Example:
    \code
    using namespace QtUiTest;
    QString text = "hello world";
    // ...
    foreach (QChar c, text) {
        keyClick( asciiToKey(c.toLatin1()), asciiToModifiers(c.toLatin1()) );
    }
    \endcode
*/
Qt::KeyboardModifiers QtUiTest::asciiToModifiers(char c)
{
    Qt::KeyboardModifiers ret = Qt::NoModifier;
    if (QChar(c).isUpper()) ret |= Qt::ShiftModifier;
    return ret;
}

/*!
    Returns a human-readable error string describing the last error which
    occurred while accessing a testwidget.

    The error string is used to report directly to a tester any unexpected
    errors.  The string will typically be used as a test failure message.

    \sa setErrorString()
*/
QString QtUiTest::errorString()
{ return QtUiTestWidgets::instance()->errorString(); }

/*!
    Sets the human-readable \a error string describing the last error which
    occurred while accessing a testwidget.

    \sa errorString()
*/
void QtUiTest::setErrorString(QString const& error)
{ QtUiTestWidgets::instance()->setErrorString(error); }

/*!
    Returns a widget or test widget wrapper of \a type.

    QtUiTest will attempt to find widgets by calling
    QtUiTest::WidgetFactory::find() on all loaded widget factories.

    Note that it is possible to call this function from an application where
    the only widget of \a type exists in the server.  In this case, the
    reference implementation returns a special wrapper which transparently
    communicates with the server when calling functions.  However, if custom
    factories are used which do not do this, this function is likely to return
    0 in this case.

    Example:
    \code
    using namespace QtUiTest;
    bool MyTextEdit::enter(QVariant const& item) {
        // Instead of explicitly simulating mouse or key clicks,
        // enter the text via the current input method.
        // Note that this works whether or not the input method is located in
        // the current process.
        InputWidget *iw = qtuitest_cast<InputWidget*>(findWidget(InputMethod));
        return (iw && iw->enter(item));
    }
    \endcode
*/
QObject* QtUiTest::findWidget(WidgetType type)
{
    return QtUiTestWidgets::instance()->findWidget(type);
}

/*!
    \internal
    Returns a test widget wrapper for \a object which implements
    \a interface.
*/
QObject* QtUiTest::testWidget(QObject* object, const char* interface)
{
    return QtUiTestWidgets::instance()->testWidget(object, interface);
}

/*!
    Causes the process to wait for \a ms milliseconds. While waiting, events will be processed.
*/
void QtUiTest::wait(int ms)
{
    // If we are currently in an alternate stack, make sure we wait on the main
    // stack instead.  This avoids hanging due to nested event loops (bug 194361).
    foreach (QAlternateStack* stack, QAlternateStack::instances()) {
        if (!stack->isCurrentStack()) continue;

        // OK, we are running in this stack.

        // We are about to switch from this stack back to the main stack.
        // Arrange an event to occur so that we switch back to this stack
        // after the given timeout.

        // Must be created on the heap and destroyed with deleteLater(),
        // because when we switch back to this stack, QCoreApplication::notify
        // still holds a pointer to timer.

        QDelayedAutoPointer<QTimer> timer = new QTimer;
        timer->setObjectName("qtuitest_wait_timer");
        timer->setSingleShot(true);
        timer->setInterval(ms);

        QObject::connect(timer, SIGNAL(timeout()), stack, SLOT(switchTo()));

        // Now switch back to the main stack.
        timer->start();
        while (timer->isActive())
            stack->switchFrom();
        QObject::disconnect(timer, 0, 0, 0);

        // OK, we've returned from the main stack to this stack, so we've
        // waited for the given timeout and can now return.
        return;
    }

    // If we get here, we are running in the main stack, so just do a usual
    // possibly-hanging wait.
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}

#include <QDebug>

/*!
    Causes the process to wait for \a ms milliseconds or until \a signal is
    emitted from \a object, whichever comes first.

    While waiting, events will be processed.

    Returns true if \a signal was emitted from \a object before timing out.
    When false is returned, errorString() is set accordingly.

    If \a connectionType specifies a direct connection, this function will return
    immediately when the signal occurs, possibly before some objects have
    received the signal.  If \a connectionType specifies a queued connection, this
    function will return once the event loop has run following the emit.
*/
bool QtUiTest::waitForSignal(QObject* object, const char* signal, int ms, Qt::ConnectionType connectionType)
{
    if (ms < 0) return false;
    if (!signal || !signal[0]) return false;

    QPointer<QObject> sender = object;

    // Dummy variable to detect signal emission.
    QTimer dummy;
    dummy.setInterval(1000);
    if (!QObject::connect(sender, signal, &dummy, SLOT(start())))
        return false;

    // If we are currently in an alternate stack, make sure we wait on the main
    // stack instead.  This avoids hanging due to nested event loops (bug 194361).
    foreach (QAlternateStack* stack, QAlternateStack::instances()) {
        if (!stack->isCurrentStack()) continue;

        // OK, we are running in this stack.

        // We are about to switch from this stack back to the main stack.
        // Arrange an event to occur so that we switch back to this stack
        // after the given timeout.

        // Must be created on the heap and destroyed with deleteLater(),
        // because when we switch back to this stack, QCoreApplication::notify
        // still holds a pointer to timer.

        QDelayedAutoPointer<QTimer> timer = new QTimer;
        timer->setObjectName("qtuitest_waitForSignal_timer");
        timer->setInterval(ms);
        timer->setSingleShot(true);
        QObject::connect(timer, SIGNAL(timeout()), stack, SLOT(switchTo()));
        timer->start();

        while (timer->isActive() && !dummy.isActive()) {
            // Arrange it so that we switch back to this stack if the
            // desired signal is emitted.
            if (!QObject::connect(sender, signal, stack, SLOT(switchTo()), connectionType)) {
                QObject::disconnect(timer, 0, 0, 0);
                return false;
            }
            // Now switch back to the main stack.
            stack->switchFrom();
            QObject::disconnect(sender, signal, stack, SLOT(switchTo()));
        }

        QObject::disconnect(timer, 0, 0, 0);

        // OK, we've returned from the main stack to this stack, so we've
        // timed out or got the signal.  Check dummy to figure out which one.
        if (!dummy.isActive()) {
            setErrorString(QString("Object %1 was expected to emit %2 within %3 milliseconds, "
                "but it didn't.")
                .arg(toString(sender))
                .arg(&signal[1])
                .arg(ms)
            );
            return false;
        }
        return true;
    }

    // We are running on the main stack.  Use a (dangerous) nested event loop
    // to wait.
    QEventLoop loop;
    if (!QObject::connect(sender, signal, &loop, SLOT(quit())))
        return false;
    QTimer::singleShot(ms, &loop, SLOT(quit()));

    loop.exec();

    return dummy.isActive();
}

/*!
    Causes the process to wait for \a ms milliseconds or until an event of
    any of the given \a types is received by \a object, whichever comes first.

    While waiting, events will be processed.

    Returns true if the event was received by \a object before timing out.
    When false is returned, errorString() is set accordingly.

    If \a connectionType specifies a direct connection, this function will return
    immediately before the event is processed by \a object.
    If \a connectionType specifies a queued connection, this function will return
    once the event loop has run following the processing of the event.
*/
bool QtUiTest::waitForEvent(QObject* object, QList<QEvent::Type> const& types, int ms, Qt::ConnectionType connectionType)
{
    QPointer<QObject> sender = object;
    QDelayedAutoPointer<QEventWatcher> w = new QEventWatcher;
    w->setObjectName("qtuitest_waitForEvent_watcher");
    w->addObject(sender);
    foreach (QEvent::Type type, types)
        w->addType(type);

    if (!QtUiTest::waitForSignal(w, SIGNAL(event(QObject*,int)), ms, connectionType)) {
        setErrorString(QString("Object %1 was expected to receive an event of type(s) %2 within "
            "%3 milliseconds, but it didn't.")
            .arg(toString(sender))
            .arg(toString(types))
            .arg(ms)
        );
        return false;
    }
    return true;
}

/*!
    \overload
    Waits for an event of the given \a type.
*/
bool QtUiTest::waitForEvent(QObject* object, QEvent::Type type, int ms, Qt::ConnectionType connectionType)
{ return waitForEvent(object, QList<QEvent::Type>() << type, ms, connectionType); }

// this struct is copied from qobject_p.h to prevent us
// from including private Qt headers.
struct QSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(QObject *caller, int method_index, void **argv);
    typedef void (*EndCallback)(QObject *caller, int method_index);
    BeginCallback signal_begin_callback,
                  slot_begin_callback;
    EndCallback signal_end_callback,
                slot_end_callback;
};
extern void Q_CORE_EXPORT qt_register_signal_spy_callbacks(const QSignalSpyCallbackSet &);

struct QtUiTestObjectConnection
{
    QPointer<QObject> receiver;
    int method_index;
};

typedef QPair<const QObject*,int>                       ConnectionKey;
typedef QHash<ConnectionKey,QtUiTestObjectConnection> ConnectionHash;
typedef QHash<ConnectionKey,void**>                     ArgumentsHash;

Q_GLOBAL_STATIC(ConnectionHash,s_qtuitestPreConnections);
Q_GLOBAL_STATIC(ConnectionHash,s_qtuitestPostConnections);
Q_GLOBAL_STATIC(ArgumentsHash,s_qtuitestConnectionsArgv);
Q_GLOBAL_STATIC(QSet<QObject*>,s_qtuitestDeadConnections);
Q_GLOBAL_STATIC(QSet<QObject*>,s_qtuitestConnectedObjects);

void qtuitest_signal_activate(ConnectionHash &hash, QObject* caller, int method_index, void **argv)
{
    static const QtUiTestObjectConnection nullConnection = { 0, -1 };

    // Construct unique sender-signal key.
    ConnectionKey key = qMakePair(const_cast<const QObject*>(caller), method_index);
    QtUiTestObjectConnection connection = hash.value(key, nullConnection);

    // Did the hash actually contain a connection for this key?
    if (connection.method_index == nullConnection.method_index) return;

    // If the receiving object has been destroyed, remove the connection.
    if (!connection.receiver) {
        hash.remove(key);
        return;
    }

    // Activate the connected method.
    connection.receiver->qt_metacall(QMetaObject::InvokeMetaMethod, connection.method_index, argv);
}

void qtuitest_signal_begin_callback(QObject* caller, int method_index, void **argv)
{
    // These will be null when the application is exiting.
    if (!s_qtuitestPostConnections() || !s_qtuitestPreConnections()) return;

    // argv doesn't get passed to the end callback.
    // If we are going to need it, save it now.
    ConnectionKey key = qMakePair(const_cast<const QObject*>(caller), method_index);
    if (s_qtuitestPostConnections()->contains(key)) {
        s_qtuitestConnectionsArgv()->insert(key, argv);
    }

    // If the object has been destroyed, prepare to remove its connections from the hash.
    if ((method_index == QObject::staticMetaObject.indexOfSignal("destroyed(QObject*)"))
            && s_qtuitestConnectedObjects()->remove(caller)) {
        s_qtuitestDeadConnections()->insert(caller);
    }


    qtuitest_signal_activate(*s_qtuitestPreConnections(), caller, method_index, argv);
}

void qtuitest_signal_end_callback(QObject* caller, int method_index)
{
    // These will be null when the application is exiting.
    if (!s_qtuitestPostConnections() || !s_qtuitestPreConnections()) return;

    // argv doesn't get passed to the end callback.
    // Grab it out of the hash.
    ConnectionKey key = qMakePair(const_cast<const QObject*>(caller), method_index);
    void** argv = s_qtuitestConnectionsArgv()->take(key);
    // If we didn't have argv, then this connection was made after the signal was
    // emitted.  Better ignore it.
    if (argv) {
        qtuitest_signal_activate(*s_qtuitestPostConnections(), caller, method_index, argv);
    }

    // If this connection is dead, remove it.
    if (s_qtuitestDeadConnections()->remove(caller)) {
        QtUiTest::disconnect(caller,0,0,0);
    }
}

/*!
    Creates a connection of the given \a type from the \a signal in the \a sender object to
    the \a method in the \a receiver object. Returns true if the connection succeeds;
    otherwise returns false.

    This function behaves similarly to QObject::connect() with the following
    important differences.
    \list
        \o The connection is guaranteed to be activated either before or after
           any connections made with QObject::connect(), according to the \a type
           parameter.
        \o The connection type is always Qt::DirectConnection.
        \o The connection cannot be disconnected using QObject::disconnect().
        \o The connection does not affect the return value of QObject::receivers().
        \o While \a method is being executed, the return value of
           QObject::sender() is undefined.
    \endlist

    This function is primarily used in conjunction with QtUiTestRecorder to
    ensure events are recorded in the correct order.

    Note that this function cannot be used in a program which uses QSignalSpy.

    \sa QObject::connect()
*/
bool QtUiTest::connect(const QObject* sender,   const char* signal,
                       const QObject* receiver, const char* method,
                       QtUiTest::ConnectionType type)
{
    // On failure, we use QObject::connect to get the same error message as
    // we normally would.
    if (sender == 0 || receiver == 0 || signal == 0 || method == 0) {
        return QObject::connect(sender,signal,receiver,method);
    }
    if (qstrlen(signal) < 1 || qstrlen(method) < 1) {
        return QObject::connect(sender,signal,receiver,method);
    }

    const QMetaObject* const senderMo = sender->metaObject();

    QByteArray normalSignal = QByteArray::fromRawData(signal+1, qstrlen(signal));
    int signal_index = senderMo->indexOfSignal(normalSignal);

    if (signal_index < 0) {
        // See if we can find the signal after normalizing.
        normalSignal = QMetaObject::normalizedSignature(normalSignal);
        signal_index = senderMo->indexOfSignal(normalSignal);
    }
    if (signal_index < 0) {
        // Nope, bail out.
        return QObject::connect(sender,signal,receiver,method);
    }

    const QMetaObject* const receiverMo = receiver->metaObject();

    QByteArray normalMethod = QByteArray::fromRawData(method+1, qstrlen(method));
    int method_index = receiverMo->indexOfMethod(normalMethod);

    if (method_index < 0) {
        // See if we can find the method after normalizing.
        normalMethod = QMetaObject::normalizedSignature(normalMethod);
        method_index = senderMo->indexOfMethod(normalMethod);
    }
    if (method_index < 0) {
        // Nope, bail out.
        return QObject::connect(sender,signal,receiver,method);
    }

    // If we get here, then everything is valid.
    // Construct the connection and store it in the hash.
    QPair<const QObject*,int> key = qMakePair(sender, signal_index);
    QtUiTestObjectConnection connection = { const_cast<QObject*>(receiver), method_index };
    if (QtUiTest::Before == type) {
        s_qtuitestPreConnections()->insert(key, connection);
    } else if (QtUiTest::After == type) {
        s_qtuitestPostConnections()->insert(key, connection);
    } else {
        Q_ASSERT(0);
    }
    s_qtuitestConnectedObjects()->insert(const_cast<QObject*>(sender));

    // Ensure the signal spy callback is installed.
    static bool installedCallback = false;
    if (!installedCallback) {
        installedCallback = true;
        QSignalSpyCallbackSet callbacks = { qtuitest_signal_begin_callback, 0, qtuitest_signal_end_callback, 0 };
        qt_register_signal_spy_callbacks(callbacks);
    }
    return true;
}

/*!
    Disconnects \a signal in object \a sender from \a method in object \a receiver. Returns true if the connection is successfully broken; otherwise returns false.

    The connection must have been established with QtUiTest::connect().

    \sa QObject::disconnect()
*/
bool QtUiTest::disconnect(const QObject* sender,   const char* signal,
                          const QObject* receiver, const char* method)
{
    // On failure, we use QObject::disconnect to get the same error message as
    // we normally would.
    if (sender == 0) {
        return QObject::disconnect(sender,signal,receiver,method);
    }

    const QMetaObject* const senderMo = sender->metaObject();

    QByteArray normalSignal = (signal)
        ? QByteArray::fromRawData(signal, qstrlen(signal))
        : QByteArray();
    int signal_index = (signal) ? senderMo->indexOfSignal(normalSignal) : -1;
    if (signal && signal_index < 0) {
        // See if we can find the signal after normalizing.
        normalSignal = QMetaObject::normalizedSignature(signal);
        signal_index = senderMo->indexOfSignal(normalSignal);
    }
    if (signal && signal_index < 0) {
        // Nope, bail out.
        return QObject::disconnect(sender,signal,receiver,method);
    }

    if (method && !receiver) {
        return QObject::disconnect(sender,signal,receiver,method);
    }

    const QMetaObject* const receiverMo = (receiver) ? receiver->metaObject() : 0;

    QByteArray normalMethod = (method)
        ? QByteArray::fromRawData(method, qstrlen(method))
        : QByteArray();
    int method_index = (method) ? receiverMo->indexOfMethod(normalMethod) : -1;

    if (method && method_index < 0) {
        // See if we can find the method after normalizing.
        normalMethod = QMetaObject::normalizedSignature(method);
        method_index = senderMo->indexOfMethod(normalMethod);
    }
    if (method && method_index < 0) {
        // Nope, bail out.
        return QObject::disconnect(sender,signal,receiver,method);
    }

    QList<ConnectionHash*> hashes;
    hashes << s_qtuitestPreConnections() << s_qtuitestPostConnections();

    bool atLeastOneRemoved = false;

    ConnectionKey senderKey = qMakePair(sender, signal_index);
    foreach (ConnectionHash* hash, hashes) {
        foreach (ConnectionKey key, hash->keys()) {
            bool remove = true;

            if (sender && key.first != sender)
                remove = false;

            if (signal && key.second != signal_index)
                remove = false;

            if (receiver && hash->value(key).receiver != receiver)
                remove = false;

            if (method && hash->value(key).method_index != method_index)
                remove = false;

            if (remove) {
                hash->remove(key);
                atLeastOneRemoved = true;
            }
        }
    }

    return atLeastOneRemoved;
}

