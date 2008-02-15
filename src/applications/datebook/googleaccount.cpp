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

#include "googleaccount.h"
#include <QtopiaApplication>
#ifndef QT_NO_OPENSSL

#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>

GoogleAccount::GoogleAccount( QWidget *parent )
    : QDialog( parent )
{
    QFormLayout *fl = new QFormLayout();

    emailText = new QLineEdit();

    nameText = new QLineEdit();

    passwordText = new QLineEdit();
    passwordText->setEchoMode(QLineEdit::PasswordEchoOnEdit);

    nameLabel = new QLabel();
    nameLabel->setText(tr("Name"));
    nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    nameLabel->setBuddy(nameText);

    accessCombo = new QComboBox();
    accessCombo->addItems(QStringList() <<
            tr("Private Full") <<
            tr("Public Full") <<
            tr("Public Free/Busy"));

    fl->addRow(nameLabel, nameText);
    fl->addRow(tr("Email"), emailText);
    fl->addRow(tr("Password"), passwordText);
    fl->addRow(tr("Access:", "Type of access to google account"), accessCombo);

    // Hmm
    fl->addItem(new QSpacerItem(225, 141, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setWindowTitle(tr("Google Account"));

    setLayout(fl);
    QtopiaApplication::setInputMethodHint(emailText, "email");
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

QString GoogleAccount::name() const
{
    return nameText->text();
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
#endif
