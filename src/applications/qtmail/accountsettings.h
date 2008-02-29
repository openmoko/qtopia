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

#include <qdialog.h>
#include <qmap.h>

class AccountList;
class MailAccount;
class QMenu;
class QListWidget;
class QListWidgetItem;
class QAction;

class AccountSettings : public QDialog
{
    Q_OBJECT
public:
    AccountSettings(AccountList *al, QWidget *parent=0, const char *name=0, bool modal=true);

signals:
    void changedAccount(MailAccount *account);
    void deleteAccount(MailAccount *account);

public slots:
    void addAccount();
private slots:
    void editAccount();
    void removeAccount();
    void accountSelected(QListWidgetItem*);
    void accountSelected(int idx);
    void accountHighlighted(int idx);

private:
    void populateAccountList();
    void editAccount(MailAccount *account, bool newAccount=false);

private:
    AccountList *accountList;
    QMap<int,int> listToAccountIdx;
    QListWidget *accountListBox;
    QMenu *context;
    QAction *addAccountAction;
    QAction *removeAccountAction;
};

