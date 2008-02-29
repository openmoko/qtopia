/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "emaildlgimpl.h"
#include <qstringlist.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpushbutton.h>

class SimpleEmailValidator : public QValidator
{
    public:
	SimpleEmailValidator( QWidget *parent, const char *name = 0)
	    : QValidator(parent, name) {}
	~SimpleEmailValidator() {}

	State validate( QString &str, int & ) const 
	{
	    QRegExp r = QRegExp("[,;']", TRUE, FALSE);
	    if (str.contains(r))
		return Invalid;
	    return Acceptable;
	}
};

EmailDialog::EmailDialog( QWidget *parent, const char *name, bool modal, WFlags fl)
    : EmailDialogBase(parent, name, modal, fl)
{
    emailEdit->setValidator(new SimpleEmailValidator(emailEdit));
#ifndef QTOPIA_DESKTOP
    okButton->hide();
    cancelButton->hide();
#endif
}

EmailDialog::~EmailDialog()
{
}

void EmailDialog::setEmails(const QString &def, const QStringList &em)
{
    if (!def.isEmpty())
	emailList->insertItem(def);
    QStringList::ConstIterator it;
    for (it = em.begin(); it != em.end(); ++it) {
	if (*it != def && !(*it).isEmpty()) 
	    emailList->insertItem(*it);
    }
    emailEdit->setText(emailList->currentText());
}

QString EmailDialog::defaultEmail() const
{
    return emailList->text(0);
}

QStringList EmailDialog::emails() const
{
    QStringList em;

    for (int i =0; i < emailList->numRows(); i++) {
	em.append(emailList->text(i));
    }
    return em;
}

void EmailDialog::editCurrent( const QString &s )
{
    if (emailList->currentItem() != -1) {
	emailList->blockSignals(TRUE);
	emailList->changeItem(s, emailList->currentItem());
	emailList->blockSignals(FALSE);
    }
}

void EmailDialog::addEmail( )
{
    emailList->insertItem("user@domain");
    emailList->setCurrentItem(emailList->numRows() - 1);
    emailEdit->selectAll();
    emailEdit->setFocus();
}

void EmailDialog::removeCurrent()
{
    if (emailList->currentItem() != -1)
	emailList->removeItem(emailList->currentItem());
}

void EmailDialog::moveCurrentUp()
{
    int cur = emailList->currentItem();
    if (cur > 0) {
	emailEdit->blockSignals(TRUE);
	QString old = emailList->text(cur - 1);
	emailList->changeItem(emailList->currentText(), cur - 1);
	emailList->changeItem(old, cur);
	emailList->setCurrentItem(cur - 1);
	emailEdit->blockSignals(FALSE);
    }
}

void EmailDialog::moveCurrentDown()
{
    int cur = emailList->currentItem();
    if (emailList->numRows() > 1 && cur < emailList->numRows() - 1) {
	emailEdit->blockSignals(TRUE);
	QString old = emailList->text(cur + 1);
	emailList->changeItem(emailList->currentText(), cur + 1);
	emailList->changeItem(old, cur);
	emailList->setCurrentItem(cur + 1);
	emailEdit->blockSignals(FALSE);
    }
}
