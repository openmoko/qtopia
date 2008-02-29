/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef HOME_SCREEN
#define HOME_SCREEN

#include <themedview.h>
#include <qvaluespace.h>
#include "phonethemeview.h"
#include <qwidget.h>
#include <qpixmap.h>

#ifdef QTOPIA_PHONEUI
#include <qsmsmessage.h>
#endif

#ifdef QTOPIA_CELL
#include "cellbroadcastcontrol.h"
#endif
#include "devicebuttontask.h"
#include "phonelock.h"

class ThemedItemInterface;

//===========================================================================

class QAction;
class QTimer;
class HomeScreen : public PhoneThemedView
{
    Q_OBJECT
public:
    ~HomeScreen();

    uint showInformation(const QString &pix, const QString &text, int priority=10);
    void clearInformation(uint id);

#ifdef QTOPIA_PHONEUI
    void setNewMessages(int);
    void setMissedCalls(int);
    void showCallHistory(bool missed, const QString &hint);
#endif

    static HomeScreen *getInstancePtr();

    bool keyLocked() const;
    void setKeyLocked(bool) const;
    void applyBackgroundImage();
signals:
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

private slots:
    void showLockInformation();

protected:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void inputMethodEvent(QInputMethodEvent *);
    virtual void themeLoaded(const QString &);
    virtual bool eventFilter(QObject *, QEvent *);
    QWidget *newWidget(ThemeWidgetItem* input, const QString& name);

private:
    HomeScreen(QWidget *parent = 0, Qt::WFlags f=0);

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
    void updateInfo();

    void loadPlugins();
    void setLockLevel(int);

private slots:
    void setContextBarLocked(bool locked);
#ifdef QTOPIA_PHONEUI
    void phoneStateChanged();
    void viewNewMessages();
    void viewMissedCalls();
    void hideCBSMessage();
#endif
    void activateSpeedDial();

#ifdef QTOPIA_CELL
    void cellBroadcast(CellBroadcastControl::Type, const QString &,
                       const QString &);
#endif
    void specialButton(int keycode, bool held);
    void showProfileSelector();
    void themeItemClicked(ThemeItem *item);

private:
    BasicKeyLock *keyLock;
#ifdef QTOPIA_CELL
    BasicSimPinLock *simLock;
#endif

    QAction *actionLock;
#ifdef QTOPIA_PHONEUI
    QAction *actionMessages;
    QAction *actionCalls;
#endif
    QTimer *lockTimer;
    int lockMsgId, infoMsgId;
    int missedCalls;
    ThemedItemPlugin *bgIface;
    QList<InfoData> infoData;
    QTimer *speeddialTimer;
    int speeddial_preedit;
    bool speeddialdown;
    QString speeddial;
    QString speeddial_activated_preedit;
    PressHoldGate *ph;
    QMenu *m_contextMenu;
};

#endif
