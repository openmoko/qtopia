/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef __QUERY_H__
#define __QUERY_H__

#include <QMainWindow>

class QBluetoothLocalDevice;
class QBluetoothSdpQuery;
class QBluetoothSdpQueryResult;

class QListWidget;
class QAction;

class Query : public QMainWindow
{
    Q_OBJECT

public:
    Query(QWidget *parent = 0, Qt::WFlags f = 0);
    ~Query();

public slots:
    void cancelQuery();

private slots:
    void startQuery();
    void searchCancelled();
    void searchComplete(const QBluetoothSdpQueryResult &result);

private:
    QBluetoothLocalDevice *btDevice;
    QBluetoothSdpQuery *sdap;
    bool canceled;
    QListWidget *serviceList;
    QAction *startQueryAction;
};

#endif
