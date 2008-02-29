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

#ifndef __SYSTRAY_H__
#define __SYSTRAY_H__

#ifndef QT_NO_COMPONENT
#include <qtopia/taskbarappletinterface.h>
#endif

#include <qframe.h>
#include <qvaluelist.h>

class QHBoxLayout;
class QLibrary;
class PluginLoader;

struct TaskbarApplet
{
#ifndef QT_NO_COMPONENT
    QLibrary *library;
    TaskbarAppletInterface *iface;
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
    QHBoxLayout *layout;
    QValueList<TaskbarApplet*> appletList;
    PluginLoader *loader;
};


#endif // __SYSTRAY_H__

