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

#include "mmseditaccount.h"
#include "account.h"
#include <qtopiaapplication.h>
#include <qtopiaservices.h>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>

MmsEditAccount::MmsEditAccount(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(networkBtn, SIGNAL(clicked()), this, SLOT(configureNetworks()));
    QtopiaIpcAdaptor* netChannel = new QtopiaIpcAdaptor("QPE/NetworkState", this);
    QtopiaIpcAdaptor::connect(netChannel, MESSAGE(wapChanged()),
            this, SLOT(updateNetwork()));
}

void MmsEditAccount::populateNetwork()
{
    // Find available configs.
    QString path = Qtopia::applicationFileName("Network", "wap");
    QDir configDir(path);
    configDir.mkdir(path);

    QStringList files = configDir.entryList( QStringList("*.conf") );
    QStringList configList;
    foreach( QString item, files ) {
        configList.append( configDir.filePath( item ) );
    }

    // Get default
    QSettings cfg("Trolltech", "Network");
    cfg.beginGroup("WAP");
    QString defaultWap = cfg.value("DefaultAccount").toString();
    cfg.endGroup();
    int defaultConfig = -1;

    // Add to combo
    networkCombo->clear();
    foreach( QString config, configList ) {
        QSettings cfg( config, QSettings::IniFormat );
        QString name = cfg.value("Info/Name").toString();
        networkCombo->addItem(QIcon(":icon/netsetup/wap"), name, config);
        if ( config == defaultWap ) {
            defaultConfig = networkCombo->count()-1;
        }
        if ( config == account->networkConfig() ) {
            networkCombo->setCurrentIndex(networkCombo->count()-1);
        }
    }

    if (networkCombo->currentIndex() == -1 && defaultConfig >= 0)
        networkCombo->setCurrentIndex(defaultConfig);

    if (!networkCombo->count()) {
        networkCombo->addItem(tr("<None configured>", "No network profiles have been configured"));
        networkCombo->setCurrentIndex(0);
    }
}

void MmsEditAccount::setAccount(MailAccount *in)
{
    account = in;
    populateNetwork();
    autoRetrieve->setChecked(account->autoDownload());
}

void MmsEditAccount::accept()
{
    int currItem = networkCombo->currentIndex();
    if (currItem >= 0 && networkCombo->itemData(currItem).isValid()) {
        account->setNetworkConfig(networkCombo->itemData(currItem).toString());
        account->setAutoDownload(autoRetrieve->isChecked());
    }
    QDialog::accept();
}

void MmsEditAccount::configureNetworks()
{
    QtopiaServiceRequest serv("NetworkSetup", "configureWap()");
    serv.send();
}

void MmsEditAccount::updateNetwork()
{
    populateNetwork();
}

