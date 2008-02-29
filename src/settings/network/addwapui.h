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

#ifndef ADDWAPUI_H
#define ADDWAPUI_H

#include <QDialog>
#include <QWapAccount>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class WapAccountPage;
class GatewayPage;
class MMSPage;
class BrowserPage;

class AddWapUI : public QDialog
{
    Q_OBJECT
public:
    AddWapUI( QString file = QString(), QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~AddWapUI();

public slots:
    void accept();

private slots:
    void optionSelected(QListWidgetItem* item);
    void updateUserHint(QListWidgetItem* cur, QListWidgetItem* prev );

private:
    void init();
    //void writeConfig( const QtopiaNetworkProperties prop );

private:
    QListWidget* options;
    QStackedWidget* stack;
    QLabel* hint;
    QWapAccount acc;
    QString configFile;
    WapAccountPage* accountPage;
    GatewayPage* gatewayPage;
    MMSPage* mmsPage;
    BrowserPage* browserPage;
};

#endif
