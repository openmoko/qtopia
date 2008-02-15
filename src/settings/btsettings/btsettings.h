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

#ifndef __BTSETTINGS_H__
#define __BTSETTINGS_H__

#include <qcommdevicecontroller.h>
#include <QMainWindow>

class QMenu;
class QTabWidget;
class QBluetoothLocalDevice;

class BTSettingsMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    BTSettingsMainWindow(QWidget *parent = 0, Qt::WFlags fl = 0);
    virtual ~BTSettingsMainWindow();

private slots:
    void init();
    void tabChanged(int index);
    void setTabsEnabled(QCommDeviceController::PowerState state);

private:
    QMenu *m_menu;
    QTabWidget *m_tabs;
    QBluetoothLocalDevice *const m_localDevice;
    QCommDeviceController *m_controller;
};

#endif
