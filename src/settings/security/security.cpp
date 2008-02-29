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
#include "security.h"

#include <qtopia/config.h>
#include <qtopia/password.h>
#include <qtopia/qpedialog.h>

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qhostaddress.h>

Security::Security( QWidget* parent,  const char* name, WFlags fl )
    : SecurityBase( parent, name, TRUE, fl )
{
    valid=FALSE;
    Config cfg("Security");
    cfg.setGroup("Passcode");
    passcode = cfg.readEntry("passcode");
    passcode_poweron->setChecked(cfg.readBoolEntry("passcode_poweron",FALSE));
    cfg.setGroup("Sync");
    QHostAddress allowed;
    allowed.setAddress(cfg.readEntry("auth_peer","192.168.0.0"));
    uint auth_peer = allowed.ip4Addr();
    int auth_peer_bits = cfg.readNumEntry("auth_peer_bits",16);
    selectNet(auth_peer,auth_peer_bits);
    connect(syncnet, SIGNAL(textChanged(const QString&)),
	    this, SLOT(setSyncNet(const QString&)));

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
    updateGUI();

    dl = new QPEDialogListener(this);
}

Security::~Security()
{
}


void Security::updateGUI()
{
    bool empty = passcode.isEmpty();

    changepasscode->setText( empty ? tr("Set code" ) 
			     : tr("Change code" )  );
    passcode_poweron->setEnabled( !empty );
    clearpasscode->setEnabled( !empty );
}


void Security::show()
{
    valid=FALSE;
    setEnabled(FALSE);
    SecurityBase::show();
    if ( passcode.isEmpty() ) {
	// could insist...
	//changePassCode();
	//if ( passcode.isEmpty() )
	    //reject();
    } else if ( timeout.isNull() || timeout.elapsed() > 2000 ) {
	// Insist on re-entry of passcode if more than 2 seconds have elapsed.
	QString pc = enterPassCode(tr("Enter passcode"));
	if ( pc != passcode ) {
	    QMessageBox::critical(this, tr("Passcode incorrect"), 
		    tr("The passcode entered is incorrect.\nAccess denied"));
	    reject();
	    return;
	}
	timeout.start();
    }
    setEnabled(TRUE);
    valid=TRUE;
}

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
	cfg.writeEntry("passcode_poweron",passcode_poweron->isChecked());
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
}

void Security::changePassCode()
{
    QString new1;
    QString new2;
    bool    mismatch = FALSE;
    bool    valid = TRUE;

    do {
	if (mismatch) {
	    new1 = enterPassCode(tr("Mismatch: Retry new code"));
	} else if (!valid) {
	    new1 = enterPassCode(tr("Invalid: Retry new code"));
	} else {
	    new1 = enterPassCode(tr("Enter new passcode"));
	}
	if ( new1.isNull() )
	    return;
	if (new1.isEmpty()) {
	    valid = FALSE;
	} else {
	    new2 = enterPassCode(tr("Re-enter new passcode"));
	    if ( new2.isNull() )
		return;

	    valid = !new2.isEmpty();
	    mismatch = new1 != new2;
	}

    } while (mismatch || !valid);

    passcode = new1;
    updateGUI();
}

void Security::clearPassCode()
{
    passcode = QString::null;
    updateGUI();
}


QString Security::enterPassCode(const QString& prompt)
{
    return Password::getPassword(prompt);
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
