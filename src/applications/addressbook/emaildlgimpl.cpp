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

#include "emaildlgimpl.h"
#include <qstringlist.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>

#include <QListWidget>

class SimpleEmailValidator : public QValidator
{
    public:
        SimpleEmailValidator( QWidget *parent )
            : QValidator(parent) {}
        ~SimpleEmailValidator() {}

        State validate( QString &str, int & ) const
        {
            QRegExp r = QRegExp("[,;']", Qt::CaseInsensitive, QRegExp::RegExp);
            if (str.contains(r))
                return Invalid;
            return Acceptable;
        }
};

EmailDialog::EmailDialog( QWidget *parent )
    : QDialog(parent)
{
    Ui::EmailDialogBase::setupUi(this);
    emailUpBtn->setIcon( QIcon( ":icon/up" ) );
    emailDownBtn->setIcon( QIcon( ":icon/down" ) );

    //  This really should be handled by the .ui file, but that
    //  functionality seems to be broken for now.
    connect(addEmailBtn, SIGNAL(clicked()), this, SLOT(addEmail()));
    connect(removeEmailBtn, SIGNAL(clicked()), this, SLOT(removeCurrent()));
    connect(emailUpBtn, SIGNAL(clicked()), this, SLOT(moveCurrentUp()));
    connect(emailDownBtn, SIGNAL(clicked()), this, SLOT(moveCurrentDown()));
    connect(emailEdit, SIGNAL(textChanged(const QString&)), this, SLOT(editCurrent(const QString&)));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(emailList, SIGNAL(currentTextChanged(const QString&)), emailEdit, SLOT(setText(const QString&)));

    emailEdit->setValidator(new SimpleEmailValidator(emailEdit));
    okButton->hide();
    cancelButton->hide();
}

EmailDialog::~EmailDialog()
{
}

void EmailDialog::setEmails(const QString &def, const QStringList &em)
{
    if (!def.isEmpty())
        emailList->addItem(def);

    QStringList::ConstIterator it;

    for (it = em.begin(); it != em.end(); ++it)
    {
        if (*it != def && !(*it).isEmpty())
            emailList->addItem(*it);
    }

    if(emailList->count())
    {
        emailList->setCurrentRow(0);
        emailEdit->setText(emailList->currentItem()->text());
    }
    else
        addEmail();
    updateButtons();
}

QString EmailDialog::defaultEmail() const
{
    if(emailList->count())
        return emailList->item(0)->text();
    else
        return QString("");
}

QStringList EmailDialog::emails() const
{
    QStringList em;

    for(int i =0; i < emailList->count(); i++)
        em.append(emailList->item(i)->text());

    return em;
}

void EmailDialog::editCurrent( const QString &s )
{
    if(emailList->currentItem() == NULL)
    {
        if( s == "" )
            return;

        addEmail(s);
    }

    emailEdit->setText(s);

    emailList->blockSignals(true);
    emailList->currentItem()->setText(s);
    emailList->blockSignals(false);

    updateButtons();
}

void EmailDialog::addEmail()
{
    // added to override addEmail in the parent class for the slot for the New button
    addEmail("user@domain");
}

void EmailDialog::addEmail( const QString &address )
{
    emailList->addItem(address);
    emailList->setCurrentRow(emailList->count() - 1);
    emailEdit->selectAll();
    emailEdit->setFocus();
    updateButtons();
}

void EmailDialog::removeCurrent()
{
    if (emailList->currentItem() != NULL) {
        emailEdit->setText("");
        delete emailList->currentItem();
        if(emailList->currentItem() != NULL)
            emailEdit->setText(emailList->currentItem()->text());
        updateButtons();
    }
}

void EmailDialog::moveCurrentUp()
{
    int cur = emailList->currentRow();
    if (cur > 0)
    {
        QListWidgetItem *curItem = emailList->item(cur);
        QListWidgetItem *prevItem = emailList->item(cur - 1);
        QString old = prevItem->text();

        emailEdit->blockSignals(true);

        prevItem->setText(curItem->text());
        curItem->setText(old);
        emailList->setCurrentRow(cur - 1);

        emailEdit->blockSignals(false);
        updateButtons();
    }
}

void EmailDialog::moveCurrentDown()
{
    int cur = emailList->currentRow();
    if(emailList->count() > 1 && cur < emailList->count() - 1)
    {
        QListWidgetItem *curItem = emailList->item(cur);
        QListWidgetItem *nextItem = emailList->item(cur + 1);
        QString old = nextItem->text();

        emailEdit->blockSignals(true);

        nextItem->setText(curItem->text());
        curItem->setText(old);
        emailList->setCurrentRow(cur + 1);

        emailEdit->blockSignals(false);
        updateButtons();
    }
}

void EmailDialog::updateButtons(void)
{
    if(emailList->count() == 0)
    {
        removeEmailBtn->setEnabled(false);
        emailUpBtn->setEnabled(false);
        emailDownBtn->setEnabled(false);
    }
    else
    {
        int row = emailList->currentRow();

        removeEmailBtn->setEnabled(true);
        emailUpBtn->setEnabled(row > 0);
        emailDownBtn->setEnabled(row > -1 && row < emailList->count() - 1);
    }
}
