/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#include "keyboardhandler.h"

#include "predictivekeyboard.h"
#include "keyboard.h"

#include <QDBusArgument>
#include <QDBusMetaType>

#include <QSettings>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>


static Atom s_keyboardAtom = 0;

Q_DECLARE_METATYPE(KeyboardData)
Q_DECLARE_METATYPE(QList<KeyboardData>)

QDBusArgument& operator<<(QDBusArgument &argument, const KeyboardData& data)
{
    argument.beginStructure();
    argument << data.name;
    argument << data.id;
    argument.endStructure();

    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, KeyboardData& data)
{
    argument.beginStructure();
    argument >> data.name;
    argument >> data.id;
    argument.endStructure();

    return argument;
}

KeyboardHandler::KeyboardHandler(QObject* parent)
    : QObject(parent)
    , m_currentWidget(0)
{
    if (!s_keyboardAtom)
        s_keyboardAtom = XInternAtom(QX11Info::display(), "_E_VIRTUAL_KEYBOARD", False);

    qRegisterMetaType<KeyboardData>("KeyboardData");
    qDBusRegisterMetaType<KeyboardData>();
    qDBusRegisterMetaType<QList<KeyboardData> >();

    QSettings cfg("Openmoko", "Keyboard");
    cfg.beginGroup("default");
    setKeyboard(cfg.value("keyboard", 0).toInt());
}

KeyboardHandler::~KeyboardHandler()
{
    foreach(int key, m_widgets.keys())
        delete m_widgets[key];
}

QList<KeyboardData> KeyboardHandler::keyboards() const
{
    static QList<KeyboardData> impl;
    if (impl.isEmpty()) {
        impl.append(KeyboardData(tr("Predictive Keyboard"), 0));
        impl.append(KeyboardData(tr("QWERTY Keyboard"), 1));
    }

    return impl;
}

void KeyboardHandler::setKeyboard(int keyboard)
{
    // Fallback to the predictive keyboard
    if (keyboard < 0 || keyboard > 1)
        keyboard = 0;

    // Remove the keyboard hint
    if (m_currentWidget) {
        XDeleteProperty(QX11Info::display(), m_currentWidget->winId(), s_keyboardAtom);
        m_currentWidget->hide();
        m_currentWidget = 0;

        XSync(QX11Info::display(), False);
    }

    // Create a keyboard
    if (!m_widgets.contains(keyboard)) {
        if (keyboard == 0) {
            PredictiveKeyboard* keyboard = new PredictiveKeyboard(0);
            m_widgets[0] = keyboard->widget();
        } else if (keyboard == 1) {
            Keyboard* keyboard = new Keyboard(0);
            m_widgets[1] = keyboard->frame();
        } else {
            Q_ASSERT(false);
        }
    }

    // Show the current keyboard
    Q_ASSERT(m_widgets[keyboard]);
    m_currentWidget = m_widgets[keyboard];

    unsigned char data = 1;
    XChangeProperty(QX11Info::display(), m_currentWidget->winId(), s_keyboardAtom,
            XA_CARDINAL, 32, PropModeReplace, &data, 1);
    m_currentWidget->setFocusPolicy(Qt::NoFocus);
    m_currentWidget->show();

    // Save the config
    QSettings cfg("Openmoko", "Keyboard");
    cfg.beginGroup("default");
    cfg.setValue("keyboard", keyboard);
    cfg.sync();
}
