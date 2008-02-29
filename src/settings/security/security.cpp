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
#include "security.h"

#include <qtopiaapplication.h>
#include <qpassworddialog.h>
#include <qtopiaipcenvelope.h>
#ifdef QTOPIA_CELL
#include <qsoftmenubar.h>
#include "phonesecurity.h"
#endif

#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <QTimer>
#include <QHostAddress>
#include <QCloseEvent>
#include <QSettings>
#include <QDebug>


class WaitScreen : public QLabel
{
    Q_OBJECT
public:
    WaitScreen( QWidget *parent ) : QLabel( parent )
    {
        setText(tr("Please wait ..."));
#ifdef QTOPIA_CELL
        QSoftMenuBar::clearLabel(this, Qt::Key_Back);
#endif
    }

public slots:
    void makeClosable()
    {
        canClose = true;
        close();
    }
protected:
    void show()
    {
        canClose = false;
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

Security::Security( QWidget* parent, Qt::WFlags fl )
#ifdef QTOPIA_CELL
    : QDialog( parent, fl ), mStatus(0)
{
#else
    : QDialog( parent, fl )
{
    setModal( true );
#endif
    setupUi( this );
    valid=false;
    QSettings cfg("Trolltech","Security");
    cfg.beginGroup("Passcode");
    passcode = cfg.value("passcode").toString();
#ifdef QTOPIA_CELL
    connect(passcode_poweron, SIGNAL(toggled(bool)), SLOT(markProtected(bool)));
#else
    passcode_poweron->setChecked(cfg.value("passcode_poweron",false).toBool());
#endif
    cfg.endGroup();
    cfg.beginGroup("Sync");
    QHostAddress allowed;
    allowed.setAddress(cfg.value("auth_peer","192.168.0.0").toString());
    uint auth_peer = allowed.toIPv4Address();
    int auth_peer_bits = cfg.value("auth_peer_bits",16).toInt();
    selectNet(auth_peer,auth_peer_bits);
    connect(syncnet, SIGNAL(editTextChanged(const QString&)),
            this, SLOT(setSyncNet(const QString&)));
#ifdef QTOPIA_CELL
    QtopiaApplication::setInputMethodHint(syncnet,QtopiaApplication::Named,"netmask");
    phonesec = new PhoneSecurity(this);
    connect(phonesec,SIGNAL(changed(bool)),this,SLOT(phoneChanged(bool)));
    connect(phonesec,SIGNAL(locked(bool)),this,SLOT(phoneLocked(bool)));
    connect(phonesec,SIGNAL(lockDone(bool)),this,SLOT(phoneLockDone(bool)));

    (void)QSoftMenuBar::menuFor(this); // just to get help
#endif

    /*
    { cfg.endGroup(); cfg.beginGroup("Remote"); };
    if ( telnetAvailable() )
        telnet->setChecked(cfg.value("allow_telnet").toString());
    else
        telnet->hide();

    if ( sshAvailable() )
        ssh->setChecked(cfg.value("allow_ssh").toString());
    else
        ssh->hide();
    */

    connect(changepasscode,SIGNAL(clicked()), this, SLOT(changePassCode()));
    connect(clearpasscode,SIGNAL(clicked()), this, SLOT(clearPassCode()));

#ifndef QTOPIA_DESKTOP
#ifdef QTOPIA_CELL
    connect(pinSelection, SIGNAL(activated(int)), this, SLOT(updateGUI()));
    // XXX hide sync tab until Qtopia desktop is ported
    TabWidget2->removeTab( 1 );
#else
    // Hide sync tab until Qtopia desktop is ported.
    GroupBox1->hide();
#endif  // QTOPIA_CELL
#endif // QTOPIA_DESKTOP

    updateGUI();
}

Security::~Security()
{
}

void Security::phoneChanged(bool success)
{
#ifdef QTOPIA_CELL
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
#ifdef QTOPIA_CELL
    if (!success)
        QMessageBox::warning(this, tr("Failure"), tr("<p>Could not change protection state."));
#else
    Q_UNUSED(success);
#endif
}

void Security::phoneLockDone(bool success)
{
    passcode_poweron->blockSignals(true);
    passcode_poweron->setChecked(success);
    passcode_poweron->blockSignals(false);
    passcode_poweron->setEnabled(true);
}

void Security::updateGUI()
{
#ifdef QTOPIA_CELL
    // phone passcode.  they are never 'cleared'
    clearpasscode->hide();
    passcode_poweron->setEnabled(false);
    phonesec->setLockType(pinSelection->currentIndex());
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
#ifdef QTOPIA_CELL
    passcode_poweron->setEnabled(false);
    QString p;
    if (pinSelection->currentIndex() == 0) {
        p = enterPassCode(tr("Enter SIM PIN"), false);
    } else {
        p = enterPassCode(tr("Enter Phone PIN"), false);
    }
    if (!p.isNull()) {
        phonesec->markProtected(pinSelection->currentIndex(), b, p);
    } else {
        passcode_poweron->setEnabled( true );
        passcode_poweron->setChecked( !b );
    }

#else
    Q_UNUSED(b);
#endif
}

void Security::setVisible(bool vis)
{
#ifndef QTOPIA_CELL
    if (vis) {
        valid=false;
        setEnabled(false);
        QDialog::setVisible(vis);
        if ( passcode.isEmpty() ) {
            // could insist...
            //changePassCode();
            //if ( passcode.isEmpty() )
            //reject();
        } else if ( timeout.isNull() || timeout.elapsed() > 2000 ) {
            // Insist on re-entry of passcode if more than 2 seconds have elapsed.
            QString pc = enterPassCode(tr("Enter Security passcode"));
            if ( pc != passcode ) {
                QMessageBox::critical(this, tr("Passcode incorrect"),
                        tr("<qt>The passcode entered is incorrect. Access denied</qt>"));
                //          reject();
                qApp->quit();
                return;
            }
            timeout.start();
        }
        setEnabled(true);
    } else {
        QDialog::setVisible(vis);
    }
#else
    QWidget::setVisible(vis);
#endif
    valid=true;
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
        if ( sn == syncnet->itemText(i) || syncnet->itemText(i).left(sn.length()+1) == sn+" " )
        {
            syncnet->setCurrentIndex(i);
            return;
        }
    }
    syncnet->addItem(sn);
    syncnet->setCurrentIndex(syncnet->count()-1);
}

bool Security::parseNet(const QString& sn,int& auth_peer,int& auth_peer_bits)
{
    auth_peer=0;
    if ( sn == tr("Any") ) {
        auth_peer = 0;
        auth_peer_bits = 0;
        return true;
    } else if ( sn == tr("None") ) {
        auth_peer = 0;
        auth_peer_bits = 32;
        return true;
    } else {
        QString a;
        int sl = sn.indexOf('/');
        if ( sl < 0 ) {
            auth_peer_bits = 32;
            a = sn;
        } else {
            int n=(uint)sn.indexOf(' ',sl);
            if ( n>sl ) n-=sl;
            auth_peer_bits = sn.mid(sl+1,(uint)n).toInt();
            a = sn.left(sl);
        }
        QStringList b= a.split( QChar( '.' ));
        if ( b.count() != 4 )
            return false;
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
        QSettings cfg("Trolltech","Security");
#ifndef QTOPIA_CELL
        cfg.beginGroup("Passcode");
        cfg.setValue("passcode",passcode);
        cfg.setValue("passcode_poweron",passcode_poweron->isChecked());
#endif
        cfg.endGroup();
        cfg.beginGroup("Sync");
        int auth_peer=0;
        int auth_peer_bits;
        QString sn = syncnet->currentText();
        parseNet(sn,auth_peer,auth_peer_bits);

        QHostAddress allowed((quint32)auth_peer);
        cfg.setValue("auth_peer",allowed.toString());
        cfg.setValue("auth_peer_bits",auth_peer_bits);

        /*
        { cfg.endGroup(); cfg.beginGroup("Remote"); };
        if ( telnetAvailable() )
            cfg.setValue("allow_telnet",telnet->isChecked());
        if ( sshAvailable() )
            cfg.setValue("allow_ssh",ssh->isChecked());
        // ### write ssh/telnet sys config files
        */
    }

    QtopiaIpcEnvelope("QPE/System", "securityChanged()");
}

// for the phone, it requires: once to check old, (even though in
// security app already), and
// twice for new.  Once it has all three, you can attempt to set the sim pin,
// or security pin as required.
void Security::changePassCode()
{
#ifdef QTOPIA_CELL
    QString old;
#endif
    QString new1;
    QString new2;
    bool    mismatch = false;
    bool    valid = true;

    do {
#ifdef QTOPIA_CELL
        // should check current pin first, may not be set.
        if (old.isNull()) {
            old = enterPassCode(tr("Enter current PIN"), false, false);
            // indicates dialog was canceled.
            if ( old.isNull() )
                return;
        }
        if (mismatch) {
            new1 = enterPassCode(tr("Mismatch: Retry new PIN"), false);
        } else if (!valid) {
            new1 = enterPassCode(tr("Invalid: Retry new PIN"), false);
        } else {
            new1 = enterPassCode(tr("Enter new passcode"), false);
        }
#else
        if (mismatch) {
            new1 = enterPassCode(tr("Mismatch: Retry new code"), true);
        } else if (!valid) {
            new1 = enterPassCode(tr("Invalid: Retry new code"), true);
        } else {
            new1 = enterPassCode(tr("Enter new passcode"), true);
        }
#endif
        // indicates dialog was canceled.
        if ( new1.isNull() )
            return;

        if (new1.isEmpty()) {
            valid = false;
        } else {
#ifdef QTOPIA_CELL
            new2 = enterPassCode(tr("Re-enter new PIN"), false);
#else
            new2 = enterPassCode(tr("Re-enter new passcode"), true);
#endif
            if ( new2.isNull() )
                return;

            valid = !new2.isEmpty();
            mismatch = new1 != new2;
        }

    } while (mismatch || !valid);

#ifdef QTOPIA_CELL
    phonesec->changePassword(pinSelection->currentIndex(),old,new2);

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
    passcode = QString();
    updateGUI();
}


QString Security::enterPassCode(const QString& prompt, bool encrypt, bool last)
{
    return QPasswordDialog::getPassword(this, prompt, encrypt ? QPasswordDialog::Crypted : QPasswordDialog::Plain, last);
}

bool Security::telnetAvailable() const
{
    // ### not implemented
    return false;
}

bool Security::sshAvailable() const
{
    // ### not implemented
    return false;
}

#include "security.moc"
