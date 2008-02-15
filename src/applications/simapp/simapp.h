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

#ifndef SIMAPP_H
#define SIMAPP_H

#include <QMainWindow>
#include <QSimTerminalResponse>
#include <QtopiaAbstractService>

#include "simwidgets.h"

class QStackedWidget;
class QSimCommand;
class QSimToolkit;
class QSimIconReader;
class SimText;
class QPhoneCall;
class QPhoneCallManager;
class QTimer;
class QValueSpaceObject;
class QLabel;

#ifndef QTOPIA_TEST
#include "../../server/core_server/applicationlauncher.h"
#endif

class SimApp : public QMainWindow
{
    Q_OBJECT
    friend class SimAppService;

public:
    SimApp(QWidget *parent, Qt::WFlags f=0);
    ~SimApp();

    static SimApp *instance();

    SimCommandView *currentView() const { return view; }

protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
    void showEvent(QShowEvent *);
    bool eventFilter(QObject *, QEvent *);

private slots:
    void activate();
    void beginFailed();
    void simCommand(const QSimCommand &);
    void simRemoved();
    void updateValueSpace();
    void iconAvailable(int iconId);
    void removeView(QWidget *);
#ifndef QTOPIA_TEST
    void applicationTerminated(const QString &app, ApplicationTypeLauncher::TerminationReason reason, bool filtered);
#endif
    void userActivityOccurred();
    void sendEnvelope(const QSimEnvelope&);
    void sendResponse(const QSimTerminalResponse&);

public slots:
    void terminateSession();
    void hideApp();

signals:
    void viewChanged(SimCommandView *);

private:
    void cmdMenu(const QSimCommand &);
    void cmdDisplayText(const QSimCommand &);
    void cmdInKey(const QSimCommand &);
    void cmdInput(const QSimCommand &);
    void cmdSetupCall(const QSimCommand &);
    void cmdTone(const QSimCommand &);
    void cmdRefresh(const QSimCommand &);
    void cmdChannel(const QSimCommand &);
    void cmdIdleModeText(const QSimCommand &);
    void cmdSetupEventList(const QSimCommand &);
    void cmdLaunchBrowser(const QSimCommand &);
    void showNotification(const QSimCommand &);
    void hideNotification();
    void setView(SimCommandView *);
    bool listViewPreferred(const QSimCommand &);
    void softKeysMenu(const QSimCommand &);

private:
    QPhoneCallManager *callManager;
    QSimToolkit *stk;
    QSimIconReader *iconReader;
    QStackedWidget *stack;
    SimCommandView *view;
    SimText *notification;
    bool hasStk;
    bool simToolkitAvailable;
    QSimCommand idleModeText;
    QImage idleModeImage;
    QString mainMenuTitle;
    QValueSpaceObject *status;
    int eventList;
    uint idleModeMsgId;
    QLabel *failLabel;
    bool commandOutsideMenu;
    bool hasSustainedDisplayText;

    void createIconReader();
    void changeEventList(int newEvents);
    void changeUserActivityEvent(bool value);
    void changeIdleScreenEvent(bool value);
    void changeBrowserTerminationEvent(bool value);
};

#endif

