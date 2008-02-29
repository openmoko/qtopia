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

#ifndef VPNUI
#define VPNUI

#include <QWidget>
#include <QModelIndex>
#include <qvpnclient.h>

class QAction;
class QListView;
class QValueSpaceItem;
class VPNListModel;

class VpnUI : public QWidget {
    Q_OBJECT
public:
    VpnUI( QWidget* parent = 0, Qt::WFlags f = 0 ) ;
    ~VpnUI();

private:
    void init();

private slots:
    void newConnection();
    void editConnection();
    void removeConnection();
    void newVPNSelected( const QModelIndex& cur, const QModelIndex& prev = QModelIndex() );
    void vpnActivated( const QModelIndex& item );

private:
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QAction* propertyAction;
    QAction* removeAction;
#endif
    QListView* vpnList;
    VPNListModel* model;

    friend class NetworkUI;
};

#endif //VPNUI
