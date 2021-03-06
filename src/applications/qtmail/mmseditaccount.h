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

#ifndef MMSEDITACCOUNT_H
#define MMSEDITACCOUNT_H

#include "ui_mmseditaccountbase.h"
#include <QDialog>

class QMailAccount;

class MmsEditAccount : public QDialog, Ui_MmsEditAccountBase
{
    Q_OBJECT
public:
    MmsEditAccount(QWidget *parent=0);

    void setAccount(QMailAccount *in);

protected slots:
    void accept();
    void configureNetworks();
    void updateNetwork();

private:
    void populateNetwork();

private:
    QMailAccount *account;
};

#endif
