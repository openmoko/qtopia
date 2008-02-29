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
#ifndef NETWORKUI_H
#define NETWORKUI_H

#include <QDialog>
#include <QTableWidget>
#include <qtopianetworkinterface.h>
#include <qtopiaabstractservice.h>
#include <QDSActionRequest>

class QAction;
class QTabWidget;
class QMenu;
class QOtaReader;

class NetworkWidgetItem;
class WapUI;
class VpnUI;
class QWspPush;

class NetworkUI : public QDialog
{
    Q_OBJECT
public:
    NetworkUI( QWidget * parent = 0, Qt::WFlags fl = 0);
    ~NetworkUI();

    void setCurrentTab(int tab);

private:
    void init();
    void updateExtraActions( const QString& config, QtopiaNetworkInterface::Status newState );
#ifdef QTOPIA_CELL
    void applyRemoteSettings( const QString& from, const QtopiaNetworkProperties& prop );
#endif
    void addService(const QString& newConfFile);

private slots:
    void addService();
    void removeService();
    void serviceSelected();
    void doProperties();
    void updateActions();
    void updateIfaceStates();
    void tabChanged( int index );
    void setGateway();
    void updateConfig();

public slots:
#ifdef QTOPIA_CELL
    void otaDatagram( QOtaReader *reader, const QByteArray& data,
                      const QString& sender );
#endif


private:
    QTableWidget* table;
    QTabWidget* tabWidget;
    WapUI* wapPage;
    VpnUI* vpnPage;
    QMenu* contextMenu;
    QMultiMap<QString, QPointer<QAction> > actionMap;
    QAction *a_add, *a_remove, *a_props, *a_gateway;
};

class NetworkSetupService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class NetworkUI;
private:
    NetworkSetupService(NetworkUI *parent)
        : QtopiaAbstractService("NetworkSetup", parent)
        { this->parent = parent; publishAll(); }

public:
    ~NetworkSetupService();

public slots:
    void configureData();
#ifdef QTOPIA_CELL
    void configureWap();
    void pushWapNetworkSettings( const QDSActionRequest& request );
    void pushNokiaNetworkSettings( const QDSActionRequest& request );
#endif

private:
    NetworkUI *parent;
};


#endif // NETWORKUI_H
