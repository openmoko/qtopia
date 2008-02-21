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
#ifndef __LOCALSERVICESDIALOG_H__
#define __LOCALSERVICESDIALOG_H__

#include <QDialog>

class QListView;
class ServicesModel;
class QBluetoothServiceController;
class QModelIndex;
class QItemSelection;
class QAction;

class LocalServicesDialog : public QDialog
{
    Q_OBJECT
public:
    LocalServicesDialog(QWidget *parent = 0, Qt::WFlags fl=0);
    ~LocalServicesDialog();

public slots:
    void start();

private slots:
    void activated(const QModelIndex &index);
    void serviceStarted(const QString &name, bool error, const QString &desc);
    void toggleCurrentSecurity(bool checked);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void toggleState(const QModelIndex &index);

    QBluetoothServiceController *m_serviceController;
    ServicesModel *m_model;
    QListView *m_view;

    QAction *m_securityAction;
    QString m_lastStartedService;
};

#endif
