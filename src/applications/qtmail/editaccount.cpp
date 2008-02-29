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

#include <qtopiaapplication.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qgroupbox.h>
#include <qheaderview.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qmenu.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qtoolbutton.h>
#include <qscrollarea.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qvalidator.h>

#include "account.h"
#include "editaccount.h"

class PortValidator : public QValidator
{
public:
    PortValidator(QWidget *parent = 0, const char *name = 0)
        : QValidator(parent) {
      setObjectName(name);
    }

    State validate(QString &str, int &) const
    {
        // allow empty strings, as it's a bit awkward to edit otherwise
        if ( str.isEmpty() )
            return QValidator::Acceptable;

        bool ok = false;
        int i = str.toInt(&ok);
        if ( !ok )
            return QValidator::Invalid;

        if ( i <= 0 || i >= 65536 )
            return QValidator::Invalid;

        return QValidator::Acceptable;
    }
};

EditAccount::EditAccount( QWidget* parent, const char* name, Qt::WFlags fl )
  : QDialog(parent, fl)
{
#if defined(QTOPIA_PHONE)
    tabWidget =0;
#endif
    setupUi(this);

    //connect custom slots
    connect(ToolButton2,SIGNAL(clicked()),SLOT(sigPressed()));
    connect(accountType,SIGNAL(activated(int)),SLOT(typeChanged(int)));
    connect(authentication,SIGNAL(activated(int)),SLOT(authChanged(int)));
    connect(emailInput,SIGNAL(textChanged(const QString&)),SLOT(emailModified()));
    //connect(mailboxButton,SIGNAL(clicked()),SLOT(configureFolders()));

    setObjectName(name);
    emailTyped = false;

#ifdef QTOPIA_PHONE
    int imHint = QtopiaApplication::Number;
    QtopiaApplication::setInputMethodHint(mailPortInput,
                                          (QtopiaApplication::InputMethodHint)imHint );
    QtopiaApplication::setInputMethodHint(smtpPortInput,
                                          (QtopiaApplication::InputMethodHint)imHint );
#endif

#ifdef QTOPIA_PHONE
    // Too easy to mistype numbers in phone mode
    mailPasswInput->installEventFilter( this );
    accountNameInput->installEventFilter( this );
    defaultMailCheckBox->installEventFilter( this );
    installEventFilter(this);
    mailboxButton->hide();
#endif
    PortValidator *pv = new PortValidator(this);
    mailPortInput->setValidator(pv);
    smtpPortInput->setValidator(pv);
#ifndef SMTPAUTH
        encryption->hide();
        authentication->hide();
        lblEncryption->hide();
        lblAuthentication->hide();
        lblSmtpUsername->hide();
        lblSmtpPassword->hide();
        smtpUsernameInput->hide();
        smtpPasswordInput->hide();
    encryptionCheckBox->hide();
#endif
    typeChanged(1);
#if defined(QTOPIA_PHONE)
    createTabbedView();
#endif
    setLayoutDirection( qApp->layoutDirection() );
    mailPasswInput->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    smtpPasswordInput->setEchoMode(QLineEdit::PasswordEchoOnEdit);

}

void EditAccount::setAccount(MailAccount *in, bool newOne)
{
    account = in;

    if (newOne) {
        accountNameInput->setText("");
        emailInput->setText("");
        mailUserInput->setText("");
        mailPasswInput->setText("");
        mailServerInput->setText("");
        smtpServerInput->setText("");
        syncCheckBox->setChecked(true);
        mailPortInput->setText("110");
        smtpPortInput->setText("25");
#ifdef SMTPAUTH
        smtpUsernameInput->setText("");
        smtpPasswordInput->setText("");
        encryption->setCurrentIndex(0);
        authentication->setCurrentIndex(0);
        smtpUsernameInput->setEnabled(false);
        smtpPasswordInput->setEnabled(false);
    encryptionCheckBox->setChecked(false);
#endif
        setWindowTitle( tr("Create New Account", "translation not longer than English") );

        typeChanged( 0 );
    } else {

        if ( account->accountType() == MailAccount::POP ) {

        } else if ( account->accountType() == MailAccount::IMAP ) {
            accountType->setCurrentIndex(1);
            typeChanged(1);
            imapBaseDir->setText( account->baseFolder() );
        } else {
            accountType->setCurrentIndex(2);
            typeChanged(2);
        }

        accountNameInput->setText( account->accountName() );
        nameInput->setText( account->userName() );
        emailInput->setText( account->emailAddress() );
        mailUserInput->setText( account->mailUserName() );
        mailPasswInput->setText( account->mailPassword() );
        mailServerInput->setText( account->mailServer() );
        smtpServerInput->setText( account->smtpServer() );
        syncCheckBox->setChecked( account->synchronize() );
        deleteCheckBox->setChecked( account->deleteMail() );

        sigCheckBox->setChecked( account->useSig() );
        sig = account->sig();

        maxSize->setValue(account->maxMailSize());
        thresholdCheckBox->setChecked( account->maxMailSize() != -1 );
        mailPortInput->setText( QString::number( account->mailPort() ) );
        smtpPortInput->setText( QString::number( account->smtpPort() ) );
        defaultMailCheckBox->setChecked( account->defaultMailServer() );
#ifdef SMTPAUTH
        smtpUsernameInput->setText(account->smtpUsername());
        smtpPasswordInput->setText(account->smtpPassword());
        authentication->setCurrentIndex(static_cast<int>(account->smtpAuthentication()));
        encryption->setCurrentIndex(static_cast<int>(account->smtpEncryption()));
        smtpUsernameInput->setEnabled(authentication->currentIndex() != 0);
        smtpPasswordInput->setEnabled(authentication->currentIndex() != 0);
    encryptionCheckBox->setChecked(account->mailEncryption() != MailAccount::Encrypt_NONE);
#endif
    }

    nameInput->setText( account->userName() );
}

#ifdef QTOPIA_PHONE

void EditAccount::createTabbedView()
{
    delete layout();
    QHBoxLayout* thelayout = new QHBoxLayout(this);
    thelayout->setMargin(0);
    thelayout->setSpacing(0);
    tabWidget = new QTabWidget(this);
    thelayout->addWidget(tabWidget);

    QScrollArea* scroll = new QScrollArea(tabWidget);
    scroll->setFocusPolicy(Qt::NoFocus);
    scroll->setWidget(incomingFrame);
    scroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    scroll->setWidgetResizable(true);
    scroll->setFrameStyle(QFrame::NoFrame);
    incomingFrame->setLineWidth(0);
    incomingFrame->setMidLineWidth(0);
    incomingFrame->setFrameStyle(QFrame::NoFrame);
    tabWidget->addTab(scroll,tr("Incoming"));

    scroll = new QScrollArea(tabWidget);
    scroll->setFocusPolicy(Qt::NoFocus);
    scroll->setWidget(outgoingFrame);
    scroll->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    scroll->setWidgetResizable(true);
    scroll->setFrameStyle(QFrame::NoFrame);
    outgoingFrame->setLineWidth(0);
    outgoingFrame->setMidLineWidth(0);
    outgoingFrame->setFrameStyle(QFrame::NoFrame);
    tabWidget->addTab(scroll,tr("Outgoing"));

    updateGeometry();
}

// bool EditAccount::event(QEvent* e)
// {
//     if(e->type() == QEvent::LayoutRequest)
//     {
//         if(width() < (incomingFrame->sizeHint().width() + outgoingFrame->sizeHint().width()))
//         {
//             if(!tabWidget)
//             {
//                 createTabbedView();
//                 return QDialog::event(e);
//             }
//         }
//     }
//     return QDialog::event(e);
// }

bool EditAccount::eventFilter( QObject* o, QEvent *e )
{
    if ((o == accountNameInput) && (e->type() == QEvent::FocusIn))
    {
        if (tabWidget && tabWidget->currentIndex() != 1)
            tabWidget->setCurrentIndex( 1 );
        return QWidget::eventFilter( o, e );
    }
    else if ((o == defaultMailCheckBox) && (e->type() == QEvent::FocusIn)) {
        if (tabWidget && tabWidget->currentIndex() != 1)
            tabWidget->setCurrentIndex( 1 );
        return QWidget::eventFilter( o, e );
    }
    return QDialog::eventFilter(o,e);
}
#endif

//TODO fix, unused code, slot inaccessible since button not visible on form

// void EditAccount::configureFolders()
// {
//     MailboxSelector sel(this, "sel", account);
//     QtopiaApplication::execDialog(&sel);
// }

void EditAccount::sigPressed()
{
    if ( sigCheckBox->isChecked() ) {
      SigEntry sigEntry(this, "sigEntry", (Qt::WFlags)1);

        if ( sig.isEmpty() ) {
            sigEntry.setEntry( "~~\n" + nameInput->text() );
        } else {
            sigEntry.setEntry( sig );
        }

        if ( sigEntry.exec() == QDialog::Accepted) {
            sig = sigEntry.entry();
        }
    }
}

void EditAccount::emailModified()
{
    emailTyped = true;
}

void EditAccount::authChanged(int index)
{
        smtpUsernameInput->setEnabled(index != 0);
        smtpPasswordInput->setEnabled(index != 0);
        if(index == 0)
        {
        smtpUsernameInput->clear();
        smtpPasswordInput->clear();
        }
}

void EditAccount::typeChanged(int)
{
#ifdef QTOPIA_PHONE
    if ( accountType->currentText() == tr("Sync") ) {
#else
    if ( accountType->currentText() == tr("Synchronized") ) {
#endif
        imapSettings->hide();
        syncCheckBox->hide();

        mailboxButton->setEnabled( false );
        mailPortInput->setEnabled( false );

        smtpPortInput->setEnabled( false );
        smtpServerInput->setEnabled( false );
        deleteCheckBox->setEnabled( false );
        defaultMailCheckBox->setEnabled( false );
        thresholdCheckBox->setEnabled( false );

        return;
    } else {
        mailboxButton->setEnabled( true );
        mailPortInput->setEnabled( true );

        smtpPortInput->setEnabled( true );
        smtpServerInput->setEnabled( true );
        deleteCheckBox->setEnabled( true );
        defaultMailCheckBox->setEnabled( true );
        thresholdCheckBox->setEnabled( true );
    }

    if (accountType->currentText() == "POP") {
        mailPortInput->setText("110");

        imapSettings->hide();
        syncCheckBox->hide();
        deleteCheckBox->setChecked( false );
    } else if (accountType->currentText() == "IMAP") {
        syncCheckBox->hide();
        imapSettings->show();
        mailboxButton->setEnabled( account->mailboxes.count() > 0 );
        mailPortInput->setText("143");
        deleteCheckBox->setChecked( true );
    }
}

void EditAccount::deleteAccount()
{
    done(2);
}

void EditAccount::accept()
{
    QString name = accountNameInput->text();
    if ( name.trimmed().isEmpty() ) {
        name = mailServerInput->text();
        if ( name.trimmed().isEmpty() )
            name = smtpServerInput->text();
    }

    if (name.trimmed().isEmpty()) {
        int ret = QMessageBox::warning(this, tr("Empty account name"),
                tr("<qt>Do you want to continue and discard any changes?</qt>"),
                QMessageBox::Yes, QMessageBox::No|QMessageBox::Default|QMessageBox::Escape);
        if (ret == QMessageBox::Yes)
            reject();
        return;
    }

    account->setAccountName( accountNameInput->text() );
    account->setUserName( nameInput->text() );
    account->setEmailAddress( emailInput->text() );
    account->setMailUserName( mailUserInput->text() );
    account->setMailPassword( mailPasswInput->text() );
    account->setMailServer( mailServerInput->text() );
    account->setSmtpServer( smtpServerInput->text() );
    account->setSynchronize( syncCheckBox->isChecked() );
    account->setDeleteMail( deleteCheckBox->isChecked() );

    if ( accountType->currentText() == "POP" ) {
        account->setAccountType( MailAccount::POP );
    } else if ( accountType->currentText() == "IMAP") {
        account->setAccountType( MailAccount::IMAP );
    } else {
        account->setAccountType( MailAccount::Synchronized );
    }

    account->setUseSig( sigCheckBox->isChecked() );
    account->setSig( sig );
    account->setDefaultMailServer( defaultMailCheckBox->isChecked() );

    if ( thresholdCheckBox->isChecked() ) {
        account->setMaxMailSize( maxSize->value() );
    } else {
        account->setMaxMailSize( -1 );
    }
    account->setCheckInterval( -1 );
    if ( account->accountType() == MailAccount::IMAP )
        account->setBaseFolder( imapBaseDir->text() );

    QString temp;
    bool result;
    temp = mailPortInput->text();
    account->setMailPort( temp.toInt(&result) );
    if ( (!result) ) {
        // should only happen when the string is empty, since we use a validator.
        if (accountType->currentText() == "POP")
            account->setMailPort( 110 );
        else
            account->setMailPort( 143 );
    }

    temp = smtpPortInput->text();
    account->setSmtpPort( temp.toInt(&result) );
    // should only happen when the string is empty, since we use a validator.
    if ( !result )
        account->setSmtpPort( 25 );

    //try to guess email address
    if ( (!emailTyped) && (account->emailAddress().isEmpty()) ) {
        QString address = account->smtpServer();

        if ( address.count('.')) {
            account->setEmailAddress( account->mailUserName() + "@" +
            address.mid( address.indexOf('.') + 1, address.length() ) );
        } else if (address.count('.') == 1) {
            account->setEmailAddress( account->mailUserName() + "@" + address );
        }
    }

    //set an accountname
    if ( account->accountName().isEmpty() ) {
        int pos = name.indexOf('.');
        if ( pos != -1) {
            name = name.mid( pos + 1, name.length() );

            pos = name.indexOf('.', pos);
            if (pos != -1)
                name = name.mid(0, pos);
        }

        account->setAccountName( name );
    }

#ifdef SMTPAUTH
        account->setSmtpUsername(smtpUsernameInput->text());
        account->setSmtpPassword(smtpPasswordInput->text());
        account->setSmtpAuthentication(static_cast<MailAccount::AuthType>(authentication->currentIndex()));
        account->setSmtpEncryption(static_cast<MailAccount::EncryptType>(encryption->currentIndex()));
    if(encryptionCheckBox->isChecked())
        account->setMailEncryption(MailAccount::Encrypt_SSL);
    else
        account->setMailEncryption(MailAccount::Encrypt_NONE);
#endif

    QDialog::accept();
}

SigEntry::SigEntry(QWidget *parent, const char *name, Qt::WFlags fl )
    : QDialog(parent,fl)
{
    setObjectName(name);
    setWindowTitle( tr("Signature") );

    QGridLayout *grid = new QGridLayout(this);
    input = new QTextEdit(this);
    grid->addWidget(input, 0, 0);
    resize(180, 100);
}

// /*  class MailboxView    */
// MailboxView::MailboxView(QWidget *parent, const char* name)
//     : QListWidget(parent)
// {
//     setObjectName(name);
//     pop = new QMenu(this);
//     connect(pop, SIGNAL( triggered(QAction*) ), this,
//      SLOT( changeMessageSettings(QAction*) ) );
//
//     pop->addAction( tr("All"));
//     pop->addAction( tr("Only recent"));
//     pop->addAction( tr("None (only headers)"));
//
//     connect( &menuTimer, SIGNAL( timeout() ), SLOT( showMessageOptions() ) );
//     connect( this, SIGNAL( itemChanged() ), SLOT( cancelMenuTimer() ) );
// }
//
// void MailboxView::changeMessageSettings(QAction* id)
// {
//     QListWidgetItem *item = currentItem();
//     if ( !item )
//  return;
//
//     item->setText(id->text());
// }
//
// void MailboxView::showMessageOptions()
// {
//     pop->popup( QCursor::pos() );
// }
//
// void MailboxView::mousePressEvent( QMouseEvent * e )
// {
//     QListView::mousePressEvent( e );
//     menuTimer.start( 500);
//     menuTimer.setSingleShot(true);
// }
//
// void MailboxView::mouseReleaseEvent( QMouseEvent * e )
// {
//     QListView::mouseReleaseEvent( e );
//     menuTimer.stop();
// }
//
// void MailboxView::cancelMenuTimer()
// {
//     if( menuTimer.isActive() )
//     menuTimer.stop();
// }
//
// /*  Class MailboxSelector   */
// MailboxSelector::MailboxSelector(QWidget *parent, const char *name, MailAccount *account)
//     : QDialog(parent)
// {
//     setObjectName(name);
//     _account = account;
//
//     setWindowTitle( tr("Configure IMAP folders") );
//
//     QGridLayout *grid = new QGridLayout(this);
// //    grid->setSpacing(4);
// //    grid->setMargin(4);
//
//     view = new MailboxView(this, "mview");
//     QAction* what = QWhatsThis::createAction(view);
//     what->setText(tr("Lists your IMAP mailboxes.  Tick the mailboxes which you want to access on your device."));
//
//    // what = QWhatsThis::createAction(view->header());
//    // what->setText(tr("For selected mailboxes, you can choose which type of messages to download."));
//
//     view->addColumn(tr("Keep local copy"), 120);
//     view->addColumn(tr("Complete messages"), 115);
//     view->setAllColumnsShowFocus( true );
//     grid->addMultiCellWidget(view, 0, 0, 0, 3);
//
//     QPushButton *button = new QPushButton( tr("Select all"), this);
//
//     what = QWhatsThis::createAction(button);
//     what->setText(tr("Select all mailboxes in your account."));
//     connect(button, SIGNAL( clicked() ), this, SLOT( selectAll() ) );
//     grid->addWidget(button, 1, 0);
//
//     button = new QPushButton( tr("Deselect all"), this);
//
//     what = QWhatsThis::createAction(button);
//     what->setText(tr("Deselect all mailboxes in your account."));
//     connect(button, SIGNAL( clicked() ), this, SLOT( clearAll() ) );
//     grid->addWidget(button, 1, 1);
//
//     QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
// //     grid->addItem(spacer, 1, 2);
//
//     button = new QPushButton( tr("Change message type"), this );
//     what = QWhatsThis::createAction(button);
//     what->setText(tr("Choose the message types you want to download for the selected mailbox."));
//     connect(button, SIGNAL(clicked() ), this, SLOT( itemSelected() ) );
//     grid->addWidget(button, 1, 3);
//
//     Mailbox *box;
//     QCheckListItem *item;
//     uint row = 0;
//     for (box = _account->mailboxes.first(); box != NULL;
//  box = _account->mailboxes.next() ) {
//
//  item = new QCheckListItem(view, box->pathName(), QCheckListItem::CheckBox);
//  item->setOn( box->localCopy() );
//
//  QString sync = tr("All");
//  FolderSyncSetting fs = box->folderSync();
//  if ( fs == Sync_OnlyNew)
//      sync = tr("Only recent");
//  if ( fs == Sync_OnlyHeaders)
//      sync = tr("None (only headers)");
//
//  item->setText(1, sync );
//
//  row++;
//     }
//
// }
//
// void MailboxSelector::accept()
// {
//     QCheckListItem *item;
//     QListViewItemIterator it(view);
//     uint row = 0, localCopy = 0;
//
//     for ( ; it.current(); ++it ) {
//      item = (QCheckListItem *) it.current();
//  if ( item->isOn() )
//      localCopy++;
//     }
//
//     if ( localCopy == 0 && view->childCount() > 0 ) {
//  QString message = tr("<qt>No local copies? You must select the mailboxes you want to access mail from</qt>");
//
//  switch( QMessageBox::warning( this, tr("Email"), message,
//      tr("I know"), tr("Whoops"), 0, 0, 1 ) ) {
//
//      case 0: break;
//      case 1: return;
//  }
//
//     }
//
//     localCopy = 0;
//     it = view->firstChild();
//     for ( ; it.current(); ++it) {
//      item = (QCheckListItem *) it.current();
//  Mailbox *box =  _account->getMailboxRef( item->text(0) );
//  if ( box ) {
//      box->setLocalCopy( item->isOn() );
//      if ( item->isOn() )
//      localCopy++;
//
//      FolderSyncSetting fs = Sync_AllMessages;
//      QString sync = item->text(1);
//      if ( sync == tr("Only recent") ) {
//      fs = Sync_OnlyNew;
//      } else if ( sync == tr("None (only headers)") ) {
//      fs = Sync_OnlyHeaders;
//      }
//      box->setFolderSync( fs );
//  }
//
//  row++;
//     }
//
//     QDialog::accept();
// }
//
// void MailboxSelector::selectAll()
// {
//     QListViewItemIterator it(view);
//     for ( ; it.current(); ++it) {
//  ( (QCheckListItem *) it.current())->setOn(true);
//     }
// }
//
// void MailboxSelector::clearAll()
// {
//     QListViewItemIterator it(view);
//     for ( ; it.current(); ++it) {
//  ( (QCheckListItem *) it.current())->setOn(false);
//     }
// }
//
// void MailboxSelector::itemSelected()
// {
//     QListViewItem *item = view->selectedItem();
//     if ( item )
//  view->showMessageOptions();
// }
