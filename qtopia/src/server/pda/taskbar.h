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

#ifndef TASKBAR_H
#define TASKBAR_H

#include <qhbox.h>
#include "serverinterface.h"
#include "startmenu.h"

class QLabel;
class QTimer;
class InputMethods;
class Wait;
class SysTray;
class RunningAppBar;
class QWidgetStack;
class QTimer;
class QLabel;
class LockKeyState;
class AppLnkSet;

class TaskBar : public QHBox {
    Q_OBJECT
public:
    TaskBar();
    ~TaskBar();

    void launchStartMenu() { if (sm) sm->launch(); }
    void refreshStartMenu() { if (sm) sm->refreshMenu(); }
    void setApplicationState( const QString &name, ServerInterface::ApplicationState state );

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
    void styleChange( QStyle & );
    void setStatusMessage( const QString &text );
    
private slots:
    void receive( const QCString &msg, const QByteArray &data );

private:
    QTimer *waitTimer;
    Wait *waitIcon;
    InputMethods *inputMethods;
    SysTray *sysTray;
    RunningAppBar* runningAppBar;
    QWidgetStack *stack;
    QTimer *clearer;
    QLabel *label;
    LockKeyState* lockState;
    StartMenu *sm;
};


#endif // TASKBAR_H
