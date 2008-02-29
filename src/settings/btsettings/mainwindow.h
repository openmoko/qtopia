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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

template <class T> class QList;
class QBluetoothRemoteDevice;
class QBluetoothSdpQueryResult;

class BTSettings_Private;
class QListWidgetItem;

class BTSettingsMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    BTSettingsMainWindow( QWidget *parent=0, Qt::WFlags fl=0 );
    ~BTSettingsMainWindow();

public slots:
    void itemSelected(QListWidgetItem *item);
    void deviceEnabled(bool enable);

private:
    BTSettings_Private *m_data;
};

#endif

