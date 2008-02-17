/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef _QABSTRACTHOMESCREEN_H
#define _QABSTRACTHOMESCREEN_H

#include <QWidget>
#include <QPixmap>

#ifdef QTOPIA_CELL
#ifdef QTOPIA_PHONEUI
#include <qsmsmessage.h>
#endif
#include "cellbroadcastcontrol.h"
#endif
#include "devicebuttontask.h"
#include "phonelock.h"

class QMenu;

class QAbstractHomeScreen : public QWidget
{
    Q_OBJECT

public:
    QAbstractHomeScreen(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~QAbstractHomeScreen();

    virtual uint showInformation(const QString &pix, const QString &text, int priority = 10);
    virtual void showPinboxInformation(const QString &pix, const QString &text);
    virtual void clearInformation(uint id);

    bool keyLocked() const;
#ifdef QTOPIA_CELL
    bool simLocked() const;
#endif
    void setKeyLocked(bool) const;
    virtual void updateBackground();
    virtual void updateHomeScreenInfo();
    virtual void activatePinbox(bool enable);
    virtual void updateInformation();

#ifdef QTOPIA_PHONEUI
    void setNewMessages(int);
    void setSmsMemoryFull(bool);
    void setMissedCalls(int);
    void showCallHistory(bool missed, const QString &hint);
#endif

Q_SIGNALS:
#ifdef QTOPIA_PHONEUI
    void callEmergency(const QString &);
    void showCallScreen();
    void showCallHistory();
    void showMissedCalls();
    void dialNumber(const QString &);
#endif

    void speedDial(const QString &);
    void keyLockedChanged(bool);
    void showPhoneBrowser();


protected:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void inputMethodEvent(QInputMethodEvent *);
    bool eventFilter(QObject *, QEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

    struct InfoData {
        int id;
        int priority;
        QString text;
        QPixmap pixmap;

        bool operator<=(const InfoData &d) const {
            return priority <= d.priority;
        }
        bool operator<(const InfoData &d) const {
            return priority < d.priority;
        }
        bool operator>(const InfoData &d) const {
            return priority > d.priority;
        }
    };

    void loadPlugins();
    void setLockLevel(int);

public Q_SLOTS:
    virtual void applyHomeScreenImage();

protected Q_SLOTS:
    void showLockInformation();
    void setContextBarLocked(bool locked, bool waiting = false);
    void activateSpeedDial();
    void specialButton(int keycode, bool held);
    void showProfileSelector();
    void lockScreen();

#ifdef QTOPIA_PHONEUI
    void phoneStateChanged();
    void viewNewMessages();
    void viewMissedCalls();
    void hideCBSMessage();
#endif

#ifdef QTOPIA_CELL
    void cellBroadcast(CellBroadcastControl::Type, const QString &, const QString &);
#endif

protected:
#ifdef QTOPIA_CELL
    BasicEmergencyLock  *emLock;
    BasicSimPinLock     *simLock;
#endif

#ifdef QTOPIA_PHONEUI
    QAction *actionMessages;
    QAction *actionCalls;
#endif

    BasicKeyLock    *keyLock;
    QAction         *actionLock;
    QList<InfoData> infoData;
    QTimer          *speeddialTimer;
    QString         speeddial;
    QString         speeddial_activated_preedit;
    PressHoldGate   *ph;
    QMenu           *m_contextMenu;
    int             lockMsgId;
    int             infoMsgId;
    int             missedCalls;
    int             speeddial_preedit;
    bool            speeddialdown;
    int             newMessages;
    bool            smsMemoryFull;
};

#endif // _QABSTRACTHOMESCREEN_H
