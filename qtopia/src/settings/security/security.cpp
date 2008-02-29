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
#include "security.h"

#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qtopia/password.h>
#include <qtopia/qpedialog.h>
#include <qtopia/qcopenvelope_qws.h>
#ifdef QTOPIA_PHONE
#include <qtopia/contextbar.h>
#include <qtopia/private/contextkeymanager_p.h>
#include <qtopia/contextmenu.h>
#include <qtopia/global.h>
#include "phonesecurity.h"
#endif

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qhostaddress.h>
#include <qtabwidget.h>


class WaitScreen : public QLabel
{
    Q_OBJECT
public:
    WaitScreen(QWidget *parent, char *name = 0) : QLabel(parent, name)
    {
	setText(tr("Please wait ..."));
#ifdef QTOPIA_PHONE
	ContextBar::clearLabel(this, Key_Back);
#endif
    }

public slots:
    void makeClosable()
    {
	canClose = TRUE;
	close();
    }
protected:
    void show() 
    {
	canClose = FALSE;
	// should show error if it timesout.
	QTimer::singleShot(5000, this, SLOT(makeClosable()));
	QLabel::show();
    }

    void closeEvent(QCloseEvent *e){
	// great way to appear to freeze device
	if (canClose)
	    e->accept();
	else
	    e->ignore();
    }

    void keyPressEvent(QKeyEvent *e)
    {
	// great way to appear to freeze device
	e->accept();
    }

private:
    bool canClose;

};

Security::Security( QWidget* parent,  const char* name, WFlags fl )
#ifdef QTOPIA_PHONE
    : SecurityBase( parent, name, fl ), mStatus(0)
#else
    : SecurityBase( parent, name, TRUE, fl )
#endif
{
    valid=FALSE;
    Config cfg("Security");
    cfg.setGroup("Passcode");
    passcode = cfg.readEntry("passcode");
#ifdef QTOPIA_PHONE
    if (Global::mousePreferred()) {
        tabWidget->setCurrentPage(0);
        tabWidget->removePage(tabWidget->currentPage());
    }
    phoneCode = cfg.readEntry("PhoneCode");
    secureKeyLock->setChecked(cfg.readBoolEntry("SecureKeyLock", FALSE));
    connect(secureKeyLock, SIGNAL(stateChanged(int)),
            this, SLOT(secureLockChanged(int)));
    connect(passcode_poweron, SIGNAL(toggled(bool)), SLOT(markProtected(bool)));
#else
    passcode_poweron->setChecked(cfg.readBoolEntry("passcode_poweron",FALSE));
#endif
    cfg.setGroup("Sync");
    QHostAddress allowed;
    allowed.setAddress(cfg.readEntry("auth_peer","192.168.0.0"));
    uint auth_peer = allowed.ip4Addr();
    int auth_peer_bits = cfg.readNumEntry("auth_peer_bits",16);
    selectNet(auth_peer,auth_peer_bits);
    connect(syncnet, SIGNAL(textChanged(const QString&)),
	    this, SLOT(setSyncNet(const QString&)));
#ifdef QTOPIA_PHONE
    QPEApplication::setInputMethodHint(syncnet,QPEApplication::Named,"netmask");
    phonesec = new PhoneSecurity(this);
    connect(phonesec,SIGNAL(changed(bool)),this,SLOT(phoneChanged(bool)));
    connect(phonesec,SIGNAL(locked(bool)),this,SLOT(phoneLocked(bool)));
    connect(phonesec,SIGNAL(lockDone(bool)),this,SLOT(phoneLockDone(bool)));

    connect(changeCode, SIGNAL(pressed()), this, SLOT(changePhoneCode()));

    new ContextMenu(this);
#endif

    /*
    cfg.setGroup("Remote");
    if ( telnetAvailable() )
	telnet->setChecked(cfg.readEntry("allow_telnet"));
    else
	telnet->hide();

    if ( sshAvailable() )
	ssh->setChecked(cfg.readEntry("allow_ssh"));
    else
	ssh->hide();
    */

    connect(changepasscode,SIGNAL(clicked()), this, SLOT(changePassCode()));
    connect(clearpasscode,SIGNAL(clicked()), this, SLOT(clearPassCode()));
#ifdef QTOPIA_PHONE
    connect(pinSelection, SIGNAL(activated(int)), this, SLOT(updateGUI()));
#else
    dl = new QPEDialogListener(this);
#endif
    updateGUI();
}

Security::~Security()
{
}

void Security::phoneChanged(bool success)
{
#ifdef QTOPIA_PHONE
    mStatus->makeClosable();
    if (success)
	QMessageBox::information(this, tr("Success"), tr("<p>Successfully changed PIN."));
    else
	QMessageBox::warning(this, tr("Failure"), tr("<p>Could not change PIN."));
#else
    Q_UNUSED(success);
#endif
}

void Security::phoneLocked(bool success)
{
#ifdef QTOPIA_PHONE
    if (!success)
	QMessageBox::warning(this, tr("Failure"), tr("<p>Could not change protection state."));
#else
    Q_UNUSED(success);
#endif
}

void Security::phoneLockDone(bool success)
{
    passcode_poweron->blockSignals(TRUE);
    passcode_poweron->setChecked(success);
    passcode_poweron->blockSignals(FALSE);
    passcode_poweron->setEnabled(TRUE);
}

void Security::updateGUI()
{
#ifdef QTOPIA_PHONE
    // phone passcode.  they are never 'cleared'
    clearpasscode->hide();
    passcode_poweron->setEnabled(FALSE);
    phonesec->setLockType(pinSelection->currentItem());
#else
    bool empty = passcode.isEmpty();

    changepasscode->setText( empty ? tr("Set code" ) 
			     : tr("Change code" )  );
    passcode_poweron->setEnabled( !empty );
    clearpasscode->setEnabled( !empty );
#endif
}

void Security::markProtected(bool b)
{
#ifdef QTOPIA_PHONE
    ContextKeyManager::instance()->setClassStandardLabel("PasswordDialog",
	    Key_Back, ContextBar::Back, ContextBar::ModalAndNonModal);
    passcode_poweron->setEnabled(FALSE);
    QString p;
    if (pinSelection->currentItem() == 0) {
	p = enterPassCode(tr("Enter SIM PIN"), FALSE);
    } else {
	p = enterPassCode(tr("Enter Phone PIN"), FALSE);
    }
    if (!p.isNull())
	phonesec->markProtected(pinSelection->currentItem(), b, p);
#else
    Q_UNUSED(b);
#endif
}

void Security::show()
{
    valid=FALSE;
    setEnabled(FALSE);
    SecurityBase::show();
#ifndef QTOPIA_PHONE
    if ( passcode.isEmpty() ) {
	// could insist...
	//changePassCode();
	//if ( passcode.isEmpty() )
	    //reject();
    } else if ( timeout.isNull() || timeout.elapsed() > 2000 ) {
	// Insist on re-entry of passcode if more than 2 seconds have elapsed.
	/*
	ContextKeyManager::instance()->setClassStandardLabel("PasswordDialog",
		Key_Back, ContextBar::Next, ContextBar::ModalAndNonModal);
	*/
	QString pc = enterPassCode(tr("Enter Security passcode"));
	if ( pc != passcode ) {
	    QMessageBox::critical(this, tr("Passcode incorrect"), 
		    tr("<qt>The passcode entered is incorrect. Access denied</qt>"));
//	    reject();
	    qApp->quit();
	    return;
	}
	timeout.start();
    }
#endif
    setEnabled(TRUE);
    valid=TRUE;
}

#ifdef QTOPIA_PHONE
bool Security::close(bool b)
{
    applySecurity();
    return QWidget::close(b);
}
#else
void Security::accept()
{
    applySecurity();
    QDialog::accept();
}

void Security::done(int r)
{
    QDialog::done(r);
    close();
}
#endif

void Security::selectNet(int auth_peer,int auth_peer_bits)
{
    QString sn;
    if ( auth_peer_bits == 0 && auth_peer == 0 ) {
	sn = tr("Any");
    } else if ( auth_peer_bits == 32 && auth_peer == 0 ) {
	sn = tr("None");
    } else {
	sn =
	    QString::number((auth_peer>>24)&0xff) + "."
	    + QString::number((auth_peer>>16)&0xff) + "."
	    + QString::number((auth_peer>>8)&0xff) + "."
	    + QString::number((auth_peer>>0)&0xff) + "/"
	    + QString::number(auth_peer_bits);
    }
    for (int i=0; i<syncnet->count(); i++) {
	if ( sn == syncnet->text(i) || syncnet->text(i).left(sn.length()+1) == sn+" " )
	{
	    syncnet->setCurrentItem(i);
	    return;
	}
    }
    syncnet->insertItem(sn);
    syncnet->setCurrentItem(syncnet->count()-1);
}

bool Security::parseNet(const QString& sn,int& auth_peer,int& auth_peer_bits)
{
    auth_peer=0;
    if ( sn == tr("Any") ) {
	auth_peer = 0;
	auth_peer_bits = 0;
	return TRUE;
    } else if ( sn == tr("None") ) {
	auth_peer = 0;
	auth_peer_bits = 32;
	return TRUE;
    } else {
	QString a;
	int sl = sn.find('/');
	if ( sl < 0 ) {
	    auth_peer_bits = 32;
	    a = sn;
	} else {
	    int n=(uint)sn.find(' ',sl);
	    if ( n>sl ) n-=sl;
	    auth_peer_bits = sn.mid(sl+1,(uint)n).toInt();
	    a = sn.left(sl);
	}
	QStringList b=QStringList::split(QChar('.'),a);
	if ( b.count() != 4 )
	    return FALSE;
	int x=1<<24;
	auth_peer = 0;
	for (QStringList::ConstIterator it=b.begin(); it!=b.end(); ++it) {
	    auth_peer += (*it).toInt()*x;
	    x >>= 8;
	}
	return x!=0 && auth_peer_bits && auth_peer;
    }
}

void Security::setSyncNet(const QString& sn)
{
    int auth_peer,auth_peer_bits;
    if ( parseNet(sn,auth_peer,auth_peer_bits) )
	selectNet(auth_peer,auth_peer_bits);
}

void Security::applySecurity()
{
    if ( valid ) {
	Config cfg("Security");
	cfg.setGroup("Passcode");
	cfg.writeEntry("passcode",passcode);
#ifndef QTOPIA_PHONE
	cfg.writeEntry("passcode_poweron",passcode_poweron->isChecked());
#else
	cfg.writeEntry("PhoneCode",phoneCode);
        cfg.writeEntry("SecureKeyLock", secureKeyLock->isChecked());
#endif
	cfg.setGroup("Sync");
	int auth_peer=0;
	int auth_peer_bits;
	QString sn = syncnet->currentText();
	parseNet(sn,auth_peer,auth_peer_bits);
	
	QHostAddress allowed((Q_UINT32)auth_peer);
	cfg.writeEntry("auth_peer",allowed.toString());
	cfg.writeEntry("auth_peer_bits",auth_peer_bits);
	
	/*
	cfg.setGroup("Remote");
	if ( telnetAvailable() )
	    cfg.writeEntry("allow_telnet",telnet->isChecked());
	if ( sshAvailable() )
	    cfg.writeEntry("allow_ssh",ssh->isChecked());
	// ### write ssh/telnet sys config files
	*/
    }

    QCopEnvelope("QPE/System", "securityChanged()");
}

// for the phone, it requires: once to check old, (even though in
// security app already), and
// twice for new.  Once it has all three, you can attempt to set the sim pin,
// or security pin as required.
void Security::changePassCode()
{
#ifdef QTOPIA_PHONE
    QString old;
#endif
    QString new1;
    QString new2;
    bool    mismatch = FALSE;
    bool    valid = TRUE;

    do {
#ifdef QTOPIA_PHONE
	ContextKeyManager::instance()->setClassStandardLabel("PasswordDialog",
		Key_Back, ContextBar::Next, ContextBar::ModalAndNonModal);
	// should check current pin first, may not be set.
	if (old.isNull()) {
	    old = enterPassCode(tr("Enter current PIN"), FALSE);
	    // indicates dialog was canceled.
	    if ( old.isNull() )
		return;
	}
	if (mismatch) {
	    new1 = enterPassCode(tr("Mismatch: Retry new PIN"), FALSE);
	} else if (!valid) {
	    new1 = enterPassCode(tr("Invalid: Retry new PIN"), FALSE);
	} else {
	    new1 = enterPassCode(tr("Enter new passcode"), FALSE);
	}
#else
	if (mismatch) {
	    new1 = enterPassCode(tr("Mismatch: Retry new code"), TRUE);
	} else if (!valid) {
	    new1 = enterPassCode(tr("Invalid: Retry new code"), TRUE);
	} else {
	    new1 = enterPassCode(tr("Enter new passcode"), TRUE);
	}
#endif
	// indicates dialog was canceled.
	if ( new1.isNull() )
	    return;

	if (new1.isEmpty()) {
	    valid = FALSE;
	} else {
#ifdef QTOPIA_PHONE
	    ContextKeyManager::instance()->setClassStandardLabel("PasswordDialog",
		    Key_Back, ContextBar::Back, ContextBar::ModalAndNonModal);
	    new2 = enterPassCode(tr("Re-enter new PIN"), FALSE);
#else
	    new2 = enterPassCode(tr("Re-enter new passcode"), TRUE);
#endif
	    if ( new2.isNull() )
		return;

	    valid = !new2.isEmpty();
	    mismatch = new1 != new2;
	}

    } while (mismatch || !valid);

#ifdef QTOPIA_PHONE
    phonesec->changePassword(pinSelection->currentItem(),old,new2);
	
    if (!mStatus)
	mStatus = new WaitScreen(0);
    mStatus->showMaximized();
#else
    passcode = new1;
    updateGUI();
#endif
}

void Security::clearPassCode()
{
    passcode = QString::null;
    updateGUI();
}


QString Security::enterPassCode(const QString& prompt, bool encrypt)
{
    // magic to stop it crypting.
    if (encrypt)
	return Password::getPassword(prompt);
    else
	return Password::getPassword("@:" + prompt); // Magic, No tr.
}

bool Security::telnetAvailable() const
{
    // ### not implemented
    return FALSE;
}

bool Security::sshAvailable() const
{
    // ### not implemented
    return FALSE;
}

void Security::changePhoneCode()
{
#ifdef QTOPIA_PHONE
    QString old;
    QString new1;
    QString new2;
    bool    mismatch = FALSE;
    bool    valid = TRUE;

    ContextKeyManager::instance()->setClassStandardLabel("PasswordDialog",
            Key_Back, ContextBar::Next, ContextBar::ModalAndNonModal);

    do {
        if (old.isNull())
            old = enterPassCode(tr("Enter current code"), TRUE);
        else
            old = enterPassCode(tr("Invalid: Enter current code"), TRUE);

        // indicates dialog was canceled.
        if (old.isNull())
            return;
    } while (phoneCode != old);

    do {
	ContextKeyManager::instance()->setClassStandardLabel("PasswordDialog",
		Key_Back, ContextBar::Next, ContextBar::ModalAndNonModal);
	if (mismatch) {
	    new1 = enterPassCode(tr("Mismatch: Retry new code"), TRUE);
	} else if (!valid) {
	    new1 = enterPassCode(tr("Invalid: Retry new code"), TRUE);
	} else {
	    new1 = enterPassCode(tr("Enter new code"), TRUE);
	}
	// indicates dialog was canceled.
	if ( new1.isNull() )
	    return;

	if (new1.isEmpty()) {
	    valid = FALSE;
	} else {
	    ContextKeyManager::instance()->setClassStandardLabel("PasswordDialog",
		    Key_Back, ContextBar::Back, ContextBar::ModalAndNonModal);
	    new2 = enterPassCode(tr("Re-enter new code"), TRUE);
	    if (new2.isNull())
		return;

	    valid = !new2.isEmpty();
	    mismatch = new1 != new2;
	}
    } while (mismatch || !valid);

    phoneCode = new1;
#endif
}

void Security::secureLockChanged(int)
{
#ifdef QTOPIA_PHONE
    QString code;
    do {
        QString msg;
        if (code.isNull())
            msg = tr("Enter phone code");
        else
            msg = tr("Invalid: Enter phone code");
        code = enterPassCode(msg, TRUE);
        if (code.isNull()) {
            // Cancelled
            secureKeyLock->blockSignals(TRUE);
            secureKeyLock->setChecked(!secureKeyLock->isChecked());
            secureKeyLock->blockSignals(FALSE);
            return;
        }
    } while (code != phoneCode);
#endif
}

#include "security.moc"
