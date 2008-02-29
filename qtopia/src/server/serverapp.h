/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef SERVERAPP_H
#define SERVERAPP_H

#include <qtopia/qpeapplication.h>

#include <qwidget.h>
#ifdef QWS
#include <qwindowsystem_qws.h>
#endif

#include "shutdownimpl.h"

#ifdef QTOPIA_TEST
# include "qpetestslave.h"
#endif

class PowerStatus;
class DesktopPowerAlerter;
class DeviceButton;
class StorageMonitor;
class QMessageBox;

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

class ServerApplication : public QPEApplication
{
    Q_OBJECT
public:
    ServerApplication( int& argc, char **argv, Type t );
    ~ServerApplication();

    static bool doRestart;
    static bool allowRestart;
    static bool screenLocked();
    static void login(bool at_poweron);
    static void lockScreen(bool);
    static bool haveFeature(const QString &);

    void restart();

    enum MemState { MemUnknown, MemVeryLow, MemLow, MemNormal, MemCritical };
    static MemState memstate;
    static QArray<int> pgFaults; 
signals:
    void home();
    void launch();
    void power();
    void backlight();
    void symbol();
    void numLockStateToggle();
    void capsLockStateToggle();
    void prepareForRestart();
    void activate(const DeviceButton*,bool);
    void recoverMemory(ServerApplication::MemState);
    void preProcessKeyLockEvent(ushort, ushort, bool);
    void authenticate(bool);

protected:
#ifdef Q_WS_QWS
    bool qwsEventFilter( QWSEvent * );
#endif
    void shutdown();
    void checkMemory();
    void keyClick(int unicode, int keycode, int modifiers, bool press, bool repeat);
    void screenClick(bool press);
    void evalMemory();
    int readMemInfo(int *freeMem); 

protected slots:
    void shutdown(ShutdownImpl::Type);
    void psTimeout();
    void showSafeMode();
    void clearSafeMode();
    void togglePower();
    void toggleLight();
    void rereadVolumes();
    void memoryMonitor();
    void showVMStatusToUser();
    
private:
    DesktopPowerAlerter *pa;
    PowerStatus *ps;
    StorageMonitor *sm;
    bool keyclick;
    bool touchclick;
    
    enum VMMonitorType { VMUnknown, VMLinux_2_4, VMLinux_2_6 };
    static VMMonitorType VMStatType;
    QTimer *delayedVMInfo;
    QTimer *vmMonitor;
    QMessageBox *userInfoBox;
    static QStringList *features;
     
    friend class KeyFilter;

#ifdef QTOPIA_TEST
    QPEServerTestSlave server_slave;
#endif
    

};


#endif // SERVERAPP_H

