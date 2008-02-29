/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef WAPUI_H
#define WAPUI_H

class QAction;
class QLabel;
class QListWidget;

#include <QWidget>

class WapUI : public QWidget
{
   Q_OBJECT
public:
    WapUI( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~WapUI();

    enum {
        ConfigRole = Qt::UserRole,
        DefaultRole = Qt::UserRole+1} WapRoles;

private slots:
    void addWap();
    void removeWap();
    void doWap();
    void updateActions();
    void selectDefaultWAP();

private:
    void loadConfigs();
    void init();
    QStringList availableWapConfigs();
    void updateNetStates();

private:

    QListWidget* wapList;
    QLabel* dfltAccount;
    QAction* wap_add;
    QAction* wap_remove;
    QAction* wap_props;

    friend class NetworkUI;
};

#endif // WAPUI_H
