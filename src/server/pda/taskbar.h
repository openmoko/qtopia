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

#ifndef TASKBAR_H
#define TASKBAR_H

#include "startmenu.h"
#include "applicationmonitor.h"
#include "applicationlauncher.h"

class QLabel;
class QTimer;
class InputMethods;
class Wait;
class SysTray;
class RunningAppBar;
class QStackedWidget;
class QTimer;
class QLabel;
class LockKeyState;

class TaskBar : public QWidget {
    Q_OBJECT
public:
    TaskBar();
    ~TaskBar();

    void launchStartMenu() { if (sm) sm->launch(); }
    void refreshStartMenu() { if (sm) sm->refreshMenu(); }

signals:
    void tabSelected(const QString&);

public slots:
    void startWait();
    void stopWait(const QString&);
    void stopWait();

    void clearStatusBar();
    void toggleNumLockState();
    void toggleCapsLockState();
    void toggleSymbolInput();
    void calcMaxWindowRect();

protected:
    void resizeEvent( QResizeEvent * );
    void changeEvent(QEvent *);
    void setStatusMessage( const QString &text );

private slots:
    void receive( const QString &msg, const QByteArray &data );
    void applicationStateChanged(const QString &,
                                 ApplicationTypeLauncher::ApplicationState);

private:
    QTimer *waitTimer;
    Wait *waitIcon;
    InputMethods *inputMethods;
    SysTray *sysTray;
    RunningAppBar* runningAppBar;
    QStackedWidget *stack;
    QTimer *clearer;
    QLabel *label;
    LockKeyState* lockState;
    StartMenu *sm;
};


#endif // TASKBAR_H
