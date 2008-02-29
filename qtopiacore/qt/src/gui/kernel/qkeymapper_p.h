/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef QKEYMAPPER_P_H
#define QKEYMAPPER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qobject.h>
#include <private/qobject_p.h>
#include <qkeysequence.h>
#include <qlist.h>
#include <qlocale.h>
#include <qevent.h>


class QKeyMapperPrivate;
class QKeyMapper : public QObject
{
    Q_OBJECT
public:
    explicit QKeyMapper();
    ~QKeyMapper();

    static QKeyMapper *instance();
    static void changeKeyboard();
    static bool sendKeyEvent(QWidget *widget, bool grab,
                             QEvent::Type type, int code, Qt::KeyboardModifiers modifiers,
                             const QString &text, bool autorepeat, int count,
                             quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers);
    static QList<int> possibleKeys(QKeyEvent *e);

private:
    friend QKeyMapperPrivate *qt_keymapper_private();
    Q_DECLARE_PRIVATE(QKeyMapper)
    Q_DISABLE_COPY(QKeyMapper)
};



#if defined(Q_OS_WIN)
enum WindowsNativeModifiers {
    ShiftLeft            = 0x00000001,
    ControlLeft          = 0x00000002,
    AltLeft              = 0x00000004,
    MetaLeft             = 0x00000008,
    ShiftRight           = 0x00000010,
    ControlRight         = 0x00000020,
    AltRight             = 0x00000040,
    MetaRight            = 0x00000080,
    CapsLock             = 0x00000100,
    NumLock              = 0x00000200,
    ScrollLock           = 0x00000400,

    // Convenience mappings
    ShiftAny             = 0x00000011,
    ControlAny           = 0x00000022,
    AltAny               = 0x00000044,
    MetaAny              = 0x00000088,
    LockAny              = 0x00000700
};
# if !defined(tagMSG)
    typedef struct tagMSG MSG;
# endif
#elif defined(Q_WS_MAC)
# include <private/qt_mac_p.h>
#elif defined(Q_WS_X11)
typedef ulong XID;
typedef XID KeySym;

struct QXCoreDesc {
    int min_keycode;
    int max_keycode;
    int keysyms_per_keycode;
    KeySym *keysyms;
    uchar mode_switch;
    uchar num_lock;
    KeySym lock_meaning;
};

#endif

struct KeyboardLayoutItem;
class QKeyEvent;
class QKeyMapperPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QKeyMapper)
public:
    QKeyMapperPrivate();
    ~QKeyMapperPrivate();

    void clearMappings();
    QList<int> possibleKeys(QKeyEvent *e);

    QLocale keyboardInputLocale;
    Qt::LayoutDirection keyboardInputDirection;

#if defined(Q_OS_WIN)
    void clearRecordedKeys();
    void updateKeyMap(const MSG &msg);
    bool translateKeyEvent(QWidget *receiver, const MSG &msg, bool grab);
    void updatePossibleKeyCodes(unsigned char *kbdBuffer, quint32 scancode, quint32 vk_key);

    KeyboardLayoutItem *keyLayout[256];

#elif defined(Q_WS_X11)

    QList<int> possibleKeysXKB(QKeyEvent *event);
    QList<int> possibleKeysCore(QKeyEvent *event);

    bool translateKeyEventInternal(QWidget *keywidget,
                                   const XEvent *,
                                   KeySym &keysym,
                                   int& count,
                                   QString& text,
                                   Qt::KeyboardModifiers& modifiers,
                                   int &code,
                                   QEvent::Type &type,
                                   bool statefulTranslation = true);
    bool translateKeyEvent(QWidget *keywidget,
                           const XEvent *,
                           bool grab);

    bool useXKB;
    QXCoreDesc coreDesc;

#elif defined(Q_WS_MAC)
    bool updateKeyboard();
    void updateKeyMap(EventHandlerCallRef, EventRef, void *);
    bool translateKeyEvent(QWidget *, EventHandlerCallRef, EventRef, void *, bool);

    enum { NullMode, UnicodeMode, OtherMode } keyboard_mode;
    union {
        UCKeyboardLayout *unicode;
        void *other;
    } keyboard_layout_format;
    KeyboardLayoutRef currentKeyboardLayout;
    KeyboardLayoutKind keyboard_kind;
    UInt32 keyboard_dead;
    KeyboardLayoutItem *keyLayout[256];
#elif defined(Q_WS_QWS)
#endif
};

QKeyMapperPrivate *qt_keymapper_private(); // from qkeymapper.cpp

#endif // QKEYMAPPER_P_H
