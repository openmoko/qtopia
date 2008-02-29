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
#ifndef QDPLUGIN_P_H
#define QDPLUGIN_P_H

#include <qdplugindefs.h>

#include <qdglobal.h>

class MainWindow;
class QDLinkHelper;

class QD_EXPORT QDPluginData
{
public:
    QDPluginData();
    CenterInterface *center;
    QList<QDPlugin*> lockers;
};

class QD_EXPORT QDAppPluginData : public QDPluginData
{
public:
    QDAppPluginData();
    QWidget *appWidget;
    QWidget *settingsWidget;
    MainWindow *mainWindow;
};

class QD_EXPORT QDLinkPluginData : public QDPluginData
{
public:
    QDLinkPluginData();
    QDLinkHelper *helper;
};

#endif
