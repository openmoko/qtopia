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

#ifndef ADDWAPUI_H
#define ADDWAPUI_H

#include <QDialog>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class QtopiaNetworkProperties;

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
#ifdef QTOPIA_PHONE
    void optionSelected(QListWidgetItem* item);
    void updateUserHint(QListWidgetItem* cur, QListWidgetItem* prev );
#endif
private:
    void init();
    void writeConfig( const QtopiaNetworkProperties prop );

private:
#ifdef QTOPIA_PHONE
    QListWidget* options;
    QStackedWidget* stack;
    QLabel* hint;
#else
    //TODO
#endif
    QString configFile;
    WapAccountPage* accountPage;
    GatewayPage* gatewayPage;
    MMSPage* mmsPage;
    BrowserPage* browserPage;
};

#endif
