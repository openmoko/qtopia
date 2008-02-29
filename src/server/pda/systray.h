/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef __SYSTRAY_H__
#define __SYSTRAY_H__

#ifndef QT_NO_COMPONENT
#include <qtaskbarappletplugin.h>
#endif

#include <qframe.h>
#include <qlist.h>

class QHBoxLayout;
class QLibrary;
class QPluginManager;

struct TaskbarApplet
{
#ifndef QT_NO_COMPONENT
    QObject *instance;
    QTaskbarAppletPlugin *iface;
#endif
    QWidget *applet;
    QString name;
};

class SysTray : public QFrame {
    Q_OBJECT
public:
    SysTray( QWidget *parent );
    ~SysTray();

    void clearApplets();
    void addApplets();

private:
    void loadApplets();
    QHBoxLayout *trayLayout;
    QList<TaskbarApplet*> appletList;
    QPluginManager *loader;
};


#endif // __SYSTRAY_H__

