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

#include <QDialog>

#include "ui_mainwindow.h"

class QEvent;

class ServerWidgetSettings : public QDialog
{
    Q_OBJECT
public:
    enum {
        ServerInterface,
        SecondaryDisplay,
        DialerScreen,
        MessageBox,
        BrowserScreen,
        CallScreen,
    } WidgetType;

    ServerWidgetSettings( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~ServerWidgetSettings();

public slots:
    void accept();

protected:
    bool eventFilter( QObject* watched, QEvent* event );


private slots:
    void shutdownClicked();
    void newWidgetSelected( QTreeWidgetItem* item, int column );

private:
    void init();
    void saveSettings();

private:
    Ui::MainWindow ui;
};

