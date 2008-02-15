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

#include "accountsettings.h"
#include "accountlist.h"
#include "editaccount.h"
#include "emailclient.h"
#ifndef QTOPIA_NO_MMS
#  include "mmseditaccount.h"
#endif
#include "statusdisplay.h"
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qsoftmenubar.h>
#include <qpushbutton.h>
#include <qlistwidget.h>
#include <qmessagebox.h>
#include <qaction.h>
#include <qmenu.h>
#include <qlayout.h>


AccountSettings::AccountSettings(AccountList *al, EmailClient *parent, const char *name, bool modal)
    : QDialog(parent), accountList(al)
{
    setObjectName( name );
    setModal( modal );
    setWindowTitle(tr("Account settings"));
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setContentsMargins(0, 0, 0, 0);
    accountListBox = new QListWidget(this);
    vb->addWidget(accountListBox);
    accountListBox->setFrameStyle(QFrame::NoFrame);
    context = QSoftMenuBar::menuFor(accountListBox);
    addAccountAction = new QAction( QIcon(":icon/new"), tr("Add account..."), this );
    connect(addAccountAction, SIGNAL(triggered()), this, SLOT(addAccount()));
    context->addAction( addAccountAction );
    removeAccountAction = new QAction( QIcon(":icon/trash"), tr("Remove account..."), this );
    connect(removeAccountAction, SIGNAL(triggered()), this, SLOT(removeAccount()));
    context->addAction(removeAccountAction);
    populateAccountList();

    statusDisplay = new StatusDisplay(this);
    statusDisplay->setVisible(false);
    vb->addWidget(statusDisplay);

    connect(accountListBox, SIGNAL(itemActivated(QListWidgetItem*)),
                                this, SLOT(accountSelected(QListWidgetItem*)) );
    connect(accountListBox, SIGNAL(currentRowChanged(int)),
            this, SLOT(accountHighlighted(int)));

    connect(parent, SIGNAL(updateProgress(uint, uint)), 
            this, SLOT(displayProgress(uint, uint)));
}

void AccountSettings::populateAccountList()
{
    static bool accountsExist = false;
    int oldSel = accountListBox->currentRow();
    int idx = -1;
    accountListBox->clear();
    listToAccountIdx.clear();
    QListIterator<QMailAccount*> it = accountList->accountIterator();
    while ( it.hasNext() ) {
        ++idx;
        QMailAccount *account = it.next();
        if( account->hasSettings() ) {
            if (account->accountType() == QMailAccount::SMS
                || account->accountType() == QMailAccount::System) {
                continue; // Don't show SMS or System accounts
            }
            listToAccountIdx[accountListBox->count()] = idx;
            accountListBox->addItem(account->accountName());
            accountsExist = true;
        }
    }
    oldSel = qMin(qMax(0, oldSel), accountListBox->count()-1);
    accountListBox->setCurrentRow(oldSel);
    accountHighlighted(oldSel);
//     if (!accountsExist)
//      addAccount();
//     accountsExist = true;
}

void AccountSettings::addAccount()
{
    QMailAccount *newAccount = new QMailAccount;
    newAccount->setUserName(accountList->getUserName());
    editAccount(newAccount, true);
}

void AccountSettings::editAccount()
{
    int idx = accountListBox->currentRow();
    editAccount(accountList->at( listToAccountIdx[idx] ));
}

void AccountSettings::removeAccount()
{
    int idx = listToAccountIdx[ accountListBox->currentRow() ];
    QMailAccount *account = accountList->at(idx);

    QString message = tr("<qt>Delete account: %1</qt>").arg(Qt::escape(account->accountName()));
    if (QMessageBox::warning( this, tr("Email"), message, tr("Yes"), tr("No"), 0, 0, 1 ) == 0) {
        // Display any progress signals from the parent
        statusDisplay->setVisible(true);
        emit deleteAccount(account);
        statusDisplay->setVisible(false);

        populateAccountList();
    }
}

void AccountSettings::accountSelected( QListWidgetItem *si )
{
    if( si ) {
        accountSelected( accountListBox->row( si ) );
    }
}

void AccountSettings::accountSelected(int idx)
{
    QMailAccount *account = accountList->at(listToAccountIdx[idx]);
    if (account->accountType() != QMailAccount::SMS)
        editAccount( account );
}

void AccountSettings::accountHighlighted(int idx)
{
    if ( idx == -1 ) {
        removeAccountAction->setVisible(false);
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
        return;
    }
    QMailAccount *account = accountList->at( listToAccountIdx[idx] );
    bool isSms = (account->accountType() == QMailAccount::SMS);
    bool isMms = (account->accountType() == QMailAccount::MMS);
    removeAccountAction->setVisible(!isSms && !isMms);
    QSoftMenuBar::setLabel(this, Qt::Key_Select, isSms ? QSoftMenuBar::NoLabel : QSoftMenuBar::Select);
}

void AccountSettings::editAccount(QMailAccount *account, bool newAccount)
{
    QDialog *editAccountView;
#ifndef QTOPIA_NO_MMS
    if (account->accountType() == QMailAccount::MMS) {
        MmsEditAccount *e = new MmsEditAccount(this);
        e->setModal(true);
        e->setAccount(account);
        editAccountView = e;
    } else
#endif
    {
        EditAccount *e = new EditAccount(this, "EditAccount");

        e->setModal(true);
        e->setAccount(account, newAccount);
        editAccountView = e;
    }

    int ret = QtopiaApplication::execDialog(editAccountView);
    delete editAccountView;

    if (ret == QDialog::Accepted) {
        if (newAccount) {
            accountList->append(account);
            qLog(Messaging) << "addedAccount";
        }

        if (account->defaultMailServer()) {
            if (accountList->defaultMailServerCount() > 1) {
                accountList->setDefaultMailServer(account);

                QMessageBox::warning(qApp->activeWindow(),
                    tr("New default account"),
                    tr("<qt>Your previous default mail account has been unchecked</qt>"),
                    tr("OK"));
            }
        }

        emit changedAccount(account);
        populateAccountList();
    } else {
        if (newAccount) { //don't touch existing account
            delete account;
        }
    }
}

void AccountSettings::displayProgress(uint value, uint range)
{
    if (statusDisplay->isVisible()) {
        statusDisplay->displayProgress(value, range);

        if (value == 0) 
            statusDisplay->displayStatus(tr("Deleting messages"));
    }
}

