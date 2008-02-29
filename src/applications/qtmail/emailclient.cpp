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

#ifdef QTOPIA_DESKTOP
#include <qcopenvelope_qd.h>
#else
#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#endif

#include <qtopianamespace.h>
#include <qmimetype.h>
#include <qsoftmenubar.h>
#include <qtopialog.h>

#include "emailclient.h"
#include "folder.h"
#include "searchview.h"
#include "accountlist.h"
#include "common.h"
#ifdef QTOPIA_PHONE
#include "composer.h"
#include "selectfolder.h"
#include "accountsettings.h"
#endif

#include <qapplication.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qcursor.h>
#include <qstackedwidget.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qdesktopwidget.h>
#include <qtcpsocket.h>
#include <QHeaderView>
#include <QListWidget>
#include <QDebug>
#include <qtmailwindow.h>
#include <qtreewidget.h>
#include <QDSActionRequest>
#include <QDSData>
#include <QtopiaApplication>
#include <QPhoneStyle>
#include "longstream.h"

#ifndef LED_MAIL
#define LED_MAIL 0
#endif

using namespace QtMail;

static QIcon* pm_folder = 0;
static QIcon* pm_trash = 0;

//paths for qtmail, is settings, inbox, enclosures
QString getPath(const QString& fn, bool isdir=false)
{
    QString p = Qtopia::applicationFileName("qtmail",fn);
    if (isdir) {
        QDir dir(p);
        if ( !dir.exists() )
            dir.mkdir( dir.path() );
        p += "/";
    }
    return p;
}

class StatusProgressBarStyle : public QPhoneStyle
{
public:
    void drawControl( ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget ) const
    {
        if (ce != CE_ProgressBarGroove)
            QPhoneStyle::drawControl(ce, opt, p, widget);
    }
};

// A QProgressBar and status label combined. No percentage is show, as
// that's represented by the bar alone.
//
class StatusProgressBar : public QProgressBar {
public:
    StatusProgressBar( QWidget* parent ) :
        QProgressBar(parent), txtchanged(false)
    {
        QPalette p(palette());
        p.setBrush(QPalette::Base,p.brush(QPalette::Window));
        p.setBrush(QPalette::HighlightedText,p.brush(QPalette::WindowText));
        setPalette(p);
        setAlignment(Qt::AlignHCenter);
        setStyle(new StatusProgressBarStyle);
    }
    ~StatusProgressBar()
    {
        delete style();
    }

    QSize sizeHint() const
    {
        return QProgressBar::sizeHint()-QSize(0,8);
    }

    void setText(const QString& s)
    {
        if ( txt != s ) {
            if ( value() == maximum() )
                reset();
            txt = s;
            txtchanged = true;
        repaint();
        }
    }

    QString text() const
    {
      return txt;
    }

private:
    QString txt;
    bool txtchanged;
};

EmailClient::EmailClient( QWidget* parent, const QString name, Qt::WFlags fl )
        : QMainWindow( parent, fl ), accountIdCount(0), emailHandler(0), mb(0),
          fetchTimer(this), showMessageType(MailAccount::SMS),
          autoDownloadMail(false)
{
    setObjectName( name );
#ifdef QTOPIA_PHONE
    appTitle = tr("Messages");
#else
    appTitle = tr("Email");
#endif

    connect( &fetchTimer, SIGNAL(timeout()), this, SLOT( automaticFetch() ) );
    autoGetMail = false;

    mailboxList = new MailboxList(this);

    accountList = new AccountList(this, "accountList");

    getPath("enclosures/", true);  //create directory enclosures

    suspendMailCount = false;
    sending = false;
    receiving = false;
    previewingMail = false;
    mailIdCount = 1;
    allAccounts = false;
    closeAfterWrite = false;
    closeAfterView = false;
    closeAfterTransmissions = false;
    mMessageView = 0;
    invalidId = -1;
    folderId = -2;
    messageId = -3;
    init();

    sysMessagesChannel =  new QtopiaChannel("QPE/SysMessages", this);
    connect(sysMessagesChannel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT( handleSysMessages(const QString&, const QByteArray&)));

    

    connect(folderView, SIGNAL(folderSelected(Folder*)), this, SLOT(folderSelected(Folder*)) );
    connect(folderView, SIGNAL( emptyFolder() ), this, SLOT( emptyFolder() ) );

    connect(mailboxList, SIGNAL( stringStatus(QString&) ), this,
            SLOT( setStatusText(QString&) ) );

    connect(&showMessageTimer, SIGNAL( timeout() ), this,
            SLOT( displayRecentMessage() ));

    connect(accountList, SIGNAL( checkAccount(int) ),
            this, SLOT(selectAccount(int)) );

    connect(&checkAccountTimer, SIGNAL( timeout() ),
            this, SLOT(selectAccountTimeout()) );

    queueStatus = 0;
    
    accountList->readAccounts();
    
    createEmailHandler();
    
    updateAccounts();
    folderView->setupFolders(accountList );

    readSettings();

    nosuspend = 0;
    filesRead = false;
    showMsgList = false;
    showMsgRetryCount = 0;

    QTimer::singleShot(0, this, SLOT( openFiles() ) );

    // Hook up the "Email" and "SMS" QCop service handlers.
    new EmailService( this );
    new SMSService( this );

    QTimer::singleShot(0, this, SLOT( collectSysMessages() ) );

    qLog(Messaging) << "Created EMailClient";
}

EmailClient::~EmailClient()
{
    delete pm_folder;
    delete pm_trash;
    EmailListItem::deletePixmaps();
    delete emailHandler;

    qLog(Messaging) << "Destroyed EMailClient";
}

void EmailClient::openFiles()
{
    if ( filesRead ) {
        if ( !cachedDisplayMailbox.isEmpty() )
            displayMail(cachedDisplayMailbox, cachedDisplayMailUuid);

        return;
    }

    filesRead = true;

    readMail();
    
    //connect after mail has been read to speed up reading */
    connect(mailboxList, SIGNAL( mailAdded(Email*,const QString&) ), this,
                    SLOT( mailFromDisk(Email*,const QString&) ) );
    connect(mailboxList, SIGNAL( mailUpdated(Email*,const QString&) ), this,
                    SLOT( mailUpdated(Email*,const QString&) ) );
    connect(mailboxList, SIGNAL( mailRemoved(const QUuid&,const QString&) ), this,
                    SLOT( mailRemoved(const QUuid&,const QString&) ) );
    connect(mailboxList,SIGNAL(mailMoved(Email*,const QString&, const QString&)),this,
                    SLOT( mailMoved(Email*, const QString&, const QString&)));
    connect(mailboxList, SIGNAL( externalEdit(const QString&) ), this,
            SLOT(externalEdit(const QString&) ) );
   
    if ( !cachedDisplayMailbox.isEmpty() ) {
        displayMail(cachedDisplayMailbox, cachedDisplayMailUuid);
    } else {
      //No default select for QTreeWidget
      Folder* currentFolder = folderView->currentFolder();
      if(!currentFolder)
        folderView->changeToSystemFolder(InboxString);
      else
        folderSelected( folderView->currentFolder() );
      showFolderList();

      displayPreviousMail();
    }
    
    qLog(Messaging) << "Mail boxes connected";
}

void EmailClient::displayPreviousMail()
{
    if (!mMessageView)
        return;

    QSettings mailconf("Trolltech","qtmail");
    mailconf.beginGroup("qtmailglobal");
    QUuid id( mailconf.value("currentmail").toString() );
    mailconf.endGroup();
    if ( !id.isNull() ) {
        EmailListItem *item = messageView()->getRef( id );
        if (item) {
            messageView()->clearSelection();
            messageView()->setItemSelected( item, TRUE );
            messageView()->setCurrentItem( item );
            messageView()->scrollToItem( item );
        }
    }
}

void EmailClient::displayMail(const QString &mailbox, const QUuid &id)
{
    EmailFolderList *box = mailboxList->mailbox(mailbox);
    if ( box ) {

        folderView->changeToSystemFolder( mailbox );
        folderSelected( folderView->currentFolder() );
        EmailListItem *item = messageView()->getRef( id );

        if ( item ) {
            messageView()->clearSelection();
            messageView()->setItemSelected( item, TRUE );
            messageView()->setCurrentItem( item );
            queryItemSelected();
        }
    }

    cachedDisplayMailbox = "";
}


void EmailClient::delayedShowMessage(MailAccount::AccountType acct, QUuid id, bool showList)
{
    QtopiaApplication::instance()->registerRunningTask(QLatin1String("display"));
    showMsgList = showList;
    showMessageType = acct;
    showMessageTimer.setSingleShot( true );
    showMessageTimer.start( 700 );
    showMsgId = id;
}

void EmailClient::displayRecentMessage()
{
    if(showMessageType == MailAccount::SMS ||
       showMessageType == MailAccount::MMS)
    {
        if(emailHandler->unreceivedSmsCount() > 0)
        {
            if(showMsgRetryCount < 10) //wait max ~ 7 seconds
            {
                showMessageTimer.start(700);
                ++showMsgRetryCount;
                return;
            }
        }
    }
    showMsgRetryCount = 0;
    qLog(Messaging) << "Displaying message";
    QtopiaApplication::instance()->unregisterRunningTask(QLatin1String("display"));
    if ( checkMailConflict(
        tr("Should this mail be saved in Drafts before viewing the new mail?"),
        tr("View Mail message will be ignored")) )
        return;

    Folder *folder = 0;
    QListIterator<MailAccount*> it = accountList->accountIterator();
    while ( it.hasNext() ) {
        if ( it.next()->accountType() == showMessageType ) {
            folder = it.peekPrevious();
            break;
        }
    }

    openFiles();

    if (!folder)
        return;

    FolderListItem *item = 0;
    FolderListItem* top = 0;
    for (int i = 0; i < folderView->topLevelItemCount(); ++i){
        top = static_cast<FolderListItem*>(folderView->topLevelItem(i));
        if(top->folder() == folder){
            item = top;
            break;
        }
        else{
            for(int j = 0; j < top->childCount(); ++j){
                item = static_cast<FolderListItem*>(top->child(j));
                if(item->folder() == folder)
                    break;
                else
                    item = 0;
        }
        if(item)
            break;
        }
    }

    if (!item || (item->folder() != folder))
        return;

    bool oldCloseAfterView = closeAfterView;
    closeAfterView = false;
    folderView->setCurrentItem( item );
    folderSelected( folderView->currentFolder() );
    showMessageList();
    if (!showMsgList) {
        if (!showMsgId.isNull()) {
            EmailListItem *item = messageView()->getRef(showMsgId);
            if (item) {
                messageView()->clearSelection();
                messageView()->setItemSelected(item, true);
                messageView()->setCurrentItem(item);
                messageView()->scrollToItem(item);
            }
        }
        queryItemSelected();
    }

    closeAfterView = oldCloseAfterView;
}

bool EmailClient::cleanExit(bool force)
{
    bool result = true;

    if (sending || receiving) {
        if (force) {
            qLog(Messaging) << "EmailClient::cleanExit: forcing cancel to exit";
            cancel();   //abort all transfer
        }
        result = false;
    }

    if (mailboxList)
        mailboxList->writeDirtyHeaders();

    saveSettings();
    accountList->saveAccounts();

    return result;
}

void EmailClient::closeAfterTransmissionsFinished()
{
    closeAfterWrite = false;
    if (!closeAfterTransmissions) {
        closeAfterTransmissions = true;
    }
}

bool EmailClient::isTransmitting()
{
    return nosuspend;
}

void EmailClient::createEmailHandler()
{
    if(emailHandler)
        return;
        //create the email handler
        
     emailHandler = new EmailHandler();
        
     //connect it up
        
     connect(emailHandler, SIGNAL(updatePopStatus(const QString&)),
             this, SLOT(updatePopStatusLabel(const QString&)) );
     connect(emailHandler, SIGNAL(updateSendingStatus(const QString&)),
             this, SLOT(updateSendingStatusLabel(const QString&)) );
    
     connect(emailHandler, SIGNAL(mailboxSize(int)),
             this, SLOT(setTotalPopSize(int)) );
     connect(emailHandler, SIGNAL(downloadedSize(int)),
             this, SLOT(setDownloadedSize(int)) );
     //smtp
     connect(emailHandler, SIGNAL(transferredSize(int)),
             this, SLOT(setTransferredSize(int)) );
     connect(emailHandler, SIGNAL(mailSendSize(int)),
             this, SLOT(setTotalSmtpSize(int)) );
     connect(emailHandler, SIGNAL(mailSent(int)), this, SLOT(mailSent(int)) );
    
     connect(emailHandler, SIGNAL(smtpError(int,QString&)), this,
             SLOT(smtpError(int,QString&)) );
     connect(emailHandler, SIGNAL(popError(int,QString&)), this,
             SLOT(popError(int,QString&)) );
#ifndef QTOPIA_NO_SMS
     connect(emailHandler, SIGNAL(smsError(int,QString&)), this,
             SLOT(smsError(int,QString&)) );
#endif
#ifndef QTOPIA_NO_MMS
     connect(emailHandler, SIGNAL(mmsError(int,QString&)), this,
             SLOT(mmsError(int,QString&)) );
#endif
     connect(emailHandler, SIGNAL(unresolvedUidlList(QString&,QStringList&)),
             this, SLOT(unresolvedUidlArrived(QString&,QStringList&)) );
     connect(emailHandler, SIGNAL(failedList(QStringList&)), this,
             SLOT(failedList(QStringList&)) );
        
     connect(emailHandler, SIGNAL(mailArrived(const Email&)), this,
             SLOT(mailArrived(const Email&)) );
     connect(emailHandler, SIGNAL(mailTransferred(int)), this,
             SLOT(allMailArrived(int)) );
     //imap
     connect(emailHandler, SIGNAL(serverFolders()), this,
             SLOT(imapServerFolders()) );
        
     //set relevant accounts
        
     QListIterator<MailAccount*> it = accountList->accountIterator();
     while ( it.hasNext() ) {
         MailAccount *account = it.next();
#ifndef QTOPIA_NO_SMS
         if ( account->accountType() == MailAccount::SMS )
             emailHandler->setSmsAccount( account );
#endif
#ifndef QTOPIA_NO_MMS
         if ( account->accountType() == MailAccount::MMS ) 
             emailHandler->setMmsAccount( account );
#endif
     }
}

void EmailClient::init()
{
    mReadMail = 0;
    mWriteMail = 0;

    pm_folder = new QIcon(":icon/folder");
    pm_trash = new QIcon(":icon/trash");

    vbox = new QFrame(this);
    vboxLayout = new QVBoxLayout(vbox);
    vboxLayout->setMargin( 0 );
    vboxLayout->setSpacing( 0 );

    setBackgroundRole( QPalette::Button );

#ifndef QTOPIA_PHONE
    bar = new QToolBar( this );
    bar->setMovable( false );
#ifdef QTOPIA4_TODO
    bar->setHorizontalStretchable( true );
#endif

    mb = new QMenuBar( bar );

    QMenu *mail = mb->addMenu( tr( "&Mail" ) );
    configure = mb->addMenu( tr( "&Accounts" ) );
    connect(configure, SIGNAL(triggered(QAction*)),
            this, SLOT(editAccount(QAction*)));

    bar = new QToolBar( this );
    bar->setMovable( false );
#endif

//     sendMailButton = new QAction( QIcon(":icon/sendmail"), tr("Send all mail"), this );
//     connect(sendMailButton, SIGNAL(triggered()), this, SLOT(sendAllQueuedMail()) );
//     sendMailButton->setWhatsThis( tr("Send all mail in the Outbox.") );

    selectAccountMenu = new QMenu(mb);
    connect(selectAccountMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(selectAccount(QAction*)));

    getMailButton = new QAction( QIcon(":icon/getmail"), tr("Get all mail"), this );
    connect(getMailButton, SIGNAL(triggered()), this, SLOT(getAllNewMail()) );
    getMailButton->setWhatsThis( tr("Get new mail from all your accounts.") );


    cancelButton = new QAction( QIcon(":icon/reset"), tr("Cancel transfer"), this );
    connect(cancelButton, SIGNAL(triggered()), this, SLOT(cancel()) );
    cancelButton->setWhatsThis( tr("Abort all transfer of mail.") );
    cancelButton->setVisible(false);


    movePop = new QMenu(this);
    copyPop = new QMenu(this);
    connect(movePop, SIGNAL(triggered(QAction*)),
            this, SLOT(moveMailItem(QAction*)));
    connect(copyPop, SIGNAL(triggered(QAction*)),
            this, SLOT(copyMailItem(QAction*)));

    composeButton = new QAction( QIcon(":icon/new"), tr("New"), this );
    connect(composeButton, SIGNAL(triggered()), this, SLOT( compose() ) );
    composeButton->setWhatsThis( tr("Write a new mail.") );

    searchButton = new QAction( QIcon(":icon/find"), tr("Search"), this );
    connect(searchButton, SIGNAL(triggered()), this, SLOT( search() ) );
    searchButton->setWhatsThis( tr("Search for mail in your folders.") );

    settingsAction = new QAction( QIcon(":icon/settings"), tr("Account Settings..."), this );
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(settings()));

    emptyTrashAction = new QAction(tr("Empty Trash"), this );
    connect(emptyTrashAction, SIGNAL(triggered()), this, SLOT(emptyFolder()));

#ifdef QTOPIA_PHONE
    moveAction = new QAction( tr("Move mail..."), this );
    connect(moveAction, SIGNAL(triggered()), this, SLOT(moveMessage()));
    copyAction = new QAction( tr("Copy mail..."), this );
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copyMessage()));
    selectAllAction = new QAction( tr("Select all"), this );
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
#endif

    deleteMailAction = new QAction( *pm_trash, tr("Delete mail"), this );
    connect(deleteMailAction, SIGNAL(triggered()), this, SLOT(deleteMailItem()));

#ifndef QTOPIA_PHONE
//     mail->addAction( sendMailButton );
    mail->addSeparator();
    mail->addAction( getMailButton );
    QAction *selectAccountMenuAction = mail->addMenu( selectAccountMenu );
    selectAccountMenuAction->setText( tr("Get Mail in") );
    mail->addSeparator();
    mail->addAction( cancelButton );
    mail->addSeparator();
    QAction *movePopMenuAction = mail->addMenu( movePop );
    movePopMenuAction->setText( tr("Move to") );
    QAction *copyPopMenuAction = mail->addMenu( copyPop );
    copyPopMenuAction->setText( tr("Copy to") );
    mail->addAction( deleteMailAction );
    mail->addSeparator();
    mail->addAction( emptyTrashAction );
    mail->addSeparator();
    mail->addAction( composeButton );
    mail->addAction( searchButton );

//     bar->addAction( sendMailButton );
    bar->addAction( getMailButton );

    if ( QApplication::desktop()->screenGeometry().width() > 176 ) {
        bar->addAction( composeButton );
        bar->addAction( searchButton );
    }
#endif

#ifdef QTOPIA_PHONE
    mailboxView = new QStackedWidget( vbox );
    mailboxView->setObjectName( "mailboxView" );
    vboxLayout->addWidget( mailboxView );
#else
    mailboxView = new QTabWidget( vbox );
    mailboxView->setObjectName( "mailboxView" );
    vboxLayout->addWidget( mailboxView );
    // remove unnecessary border.
    QStackedWidget *sw;
    sw = mailboxView->findChild<QStackedWidget*>( "QStackedWidget" );
    if (sw)
	sw->setFrameStyle( QFrame::NoFrame );
#endif

    widget_3 = new QWidget( mailboxView );
    widget_3->setObjectName( "widget_3" );
    gridQuery = new QGridLayout(widget_3);
    gridQuery->setMargin( 0 );
    gridQuery->setSpacing( 0 );

    QWidget *widget_4 = new QWidget( mailboxView );
    widget_4->setObjectName( "widget_4" );
    QGridLayout *gridFolder = new QGridLayout(widget_4);
    gridFolder->setMargin( 0 );
    gridFolder->setSpacing( 0 );

    folderView = new FolderListView(mailboxList, widget_4, "folderView");
    connect(folderView, SIGNAL(viewMessageList()),  this, SLOT(showMessageList()) );
    gridFolder->addWidget(folderView, 1, 1 );

#ifdef QTOPIA_PHONE
    folderId = mailboxView->addWidget( widget_4 );
    messageId = mailboxView->addWidget( widget_3 );

    /* Create context menus for list of folders and messages */
    QMenu *folderContext = QSoftMenuBar::menuFor( widget_4 );
    folderContext->addAction( composeButton );
    folderContext->addAction( getMailButton );
    folderContext->addAction( searchButton );
    folderContext->addAction( cancelButton );
    folderContext->addAction( emptyTrashAction );
    folderContext->addAction( settingsAction );

    QMenu *messageContext = QSoftMenuBar::menuFor( widget_3 );
    messageContext->addAction( composeButton );
    messageContext->addAction( deleteMailAction );
    messageContext->addAction( moveAction );
    messageContext->addAction( copyAction );
    messageContext->addAction( selectAllAction );
#else
    mailboxView->addTab( widget_4, tr( "Folders" ) );
    mailboxView->addTab( widget_3, appTitle );
#endif

    /*  Folder and Message View specific init not related to placement  */
    QStringList columns;
    columns << tr( "Folders" );
    folderView->setColumnCount( columns.count() );
    folderView->setHeaderLabels( columns );
    folderView->setRootIsDecorated( false );

    QHeaderView *header = folderView->header();
    header->setMovable( false );
    header->setClickable( false );

    QAction *fvWhatsThis = QWhatsThis::createAction( folderView );
    fvWhatsThis->setText( tr("A list of your folders.  You can tap Outbox and then tap the Messages tab to see the messages currently in the outbox.") );

#ifdef QTOPIA_PHONE
    folderView->header()->resizeSection( 0,
                        QApplication::desktop()->availableGeometry().width() );
    folderView->folderParentMenu(mb);
#else
    QMenu *options = folderView->folderParentMenu(mb);
    options->setTitle( tr( "&Options" ) );
    mb->addMenu( options );

    /* Corner button    */
    QPushButton *cornerButton = new QPushButton(widget_3);
    cornerButton->setIcon( QIcon(":image/qtmail/menu") );
    connect(cornerButton, SIGNAL( clicked() ),
            this, SLOT( cornerButtonClicked() ) );

    QAction *cbWhatsThis = QWhatsThis::createAction( cornerButton );
    cbWhatsThis->setText( tr("Toggle display of the column headers by tapping this icon.") );

#ifdef QTOPIA4_TODO
    messageView()->setCornerWidget( cornerButton );
#endif
#endif

    progressBar = new StatusProgressBar( vbox );
    vboxLayout->addWidget( progressBar );

    setCentralWidget( vbox );

    setWindowTitle( appTitle );
}

void EmailClient::update()
{
    QTableWidgetItem *current = messageView()->currentItem();

    if ( current && messageView()->isItemSelected( current ) )
        messageView()->scrollToItem( current );

    if ( currentMailboxWidgetId() == folderId ) {
        folderView->setFocus();
    } else if ( currentMailboxWidgetId() == messageId ) {
        messageView()->setFocus();
    }

    mailboxList->writeDirtyHeaders();
    mailboxList->compact();

    //  In case user changed status of sent/unsent or read/unread messages
    Folder *folder = folderView->currentFolder();
    if ( folder ) {
        updateFolderCount( folder->mailbox() );
    }
}

void EmailClient::cancel()
{
    if ( !cancelButton->isEnabled() )
        return;
    emailHandler->cancel();
    progressBar->reset();

    isSending( false );
    isReceiving( false );
}

/*  Called when the user just exits the writemail window.  We don't know what he wanted
    to do, but we should be able to determine it
    Close event is handled by qtmailwindow, so no raise signal is necessary
*/
void EmailClient::autosaveMail(const Email &mail)
{
    // if uuid is null, it's a new mail
    bool isNew = mail.uuid().isNull();

    //  Always autosave new messages to drafts folder
    if ( isNew ) {
        saveAsDraft( mail );
    } else {
        // update mail in same mailbox as it was previously stored
        if ( mailboxList->mailbox(OutboxString)->email( mail.uuid() ) != NULL ) {
            enqueueMail( mail );
        } else {
            saveAsDraft( mail );
        }
    }
}

/*  Enqueue mail must always store the mail in the outbox   */
void EmailClient::enqueueMail(const Email &mailIn)
{
    Email mail(mailIn);

    // if uuid is null, it's a new mail
    bool isNew = mail.uuid().isNull();

    if ( isNew ) {
        mailResponded();

        mail.setUuid( mailboxList->mailbox(OutboxString)->generateUuid() );

        //mail.encodeMail();
        if ( !mailboxList->mailbox(OutboxString)->addMail(mail) ) {
            accessError(mailboxList->mailbox(OutboxString) );
            return;
        }
    } else {

       // mail.encodeMail();

        // two possibilities, mail was originally from drafts but is now enqueued, or
        // the mail was in the outbox previously as well.
        if ( mailboxList->mailbox(DraftsString)->email( mail.uuid() ) != NULL ) {
            mailboxList->mailbox(DraftsString)->removeMail( mail.uuid(), false );
        }

        if ( !mailboxList->mailbox(OutboxString)->addMail( mail ) ) {
            accessError( mailboxList->mailbox(OutboxString) );
            return;
        }
    }

#ifndef QTOPIA_PHONE
    folderView->changeToSystemFolder( OutboxString );
    showMessageList();
#else
    sendAllQueuedMail();
    showFolderList();
    folderView->setFocus();
#endif

    emit raiseWidget(this, appTitle );

#ifndef QTOPIA_DESKTOP
    if (closeAfterWrite) {
        closeAfterTransmissionsFinished();
        if (isTransmitting()) // prevents flicker
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
    }
#endif
}

/*  Simple, do nothing  */
void EmailClient::discardMail()
{
    // Reset these in case user chose reply but discarded message
    repliedFromUuid = QUuid();
    repliedFlags = 0;

    emit raiseWidget( this, appTitle );

#ifndef QTOPIA_DESKTOP
    if (closeAfterWrite) {
        closeAfterTransmissionsFinished();
        if (isTransmitting())
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
    }
#endif
}

void EmailClient::saveAsDraft(const Email &mailIn)
{
    Email mail(mailIn);

    // if uuid is null, it's a new mail
    bool isNew = mail.uuid().isNull();

    if ( isNew ) {
        mailResponded();

        mail.setUuid( mailboxList->mailbox(DraftsString)->generateUuid() );

        //mail.encodeMail();
        if ( !mailboxList->mailbox(DraftsString)->addMail(mail) ) {
            accessError( mailboxList->mailbox(DraftsString) );
            return;
        }
    } else {

       // mail.encodeMail();

        // two possibilities, mail was originally from outbox but is now a draft, or
        // the mail was in the drafts folder previously as well.
        if ( mailboxList->mailbox(OutboxString)->email( mail.uuid() ) != NULL ) {
            mailboxList->mailbox(OutboxString)->removeMail( mail.uuid(), false );
        }

        if ( !mailboxList->mailbox(DraftsString)->addMail( mail ) ) {
            accessError( mailboxList->mailbox(DraftsString) );
            return;
        }
    }

    folderView->changeToSystemFolder( DraftsString );
    showMessageList();

    emit raiseWidget(this, appTitle );
}

/*  Mark a message as replied/repliedall/forwarded  */
void EmailClient::mailResponded()
{
    if ( !repliedFromUuid.isNull() ) {
        QString mailbox = InboxString;  //default search path
        Folder *folder = folderView->currentFolder();
        if ( folder )
            mailbox = folder->mailbox();    //could be trash, etc..

        Email *mail;
        QListIterator<Email*> it = mailboxList->mailbox( mailbox)->entryIterator();
        while ( it.hasNext() ) {
            mail = it.next();
            if ( mail->uuid() == repliedFromUuid ) {
                mail->setStatus(repliedFlags, true );
                mail->setDirty( true );
                break;
            }
        }

        repliedFromUuid = QUuid();
        repliedFlags = 0;
    }
}

/*  Find an appropriate account for the mail and format
    the mail accordingly    */
MailAccount* EmailClient::smtpForMail(Email *mail)
{
    mail->setReplyTo( "" );

    /*  Let's see if we the emailAddress matches a SMTP account */
    MailAccount *account = accountList->getSmtpRefByMail( mail->fromEmail() );
    if ( account != NULL ) {
        mail->setFromAccount( account->id() );
        return account;
    }

    /*  Let's try using a default account instead */
    account = accountList->defaultMailServer();
    if ( account != NULL ) {
        return account;
    }

    /* No default either.  Try any and setup a reply-to */
    account = accountList->getSmtpRef();
    if ( account != NULL ) {
        mail->setReplyTo( mail->fromEmail() );
        qWarning( QString( mail->fromEmail() + " used as reply-to" ).toLatin1() );
        mail->setFromAccount( account->id() );
        return account;
    }

    /*  No SMTP-account defined */
    return NULL;
}

// send all messages in outbox, by looping through the outbox, sending
// each message that belongs to the current found account
void EmailClient::sendAllQueuedMail()
{
    Email *mail;

    bool needAccount = false;
    QListIterator<Email*> it = mailboxList->mailbox( OutboxString )->entryIterator();

    while ( it.hasNext() ) {
        mail = it.next();
        if ( mail->type() & MailMessage::Email )
            needAccount = true;
    }

    if ( needAccount && !verifyAccounts(true) )
    {
        qWarning("Queued mail requires valid email accounts but none available.");

        moveOutboxMailsToDrafts();
        return;
    }

    QList<Email*> queuedMessages;
    queuedUuids.clear();
    smtpAccount = NULL;

    it = mailboxList->mailbox(OutboxString)->entryIterator();
    while ( it.hasNext() ) {
        mail = it.next();
        //mail not previously sent, and not unfinished, add to queue

        if ( !mail->status(EFlag_Sent) && !mail->unfinished() && (mail->hasRecipients()) ) {
            /* The first mail determines which range of mails to first
               send.  As we allow use of several SMTP accounts we may
               need more than one connection, but the total number of connections
               needed will never exceed the number of SMTP accounts
            */
            if ( smtpAccount == NULL ) {
                smtpAccount = smtpForMail( mail );
                queuedMessages.append(mail);
                queuedUuids.append( mail->uuid() );
            } else if ( smtpForMail(mail) == smtpAccount ) {
                queuedMessages.append(mail);
                queuedUuids.append( mail->uuid() );
            }
        }

    }

    if (queuedMessages.count() > 0) {
        emailHandler->setSmtpAccount(smtpAccount);
        sending = true;
//      sendMailButton->setVisible(false);
        cancelButton->setVisible(true);
        if (!receiving)
            queueStatus = 3;

        sendSingle = false;
        isSending(true);
        qLog(Messaging) << "Registering transfer task";
        QtopiaApplication::instance()->registerRunningTask(QLatin1String("transfer"));
        emailHandler->sendMail(&queuedMessages);
    } else {
        qWarning("no more messages to send");
    }
}

void EmailClient::sendSingleMail(Email *mail)
{
    if (sending) {
        qWarning("sending in progress, no action performed");
        return;
    }

    bool needAccount = false;
    if ( mail->type() & MailMessage::Email )
        needAccount = true;

    if ( needAccount && !verifyAccounts(true) ) {
        qWarning("Mail requires valid email accounts but none available.");

        moveOutboxMailsToDrafts();
        return;
    }

    QList<Email*> queuedMessages;
    queuedUuids.clear();

    smtpAccount = smtpForMail( mail );
    queuedMessages.append(mail);
    queuedUuids.append( mail->uuid() );
    emailHandler->setSmtpAccount(smtpAccount);

    sending = true;
//     sendMailButton->setVisible(false);
    cancelButton->setVisible(true);
    if (!receiving)
        queueStatus = 3;

    sendSingle = true;
    isSending(true);
    qLog(Messaging) << "Registering transfer task";
    QtopiaApplication::instance()->registerRunningTask(QLatin1String("transfer"));;
    emailHandler->sendMail(&queuedMessages);
}

bool EmailClient::verifyAccounts(bool outgoing)
{
    bool ok = true;

    if (accountList->count() == 0) {
        QMessageBox box(tr( "No account selected" ), tr("<qt>You must create an account</qt>"), QMessageBox::Warning,
                        QMessageBox::Ok | QMessageBox::Default , QMessageBox::NoButton, QMessageBox::NoButton );
        box.exec();
        ok = false;
    } else if (outgoing && accountList->getSmtpRef() == NULL) {
#ifdef QTOPIA_PHONE
        QMessageBox box(tr("No SMTP Server"), tr("<qt>No valid SMTP server defined.<br><br>No emails could be sent.</qt>"), QMessageBox::Warning,
                        QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton, QMessageBox::NoButton,
                        qApp->activeWindow() );
#else
        QMessageBox box(tr("No SMTP Server"), tr("<qt>No valid SMTP server defined</qt>"), QMessageBox::Warning,
                        QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton, QMessageBox::NoButton );
#endif
        box.exec();
        ok = false;
    } else if ( !outgoing && mailAccount == NULL ) {
        QMessageBox box(tr("No POP or IMAP accounts defined"), tr("<qt>Get Mail only works with POP or IMAP</qt>"), QMessageBox::Warning,
                        QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton, QMessageBox::NoButton );
        ok = false;
    }

    return ok;
}

//some mail are obviously sent, but are all mail in the outbox sent
void EmailClient::mailSent(int count)
{
    Email *mail;

    if (count == -1) {
        sending = false;
//      sendMailButton->setVisible(true);
        cancelButton->setVisible(false);
        isSending(false);
        return;
    }

    EmailFolderList *mailbox = mailboxList->mailbox(OutboxString);

    QListIterator<QUuid> qit( queuedUuids );
    while ( qit.hasNext() ) {
        mail = mailbox->email( qit.next() );
        if ( mail ) {
            mail->setStatus(EFlag_Sent, true);
            mail->setDirty( true );

            if ( !moveMailToFolder(mail, mailbox, mailboxList->mailbox(SentString) ) )
                break;      //no point continuing to move
        }
    }

    if ( !sendSingle ) {
        //loop through, if not all messages sent, start over
        QListIterator<Email*> it = mailboxList->mailbox(OutboxString)->entryIterator();
        while ( it.hasNext() ) {
            mail = it.next();
            if ( !mail->status(EFlag_Sent) ) {
                sendAllQueuedMail();
                return;
            }
        }
    }

    queuedUuids.clear();

    sending = false;
//     sendMailButton->setVisible(true);
    cancelButton->setVisible(false);
    isSending(false);
}

void EmailClient::addMailToDownloadList(Email *mail)
{
    if ( mail->status(EFlag_Downloaded) || mail->fromAccount() != mailAccount->id() )
        return;

    if ( (mailAccount->maxMailSize() > -1) && (mail->size() > (uint) ( mailAccount->maxMailSize() * 1024 ) ) )
        return;

    if ( mailAccount->accountType() == MailAccount::IMAP ) {
        Mailbox *box = mailAccount->getMailboxRef( mail->fromMailbox() );
        if ( box ) {
            FolderSyncSetting fs = box->folderSync();
            if ( fs & Sync_OnlyHeaders ) {
                return;
            } else if ( fs & Sync_OnlyNew ) {
                if ( mail->status(EFlag_IMAP_Seen) )
                    return;
            }
        }
    }

    mailDownloadList.sizeInsert(mail->serverUid(), mail->size(), mail->uuid(), mail->fromMailbox() );
}

void EmailClient::getNewMail()
{
    if ( !verifyAccounts(false) )
        return;

    qLog(Messaging) << "Registering transfer task";
    QtopiaApplication::instance()->registerRunningTask(QLatin1String("transfer"));
    receiving = true;
    previewingMail = true;
    updateGetMailButton(false);
    cancelButton->setVisible(true);
    selectAccountMenu->setEnabled(false);

    statusLabelHeader = mailAccount->accountName();

    //get any previous mails not downloaded and add to queue
    mailDownloadList.clear();
    Email *mail;
    QListIterator<Email*> it = mailboxList->mailbox(InboxString)->entryIterator();
    while ( it.hasNext() ) {
        mail = it.next();
        if ( !mail->status(EFlag_Downloaded) )
            addMailToDownloadList( mail );
    }
    emailHandler->setMailAccount(mailAccount);
    if (!sending)
        queueStatus = 1;

    quitSent = false;
    emailHandler->getMailHeaders();
    isReceiving(true);
}

void EmailClient::getAllNewMail()
{
    allAccounts = true;
    accountIdCount = 0;
    mailAccount = accountList->at(accountIdCount);

    while ( mailAccount != NULL ) {
        if ( !mailAccount->canCollectMail() ) {
            accountIdCount++;
            mailAccount = accountList->at(accountIdCount);
        } else
            break;
    }

    getNewMail();
}

void EmailClient::getSingleMail(Email *mail)
{
    if (receiving) {
        QString user = mailAccount->id();
        if ( user == mail->fromAccount() ) {
            mailDownloadList.append(mail->serverUid(), mail->size(), mail->uuid(), mail->fromMailbox() );
            setTotalPopSize( mailDownloadList.size() );
        } else {
            qWarning("receiving in progress, no action performed");
        }
        return;
    }
    mailAccount = accountList->getPopRefByAccount( mail->fromAccount() );
    if (mailAccount == NULL) {
        QString temp = tr("<qt>Mail was retrieved from account %1<br>Redefine this account to get this mail</qt>").arg(mail->fromAccount()) + "</qt>";
        QMessageBox::warning(qApp->activeWindow(),tr("Account not defined"),
            temp, tr("OK"));
        return;
    }

    qLog(Messaging) << "Registering transfer task";
    QtopiaApplication::instance()->registerRunningTask(QLatin1String("transfer"));

    receiving = true;
    previewingMail = false;
    allAccounts = false;
    updateGetMailButton(false);
    cancelButton->setVisible(true);
    selectAccountMenu->setEnabled(false);

    statusLabelHeader = mailAccount->accountName();

    mailDownloadList.clear();
    mailDownloadList.sizeInsert(mail->serverUid(), mail->size(), mail->uuid(), mail->fromMailbox() );
    emailHandler->setMailAccount(mailAccount);
    quitSent = false;
    setTotalPopSize( mailDownloadList.size() );

    isReceiving(true);
    emailHandler->getMailByList(&mailDownloadList, true);
}

void EmailClient::unresolvedUidlArrived(QString &user, QStringList &list)
{
    Email *mail;
    QString msg = tr("<qt>%1<br>The following messages have been deleted "
                     "from the server by another email client and can not be completed:<br>").arg(user);

    QString mailList = "";

   QListIterator<Email*> it = mailboxList->mailbox(InboxString)->entryIterator();
   while ( it.hasNext() ) {
        mail = it.next();
        if ( ( !mail->status(EFlag_Downloaded) ) && (mail->fromAccount() == mailAccount->id() ) ) {
            if ( (list.contains( mail->serverUid() ) ) ) {
                mailList += mail->fromName() + " - "  + mail->subject() + "<br>";
            }
        }
    }

    QMessageBox::warning(qApp->activeWindow(),
        tr("Unresolved mail"), msg + mailList + "</qt>", tr("OK"));

}

void EmailClient::readReplyRequested(Email* mail)
{
#ifdef QTOPIA_PHONE
# ifndef QTOPIA_NO_MMS
    QString netCfg;
    QListIterator<MailAccount*> it = accountList->accountIterator();
    while (it.hasNext()) {
        MailAccount *account = it.next();
        if (account->accountType() == MailAccount::MMS) {
            netCfg = account->networkConfig();
            break;
        }
    }
    QSettings mmsConf(netCfg, QSettings::IniFormat);
    mmsConf.beginGroup("MMS");
    if (mmsConf.value("AllowDeliveryReport", "n").toString() == "y") {
        QString msg(tr("<qt>Do you wish to send a Read Reply?</qt>"));
        if (QMessageBox::information(this, tr("Multimedia Message"), msg,
            QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                Email rrmail;
                rrmail.setType(MailMessage::MMS);
                rrmail.setTo(QStringList(mail->from()));
                rrmail.setSubject(mail->subject());
                rrmail.setExtraHeader("X-Mms-Message-Class", "Auto");
                rrmail.setExtraHeader("X-Mms-Delivery-Report", "No");
                rrmail.setExtraHeader("X-Mms-Read-Reply", "No");
                QString msg = tr("Sent MMS \"%1\" was read on: %2", "%1 = subject %2 = date");
                msg = msg.arg(mail->subject());
                msg = msg.arg(QDateTime::currentDateTime().toString());
                MailMessagePart part;
                part.setContentType("text/plain");
                part.setEncodedBody(msg, EightBit);
                rrmail.addMessagePart(part);
                rrmail.setStatus(EFlag_Outgoing | EFlag_Downloaded, true);
                rrmail.setUuid( mailboxList->mailbox(OutboxString)->generateUuid() );
                rrmail.encodeMail(); //TODO remove this for mms
                if ( !mailboxList->mailbox(OutboxString)->addMail(rrmail) ) {
                    accessError(mailboxList->mailbox(OutboxString) );
                    return;
                }
                sendSingleMail(&rrmail);
            }
    }
# endif
#endif
    Q_UNUSED(mail);
}

/*  This function is basically here to ensure the header only
    mail is replaced so that the ListWidget remains the same */
void EmailClient::mailUpdated(Email *entry, const QString &mailbox)
{
    if(readMailWidget()->isVisible() )
        readMailWidget()->mailUpdated( entry );
    updateQuery( entry, mailbox );
    updateFolderCount( mailbox );
}

void EmailClient::mailRemoved(const QUuid &uuid, const QString &mailbox)
{
    Folder *folder = folderView->currentFolder();
    if (!folder)
        return;

    if ( folder->mailbox() == mailbox ) {

        EmailListItem *item = messageView()->getRef( uuid );
        if ( item ) {
            EmailListItem *newItem = item;
            newItem = 0;
            int row = messageView()->row( item );
            if (row > 0) //try below
                newItem = (EmailListItem *)messageView()->item( row - 1, 0 );
            if (newItem == NULL) //try above
                newItem = (EmailListItem *)messageView()->item( row + 1, 0 );

            messageView()->removeRow( messageView()->row( item ) );
            messageView()->setFocus();
            if ( newItem ) {
                messageView()->clearSelection();
                messageView()->setItemSelected( newItem, true );
                messageView()->setCurrentItem( newItem );
            }

            setEnableMessageActions( (bool)messageView()->rowCount() );

            if ( readMailWidget()->isVisible() ) {
                readMailWidget()->update( messageView() );
            }
        } else {
            qWarning("hmm, message already removed from view?? ");
        }
    }

    updateFolderCount( mailbox );       // Need to update count of associated folder in folderlistview
}

/*  Mail arrived from server, treated a bit differently than from disk */
void EmailClient::mailArrived(const Email &m)
{   
    //make sure mailbox is connect at this point
    openFiles();
    
    Email mail(m);

    {
        QtopiaIpcEnvelope e( "QPE/TaskBar", "setLed(int,bool)" );
        e << LED_MAIL << true;
    }

    qLog(Messaging) << "Mail arrived";

    // means this is the first time we encounter this mail
    if ( mail.uuid().isNull() ) {
        mail.setUuid( mailboxList->mailbox(InboxString)->generateUuid() );
    }
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_MMS)
    bool readNow = false;
    bool getNow = false;
#endif

    /*  Test for get-this-mail activated on mail in trash.  Replace
        mail in trash for consistency sake  */
    if ( mail.status(EFlag_Downloaded) ) {
        if ( mailboxList->mailbox(TrashString)->email( mail.uuid() ) ) {
            if ( !mailboxList->mailbox(TrashString)->addMail(mail) ) {
                accessError( mailboxList->mailbox(TrashString) );
            }
            return;
        }
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_MMS)
        QString mmsType = mail.header("X-Mms-Message-Type");
        if (mmsType.contains("m-delivery-ind")) {
            QString msg;
            QString mmsStatus = mail.header("X-Mms-Status");
            qLog(Messaging) << "X-MMS-Status:" << mmsStatus;
            if (mmsStatus.contains("Retrieved")) {
                msg = tr("<qt>Multimedia message delivered to %1.</qt>");
            } else if (mmsStatus.contains("Rejected")) {
                msg = tr("<qt>Multimedia message rejected by %1.</qt>");
            } else if (mmsStatus.contains("Deferred")) {
                msg = tr("<qt>Multimedia message deferred by %1.</qt>");
            } else if (mmsStatus.contains("Expired")) {
                msg = tr("<qt>Multimedia message to %1 expired.</qt>");
            }
            QString to = mail.header("To");
            if (to.isEmpty())
                to = tr("Unspecified", "MMS recipient");
            QMessageBox::information(this, tr("Multimedia Message"), msg.arg(to),
                        QMessageBox::Yes, QMessageBox::NoButton);
            return;
        }
#endif
    } else if (!mail.header("X-Mms-Message-Type").trimmed().isEmpty()) {
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_MMS)
        static QDialog *newMMSDlg = 0;
        static QLabel *newMMSLabel = 0;
        static QListWidget *newMMSListBox = 0;
        MailAccount *account = accountList->getAccountById(mail.fromAccount());
        if (newMMSDlg) {
            // We're already showing the new message dialog.
            // Change to a generic message and accept this mail immediately.
            QString msg(tr("You have new multimedia messages."));
            newMMSLabel->setText(msg);
            newMMSListBox->model()->removeRow(2);
            emailHandler->acceptMail(mail);
//            getNow = true; //XXX read from settings.
        } else {
            QString msg(tr("You have a new multimedia message."));
            msg += "<p><small>";
            if (!mail.subject().isEmpty())
                msg += tr("Subject: <b>%1</b><br>").arg(mail.subject());
            if (!mail.from().isEmpty())
                msg += tr("From: %1<br>").arg(mail.from());
            msg += tr("%1 KBytes").arg(QString::number((mail.size()+1023)/1024));
            msg += "</small>";
            newMMSDlg = new QDialog(this, 0);
            newMMSDlg->setModal(true);
            newMMSDlg->setWindowTitle(tr("New MMS"));
            QVBoxLayout *vb = new QVBoxLayout(newMMSDlg);
            vb->setMargin(4);
            newMMSLabel = new QLabel(newMMSDlg);
            newMMSLabel->setTextFormat(Qt::RichText);
            newMMSLabel->setWordWrap(true);
            newMMSLabel->setText(msg);
            vb->addWidget(newMMSLabel);
            vb->addStretch(1);
            newMMSListBox = new QListWidget(newMMSDlg);
            vb->addWidget(newMMSListBox);
            newMMSListBox->addItem(tr("Read now"));
            newMMSListBox->addItem(tr("Read later"));
            newMMSListBox->addItem(tr("Reject message"));
            newMMSListBox->setCurrentRow(0);
            connect(newMMSListBox, SIGNAL(itemActivated(QListWidgetItem*)), newMMSDlg, SLOT(accept()));
            newMMSDlg->showMaximized();

            QtopiaApplication::setMenuLike(newMMSDlg, true);
            QtopiaApplication::execDialog(newMMSDlg);

            if (newMMSDlg->result() == QDialog::Accepted) {
                if (newMMSListBox->currentRow() == 0) {
                    // read now
                    readNow = true;
                } else if (newMMSListBox->currentRow() == 2) {
                    // reject
                    emailHandler->rejectMail(mail);
                    delete newMMSDlg;
                    newMMSDlg = 0;
                    return;
                } else {
                    // read later
                    QSettings mailconf("Trolltech","qtmail");
                    mailconf.beginGroup("MMS");
                    int count = mailconf.value("newMmsCount").toInt() + 1;
                    mailconf.setValue("newMmsCount", count);
                    mailconf.endGroup();
                    QtopiaIpcEnvelope e("QPE/System", "newMmsCount(int)");
                    e << count;
                    if (account) {
                        getNow = account->autoDownload();
                    }
                }
            }
            delete newMMSDlg;
            newMMSDlg = 0;
            emailHandler->acceptMail(mail);
        }
#endif
    }

    if ( !mailboxList->mailbox(InboxString)->addMail(mail) ) {
        cancel();
        accessError( mailboxList->mailbox(InboxString) );
    }

#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_MMS)
    if (readNow) {
        autoDownloadMail = true;
        delayedShowMessage(MailAccount::MMS, mail.uuid(), false);
    } else if (getNow && !mail.status(EFlag_Downloaded)) {
        getSingleMail(&mail);
    } else
#endif
    if ( previewingMail ) {
        addMailToDownloadList( &mail );
    }
}

// Called two times.  After all headers are fetched and
// after all mails have been picked by list.
void EmailClient::allMailArrived(int)
{
    // not previewing means all mailtransfer has been done
    if (!previewingMail) {

        // close current connection
        if ( !quitSent) {
            quitSent = true;
            emailHandler->popQuit();
            return;
        }

        getNextNewMail();
        return;
    }

    // all headers downloaded from server, start downloading remaining mails
    accountList->saveAccounts();
    previewingMail = false;
    statusLabelHeader = mailAccount->accountName();

    setTotalPopSize( mailDownloadList.size() );
    emailHandler->setMailAccount(mailAccount);
    emailHandler->getMailByList(&mailDownloadList, false);
}

void EmailClient::getNextNewMail()
{
    // must use a counter, since several other functions may mess
    // with the current item in accountlist
    accountIdCount++;
    if (accountList->count() <= accountIdCount) {
        mailAccount = 0;
    } else {
        mailAccount = accountList->at(accountIdCount);
        if ( !mailAccount->canCollectMail() ) {
            getNextNewMail();
            return;
        }
    }

    if ( (allAccounts) && (mailAccount != 0) ) {
        getNewMail();
        if (queueStatus == 1) {
            progressBar->reset();
            progressBar->setText("");
        }
    } else {
        allAccounts = false;
        receiving = false;
        autoGetMail = false;
        updateGetMailButton(true);
        cancelButton->setVisible(false);
        selectAccountMenu->setEnabled(true);
        statusLabelHeader = "";

        if (queueStatus == 1) {
            progressBar->reset();
            progressBar->setText("");
        }

        Folder *folder = folderView->currentFolder();
        if ( folder )
            updateFolderCount( folder->mailbox() );
        isReceiving(false);
    }
}

void EmailClient::moveMailFront(Email *mailPtr)
{
    if ( !mailPtr->status(EFlag_Incoming) || mailPtr->status(EFlag_Downloaded) )
        return;

    if ( (receiving) && (mailPtr->fromAccount() == mailAccount->id() ) )
        mailDownloadList.moveFront( mailPtr->serverUid() );
}

void EmailClient::smtpError(int code, QString &msg)
{
    QString temp = tr("<qt>Server: ") + smtpAccount->smtpServer() + "<br><br>";

    if (code == ErrUnknownResponse) {
        temp += tr("Unexpected response from server:<br><br>");
        QStringList list;
        list = msg.split(' ');
        int len = 0;
        for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
            if ( (*it).length() + len > 35 ) {
                temp +="\n";
                len = 0;
            }
            temp += *it + " ";
            len += (*it).length();
        }
    }

    if (code == QAbstractSocket::ConnectionRefusedError)
        temp += tr( "Connection refused" );
    if (code == QAbstractSocket::RemoteHostClosedError)
        temp += tr( "Remote host closed the connection" );
    if (code == QAbstractSocket::HostNotFoundError)
        temp += tr( "Host not found" );
    if (code == QAbstractSocket::SocketAccessError)
        temp += tr( "Permission denied" );
    if (code == QAbstractSocket::SocketResourceError)
        temp += tr( "Insufficient resources" );
    if (code == QAbstractSocket::SocketTimeoutError)
        temp += tr( "Operation timed out" );
    if (code == QAbstractSocket::DatagramTooLargeError)
        temp += tr( "Datagram too large" );
    if (code == QAbstractSocket::NetworkError)
        temp += tr( "Network error" );
    if (code == QAbstractSocket::AddressInUseError)
        temp += tr( "Address in use" );
    if (code == QAbstractSocket::SocketAddressNotAvailableError)
        temp += tr( "Address not available" );
    if (code == QAbstractSocket::UnsupportedSocketOperationError)
        temp += tr( "Unsupported operation" );
    if (code == QAbstractSocket::UnknownSocketError)
        temp += tr( "Unknown error" );

    temp += "</qt>";

    if (code != ErrCancel) {
        QMessageBox::warning(qApp->activeWindow(), tr("Sending error"), temp, tr("OK") );
        progressBar->reset();
    } else {
        progressBar->setText( tr("Aborted by user") );
    }

    sending = false;
    isSending(false);
//     sendMailButton->setVisible(true);
    cancelButton->setVisible(false);
    queuedUuids.clear();
}

void EmailClient::popError(int code, QString &msg)
{
    QString temp = tr("<qt>Server: ") + mailAccount->mailServer() + "<br><br>";

    if (code == ErrUnknownResponse) {
        temp += tr("Unexpected response from server:<br><br>" );
        QStringList list;
        list = msg.split(' ');
        int len = 0;
        for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
            if ( (*it).length() + len > 35 ) {
                temp +="\n";
                len = 0;
            }
            temp += *it + " ";
            len += (*it).length();
        }
    }

    if (code == ErrLoginFailed)
        temp += tr("Login failed. Check user name and password");
    if (code == ErrFileSystemFull)
        temp += tr("Mail check failed.") + QLatin1String(" ") + LongStream::errorMessage();
    if (code == QAbstractSocket::ConnectionRefusedError)
        temp += tr( "Connection refused" );
    if (code == QAbstractSocket::RemoteHostClosedError)
        temp += tr( "Remote host closed the connection" );
    if (code == QAbstractSocket::HostNotFoundError)
        temp += tr( "Host not found" );
    if (code == QAbstractSocket::SocketAccessError)
        temp += tr( "Permission denied" );
    if (code == QAbstractSocket::SocketResourceError)
        temp += tr( "Insufficient resources" );
    if (code == QAbstractSocket::SocketTimeoutError)
        temp += tr( "Operation timed out" );
    if (code == QAbstractSocket::DatagramTooLargeError)
        temp += tr( "Datagram too large" );
    if (code == QAbstractSocket::NetworkError)
        temp += tr( "Network error" );
    if (code == QAbstractSocket::AddressInUseError)
        temp += tr( "Address in use" );
    if (code == QAbstractSocket::SocketAddressNotAvailableError)
        temp += tr( "Address not available" );
    if (code == QAbstractSocket::UnsupportedSocketOperationError)
        temp += tr( "Unsupported operation" );
    if (code == QAbstractSocket::UnknownSocketError)
        temp += tr( "Unknown error" );

    temp += "</qt>";

    if (code != ErrCancel) {
        if ( !autoGetMail ) {
            QMessageBox::warning(qApp->activeWindow(), tr("Receiving error"), temp, tr("OK") );
        } else {
            progressBar->setText( tr("Automatic Fetch failed") );
        }
    } else {
        progressBar->setText(tr("Aborted by user"));
    }

    getNextNewMail();
}

void EmailClient::smsError(int code, QString &msg)
{
    Q_UNUSED(code)
    QString temp(tr("<qt>Failed sending SMS: %1</qt>", "%1 will contain the reason for the failure"));
    QMessageBox::warning(qApp->activeWindow(), tr("Sending error"),
                temp.arg(msg), tr("OK") );

    sending = false;
    isSending(false);
//     sendMailButton->setVisible(true);
    cancelButton->setVisible(false);
    queuedUuids.clear();
}

void EmailClient::mmsError(int code, QString &msg)
{
#ifndef QTOPIA_NO_MMS
    Q_UNUSED(code)
    if (sending) {
        QString temp(tr("<qt>Failed sending MMS: %1</qt>", "%1 will contain the reason for the failure"));
        QMessageBox::warning(qApp->activeWindow(), tr("Sending error"),
                temp.arg(msg), tr("OK") );

        sending = false;
        isSending(false);
//      sendMailButton->setVisible(true);
        queuedUuids.clear();
    } else if (receiving) {
        QString temp(tr("<qt>Failed receiving MMS: %1</qt>", "%1 will contain the reason for the failure"));
        QMessageBox::warning(qApp->activeWindow(), tr("Receiving error"),
                temp.arg(msg), tr("OK") );
        receiving = false;
        autoGetMail = false;
        isReceiving(false);
        updateGetMailButton(true);
        cancelButton->setVisible(false);
        selectAccountMenu->setEnabled(true);
    }
    cancelButton->setVisible(false);
#else
    Q_UNUSED(code)
    Q_UNUSED(msg)
#endif
}


void EmailClient::queryItemSelected()
{
    EmailListItem *item = (EmailListItem*) messageView()->currentItem();
    if (item == NULL)
        return;
    if (!messageView()->isItemSelected(item))
        return;

    if (messageView()->currentMailbox() == DraftsString) {
        modify(item->mail());
        return;
    }
    emit raiseWidget( readMailWidget(), tr("Read mail") );
    mReadMail->update( messageView() );
    if (autoDownloadMail) {
        if (!item->mail()->status(EFlag_Downloaded))
            getSingleMail(item->mail());
        autoDownloadMail = false;
    }

    QtopiaIpcEnvelope e( "QPE/TaskBar", "setLed(int,bool)" );
    e << LED_MAIL << false;

    // Reading a mail resets the new mail count
    QListIterator<MailAccount*> it = accountList->accountIterator();
    while ( it.hasNext() ) {
        MailAccount *acc = it.next();
        Client *client = emailHandler->clientFromAccount(acc);
        if (client) {
            client->resetNewMailCount();
        } else if (acc->accountType() == MailAccount::System ) {
            QSettings mailconf("Trolltech", "qtmail");
            mailconf.beginGroup("SystemMessages");
            int count = mailconf.value("newSystemCount").toInt();

            if (count) {
            count =0;
            mailconf.setValue("newSystemCount", count );
            QtopiaIpcEnvelope e("QPE/System", "newSystemCount(int)");
            e << count;
            }
        }
    }

#if 0
    bool allread = true;
    QListIterator<Email> it = mailboxList->mailbox("inbox")->entryIterator();
    for ( ; it.current(); ++it) {
        EmailListItem *item = (EmailListItem *)it.current();
        if ( !it.current()->mail()->read() ) {
            allread = false;
            break;
            }
        }
        if ( allread ) {
            QtopiaIpcEnvelope e( "QPE/TaskBar", "setLed(int,bool)" );
            e << LED_MAIL << false;
        }
#endif
}

void EmailClient::mailFromDisk(Email *mail, const QString &mailbox)
{
    updateQuery( mail, mailbox );
    updateFolderCount( mailbox );
}

void EmailClient::mailMoved(Email* m, const QString& sourceBox, const QString& destBox)
{
    mailRemoved(m->uuid(),sourceBox);
    updateQuery(m,destBox);
    updateFolderCount(destBox);
}

void EmailClient::readMail()
{
    QTime s;
    s.start();

    mailboxList->openMailboxes();

    QStringList mboxList = mailboxList->mailboxes();
    for (QStringList::Iterator it = mboxList.begin(); it != mboxList.end(); ++it) {
        updateFolderCount( *it, true );
    }
}

void EmailClient::accessError(EmailFolderList *box)
{
    QString mailbox = "mailbox"; // No tr

    if ( box )
        mailbox = mailboxTrName( box->mailbox() );

    QString msg = tr("<qt>Cannot access %1. Either there is insufficient space, or another program is accessing the mailbox.</qt>").arg(mailbox);

    QMessageBox::critical( 0, tr("Unable to save mail"), msg );
}

void EmailClient::moveError(const EmailFolderList& source, const EmailFolderList& dest)
{
    QString mailbox1 = mailboxTrName( source.mailbox() );
    QString mailbox2 = mailboxTrName( dest.mailbox() );

    QString msg = tr("<qt>Cannot move mail from %1 to %2. Either there is insufficient space, or another program is accessing the mailboxes.</qt>").arg(mailbox2).arg(mailbox2);

    QMessageBox::critical( 0, tr("Unable to move mail"), msg );
}

void EmailClient::readSettings()
{
    int y;
    QSettings mailconf("Trolltech","qtmail");
    mailconf.beginGroup("qtmailglobal");

    folderView->readQueries("qtmailglobal", &mailconf);

    if (( y = mailconf.value("mailidcount", -1).toInt()) != -1) {
        mailIdCount = y;
    }

    int page = mailconf.value( "currentpage", 0).toInt();
    if (page != -1 && mailboxView) {
        setCurrentMailboxWidget( page );
        if ( page == 0 )
            folderView->setFocus();
        else
            messageView()->setFocus();
    }

    mailconf.endGroup();

    mailconf.beginGroup("settings");
    QFont font;
    if ( mailconf.value("font").toString() == "large") {
        font.setPointSize( font.pointSize() + 4 );      // 4 larger than default
//      messageView()->setFont( font );
        folderView->setFont( font );
    } else {
//      messageView()->setFont( font );
        folderView->setFont( font );
    }

    int val = mailconf.value("interval", -1 ).toInt();
    if ( val == -1 ) {
        fetchTimer.stop();
    } else {
        fetchTimer.start( val * 60 * 1000);
    }
    mailconf.endGroup();
}

bool EmailClient::saveSettings()
{
    QSettings mailconf("Trolltech","qtmail");

    mailconf.beginGroup("qtmailglobal");
    mailconf.remove("");
    mailconf.setValue("mailidcount", mailIdCount);

    folderView->saveQueries("qtmailglobal", &mailconf);

    mailconf.endGroup();

    mailconf.beginGroup("qtmailglobal");

    if ( mailboxView )
        mailconf.setValue( "currentpage", currentMailboxWidgetId() );

    messageView()->writeConfig( &mailconf );

    EmailListItem *item = (EmailListItem *) messageView()->currentItem();
    if ( item ) {
        QUuid id;
        Email *mail = item->mail();
        if ( mail )
            id = mail->uuid();

        mailconf.setValue("currentmail", id.toString() );
    }

    mailconf.endGroup();
    return true;
}

void EmailClient::selectAccount(int id)
{
    if ( queuedAccountIds.contains( id ) )
        return;
    if ( receiving ) {
        queuedAccountIds.append( id );
        checkAccountTimer.start( 1 * 60 * 1000 );
        return;
    }

    if (accountList->count() > 0) {
        accountIdCount = id;
        mailAccount = accountList->at(id);
        allAccounts = false;
        getNewMail();
    }
}

void EmailClient::selectAccount(QAction* action)
{
    if (actionMap.contains(action))
        selectAccount(actionMap[action]);
}


void EmailClient::selectAccountTimeout()
{
    if ( receiving )
        return;
    if ( queuedAccountIds.isEmpty() ) {
        checkAccountTimer.stop();
        return;
    }

    int accountId = queuedAccountIds.first();
    queuedAccountIds.erase( queuedAccountIds.begin() );
    selectAccount( accountId );
}

void EmailClient::editAccount(int id)
{
#ifndef QTOPIA_PHONE    // we use a settings dialog
    MailAccount *newAccount;

    if (id == newAccountId) {           //new account
        newAccount = new MailAccount;
        newAccount->setUserName( accountList->getUserName() );
    } else {
        newAccount = accountList->at(id);
    }

    QDialog *editAccountView;
    EditAccount *e = new EditAccount(this, "create-new-account");
    e->(newAccount, id == newAccountId);
    editAccountView = (QDialog *)e;

#ifdef QTOPIA_DESKTOP
    int ret = editAccountView->exec();
#else
    int ret = QtopiaApplication::execDialog(editAccountView);
#endif
    delete editAccountView;

    switch ( ret ) {
        case QDialog::Accepted:
            if (id == newAccountId) {
                accountList->append(newAccount);
            }

            if ( newAccount->defaultMailServer() ) {
                if ( accountList->defaultMailServerCount() > 1 ) {
                    accountList->setDefaultMailServer( newAccount );

                    QMessageBox::warning(qApp->activeWindow(),
                        tr("New default account"),
                        tr("<qt>Your previous default mail account has been unchecked</qt>"),
                        tr("OK"));
                }
            }

            QTimer::singleShot(0, this, SLOT( updateAccounts() ) );
            folderView->updateAccountFolder( newAccount );
            accountList->saveAccounts();
            break;

        case 2:
            deleteAccount(id);
            break;
    }
#else
    Q_UNUSED( id );
#endif
}

void EmailClient::editAccount(QAction* action)
{
    if (actionMap.contains(action))
        editAccount(actionMap[action]);
}

void EmailClient::deleteAccount(int id)
{
#ifndef QTOPIA_PHONE
    MailAccount *newAccount;

    newAccount = accountList->at(id);
    if (newAccount) {
        QString message = tr("<qt>Delete account: %1</qt>").arg(newAccount->accountName());
        switch( QMessageBox::warning( this, appTitle, message,
            tr("Yes"), tr("No"), 0,     0, 1 ) ) {

            case 0:
            {
                folderView->deleteAccountFolder( accountList->at(id) );
                accountList->remove(id);
                QTimer::singleShot(0, this, SLOT( updateAccounts() ) );
                accountList->saveAccounts();
                break;
            }
            case 1: break;
        }
    }
#else
    Q_UNUSED( id );
#endif
}

void EmailClient::updateGetMailButton(bool enable)
{
    getMailButton->setVisible(false);
    if (!enable)
        return;
  
        // Enable send mail account if SMTP account exists
        //sendMailButton->setVisible( accountList->getSmtpRef() );
        QListIterator<MailAccount*> it = accountList->accountIterator();
        while ( it.hasNext() ) {
            MailAccount *account = it.next();
            if ( account->accountType() < MailAccount::SMS ) {
                // Enable send mail account if
                // POP, IMAP, or Synchronized account exists
                getMailButton->setVisible(true);
            }
        }
     
}

/*  Important:  If this method is called directly/indirectly from
    either configure or selectAccountMenu you will get a failure
    when mousemove/release/click events are tried being passed to
    invalid qmenudataitems. (invalid because this procedure clears them)
    Use QTimer:singleshot to dump the call after the mousevents
*/
void EmailClient::updateAccounts()
{
    queuedAccountIds.clear();
    newAccountId = -1;
#ifdef QTOPIA_PHONE
    updateGetMailButton(true);
//     sendMailButton->setVisible(false);
#else
    //rebuild menus, clear all first
    configure->clear();
    selectAccountMenu->clear();
    QMapIterator<QAction*, int> i(actionMap);
    while ( i.hasNext() ) {
        i.next();
        delete i.key();
    }
    actionMap.clear();
    QAction *action = configure->addAction( QIcon(":icon/new"), tr("New Account") );
    actionMap.insert( action, newAccountId );
    configure->addSeparator();

    int idCount = 0;
    QListIterator<MailAccount*> it = accountList->accountIterator();
    while ( it.hasNext() ) {
        QString accountName = it.next()->accountName();
        action = configure->addAction( accountName + "..." );
        actionMap.insert( action, idCount );

        if ( it.peekPrevious()->accountType() != MailAccount::Synchronized ) {
            action = selectAccountMenu->addAction( accountName );
            actionMap.insert( action, idCount );
        }

        idCount++;
    }
#endif

    // accounts has been changed, update writemailwidget if it's created
    if ( mWriteMail )
        mWriteMail->setAccountList( accountList );
}

void EmailClient::messageChanged()
{
    if (!messageView()->singleColumnMode())
        return;

    if ( currentMailboxWidgetId() != messageId )
        return;

    EmailListItem *item = (EmailListItem*)messageView()->currentItem();
    if (item) {
        QString statusString = EmailListItem::dateToString( item->mail()->dateTime() );
        setStatusText( statusString );
    }
}

/*  handles two primary cases.  When a mail being deleted from inbox/outbox view
        it is transferred to trash, and if from trash it is expunged  */
bool EmailClient::deleteMail(EmailListItem *mailItem)
{
    Email *mail = mailItem->mail();
    Folder *folder = folderView->currentFolder();

    if ( folder == NULL ) {
        qWarning("No folder selected, cannot delete mail");
        return false;
    }

    //if the client has "deleteImmediately" set, then do so now.
    MailAccount *account = accountList->getAccountById( mail->fromAccount() );
    Client *client = (account ? emailHandler->clientFromAccount(account) : 0);
    if ( client && client->hasDeleteImmediately() ) {
        client->deleteImmediately( mail->serverUid() );
    }

    // Deleting an unread mail resets the new mail count
    if (client && !mail->status(EFlag_Read))
        client->resetNewMailCount();

    //only request confirmation when expunging the mail from trash
    if ( folder->folderType() == FolderTypeSystem && folder->mailbox() == TrashString )  {
        // Add it to queue of mails to be deleted from server
        if ( mail->status(EFlag_Incoming) ) {
            MailAccount *account = accountList->getAccountById( mail->fromAccount() );
            if ( account != NULL ) {
         if ( account->deleteMail() ){
                    account->deleteMsg( mail->serverUid(), mail->fromMailbox() );
                }
            }
        }
        mailboxList->mailbox(TrashString)->removeMail( mail->uuid(), true );
        return true;
    }

    //if mail is in queue for download, remove it from queue if possible
    if ( folder->mailbox() == InboxString )  {
        if ( (receiving) && (mail->fromAccount() == mailAccount->id() ) ) {
            if ( !mail->status(EFlag_Downloaded) )
                mailDownloadList.remove( mail->serverUid() );
        }
    }

    moveMailToFolder( mailItem->mail(), mailboxList->mailbox( folder->mailbox() ), mailboxList->mailbox(TrashString) );

    return true;
}

/*  Be careful using routines similar to this one.  As soon as a corresponding
        email entry is removed, the mail cannot be swapped into memory */
bool EmailClient::moveMailToFolder(Email *mail, EmailFolderList *source, EmailFolderList *target)
{
    if ( source == target )
        return false;

    if(!source->moveMail(mail->uuid(),*target))
    {
        moveError(*source,*target);
        return false;
    }

//     // Automatic swapping currently not working
//     mail->readFromFile();
//
//     Email newMail = *mail;      // mail will be deleted when we call removeMail
//
//     if ( !target->addMail(newMail) ) {
//         accessError( target );
//         return false;
//     }
//
//     if ( !source->removeMail( mail->uuid(), false ) ) {
//         // Delete the copy we made
//         target->removeMail( newMail.uuid(), false );
//
//         accessError( source );
//         return false;
//     }

    return true;
}

void EmailClient::showItemMenu(EmailListItem *item)
{
    Q_UNUSED( item );

    Folder *folder = folderView->currentFolder();

    if ( folder == NULL )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( OutboxString );

    QMenu *popFolder = new QMenu(this);
    movePop->clear();
    copyPop->clear();
    moveMap.clear();
    QAction *action;
    uint pos = 0;
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        action = movePop->addAction(*pm_folder, mailboxTrName(*it) );
        moveMap.insert(action, pos);
        action = copyPop->addAction(*pm_folder, mailboxTrName(*it) );
        moveMap.insert(action, pos);

        pos++;
    }
    movePop->setTitle( tr("Move to") );
    copyPop->setTitle( tr("Copy to") );
    popFolder->addMenu( movePop );
    popFolder->addMenu( copyPop );
    popFolder->addAction(*pm_trash, tr("Delete mail"), this, SLOT( deleteMailItem() ) );
    popFolder->popup( QCursor::pos() );
}

//Can't delete message(s) because SIM is not detected/ready
void EmailClient::showDeleteNotReadyWarning()
{
    QMessageBox::warning(qApp->activeWindow(),
                         tr("SIM card not detected"),
                         tr("<qt>Unable to delete SMS messages, the "
                            "SIM card is not ready</qt>"),
                         tr("OK"));
}

void EmailClient::deleteMailItem()
{
     Folder *folder = folderView->currentFolder();
     EmailListItem *item = 0;
     if (!folder)
        return;

     if ( folder->folderType() == FolderTypeSystem &&
        folder->mailbox() == OutboxString &&
        sending) {
        return; //don't delete when sending
     }

     bool hasSms = false;
     QList<EmailListItem*> deleteList;
     for (int i = 0; i < messageView()->rowCount(); ++i) {
       item = (EmailListItem *) messageView()->item( i, 0 );
       if ( messageView()->isItemSelected(item) ) {
           if (!emailHandler->smsReadyToDelete()
               && item->mail()
               && (item->mail()->type() == MailMessage::SMS)) {
                hasSms = true;
           } else {
               deleteList.append( item );
           }
       }
     }
     if (!emailHandler->smsReadyToDelete() && hasSms)
         showDeleteNotReadyWarning();

     if ( deleteList.isEmpty() )
         return;

     if ( folder->folderType() == FolderTypeSystem &&
          folder->mailbox() == TrashString ) {
         QString prompt = tr( "the mail" );
         if (deleteList.count() > 1)
             prompt = tr( "the mails" );
         if ( !Qtopia::confirmDelete( this, tr("EMail"), prompt ) )
             return;
     }
     suspendMailCount = true;
     while (!deleteList.isEmpty())
         deleteMail( deleteList.takeFirst() );
     suspendMailCount = false;
     updateFolderCount(folder->mailbox());
     updateFolderCount(TrashString);
}


void EmailClient::moveMailItem(int index)
{
    Folder *folder = folderView->currentFolder();

    if ( folder->folderType() == FolderTypeSystem &&
         folder->mailbox() == OutboxString &&
         sending) {
        return; //don't delete when sending
    }

    QList<EmailListItem*> moveList;
    EmailListItem *item = 0;
    for (int i = 0; i < messageView()->rowCount(); ++i) {
       item = (EmailListItem *) messageView()->item( i, 0 );
       if ( messageView()->isItemSelected( item ) )
           moveList.append( item );
    }

    if ( moveList.isEmpty() )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList->mailboxes();
    list.removeAll( mailbox );
    list.removeAll(OutboxString);
    QString target = list[index];

    QListIterator<EmailListItem*> it(moveList);
    suspendMailCount = true;
    while ( it.hasNext() )
        moveMailToFolder(it.next()->mail(), mailboxList->mailbox(mailbox),
                         mailboxList->mailbox(target) );
    suspendMailCount = false;
    updateFolderCount(mailbox);
    updateFolderCount(target);
}

void EmailClient::moveMailItem(QAction *action)
{
    if (moveMap.contains(action))
        moveMailItem(moveMap[action]);
}

void EmailClient::copyMailItem(int index)
{
    Folder *folder = folderView->currentFolder();
    QList<EmailListItem*> copyList;
    EmailListItem *item = 0;
    uint size = 0;

    for (int i = 0; i < messageView()->rowCount(); ++i ) {
       item = (EmailListItem *) messageView()->item( i, 0 );
       if ( messageView()->isItemSelected( item ) ) {
           copyList.append( item );
           size += item->mail()->size();
       }
    }

    if (!LongStream::freeSpace( "", size + 1024*10 )) {
        QString title( tr("Copy error") );
        QString msg( "<qt>" + tr("Storage for messages is full.<br><br>Could not copy messages.") + "</qt>" );
        QMessageBox::warning(qApp->activeWindow(), title, msg, tr("OK") );
        return;
    }

    if ( copyList.isEmpty() )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( OutboxString );
    QString target = list[index];

    QListIterator<EmailListItem*> it(copyList);
    suspendMailCount = true;
    EmailFolderList* sourceBox = mailboxList->mailbox(mailbox);
    EmailFolderList* targetBox = mailboxList->mailbox(target);
    Q_ASSERT(sourceBox);
    Q_ASSERT(targetBox);
    while ( it.hasNext() ) {
       Email *mail = it.next()->mail();
       if(!sourceBox->copyMail(mail->uuid(),(*targetBox))){
           accessError(mailboxList->mailbox(target) );
       }
    }
    suspendMailCount = false;
    updateFolderCount(mailbox);
    updateFolderCount(target);
}

void EmailClient::copyMailItem(QAction *action)
{
    if (moveMap.contains(action))
        copyMailItem(moveMap[action]);
}

void EmailClient::moveMessage()
{
#ifdef QTOPIA_PHONE
    Folder *folder = folderView->currentFolder();
    if ( folder == NULL )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( OutboxString);
    SelectFolderDialog *selectFolderDialog = new SelectFolderDialog(list);
#ifdef QTOPIA_DESKTOP
    selectFolderDialog->exec();
#else
    QtopiaApplication::execDialog( selectFolderDialog );
#endif
    if (selectFolderDialog->result() &&
        selectFolderDialog->folder() != -1)
        moveMailItem( selectFolderDialog->folder() );
    delete selectFolderDialog;
#endif
}

void EmailClient::copyMessage()
{
#ifdef QTOPIA_PHONE
    Folder *folder = folderView->currentFolder();
    if ( folder == NULL )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( OutboxString);
    SelectFolderDialog *selectFolderDialog = new SelectFolderDialog(list);
#ifdef QTOPIA_DESKTOP
    selectFolderDialog->exec();
#else
    QtopiaApplication::execDialog( selectFolderDialog );
#endif
    if (selectFolderDialog->result() &&
        selectFolderDialog->folder() != -1)
        copyMailItem( selectFolderDialog->folder() );
    delete selectFolderDialog;
#endif
}

/* Select all messages */
void EmailClient::selectAll()
{
    for (int i = 0; i < messageView()->rowCount(); ++i)
        messageView()->setItemSelected( messageView()->item( i, 0 ), true );
}

/*  currently only allowed for trash   */
void EmailClient::emptyFolder()
{
    Email *mail;
    MailAccount *account;
    EmailFolderList *folderList;

    if (!folderView->currentFolder())
        return;
/*
    if ( !( folder->folderType() == FolderTypeSystem && folder->mailbox() == "trash") )
        return;
*/

    folderList = mailboxList->mailbox(TrashString);

    QString strName = tr("all messages in the trash");
    if (Qtopia::confirmDelete(this, appTitle, strName)) {
        QListIterator<Email*> it = folderList->entryIterator();
        while (it.hasNext()) {
            mail = it.next();

            if (mail->status( EFlag_Incoming)) {
                account = accountList->getAccountById(mail->fromAccount());
                if (account != NULL) {
                    if (account->deleteMail())
                        account->deleteMsg(mail->serverUid(), mail->fromMailbox());
                }
            }
        }

        folderList->empty();

        Folder *folder = folderView->currentFolder();
        if (folder
            && folder->folderType() == FolderTypeSystem
            && folder->mailbox() == TrashString)
            messageView()->clear();

        updateFolderCount(folderList->mailbox());
    }

    update();
}

void EmailClient::setTotalSmtpSize(int size)
{
    if (queueStatus == 1 && !receiving)
        queueStatus = 3;

    if (queueStatus == 3) {
        progressBar->reset();
        if(size != 0)
            progressBar->setRange(0,size);
    } else {
        totalSize = size;
    }
}

void EmailClient::setStatusText(QString &txt)
{
    progressBar->setText(txt);
}

void EmailClient::setTotalPopSize(int size)
{
    if (queueStatus == 3 && !sending)
        queueStatus = 1;

    if (queueStatus == 1) {
        progressBar->reset();
        if(size != 0)
            progressBar->setRange(0,size);
    } else {
        totalSize = size;
    }
}

void EmailClient::setDownloadedSize(int size)
{
    if (queueStatus == 3 && !sending) {
        queueStatus = 1;
        setTotalPopSize(totalSize);
    }

    if (queueStatus == 1) {
        int total = progressBar->maximum();

        if (size > total)
            size = total;

        progressBar->setValue(size);
    } else {
        downloadSize = size;
    }
}

void EmailClient::setTransferredSize(int size)
{
    if (queueStatus == 1 && !receiving) {
        queueStatus = 3;
        setTotalSmtpSize(totalSize);
    }

    if (queueStatus == 3) {
        int total = progressBar->maximum();

        if (size > total)
            size = total;

        progressBar->setValue(size);
    } else {
        downloadSize = size;
    }
}

void EmailClient::updateQuery(Email *mail, const QString &mailbox)
{
    Folder *folder = folderView->currentFolder();
    if (folder == NULL)
        return;

    if ( folder->mailbox() != mailbox )
        return;

    EmailListItem *item = messageView()->getRef( mail->uuid() );
    if (item != NULL) {

        if ( folder->matchesEmail(mail) ) {
            item->setMail(mail);
        } else {
            EmailListItem *newItem = item;

            if ( messageView()->isItemSelected(item) ) {
                int row = messageView()->row( item );
                if (row < messageView()->rowCount() - 1) //try below
                    newItem = (EmailListItem *)messageView()->item(row + 1, 0);
                if (!newItem && row) //try above
                    newItem = (EmailListItem *)messageView()->item(row - 1, 0);
            } else
                newItem = NULL;

            messageView()->removeRow( messageView()->row( item ) );
            if ( newItem ) {
                messageView()->clearSelection();
                messageView()->setItemSelected( newItem, true );
                messageView()->setCurrentItem( newItem );
            }
            setEnableMessageActions( (bool)messageView()->rowCount() );

            return;
        }
    } else if ( folder->matchesEmail(mail) ) {
            messageView()->treeInsert(mail);
    }

}

void EmailClient::updatePopStatusLabel(const QString &txt)
{
    if (queueStatus == 1) {
        progressBar->setText(statusLabelHeader + " - " + txt);
    }
    if (mReadMail && mReadMail->isVisible())
        mReadMail->setProgressText(txt);
}

void EmailClient::updateSendingStatusLabel(const QString &txt)
{
    if (queueStatus == 3) {
        if (smtpAccount)
            progressBar->setText(smtpAccount->accountName() + " - " + txt);
        else
            progressBar->setText(txt);
    }
    if (mReadMail && mReadMail->isVisible())
        mReadMail->setProgressText(txt);
}

void EmailClient::folderSelected(Folder *folder)
{
    if ( !mMessageView ) {
        // No messageview yet just update the statusbar
        if ( !folder )
            return;

        QString mailbox = folder->mailbox();
        updateFolderCount(mailbox);
        return;
    }
//    QTime stime;
//    stime.start();
    Email *mail;

    messageView()->clear();

    if ( folder == NULL ) {
        return;
    }

    QListIterator<Email*> it = mailboxList->mailbox(InboxString)->entryIterator();
    int count = mailboxList->mailbox(InboxString)->mailCount("all");
    QString mailbox = folder->mailbox();

    if ( mailbox == InboxString ) {
        messageView()->horizontalHeaderItem(1)->setText( tr("From") );
    } else if ( mailbox == OutboxString ) {
        messageView()->horizontalHeaderItem(1)->setText( tr("To") );
    } else if ( mailbox == DraftsString ) {
        messageView()->horizontalHeaderItem(1)->setText( tr("To") );
    } else if ( mailbox == SentString ) {
        messageView()->horizontalHeaderItem(1)->setText( tr("To") );
    } else if ( mailbox == TrashString ) {
        messageView()->horizontalHeaderItem(1)->setText( tr("From/To") );
    }

    if ( mailbox != InboxString ) {
        if (mailboxList->mailbox(mailbox)) {
            it = mailboxList->mailbox(mailbox)->entryIterator();
            count = mailboxList->mailbox(mailbox)->mailCount("all");
        }
    }

    bool slowFolder = false;
    Search *s = NULL;
    if ( folder->folderType() == FolderTypeSearch )
        s = ((SearchFolder *) folder)->search();
    else if ( folder->folderType() == FolderTypeSystem )
        s = ((SystemFolder *) folder)->search();

    if ( s  && !s->getBody().isEmpty() ) {
        if ( !receiving && !sending ) {
            slowFolder = true;
            progressBar->reset();
            progressBar->setRange(0,100);
            qApp->processEvents();
        }
    }

    int at = 0;

    //order newest first

    while ( it.hasNext() ) {
        mail = it.next();
        if (mail == NULL) {
            qWarning("null mailptr in ListWidgetItem, should never occur");
        } else if ( folder->matchesEmail(mail) ) {
            messageView()->treeInsert(mail);
        }

        at++;
        if ( slowFolder ) {
            progressBar->setValue( at*100/count);
        }
    }

    if ( slowFolder )
        progressBar->reset();

    //  Rebuild mail move/copy menus as they don't include the currently selected folder
    movePop->clear();
    copyPop->clear();
    QMapIterator<QAction*, int> i(moveMap);
    moveMap.clear();

    QStringList list = mailboxList->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( OutboxString);

    QAction *action;
    uint pos = 0;
    for ( QStringList::Iterator itList = list.begin(); itList != list.end(); ++itList ) {
        action = movePop->addAction(*pm_folder, mailboxTrName(*itList) );
        moveMap.insert(action, pos);
        action = copyPop->addAction(*pm_folder, mailboxTrName(*itList) );
        moveMap.insert(action, pos);

        pos++;
    }

    messageView()->setCurrentMailbox( mailbox );

    messageView()->clearSelection();
    if ( messageView()->item(0, 0) ) {
       messageView()->setItemSelected( messageView()->item(0, 0), true );
       messageView()->setCurrentItem( messageView()->item(0, 0) );
    }
    updateFolderCount(mailbox);

    //BUG? QTableWidget does not automatically sort the table on newly appended items
    messageView()->setByArrival(true);
    messageView()->defineSort(0,false);
}

/*  make sure that the currently displayed item in readmail is the same
    after the folder has been deleted and added again
    If this should fail the current item will be the first  */
void EmailClient::imapServerFolders()
{
    EmailListItem *item = (EmailListItem *) messageView()->currentItem();
    QUuid selected;

    if ( item && messageView()->isItemSelected(item) )
        selected = item->id();

    folderView->updateAccountFolder(mailAccount);

    if ( !selected.isNull() ) {
        item = messageView()->getRef( selected );
        if ( item ) {
            messageView()->clearSelection();
            messageView()->setItemSelected( item, true );
            messageView()->setCurrentItem( item );
        }
    }
}

void EmailClient::failedList(QStringList &list)
{
    QMessageBox::warning(qApp->activeWindow(),
        tr("<qt>The following commands failed:<br>%1</qt>").arg(list.join("<br>")), tr("OK"));
    return;
}

void EmailClient::cornerButtonClicked()
{
    if ( !messageView()->horizontalHeader()->isHidden() ) {
        messageView()->horizontalHeader()->hide();
    } else {
        messageView()->horizontalHeader()->show();
    }
}

void EmailClient::search()
{
    SearchView searchView(false, this, Qt::Dialog); // No tr
    searchView.setObjectName("search"); // No tr
    searchView.setModal(true);


    searchView.setSearch( folderView->lastSearch() );

#ifdef QTOPIA_DESKTOP
    searchView.exec();
#else
    QtopiaApplication::execDialog(&searchView);
#endif
    if (searchView.result() == QDialog::Accepted) {
        searchView.hide();   // in case it's a slow search
        // disconnect and reconnect cause we don't know whether the folderChanged signal will
        // be emitted (worst case two folder changed events)
        disconnect(folderView, SIGNAL(folderSelected(Folder*)), this, SLOT(folderSelected(Folder*)) );
        Search *search = searchView.getSearch();

        folderView->setLastSearch( search );
        folderView->changeToSystemFolder( LastSearchString );

        showMessageList();

        folderSelected( folderView->currentFolder() );
        connect(folderView, SIGNAL(folderSelected(Folder*)), this, SLOT(folderSelected(Folder*)) );
    }
}

void EmailClient::automaticFetch()
{
    if ( receiving )
        return;

    qWarning("get all new mail automatic");
    autoGetMail = true;
    getAllNewMail();
}

/*  Someone external are making changes to the mailboxes.  By this time
    we won't know what changes has been made (nor is it feasible to try
    to determine it).  Close all actions which can have become
    invalid due to the external edit.  A writemail window will as such close, but
    the information will be kept in memory (pasted when you reenter the
    writemail window (hopefully the external edit is done by then)
*/
void EmailClient::externalEdit(const QString &mailbox)
{
    cancel();
    showEmailView();
    folderSelected( folderView->currentFolder() );

    QString msg = mailboxTrName( mailbox ) + " "; //no tr
    msg += tr("was edited externally");
    progressBar->setText( msg );
}

int EmailClient::currentMailboxWidgetId()
{
    if (!mailboxView)
	return -1;
    return mailboxView->currentIndex();
}

void EmailClient::setCurrentMailboxWidget(int id )
{
    if ( mailboxView && (id >= 0) ) {
#ifdef QTOPIA_PHONE
        mailboxView->setCurrentIndex( id );
        if (id == folderId) {
            folderView->setFocus();
        } else {
            messageView()->setFocus();
            if (messageView()->currentItem())
                messageView()->setItemSelected( messageView()->currentItem(), true );
        }

        if (closeAfterView) {
            closeAfterTransmissionsFinished();
            if (isTransmitting()) // prevents flicker
                QTMailWindow::singleton()->hide();
            else
                QTMailWindow::singleton()->close();
        }
        Folder *folder = folderView->currentFolder();
        if (folder)
            updateFolderCount( folder->mailbox() );
#else
        mailboxView->setCurrentIndex( id );
#endif
    }
}

void EmailClient::showFolderList()
{
    setCurrentMailboxWidget( folderId );
}

void EmailClient::showMessageList()
{
    setCurrentMailboxWidget( messageId );
}

void EmailClient::showEmailView()
{
    emit raiseWidget(this, appTitle);
    update();
}

bool EmailClient::checkMailConflict(const QString& msg1, const QString& msg2)
{
    if ( writeMailWidget()->isVisible() ) {
        QString message = tr("<qt>You are currently editing a message:<br>%1</qt>").arg(msg1);
        switch( QMessageBox::warning( this, tr("Messages conflict"), message,
            tr("Yes"), tr("No"), 0, 0, 1 ) ) {

            case 0:
            {
                if ( !mWriteMail->tryAccept() ) {
                    QMessageBox::warning(this,
                        tr("Autosave failed"),
                        tr("<qt>Autosave failed:<br>%1</qt>").arg(msg2));
                    return true;
                }
                break;
            }
            case 1: break;
        }
    }
    return false;
}

void EmailClient::writeMailAction(const QMap<QString, QString> map )
{
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if ( checkMailConflict(
            tr("Should it be saved in Drafts\nbefore writing this mail?"),
            tr("Write Mail message will be ignored")) )
        return;

    Email mail;

    QMap<QString, QString>::ConstIterator it;
    for( it = map.begin(); it != map.end(); ++it ) {
        if (it.key().toLower() == "from")
            mail.setFrom( it.value() );
        else if (it.key().toLower() == "subject")
            mail.setSubject( it.value() );
        else if (it.key().toLower() == "date")
            mail.setDateString( it.value() );
        else if (it.key().toLower() == "to")
            mail.setTo( QStringList( it.value() ));
        else if (it.key().toLower() == "cc")
            mail.setCc( QStringList( it.value() ));
        else if (it.key().toLower() == "bcc")
            mail.setBcc( QStringList( it.value() ));
        else if (it.key().toLower() == "reply-to")
            mail.setReplyTo( it.value() );
        else if (it.key().toLower() == "message-id")
            mail.setMessageId( it.value() );
        else if (it.key().toLower() == "in-reply-to")
            mail.setInReplyTo( it.value() );
        else if (it.key().toLower() == "body")
            mail.setPlainTextBody( it.value() );
        else if (it.key().toLower().indexOf( "attachment" ) == 0) {
            QString fileName = it.value();
            MailMessagePart attachmentPart;
            QFileInfo fi( fileName );
            attachmentPart.setName( fi.baseName() );
            attachmentPart.setFilename( fi.absoluteFilePath() );
            attachmentPart.setStoredFilename( fi.absoluteFilePath() );
            attachmentPart.setLinkFile( fi.absoluteFilePath() );
            QMimeType mt( fi.absoluteFilePath() );
            attachmentPart.setContentType( mt.id() );

            //don't encode attachments anymore until sending

//             QFile f( fileName );
//             if (! f.open(QIODevice::ReadOnly) )
//             {
//                 qWarning( ("could not open file: " + fileName).toLatin1() );
//                 continue;
//             }
//             QDataStream in(&f);
//             attachmentPart.setBody(in,Base64);
//             f.close();
            mail.addMessagePart( attachmentPart );
        } else
            mail.setExtraHeader( it.key(), it.value() );
    }

    //encode when sending

    //mail.encodeMail();
    writeMailWidget()->modify( &mail );
    emit raiseWidget( mWriteMail, tr("Write mail") );

    openFiles();
}

void EmailClient::smsVCard( const QDSActionRequest& request )
{
    QtopiaApplication::instance()->showMainWidget();
    writeSmsAction( QString(), QString(), request.requestData().toString(), true );
    QDSActionRequest( request ).respond();
}

void EmailClient::writeSmsAction(const QString&, const QString& number,
                                 const QString& body, bool vcard)
{
#if defined(QTOPIA_PHONE) && !defined(QTOPIA_NO_SMS)
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if ( checkMailConflict(
            tr("Should this mail be saved in Drafts before writing the new mail?"),
            tr("'Write SMS' message will be ignored")) )
        return;

    writeMailWidget()->newMail( ComposerFactory::defaultInterface(
                                                        MailMessage::SMS ), vcard);
    QString recipient;
    if ( number.isEmpty() )
        recipient += "";
    else
        recipient += number;
    writeMailWidget()->setSmsRecipient( recipient );
    if (!body.isNull())
    {
        if(vcard)
            writeMailWidget()->setBody(SmsClient::vCardPrefix() + body);
        else
            writeMailWidget()->setBody( body );
    }
    mWriteMail->setAccountList( accountList );
    emit raiseWidget( writeMailWidget(), tr("Write mail") );

    openFiles();
    writeMailWidget()->setComposerFocus();
#else
    Q_UNUSED(number);
    Q_UNUSED(body);
    Q_UNUSED(vcard);
#endif
}

void EmailClient::writeMailAction(const QString& name, const QString& email)
{
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if ( checkMailConflict(
            tr("Should this mail be saved in Drafts before writing the new mail?"),
            tr("'Write Mail' message will be ignored")) )
        return;

    QString recipient;
    if ( !name.isEmpty() ) {
        recipient = "\"" + name + "\" <";
        if ( email.isEmpty() )
            recipient += "???";
        else
            recipient += email;
        recipient += ">";
    } else if ( !email.isEmpty() ) {
        recipient = "<"+email+">";
    }
    mWriteMail->newMail(
#ifdef QTOPIA_PHONE
    ComposerFactory::defaultInterface( MailMessage::Email )
#endif
            );
    writeMailWidget()->setRecipient( recipient );
    mWriteMail->setAccountList( accountList );
    emit raiseWidget( mWriteMail, tr("Write mail") );

    openFiles();

    // Ensure that the newly raised widget gets focus.
    writeMailWidget()->setComposerFocus();
}

void EmailClient::emailVCard( const QDSActionRequest& request )
{
    QtopiaApplication::instance()->showMainWidget();

    // Save the VCard data to file
    QString filename = Qtopia::tempDir() + "/email.vcard";
    {
        QFile temp( filename );
        if ( !temp.open( QIODevice::WriteOnly ) ) {
            return;
        }

        temp.write( request.requestData().data() );
        temp.close();
    }

    // write the Email
    writeMessageAction( QString(),
                        QString(),
                        QStringList(),
                        QStringList( filename ),
                        MailMessage::Email );

    // Respond to the request
    QDSActionRequest( request ).respond();
}

void EmailClient::collectSysMessages()
{
    QtopiaIpcEnvelope e ( "QPE/SysMessages", "collectMessages()" );
}

void EmailClient::setEnableMessageActions( bool enabled )
{
    moveAction->setVisible( enabled );
    copyAction->setVisible( enabled );
    selectAllAction->setVisible( enabled );
    deleteMailAction->setVisible( enabled );
}

void EmailClient::writeMessageAction( const QString &name,
                    const QString &addrStr, const QStringList &docAttachments,
                    const QStringList &fileAttachments,
                    int type)
{
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    QString emails;
    QString numbers;

    if (!addrStr.isEmpty()) {
        QStringList nsAddresses = addrStr.trimmed().split( "," );
        QStringList rawAddresses = addrStr.split( "," );
        QStringList emailAddresses;
        QStringList phoneNumbers;

        //parse addrStr into separate phone number and email addresses
        for( int i = 0 ; i < (int)nsAddresses.count() ; ++i )
        {
            QString c = nsAddresses[i];
            QString rc = rawAddresses[i];
            if( c.indexOf( '@' ) != -1 )
                emailAddresses += rc;
            else
            {
                bool hasPhoneChars = false;
                for( int j = 0 ; j < (int)c.length() ; ++j )
                    if( c[j].isDigit() || c[j] == '+' )
                    {
                        hasPhoneChars = true;
                        break;
                    }
                if( hasPhoneChars )
                    phoneNumbers += rc;
                else // all characters, most likely not a phone number
                    emailAddresses += rc;
            }
        }

        emails = emailAddresses.join( "," );
        numbers =  phoneNumbers.join( "," );
    }

    if( !name.isEmpty() && !emails.isEmpty() )
        emails.prepend( "<" + name + "> " );

    bool textOnly = true;
    QStringList::ConstIterator it;
    for (it = docAttachments.begin(); it != docAttachments.end(); ++it) {
        QContent lnk(*it);
        if (lnk.type() != "text/plain" && lnk.type() != "text/x-vCalendar"
                && lnk.type() != "text/x-vCard")
            textOnly = false;
    }
    for (it = fileAttachments.begin(); it != fileAttachments.end(); ++it) {
        QString extn = (*it).right(4);
        if (extn != ".txt" && extn != ".vcs" && extn != ".vcf")
            textOnly = false;
    }
#ifdef QTOPIA_PHONE
    int diid = MailMessage::MMS;
    if (textOnly && emails.trimmed().isEmpty() && type & MailMessage::SMS) {
        // if we're sending plain text and have no email recipients, use SMS
        diid = MailMessage::SMS;
    } else if (!(type & MailMessage::MMS)) {
        diid = 0;
        if (type & MailMessage::Email)
            diid = MailMessage::Email;
        if (type & MailMessage::SMS)
            diid |= MailMessage::SMS;
        if (!diid)
            diid = MailMessage::SMS;
    }
#else
    Q_UNUSED(type);
#endif
    writeMailWidget()->newMail(
#ifdef QTOPIA_PHONE
            ComposerFactory::defaultInterface( diid )
#endif
            );
    writeMailWidget()->setRecipients( emails, numbers );
    mWriteMail->setAccountList( accountList );
    emit raiseWidget( mWriteMail, tr("Write mail") );
    for( uint i = 0 ; i < (uint)docAttachments.count() ; ++i )
    {
        mWriteMail->attach( QContent( docAttachments[i] ) );
    }
    for( uint i = 0 ; i < (uint)fileAttachments.count() ; ++i )
    {
        mWriteMail->attach( fileAttachments[i] );
    }
}

void EmailClient::cleanupMessages( const QDate &removalDate, int removalSize )
{
    bool closeAfterCleanup = isHidden();

    openFiles();
    QStringList mboxList = mailboxList->mailboxes();
    QStringList::Iterator it;
    for (it = mboxList.begin(); it != mboxList.end(); ++it)
    {
        EmailFolderList *box = mailboxList->mailbox( *it );
        QListIterator<Email*> it2( box->entryIterator() );
        Email *mail = 0;
        if ( it2.hasNext() )
            mail = it2.next();
        while ( mail ) {
            Email *mail = it2.next();
            QDate mailDate( mail->dateTime().date() );
            uint mailSize( mail->size() );
            if ((mailDate <= removalDate) && ((int)mailSize >= removalSize))
                box->removeMail( mail->uuid(), true );
            else
                if ( it2.hasNext() )
                    mail = it2.next();
                else
                    mail = 0;
        }
        mailboxList->compact();
    }

#ifndef QTOPIA_DESKTOP
    if (closeAfterCleanup) {
        closeAfterTransmissionsFinished();
        if (isTransmitting())
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
    }
#endif
}

WriteMail *EmailClient::writeMailWidget()
{
    if ( !mWriteMail ) {
        mWriteMail = new WriteMail( this , "write-mail");
        if ( parentWidget()->inherits("QStackedWidget") )
            ((QStackedWidget*)parentWidget())->addWidget(mWriteMail);

        connect(mWriteMail, SIGNAL(enqueueMail(const Email&)), this,
                        SLOT(enqueueMail(const Email&)) );
        connect(mWriteMail, SIGNAL( discardMail() ), this,
                        SLOT( discardMail() ) );
        connect(mWriteMail, SIGNAL(saveAsDraft(const Email&)), this,
                        SLOT(saveAsDraft(const Email&)) );
        connect(mWriteMail, SIGNAL( autosaveMail(const Email&)), this,
                SLOT( autosaveMail(const Email&)) );

        mWriteMail->setAccountList( accountList );
    }

    return mWriteMail;
}


ReadMail *EmailClient::readMailWidget()
{   
    if ( !mReadMail ) {
        mReadMail = new ReadMail( this, "examine-mail");
        if ( parentWidget()->inherits("QStackedWidget") )
            ((QStackedWidget*)parentWidget())->addWidget(mReadMail);

        connect(mReadMail, SIGNAL(cancelView()), this, SLOT(showEmailView()) );
        connect(mReadMail, SIGNAL(resendRequested(const Email&,int)), this,
                SLOT(resend(const Email&,int)) );

        connect(mReadMail, SIGNAL(modifyRequested(Email*)), this,
                SLOT(modify(Email*)) );
        connect(mReadMail, SIGNAL(removeItem(EmailListItem*)), this,
                SLOT(deleteMailRequested(EmailListItem*)) );
        connect(mReadMail, SIGNAL(viewingMail(Email*)), this,
                SLOT(moveMailFront(Email*)) );
        connect(mReadMail, SIGNAL(getMailRequested(Email*)), this,
                SLOT(getSingleMail(Email*)) );
        connect(mReadMail, SIGNAL(sendMailRequested(Email*)), this,
                SLOT(sendSingleMail(Email*)) );
        connect(mReadMail, SIGNAL( mailto(const QString&) ), this,
                SLOT( setDocument(const QString&) ) );
        connect(mReadMail,SIGNAL(readReplyRequested(Email*)),this,
                SLOT(readReplyRequested(Email*)));
        connect(mReadMail, SIGNAL(viewingMail(Email*)),
                emailHandler, SLOT(mailRead(Email*)) );
    }

    return mReadMail;
}

void EmailClient::resend(const Email &mail, int type)
{
    repliedFromUuid = mail.uuid();
    QString caption;

    writeMailWidget()->reply(mail, type);
    if (type == 1) {
        caption = tr( "Reply to mail"  );
        repliedFlags = EFlag_Replied;
    } else if (type == 2) {
        caption = tr( "Reply to all"  );
        repliedFlags = EFlag_RepliedAll;
    } else if (type == 3) {
        caption = tr( "Forward mail"  );
        repliedFlags = EFlag_Forwarded;
    } else {
        return;
    }

    emit raiseWidget( mWriteMail, caption );
}

void EmailClient::modify(Email *mail)
{
    QString caption =  tr( "Edit mail"  );
    mail->readFromFile();
    writeMailWidget()->modify(mail);
    emit raiseWidget( mWriteMail, caption );
}


void EmailClient::compose()
{
    QString caption = " " + tr("Write mail");
    emit raiseWidget( writeMailWidget(), caption );
    writeMailWidget()->newMail();
}

void EmailClient::setDocument(const QString &_address)
{
    // strip leading 'mailto:'
    QString smsPostFix = "@sms";
    QString address = _address;
    if (address.startsWith("mailto:"))
        address = address.mid(7);

    QString caption = " " + tr("Write mail");
    if (address.right(smsPostFix.length()) == smsPostFix) {
        int smsLen = address.length() - smsPostFix.length();
        QString smsRecipient = address.left(smsLen);
#ifdef QTOPIA_PHONE
        writeMailWidget()->newMail( ComposerFactory::defaultInterface(
                                                        MailMessage::SMS ) );
        writeMailWidget()->setSmsRecipient(smsRecipient);
        writeMailWidget()->setComposerFocus();
#endif
    } else {
        writeMailWidget()->newMail(
#ifdef QTOPIA_PHONE
                ComposerFactory::defaultInterface( MailMessage::Email )
#endif
                );
        writeMailWidget()->setRecipient(address);
    }
    emit raiseWidget( mWriteMail, caption );
}

void EmailClient::deleteMailRequested(EmailListItem *item)
{
    Folder *folder = folderView->currentFolder();

    if ( folder->folderType() == FolderTypeSystem &&
         folder->mailbox() == OutboxString &&
         sending) {
        return; //don't delete when sending
    }

    // Is the SIM card ready/detected?
    if (!emailHandler->smsReadyToDelete()
        && item->mail()
        && (item->mail()->type() == MailMessage::SMS)) {
        showDeleteNotReadyWarning();
        return;
    }

    if (folder->folderType() == FolderTypeSystem
        && folder->mailbox() == TrashString ) {
        if (!Qtopia::confirmDelete( this, tr("EMail"),
                                    tr( "the mail" ) ))
            return;
    }

    deleteMail( item );
}

void EmailClient::showEvent(QShowEvent* e)
{
    closeAfterTransmissions = false;
}

void EmailClient::isSending(bool y)
{
    if ( y != (nosuspend&1) ) {
        nosuspend ^= 1;
        if ( nosuspend == 1 )
            suspendOk(false);
        else if ( nosuspend == 0 )
            suspendOk(true);
    }

    if (mReadMail)
        mReadMail->isSending(y);

    if (!y)
        moveOutboxMailsToDrafts();

#ifndef QTOPIA_DESKTOP
    if (!isTransmitting() && closeAfterTransmissions)
        QTMailWindow::singleton()->close();
    if (!isTransmitting()) {
        qLog(Messaging) << "Unregistering transfer task";
        QtopiaApplication::instance()->unregisterRunningTask(QLatin1String("transfer"));
    }
#endif
}

void EmailClient::isReceiving(bool y)
{
    if ( y != (nosuspend&2) ) {
        nosuspend ^= 2;
        if ( nosuspend == 2 )
            suspendOk(false);
        else if ( nosuspend == 0 )
            suspendOk(true);
    }

    if (mReadMail)
        mReadMail->isReceiving(y);
#ifndef QTOPIA_DESKTOP
    if (!isTransmitting() && closeAfterTransmissions)
        QTMailWindow::singleton()->close();
    if (!isTransmitting()) {
        qLog(Messaging) << "Unregistering transfer task";
        QtopiaApplication::instance()->unregisterRunningTask(QLatin1String("transfer"));
    }
#endif
}

void EmailClient::suspendOk(bool y)
{
    QtopiaApplication::setPowerConstraint(y ? QtopiaApplication::Enable : QtopiaApplication::DisableSuspend);
}

void EmailClient::moveOutboxMailsToDrafts()
{
//  Move any messages stuck in the outbox to the drafts folder here
    EmailFolderList *outbox = mailboxList->mailbox(OutboxString);
    EmailFolderList *drafts = mailboxList->mailbox(DraftsString);
    QListIterator<Email*> mail = outbox->entryIterator();
    while (mail.hasNext()) {
        if ( !moveMailToFolder(mail.next(), outbox, drafts ) )
            break;          //no point continuing to move
    }
}


void EmailClient::updateFolderCount(const QString &mailbox, bool folderListOnly)
{
    if(suspendMailCount)
        return;
    if (!mailboxList->mailbox(mailbox))
        return;

    // folder list item updates
    uint count = 0;
    uint allCount = 0;
    uint allNewCount = 0;
    bool highlight = false;

    QString s;

    QColor col = Qt::blue;
    count = mailboxList->mailbox( mailbox )->mailCount( "all"); // No tr
    allCount = count;
    if ( mailbox == InboxString ) {
        int newCount = mailboxList->mailbox( mailbox )->mailCount( "new", accountList); // No tr
        allNewCount = newCount;
        if ( newCount ) {
            highlight = true;
            count = newCount;
        }

        QListIterator<MailAccount*> it = accountList->accountIterator();
        while ( it.hasNext() ) {
            MailAccount *account = it.next();
            int count = account->unreadCount();
            QString countStr;
            if ( count )
                countStr = QString( " (%1)" ).arg( count );
            folderView->updateAccountStatus( account, countStr, count, Qt::blue);
        }
    } else if ( mailbox == OutboxString ) {
        col = Qt::red;
        int newCount = mailboxList->mailbox( mailbox )->mailCount( "unsent"); // No tr
        if ( newCount ) {
            highlight = true;
            count = newCount;
        }
    }

    if ( count ) {
        s = QString(" (%1)").arg(count);
    }

    folderView->updateFolderStatus( mailbox, s, highlight, col);

#ifdef QTOPIA_PHONE
    if (mailbox == TrashString) {
        int count = mailboxList->mailbox(TrashString)->mailCount("all");
        emptyTrashAction->setVisible(count > 0);
    }

    // Update delete mail action
    deleteMailAction->setVisible( allCount );
    moveAction->setVisible( allCount );
    copyAction->setVisible( allCount );
    selectAllAction->setVisible( allCount );
#endif

    // statusbar updates
    if ( receiving || sending || folderListOnly )
        return;

    Folder *folder = folderView->currentFolder();

    if ( folder == NULL ) {
        progressBar->setText("");
        return;
    }

    if ( currentMailboxWidgetId() == messageId ) {
        messageChanged();
        return;
    }

    QString str = folder->fullName();
    if ( (folder->folderType() == FolderTypeSystem) && ( ((SystemFolder *)folder)->systemType() == SystemTypeSearch) ) {
        str += " - " + mailboxTrName( folder->mailbox() );
    }

    str += ": ";
#ifdef QTOPIA_PHONE
    str = "";
#endif
    str += QString::number( allCount ); // No tr
    if ( mailbox == InboxString ) {
        str += " "; //no tr
        str += tr( "(%1 new)", "%1 = number of new mails" ).arg( allNewCount );
    } else if ( mailbox != TrashString ) {
        int unsent = mailboxList->mailbox( mailbox )->mailCount( "unsent" );
        int unfinished = mailboxList->mailbox( mailbox )->mailCount( "unfinished" );

        if ( unsent || unfinished ) {
            QString strUnsent = tr("%1 unsent", "%1 = number of unsent mails" ).arg(unsent);
            QString strUnfinished = tr("%1 unfinished", "%1 = number of unfinished mails" ).arg(unfinished);

            QString fmt;
            if ( unsent ) {
                fmt = strUnsent;
#ifndef QTOPIA_PHONE
                if ( unfinished )
                    fmt += " / " + strUnfinished;
#endif
            } else {    //unfinished only
                fmt = strUnfinished;
            }
            str += " (" + fmt + ")";
        }
    } else {                    //trash folder
    }

    progressBar->setText( str );
}

void EmailClient::settings()
{
#ifdef QTOPIA_PHONE
    AccountSettings settings(accountList, this, 0, true);
    connect(&settings, SIGNAL(changedAccount(MailAccount*)),
            this, SLOT(changedAccount(MailAccount*)));
    connect(&settings, SIGNAL(deleteAccount(MailAccount*)),
            this, SLOT(deleteAccount(MailAccount*)));

    QListIterator<MailAccount*> it = accountList->accountIterator();
    bool addAccount = true;
    while ( it.hasNext() ) {
        MailAccount::AccountType accountType = it.next()->accountType();
        if ( accountType != MailAccount::SMS ) {
            addAccount = false;
            break;
        }
    }
    if (addAccount)
        settings.addAccount();

    settings.showMaximized();
    QtopiaApplication::execDialog(&settings);
    QTimer::singleShot(0, this, SLOT( updateAccounts() ) );
#endif
}

void EmailClient::changedAccount(MailAccount *account)
{
#ifdef QTOPIA_PHONE
    QTimer::singleShot(0, this, SLOT(updateAccounts()));
    folderView->updateAccountFolder(account);
    accountList->saveAccounts();
#else
    Q_UNUSED(account);
#endif
}

void EmailClient::deleteAccount(MailAccount *account)
{
#ifdef QTOPIA_PHONE
    folderView->deleteAccountFolder(account);
    accountList->remove(account);
    QTimer::singleShot(0, this, SLOT(updateAccounts()));
    accountList->saveAccounts();
#else
    Q_UNUSED(account);
#endif
}

MailListView* EmailClient::messageView()
{
    if ( !mMessageView ) {
        mMessageView = new MailListView(widget_3, "messageView" );
        connect(mMessageView, SIGNAL(itemClicked(QTableWidgetItem*)),
                this, SLOT(queryItemSelected()) );
        connect(mMessageView, SIGNAL( itemPressed(EmailListItem*) ),
                this, SLOT( showItemMenu(EmailListItem*) ) );
        connect(mMessageView, SIGNAL( currentItemChanged(QTableWidgetItem*,QTableWidgetItem*) ),
                this, SLOT( messageChanged() ) );
        connect(mMessageView, SIGNAL( enableMessageActions(bool) ),
                this, SLOT( setEnableMessageActions(bool) ) );

	connect(mMessageView, SIGNAL(viewFolderList()),
		this, SLOT(showFolderList()) );
	gridQuery->addWidget(mMessageView, 2, 2 );

        QSettings mailconf("Trolltech","qtmail");
        mailconf.beginGroup("qtmailglobal");
        messageView()->readConfig( &mailconf );
        mailconf.beginGroup("settings");
        QFont font;
        if ( mailconf.value("font").toString() == "large") {
            font.setPointSize( font.pointSize() + 4 );  // 4 larger than default
            messageView()->setFont( font );
        } else {
            messageView()->setFont( font );
        }
        mailconf.endGroup();

        Folder* currentFolder = folderView->currentFolder();
        if (currentFolder)
            folderSelected( folderView->currentFolder() );

        displayPreviousMail();
    }
    return mMessageView;
}

void EmailClient::handleSysMessages(const QString &message, const QByteArray &data)
{
    if (message == "postMessage(int,QDateTime,QString,QString)")
    {
        QDataStream ds(data);
        int messageId;
        QDateTime time;
        QString subject;
        QString text;
        ds >>  messageId >> time >> subject >> text;

        Email mail;
        mail.setType( MailMessage::System );
        mail.setStatus( EFlag_Downloaded, true );
        mail.setStatus( EFlag_Incoming, true );
        mail.setStatus( EFlag_TypeSystem, true );
        mail.setDateTime( time );
        mail.setSubject( subject );
        mail.setPlainTextBody( text );
        mail.setFromAccount( "@System" );
        mail.setFrom("System");
        mail.encodeMail();
        mailArrived( mail );

        QtopiaIpcEnvelope e("QPE/SysMessages", "ackMessage(int)");
        e << messageId;
    }
}


/*!
    \service EmailService Email
    \brief Provides the Qtopia Email service.

    The \i Email service enables applications to access features of
    the system's e-mail application.
*/

/*!
    \internal
*/
EmailService::~EmailService()
{
}

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message, and then, if confirmed by the user, send the message.

    This slot corresponds to the QCop service message
    \c{Email::writeMail()}.
*/
void EmailService::writeMail()
{
    parent->writeMailAction(QString(),QString());
}

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message, and then, if confirmed by the user, send the message.
    The message is sent to \a name at \a email.

    This slot corresponds to the QCop service message
    \c{Email::writeMail(QString,QString)}.
*/
void EmailService::writeMail( const QString& name, const QString& email )
{
    parent->writeMailAction( name, email );
}

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message, and then, if confirmed by the user, send the message.
    The message is sent to \a name at \a email.  The initial body of
    the message will be based on \a docAttachments and \a fileAttachments.

    This message will choose the best message transport for the message,
    which may be e-mail, SMS, MMS, etc.  This is unlike writeMail(),
    which will always use e-mail.

    This slot corresponds to the QCop service message
    \c{Email::writeMessage(QString,QString,QStringList,QStringList)}.
*/
void EmailService::writeMessage( const QString& name, const QString& email,
                                 const QStringList& docAttachments,
                                 const QStringList& fileAttachments )
{
    parent->writeMessageAction( name, email, docAttachments, fileAttachments );
}

/*!
    Direct the \i Email service to display the user's message boxes.

    This slot corresponds to the QCop service message
    \c{Email::viewMail()}.
*/
void EmailService::viewMail()
{
    parent->raise();
}

/*!
    Direct the \i Email service to display the message identified by
    \a id in \a mailbox.

    This slot corresponds to the QCop service message
    \c{Email::viewMail(QString,QString)}.
*/
void EmailService::viewMail( const QString& mailbox, const QString& id )
{
    if ( ! parent->checkMailConflict(
            tr("Should this mail be saved in Drafts before viewing the new mail?"),
            tr("'View Mail' message will be ignored")) ) {
        parent->cachedDisplayMailbox = mailbox;
        parent->cachedDisplayMailUuid = QUuid( id );
        parent->openFiles();
    }
}

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message for sending the vcard data in \a filename.  The
    \a description argument provides an optional descriptive text message.

    This slot corresponds to the QCop service message
    \c{Email::emailVCard(QString,QString)}.
*/
void EmailService::emailVCard( const QString& filename, const QString& )
{
    parent->writeMessageAction( QString(), QString(), QStringList(),
                                QStringList( filename ),
                                MailMessage::Email);
}

/*!
    Direct the \i Email service to interact with the user to compose a new
    e-mail message for sending the vcard data in \a request.

    This slot corresponds to a QDS service with a request data type of
    "text/x-vcard" and no response data.

    This slot corresponds to the QCop service message
    \c{Email::emailVCard(QDSActionRequest)}.
*/
void EmailService::emailVCard( const QDSActionRequest& request )
{
    parent->emailVCard( request );
}

/*!
    \internal
*/
void EmailService::emailVCard( const QString&, const QString&, const QString& filename, const QString& description )
{
    // To be removed when the SetValue service is fixed.
    emailVCard( filename, description );
}

/*!
    Direct the \i Email service to purge all messages which
    are older than the given \a date and exceed the minimal mail \a size.
    This is typically called by the cleanup wizard.

    This slot corresponds to the QCop service message
    \c{Email::cleanupMessages(QDate,int)}.
*/
void EmailService::cleanupMessages( const QDate& date, int size )
{
    parent->cleanupMessages( date, size );
}

#ifdef QTOPIA_PHONE

/*!
    \service SMSService SMS
    \brief Provides the Qtopia SMS service.

    The \i SMS service enables applications to access features of
    the system's SMS application.
*/

/*!
    \internal
*/
SMSService::~SMSService()
{
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message, and then, if confirmed by the user, send the message.

    This slot corresponds to the QCop service message
    \c{SMS::writeSms()}.
*/
void SMSService::writeSms()
{
    parent->writeSmsAction(QString(),QString());
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message, and then, if confirmed by the user, send the message.
    The message is sent to \a name at \a number.

    This slot corresponds to the QCop service message
    \c{SMS::writeSms(QString,QString)}.
*/
void SMSService::writeSms( const QString& name, const QString& number )
{
    parent->writeSmsAction( name, number );
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message, and then, if confirmed by the user, send the message.
    The message is sent to \a name at \a number.  The initial body of
    the message will be read from \a filename.  After the file is
    read, it will be removed.

    This slot corresponds to the QCop service message
    \c{SMS::writeSms(QString,QString,QString)}.
*/
void SMSService::writeSms( const QString& name, const QString& number,
                           const QString& filename )
{
    QFile f( filename );
    if (! f.open(QIODevice::ReadOnly) ) {
        qWarning("could not open file: %s", filename.toLatin1().constData() );
    } else {
        QString body = QString::fromLocal8Bit( f.readAll() );
        f.close();
        f.remove();
        parent->writeSmsAction( name, number, body );
    }
}

/*!
    Show the most recently received SMS message.

    This slot corresponds to the QCop service message
    \c{SMS::viewSms()}.
*/
void SMSService::viewSms()
{
    qLog(Messaging) << "viewSms() service requested";
    parent->delayedShowMessage(MailAccount::SMS, QUuid(), false);
}

/*!
    Show the list of all received SMS messages.

    This slot corresponds to the QCop service message
    \c{SMS::viewSmsList()}.
*/
void SMSService::viewSmsList()
{
    parent->delayedShowMessage(MailAccount::SMS, QUuid(), true);
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message for sending the vcard data in \a filename.  The
    \a description argument provides an optional descriptive text message.

    This slot corresponds to the QCop service message
    \c{SMS::smsVCard(QString,QString)}.
*/
void SMSService::smsVCard( const QString& filename, const QString& )
{
    QFile f( filename );
    if (! f.open(QIODevice::ReadOnly) ) {
        qWarning("could not open file: %s", filename.toLatin1().constData() );
    } else {
        QString body =  QString::fromLocal8Bit( f.readAll() );
        parent->writeSmsAction( QString(), QString(), body, true);
    }
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message for sending the vcard data in \a request.

    This slot corresponds to a QDS service with a request data type of
    "text/x-vcard" and no response data.

    This slot corresponds to the QCop service message
    \c{SMS::smsVCard(QDSActionRequest)}.

*/
void SMSService::smsVCard( const QDSActionRequest& request )
{
    parent->smsVCard( request );
}

/*!
    \internal
*/
void SMSService::viewSysSms()
{
    parent->delayedShowMessage(MailAccount::System, QUuid(), false);
}

/*!
    \internal
*/
void SMSService::smsVCard( const QString&, const QString&, const QString& filename, const QString& description )
{
    // To be removed when the SetValue service is fixed.
    smsVCard( filename, description );
}

#endif // QTOPIA_PHONE

