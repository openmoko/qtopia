/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef ACCOUNTPAGEIMPL_H
#define ACCOUNTPAGEIMPL_H

#include <QWidget>
#include <qtopianetwork.h>
#include <qtopianetworkinterface.h>
#include <qtopiaglobal.h>

class QComboBox;
class QLabel;
class QLineEdit;

class QTOPIACOMM_EXPORT AccountPage : public QWidget
{
    Q_OBJECT
public:

    AccountPage( QtopiaNetwork::Type type,
            const QtopiaNetworkProperties& cfg,
            QWidget* parent = 0, Qt::WFlags flags = 0 );
    virtual ~AccountPage();

    QtopiaNetworkProperties properties();
private:
    void init();
    void readConfig( const QtopiaNetworkProperties& prop);

private:
    QtopiaNetwork::Type accountType;
    QLineEdit* name;
    QLabel* startup_label;
    QComboBox* startup;

    QLabel* dialup_label;
    QLineEdit* dialup;

    QLabel* user_label;
    QLineEdit* user;
    QLabel* password_label;
    QLineEdit* password;
};
#endif// ACCOUNTPAGEIMPL_H

