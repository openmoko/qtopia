/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __DESKTOP_H__
#define __DESKTOP_H__


#include "shutdownimpl.h"

#include <qtopia/qpeapplication.h>

#include <qwidget.h>
#ifdef QWS
#include <qwindowsystem_qws.h>
#endif

class Background;
class Launcher;
class TaskBar;
class PowerStatus;
class QCopBridge;
class TransferServer;
class DesktopPowerAlerter;
class PackageHandler;
class DeviceButton;
class ServiceRequest;

class KeyFilter : public QObject, public QWSServer::KeyboardFilter {
    Q_OBJECT
public:
    KeyFilter(QObject* parent);
    bool filter(int unicode, int keycode, int modifiers, bool press,
                      bool autoRepeat);

protected:
    void timerEvent(QTimerEvent*);

signals:
    void launch();
    void power();
    void backlight();
    void symbol();
    void numLockStateToggle();
    void capsLockStateToggle();
    void activate(const DeviceButton*,bool);

private:
    int held_tid;
    const DeviceButton* heldButton;
};

class DesktopApplication : public QPEApplication
{
    Q_OBJECT
public:
    DesktopApplication( int& argc, char **argv, Type t );
    ~DesktopApplication();

signals:
    void home();
    void launch();
    void backlight();
    void power();
    void symbol();
    void numLockStateToggle();
    void capsLockStateToggle();
    void prepareForRestart();
    void activate(const DeviceButton*,bool);

protected:
#ifdef Q_WS_QWS
    bool qwsEventFilter( QWSEvent * );
#endif
    void shutdown();
    void restart();

protected slots:
    void shutdown(ShutdownImpl::Type);
    void psTimeout();

private:
    DesktopPowerAlerter *pa;
    PowerStatus *ps;
};


class Desktop : public QWidget {
    Q_OBJECT
public:
    Desktop();
    ~Desktop();

    static bool screenLocked();

    void show();
    void checkMemory();

    void keyClick(int keycode, bool press, bool repeat);
    void screenClick(bool);
    static void soundAlarm();
    Launcher* appLauncher() { return launcher; }

public slots:
    void appMessage(const QCString& message, const QByteArray& data);
    void raiseMenu();
    void raiseLauncher();
    void togglePower();
    void toggleLight();
    void toggleNumLockState();
    void toggleCapsLockState();
    void toggleSymbolInput();
    void terminateServers();
    void rereadVolumes();
    void pokeTimeMonitors();
    void layoutLauncher();

private slots:
    void activate(const DeviceButton*,bool);

protected:
    void styleChange( QStyle & );
    void timerEvent( QTimerEvent *e );
    bool eventFilter( QObject *, QEvent * );

    QWidget *bg;
    Launcher *launcher;
    TaskBar *tb;

private:
    void startTransferServer();
    bool recoverMemory();

    QCopBridge *qcopBridge;
    TransferServer *transferServer;
    PackageHandler *packageHandler;

    bool keyclick,touchclick;
};


#endif // __DESKTOP_H__

