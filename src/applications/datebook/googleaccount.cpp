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

#include "googleaccount.h"

GoogleAccount::GoogleAccount( QWidget *parent )
    : QDialog( parent )
{
    setupUi(this);
    nameLabel->hide();
    nameText->hide();
}

GoogleAccount::~GoogleAccount() {}

QString GoogleAccount::email() const
{
    return emailText->text();
}

QString GoogleAccount::password() const
{
    return passwordText->text();
}

QGoogleCalendarContext::FeedType GoogleAccount::feedType() const
{
    return (QGoogleCalendarContext::FeedType)accessCombo->currentIndex();
}

void GoogleAccount::setEmail(const QString &email)
{
    emailText->setText(email);
}

void GoogleAccount::setPassword(const QString &password)
{
    passwordText->setText(password);
}

void GoogleAccount::setName(const QString &name)
{
    nameText->setText(name);
    if (name.isEmpty()) {
        nameText->hide();
        nameLabel->hide();
    } else {
        nameText->show();
        nameLabel->show();
    }
}

void GoogleAccount::setFeedType(QGoogleCalendarContext::FeedType type)
{
    accessCombo->setCurrentIndex(int(type));
}

/* validate the settings - need email and perhaps password */
void GoogleAccount::accept()
{
    if (email().isEmpty()) {
        if (QMessageBox::warning(this, tr("Google Account"),
                tr("<qt>An email address is required.  Cancel editing?</qt>"),
                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
            reject();
            return;
        } else {
            nameText->setFocus();
            return;
        }
    }

    if (feedType() == QGoogleCalendarContext::FullPrivate
            && password().isEmpty()) {
        if (QMessageBox::warning(this, tr("Google Account"),
                tr("<qt>A password is required for this account access type.  Cancel editing?</qt>"),
                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
            reject();
            return;
        } else {
            passwordText->setFocus();
            return;
        }
    }

    QDialog::accept();
}
