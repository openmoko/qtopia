/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
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
#include <qtoolbutton.h>
#include <qtopia/resource.h>

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
    emailUpBtn->setPixmap( Resource::loadIconSet( "up" ).pixmap( QIconSet::Small, TRUE ) );
    emailDownBtn->setPixmap( Resource::loadIconSet( "down" ).pixmap( QIconSet::Small, TRUE ) );

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
    if ( emailList->count() )
	emailEdit->setText(emailList->currentText());
    else
	addEmail();
    updateButtons();
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
    if (emailList->currentItem() == -1)
    {
        if ( s == "" )
            return;
            
        addEmail((const char*)s);
    }
    
    emailEdit->setText(s);
    
    emailList->blockSignals(TRUE);
    emailList->changeItem(s, emailList->currentItem());
    emailList->blockSignals(FALSE);

    updateButtons();
}

void EmailDialog::addEmail()
{
    // added to override addEmail in the parent class for the slot for the New button
    addEmail("user@domain");
}

void EmailDialog::addEmail( const char* address )
{
    emailList->insertItem(address);
    emailList->setCurrentItem(emailList->numRows() - 1);
    emailEdit->selectAll();
    emailEdit->setFocus();
    updateButtons();
}

void EmailDialog::removeCurrent()
{
    if (emailList->currentItem() != -1) {
        emailEdit->setText("");
	emailList->removeItem(emailList->currentItem());
	updateButtons();
    }
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
	updateButtons();
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
	updateButtons();
    }
}

void
EmailDialog::updateButtons(void)
{
    if (emailList->count() == 0) {
	removeEmailBtn->setEnabled(FALSE);
	emailUpBtn->setEnabled(FALSE);
	emailDownBtn->setEnabled(FALSE);
    } else {
	removeEmailBtn->setEnabled(TRUE);

	emailUpBtn->setEnabled(
	    emailList->currentItem() != emailList->topItem());

	emailDownBtn->setEnabled(
	    emailList->currentItem() < emailList->numRows() - 1);
    }
}
