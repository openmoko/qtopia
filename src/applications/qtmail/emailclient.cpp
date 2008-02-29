/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>

#include <qtopianamespace.h>
#include <qsoftmenubar.h>
#include <qtopialog.h>
#include <qperformancelog.h>

#include "emailclient.h"
#include "folder.h"
#include "searchview.h"
#include "accountlist.h"
#include "emailfolderlist.h"
#include "selectfolder.h"
#include "accountsettings.h"
#include "emailpropertysetter.h"
#include <qtopia/mail/qmailaddress.h>
#include <qtopia/mail/qmailcomposer.h>
#include <qtopia/mail/qmailtimestamp.h>
#include <QMailStore>

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
#include <QWapAccount>
#include <QStack>
#include <longstream_p.h>
#ifndef QTOPIA_NO_SMS
#include <QSMSMessage>
#endif
#include <QtopiaIpcAdaptor>
#include <QtopiaServiceRequest>

#ifndef LED_MAIL
#define LED_MAIL 0
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

enum QueueStatus {
    Inactive = 0,
    Receiving = 1,
    Sending = 2
};

// Time in ms to show new message dialog.  0 == Indefinate
static const int NotificationVisualTimeout = 0;

// Number of messages required before we use a progress indicator
static const int MinimumForProgressIndicator = 20;
static const int ProgressIndicatorUpdatePeriod = 500;

static QIcon* pm_folder = 0;
static QIcon* pm_trash = 0;

static void registerTask(const char* name)
{
    qLog(Messaging) << "Registering task:" << name;
    QtopiaApplication::instance()->registerRunningTask(QLatin1String(name));
}

static void unregisterTask(const char* name)
{
    qLog(Messaging) << "Unregistering task:" << name;
    QtopiaApplication::instance()->unregisterRunningTask(QLatin1String(name));
}

//paths for qtmail, is settings, inbox, enclosures
static QString getPath(const QString& fn, bool isdir=false)
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

class AcknowledgmentBox : public QMessageBox
{
    Q_OBJECT

public:
    static void show(const QString& title, const QString& text);

private:
    AcknowledgmentBox(const QString& title, const QString& text);
    ~AcknowledgmentBox();

    virtual void keyPressEvent(QKeyEvent* event);

    static const int _timeout = 3 * 1000;
};

AcknowledgmentBox::AcknowledgmentBox(const QString& title, const QString& text)
    : QMessageBox(0)
{
    setWindowTitle(title);
    setText(text);
    setIcon(QMessageBox::Information);
    setAttribute(Qt::WA_DeleteOnClose);

    QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Cancel);

    QDialog::show();

    QTimer::singleShot(_timeout, this, SLOT(accept()));
}

AcknowledgmentBox::~AcknowledgmentBox()
{
}

void AcknowledgmentBox::show(const QString& title, const QString& text)
{
    (void)new AcknowledgmentBox(title, text);
}

void AcknowledgmentBox::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Select) {
        event->accept();
        accept();
    } else {
        QMessageBox::keyPressEvent(event);
    }
}

// Keep track of where we are within the program
struct UILocation 
{
    UILocation(QWidget* widget, int widgetId = -1, bool email = false);
    UILocation(QWidget* widget, QMailId messageId, Folder* messageFolder, bool email);

    QWidget* widget;
    int widgetId;
    QMailId messageId;
    Folder* messageFolder;
    bool email;

private:
    friend class QVector<UILocation>;
    UILocation();
};

UILocation::UILocation()
    : widget(0), widgetId(-1), messageFolder(0), email(false)
{
}

UILocation::UILocation(QWidget* widget, int widgetId, bool email)
    : widget(widget), widgetId(widgetId), messageFolder(0), email(email)
{
}

UILocation::UILocation(QWidget* widget, QMailId messageId, Folder* messageFolder, bool email)
    : widget(widget), widgetId(-1), messageId(messageId), messageFolder(messageFolder), email(email)
{
}

QDebug& operator<< (QDebug& debug, const UILocation& location)
{
    return debug << '[' << location.widget << ':' << location.widgetId << ':' << location.messageId.toULongLong() 
                 << ':' << location.messageFolder << ':' << location.email << ']';
}


static QStack<UILocation> locationStack;

static void pushLocation(const UILocation& location)
{
    qLog(Messaging) << "pushLocation -" << locationStack.count() << ":" << location;
    locationStack.push(location);
}

static void popLocation()
{
    locationStack.pop();
    if (locationStack.count())
        qLog(Messaging) << "popLocation -" << locationStack.count() - 1 << ":" << locationStack.top();
    else 
        qLog(Messaging) << "popLocation - empty";
}

static bool haveLocation()
{
    return !locationStack.isEmpty();
}

static const UILocation& currentLocation()
{
    return locationStack.top();
}


EmailClient::EmailClient( QWidget* parent, const QString name, Qt::WFlags fl )
    : QMainWindow( parent, fl ), accountIdCount(0), emailHandler(0),
    enableMessageActions(false), mb(0), fetchTimer(this),
    showMessageType(MailAccount::SMS), autoDownloadMail(false),
    planeMode("/UI/Profile/PlaneMode"), newMessagesBox(0),
    messageCountUpdate("QPE/Messages/MessageCountUpdated"),
    initialAction(None)
{
    qLog(Messaging) << "Creating EmailClient";
    QPerformanceLog(appTitle.toLatin1().constData()) << " : " << "Begin emailclient constructor "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    setObjectName( name );
    appTitle = tr("Messages");
    waitingForNewMessage = false;
    newMessagesRequested = false;
    autoGetMail = false;
    mMailboxList = 0;
    accountList = 0;
    suspendMailCount = false;
    sending = false;
    receiving = false;
    previewingMail = false;
    mailIdCount = 1;
    allAccounts = false;
    closeAfterTransmissions = false;
    closeAfterWrite = false;
    mMessageView = 0;
    mFolderView = 0;
    mActionView = 0;
    folderId = -2;
    messageId = -3;
    queueStatus = Inactive;
    nosuspend = 0;
    filesRead = false;
    showMsgList = false;
    showMsgRetryCount = 0;
    lastSearch = 0;

    init();

    // Hook up the QCop service handlers.
    QtopiaAbstractService* svc;

    svc = new EmailService( this );

#ifndef QTOPIA_NO_SMS
    svc = new SMSService( this );
    connect(svc, SIGNAL(newMessages(bool)), this, SLOT(newMessages(bool)));
    connect(svc, SIGNAL(viewInbox()), this, SLOT(viewInbox()));
#ifndef QTOPIA_NO_MMS
    connect(svc, SIGNAL(mmsMessage(QDSActionRequest)), this, SLOT(mmsMessage(QDSActionRequest)));
#endif
#endif

    svc = new MessagesService( this );
    connect(svc, SIGNAL(newMessages(bool)), this, SLOT(newMessages(bool)));
    connect(svc, SIGNAL(message(QMailId)), this, SLOT(displayMessage(QMailId)));

    QTimer::singleShot(0, this, SLOT(delayedInit()) );
}

EmailClient::~EmailClient()
{
    // Delete any temporary files we have created
    while (!temporaries.isEmpty())
        temporaries.takeFirst().removeFiles();

    delete pm_folder;
    delete pm_trash;
    EmailListItem::deletePixmaps();
    delete emailHandler;

    qLog(Messaging) << "Destroyed EmailClient";
}

void EmailClient::openFiles()
{
    QPerformanceLog(appTitle.toLatin1().constData()) << " : " << "Begin openFiles: "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    if ( filesRead ) {
        if ( cachedDisplayMailId.isValid() )
            displayCachedMail();

        return;
    }

    filesRead = true;

    readMail();
    
    if ( cachedDisplayMailId.isValid() ) {
        displayCachedMail();
    } else {
        //No default select for QTreeWidget
        Folder* currentFolder = folderView()->currentFolder();
        if(!currentFolder)
            folderView()->changeToSystemFolder(MailboxList::InboxString);
        else
            folderSelected( folderView()->currentFolder() );

        displayPreviousMail();
    }

    QPerformanceLog(appTitle.toLatin1().constData()) << " : " << "End openFiles: "
                      << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    qLog(Messaging) << "Mail boxes connected";
}

void EmailClient::displayPreviousMail()
{
    if (!mMessageView)
        return;

    QSettings mailconf("Trolltech","qtmail");
    mailconf.beginGroup("qtmailglobal");
	QMailId id(mailconf.value("currentmail").toULongLong());
    mailconf.endGroup();
    if ( id.isValid() ) {
        messageView()->setSelectedId(id);
    }
}

void EmailClient::displayFolder(const QString &folder)
{
    delayedInit();
    messageView();
    folderView()->changeToSystemFolder( folder );
    
    if (EmailFolderList *box = mailboxList()->mailbox(folder)) {
        folderSelected( folderView()->currentFolder() );

        if (box->mailbox() == MailboxList::InboxString) {
            // If we just entered the Inbox, we should reset the new message count
            if (emailHandler->newMessageCount())
                resetNewMessages();
        }
    }

    showMessageList();
}

void EmailClient::displayCachedMail()
{
    QMailMessage mail(cachedDisplayMailId, QMailMessage::Header);
    EmailFolderList *box = mailboxList()->mailbox(mail.parentFolderId());
    if ( box ) {
        folderView()->changeToSystemFolder( box->mailbox());
        showViewer(cachedDisplayMailId, folderView()->currentFolder(), (mail.messageType() == QMailMessage::Email));
    }
    cachedDisplayMailId = QMailId();
}

void EmailClient::displayMessage(const QMailId &id)
{
    initialAction = EmailClient::View;
    delayedInit();

    if (!checkMailConflict(tr("Should this message be saved in Drafts before viewing the new message?"), 
                           tr("'View Mail' message will be ignored")) ) {
        cachedDisplayMailId = id;
        openFiles();
    }
}

void EmailClient::delayedShowMessage(MailAccount::AccountType acct, QMailId id, bool userRequest)
{
    if (initialAction != None) {
        // Ensure we don't close while we're waiting for incoming data
        registerTask("display");
    }

    showMsgList = false;
    showMessageType = acct;
    showMsgId = id;

    newMessagesRequested = userRequest;
    emailHandler->synchroniseClients();
}

void EmailClient::displayRecentMessage()
{
    qLog(Messaging) << "Displaying message";

    unregisterTask("display");
    if ( checkMailConflict(
        tr("Should this mail be saved in Drafts before viewing the new message?"),
        tr("'View Mail' message will be ignored")) )
        return;
    
    updateListViews();
    if (!showMsgList)
        queryItemSelected();
    else
        showMessageList();
}

// Ensure the folder list and message list are synchronized with the ReadMail widget
void EmailClient::updateListViews()
{
    openFiles();

    folderView()->changeToSystemFolder(MailboxList::InboxString);

    if (!showMsgList) {
        if (showMsgId.isValid()) {
            messageView()->setSelectedId(showMsgId);
        }
    }
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

    if (!accountList)
        return result;

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
        
     connect(emailHandler, SIGNAL(updateReceiveStatus(const Client*, QString)),
             this, SLOT(updateReceiveStatusLabel(const Client*, QString)) );
     connect(emailHandler, SIGNAL(updateSendStatus(const Client*, QString)),
             this, SLOT(updateSendStatusLabel(const Client*, QString)) );
    
     connect(emailHandler, SIGNAL(mailboxSize(int)),
             this, SLOT(setTotalPopSize(int)) );
     connect(emailHandler, SIGNAL(downloadedSize(int)),
             this, SLOT(setDownloadedSize(int)) );
     //smtp
     connect(emailHandler, SIGNAL(transferredSize(int)),
             this, SLOT(setTransferredSize(int)) );
     connect(emailHandler, SIGNAL(mailSendSize(int)),
             this, SLOT(setTotalSmtpSize(int)) );
     connect(emailHandler, SIGNAL(mailSent(int)), 
             this, SLOT(mailSent(int)) );
     connect(emailHandler, SIGNAL(transmissionCompleted()), 
             this, SLOT(transmissionCompleted()) );
    
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
        
     connect(emailHandler, SIGNAL(mailArrived(QMailMessage)), this,
             SLOT(mailArrived(QMailMessage)) );
     connect(emailHandler, SIGNAL(mailTransferred(int)), this,
             SLOT(allMailArrived(int)) );
     //imap
     connect(emailHandler, SIGNAL(serverFolders()), this,
             SLOT(imapServerFolders()) );
     connect(emailHandler, SIGNAL(nonexistentMessage(QMailId)), this,
             SLOT(nonexistentMessage(QMailId)) );
        
     connect(emailHandler, SIGNAL(allMessagesReceived()), this,
             SLOT(clientsSynchronised()) );

     //set relevant accounts
        
     QListIterator<MailAccount*> it = accountList->accountIterator();
     while ( it.hasNext() ) {
#if !defined(QTOPIA_NO_SMS) || !defined(QTOPIA_NO_MMS)
         MailAccount *account = it.next();
#endif
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

void EmailClient::initActions()
{
    if (selectAccountMenu)
        return; // Already inited
    
    QMenu *actionContext = QSoftMenuBar::menuFor( mActionView );
    QMenu *folderContext = QSoftMenuBar::menuFor( folderView() );
    QMenu *messageContext = QSoftMenuBar::menuFor( messageView() );
        
    if (!pm_folder)
        pm_folder = new QIcon(":icon/folder");
    if (!pm_trash)
        pm_trash = new QIcon(":icon/trash");
    
    selectAccountMenu = new QMenu(mb);
    connect(selectAccountMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(selectAccount(QAction*)));

    getMailButton = new QAction( QIcon(":icon/getmail"), tr("Get all mail"), this );
    connect(getMailButton, SIGNAL(triggered()), this, SLOT(getAllNewMail()) );
    getMailButton->setWhatsThis( tr("Get new mail from all your accounts.") );
    setActionVisible(getMailButton, false);

    cancelButton = new QAction( QIcon(":icon/reset"), tr("Cancel transfer"), this );
    connect(cancelButton, SIGNAL(triggered()), this, SLOT(cancel()) );
    cancelButton->setWhatsThis( tr("Abort all transfer of mail.") );
    setActionVisible(cancelButton, false);

    movePop = new QMenu(this);
    copyPop = new QMenu(this);
    connect(movePop, SIGNAL(triggered(QAction*)),
            this, SLOT(moveMailItem(QAction*)));
    connect(copyPop, SIGNAL(triggered(QAction*)),
            this, SLOT(copyMailItem(QAction*)));

    composeButton = new QAction( QIcon(":icon/new"), tr("New"), this );
    connect(composeButton, SIGNAL(triggered()), this, SLOT(compose()) );
    composeButton->setWhatsThis( tr("Write a new message.") );

    lastSearch = new Search();
    QSettings mailconf("Trolltech","qtmail");
    mailconf.beginGroup("lastSearch");
    lastSearch->readSettings( &mailconf );
    mailconf.endGroup();
    
    searchButton = new QAction( QIcon(":icon/find"), tr("Search"), this );
    connect(searchButton, SIGNAL(triggered()), this, SLOT(search()) );
    searchButton->setWhatsThis( tr("Search for messages in your folders.") );

    settingsAction = new QAction( QIcon(":icon/settings"), tr("Account settings..."), this );
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(settings()));

    emptyTrashAction = new QAction(tr("Empty trash"), this );
    connect(emptyTrashAction, SIGNAL(triggered()), this, SLOT(emptyTrashFolder()));
    setActionVisible(emptyTrashAction, false);

    moveAction = new QAction( this );
    connect(moveAction, SIGNAL(triggered()), this, SLOT(moveMessage()));
    setActionVisible(moveAction, false);

    copyAction = new QAction( this );
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copyMessage()));
    setActionVisible(copyAction, false);

    selectAllAction = new QAction( tr("Select all"), this );
    connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
    setActionVisible(selectAllAction, false);

    deleteMailAction = new QAction( this );
    deleteMailAction->setIcon( *pm_trash );
    connect(deleteMailAction, SIGNAL(triggered()), this, SLOT(deleteMailItem()));
    setActionVisible(deleteMailAction, false);

    actionContext->addAction( searchButton );
    actionContext->addAction( emptyTrashAction );
    actionContext->addAction( settingsAction );

    folderContext->addAction( composeButton );
    folderContext->addAction( getMailButton );
    folderContext->addAction( searchButton );
    folderContext->addAction( cancelButton );
    folderContext->addAction( emptyTrashAction );
    folderContext->addAction( settingsAction );

    messageContext->addAction( composeButton );
    messageContext->addAction( deleteMailAction );
    messageContext->addAction( moveAction );
    messageContext->addAction( copyAction );
    messageContext->addAction( selectAllAction );

    updateAccounts();
}

void EmailClient::updateActions()
{
    openFiles();
    
    // Ensure that the actions have been initialised
    initActions();

    // Only enable empty trash action if the trash has messages in it
    EmailFolderList *trash = mailboxList()->mailbox(MailboxList::TrashString);
    int type = QMailMessage::AnyType;
    if ( currentMailboxWidgetId() == actionId ) {
        type = QMailMessage::Mms | QMailMessage::Sms | QMailMessage::System;
    } else if (currentMailboxWidgetId() == folderId) {
        type = QMailMessage::Email;
    }
    
    int count = trash->mailCount(EmailFolderList::All, type);
    setActionVisible(emptyTrashAction, (count > 0));

    // Set the visibility for each action to whatever was last configured   
    QMap<QAction*, bool>::iterator it = actionVisibility.begin(), end = actionVisibility.end();
    for ( ; it != end; ++it)
        it.key()->setVisible(it.value());
}

void EmailClient::delayedInit()
{
    if (accountList)
        return; // delayedInit already done
    
    if (initialAction == None) {
        // We have been launched and raised by QPE - we'll start in the actionlist
        pushLocation(UILocation(this, actionId));
    }

    connect( &fetchTimer, SIGNAL(timeout()), this, SLOT(automaticFetch()) );
    accountList = new AccountList(this, "accountList");
    getPath("enclosures/", true);  //create directory enclosures

    sysMessagesChannel =  new QtopiaChannel("QPE/SysMessages", this);
    connect(sysMessagesChannel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(handleSysMessages(QString,QByteArray)));
    
    connect(&showMessageTimer, SIGNAL(timeout()), this,
            SLOT(displayRecentMessage()));

    connect(accountList, SIGNAL(checkAccount(int)),
            this, SLOT(selectAccount(int)) );

    connect(&checkAccountTimer, SIGNAL(timeout()),
            this, SLOT(selectAccountTimeout()) );

    connect(&planeMode, SIGNAL(contentsChanged()),
            this, SLOT(planeModeChanged()) );

    accountList->readAccounts();
    createEmailHandler();
    readSettings();

    // Ideally would make actions functions methods and delay their
    // creation until context menu is shown.
    initActions(); 

    folderView()->setupFolders( accountList );

    QTimer::singleShot(0, this, SLOT(collectSysMessages()) );
    QTimer::singleShot(0, this, SLOT(openFiles()) );
    qLog(Messaging) << "Created EMailClient";
}

void EmailClient::init()
{
    mReadMail = 0;
    mWriteMail = 0;
    selectAccountMenu = 0;
    getMailButton = 0;
    cancelButton = 0;
    movePop = 0;
    copyPop = 0;
    composeButton = 0;
    searchButton = 0;
    settingsAction = 0;
    emptyTrashAction = 0;
    moveAction = 0;
    copyAction = 0;
    selectAllAction = 0;
    deleteMailAction = 0;

    vbox = new QFrame(this);
    vboxLayout = new QVBoxLayout(vbox);
    vboxLayout->setMargin( 0 );
    vboxLayout->setSpacing( 0 );

    mailboxView = new QStackedWidget( vbox );
    mailboxView->setObjectName( "mailboxView" );
    vboxLayout->addWidget( mailboxView );

    mActionView = new ActionListView( mailboxView );
    mActionView->setObjectName( "actionView" );
    mActionView->setFrameStyle( QFrame::NoFrame );
    actionId = mailboxView->addWidget( mActionView );

    connect(mActionView, SIGNAL(composeMessage()), 
	    this, SLOT(compose()) );
    connect(mActionView, SIGNAL(emailSelected()), 
	    this, SLOT(showFolderList()) );
    connect(mActionView, SIGNAL(displayFolder(QString)),
	    this, SLOT(displayFolder(QString)) );
    connect(mActionView, SIGNAL(currentFolderChanged(QString)),
	    this, SLOT(currentActionViewChanged(QString)) );

    /* Create context menus for list of folders and messages */
    QMenu *actionContext = QSoftMenuBar::menuFor( mActionView );
    QMenu *folderContext = QSoftMenuBar::menuFor( folderView() );
    QMenu *messageContext = QSoftMenuBar::menuFor( messageView() );

    connect( actionContext, SIGNAL(aboutToShow()), this, SLOT(updateActions()) );
    connect( folderContext, SIGNAL(aboutToShow()), this, SLOT(updateActions()) );
    connect( messageContext, SIGNAL(aboutToShow()), this, SLOT(updateActions()) );

    QtopiaIpcAdaptor::connect(this, SIGNAL(messageCountUpdated()),
                              &messageCountUpdate, MESSAGE(changeValue()));

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

    //  In case user changed status of sent/unsent or read/unread messages
    if (mFolderView) {
        Folder *folder = folderView()->currentFolder();
        if ( folder ) {
            updateFolderCount( folder->mailbox() );
        }
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
void EmailClient::autosaveMail(const QMailMessage& mail)
{
    // if uuid is not valid, it's a new mail
    bool isNew = !mail.id().isValid();

    //  Always autosave new messages to drafts folder
    if ( isNew ) {
        saveAsDraft( mail );
    } else {
        // update mail in same mailbox as it was previously stored
        if ( mailboxList()->mailbox(MailboxList::OutboxString)->contains( mail.id() ) ) {
            enqueueMail( mail );
        } else {
            saveAsDraft( mail );
        }
    }
}

/*  Enqueue mail must always store the mail in the outbox   */
void EmailClient::enqueueMail(const QMailMessage& mailIn)
{
    QMailMessage mail(mailIn);

    // if uuid is not valid , it's a new mail
    bool isNew = !mail.id().isValid();

    if ( isNew ) {
        mailResponded();

        if ( !mailboxList()->mailbox(MailboxList::OutboxString)->addMail(mail) ) {
            accessError(mailboxList()->mailbox(MailboxList::OutboxString) );
            return;
        }
    } else {
        // two possibilities, mail was originally from drafts but is now enqueued, or
        // the mail was in the outbox previously as well.
        
        EmailFolderList* draftsFolder = mailboxList()->mailbox(MailboxList::DraftsString);
        EmailFolderList* outboxFolder = mailboxList()->mailbox(MailboxList::OutboxString);

        if(draftsFolder->contains(mail.id()) )
        {
            if(!draftsFolder->moveMail(mail.id(),*outboxFolder))
            {
                moveError( *draftsFolder, *outboxFolder );
                return;
            }
        }
        //have to re-add since this updates any changes to ogl mail.
        //TODO refactor to an explicit update when emailfolderlist is refactored.
        if ( !mailboxList()->mailbox(MailboxList::OutboxString)->addMail( mail ) ) {
            accessError( mailboxList()->mailbox(MailboxList::OutboxString) );
            return;
        }
    }

    if (!closeAfterWrite)
        restoreView();

    if (planeMode.value().toBool()) {
        // Cannot send right now, in plane mode!
        QMessageBox::information(0, 
                                 tr("Airplane safe mode"),
                                 tr("Saved message to Outbox. Message will be sent after exiting Airplane Safe mode."));
    } else {
        sendAllQueuedMail(true);
    }

    if (closeAfterWrite) {
        closeAfterTransmissionsFinished();
        if (isTransmitting()) // prevents flicker
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
    }
}

/*  Simple, do nothing  */
void EmailClient::discardMail()
{
    // Reset these in case user chose reply but discarded message
    repliedFromMailId = QMailId();
    repliedFlags = 0;

    restoreView();

    if (closeAfterWrite) {
        closeAfterTransmissionsFinished();
        if (isTransmitting())
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
    }
}

void EmailClient::saveAsDraft(const QMailMessage& mailIn)
{
    QMailMessage mail(mailIn);

    // if uuid is not valid, it's a new mail
    bool isNew = !mail.id().isValid();

    if ( isNew ) {
        mailResponded();

        if ( !mailboxList()->mailbox(MailboxList::DraftsString)->addMail(mail) ) {
            accessError( mailboxList()->mailbox(MailboxList::DraftsString) );
            return;
        }
    } else {

        // two possibilities, mail was originally from outbox but is now a draft, or
        // the mail was in the drafts folder previously as well.
        
        EmailFolderList* outboxFolder = mailboxList()->mailbox(MailboxList::OutboxString);
        EmailFolderList* draftsFolder = mailboxList()->mailbox(MailboxList::DraftsString);
        
        if ( outboxFolder->contains( mail.id() ) ) {
            if(!outboxFolder->moveMail(mail.id(),*draftsFolder))
            {
                moveError( *outboxFolder, *draftsFolder );
                return;
            }
        }
        //have to re-add since this updates any changes.
        //TODO refactor to an update when emailfolderlist is refactored.
        if( !mailboxList()->mailbox(MailboxList::DraftsString)->addMail( mail ) ) {
            accessError( mailboxList()->mailbox(MailboxList::DraftsString) );
            return;
        }
    }

    restoreView();
}

/*  Mark a message as replied/repliedall/forwarded  */
void EmailClient::mailResponded()
{
    if ( repliedFromMailId.isValid() ) {
        QString mailbox = MailboxList::InboxString;  //default search path
        Folder *folder = folderView()->currentFolder();
        if ( folder )
            mailbox = folder->mailbox();    //could be trash, etc..

        QMailMessage replyMail(repliedFromMailId,QMailMessage::Header);
        replyMail.setStatus(replyMail.status() | repliedFlags);
        QMailStore::instance()->updateMessage(&replyMail);
    }
    repliedFromMailId = QMailId();
    repliedFlags = 0;
}

/*  Find an appropriate account for the mail and format
    the mail accordingly    */
MailAccount* EmailClient::smtpForMail(QMailMessage& message)
{
    message.setReplyTo( QMailAddress() );

    /*  Let's see if we the emailAddress matches a SMTP account */
    QMailAddress fromAddress( message.from() );
    MailAccount *account = accountList->getSmtpRefByMail( fromAddress.address() );
    if ( account != NULL ) {
        message.setFromAccount( account->id() );
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
        message.setReplyTo( fromAddress );
        message.setFromAccount( account->id() );
        return account;
    }

    /*  No SMTP-account defined */
    return NULL;
}

// send all messages in outbox, by looping through the outbox, sending
// each message that belongs to the current found account
void EmailClient::sendAllQueuedMail(bool userRequest)
{
    if (planeMode.value().toBool()) {
        // Cannot send right now, in plane mode!
        return;
    }

    bool verifiedAccounts = false;
    bool haveValidAccount = false;
    QList<QMailMessage> queuedMessages;
    queuedMailIds.clear();
    smtpAccount = 0;

    EmailFolderList* outbox = mailboxList()->mailbox(MailboxList::OutboxString);
    QMailIdList outgoingIds = outbox->messages();

    int outgoingCount(outgoingIds.count());
    if (outgoingCount == 0)
        return;

    if (userRequest) {
        // Tell the user we're responding
        QString detail;
        if (outgoingCount == 1) {
            QMailMessage mail(*outgoingIds.begin(), QMailMessage::Header);
            detail = mailType(mail);
        } else {
            detail = tr("%1 messages", "%1 >=2").arg(outgoingCount);
        }
        AcknowledgmentBox::show(tr("Sending"), tr("Sending:") + " " + detail);
    }

    foreach(QMailId id, outgoingIds)
    {
        QMailMessage mail(id,QMailMessage::Header);

        // mail not previously sent, and has recipients defined, add to queue
        if ( !(mail.status() & QMailMessage::Sent) 
             && mail.hasRecipients() ) {

            if (mail.messageType() == QMailMessage::Email) {
                // Make sure we have a valid account
                if (!verifiedAccounts) {
                    haveValidAccount = verifyAccounts(true);
                    verifiedAccounts = true;
                    if (!haveValidAccount)
                        qWarning("Queued mail requires valid email accounts but none available.");
                }
                if (!haveValidAccount) {
                    // No valid account.  Move to Drafts and continue.
                    EmailFolderList *outbox = mailboxList()->mailbox(MailboxList::OutboxString);
                    EmailFolderList *drafts = mailboxList()->mailbox(MailboxList::DraftsString);
                    moveMailToFolder(mail.id(), outbox, drafts);
                    continue;
                }
            }

            /* The first mail determines which range of mails to first
               send.  As we allow use of several SMTP accounts we may
               need more than one connection, but the total number of connections
               needed will never exceed the number of SMTP accounts
            */
            if ( !smtpAccount ) {
                smtpAccount = smtpForMail( mail );
                queuedMessages.append(mail);
                queuedMailIds.append( mail.id() );
            } else if ( smtpForMail(mail) == smtpAccount ) {
                queuedMessages.append(mail);
                queuedMailIds.append( mail.id() );
            }
        }

    }

    if (queuedMessages.count() > 0) {
        emailHandler->setSmtpAccount(smtpAccount);
        sending = true;
        setActionVisible(cancelButton, true);
        if (!receiving)
            queueStatus = Sending;

        sendSingle = false;
        isSending(true);

        registerTask("transfer");
        emailHandler->sendMail(queuedMessages);
    } else {
        qWarning("no more messages to send");
    }
}

void EmailClient::sendSingleMail(const QMailMessage& message)
{
    if (sending) {
        qWarning("sending in progress, no action performed");
        return;
    }

    if (planeMode.value().toBool()) {
        // Cannot send right now, in plane mode!
        return;
    }

    bool needAccount = false;
    if ( message.messageType() == QMailMessage::Email )
        needAccount = true;

    if ( needAccount && !verifyAccounts(true) ) {
        qWarning("Mail requires valid email accounts but none available.");

        moveOutboxMailsToDrafts();
        return;
    }

    QList<QMailMessage> queuedMessages;
    queuedMailIds.clear();

    QMailMessage sendMessage = message;

    smtpAccount = smtpForMail( sendMessage);
    queuedMessages.append(sendMessage);
    queuedMailIds.append( sendMessage.id() );
    emailHandler->setSmtpAccount(smtpAccount);

    sending = true;
    setActionVisible(cancelButton, true);
    if (!receiving)
        queueStatus = Sending;

    sendSingle = true;
    isSending(true);

    registerTask("transfer");
    emailHandler->sendMail(queuedMessages);
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
        QMessageBox box(tr("No SMTP Server"), tr("<qt>No valid SMTP server defined.<br><br>No emails could be sent.</qt>"), QMessageBox::Warning,
                        QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton, QMessageBox::NoButton );
        box.exec();
        ok = false;
    } else if ( !outgoing && mailAccount == NULL ) {
        QMessageBox box(tr("No POP or IMAP accounts defined"), tr("<qt>Get mail only works with POP or IMAP</qt>"), QMessageBox::Warning,
                        QMessageBox::Ok | QMessageBox::Default, QMessageBox::NoButton, QMessageBox::NoButton );
        ok = false;
    }

    return ok;
}

//some mail are obviously sent, but are all mail in the outbox sent
void EmailClient::mailSent(int count)
{
    // We are no longer sending, although we won't update the UI yet, as we
    // may start again... moveMail depends on sending to update status correctly
    sending = false;

    if (count == -1) {
    }
    else {
        // Why doesn't this code check \a count?

        EmailFolderList *mailbox = mailboxList()->mailbox(MailboxList::OutboxString);

        QListIterator<QMailId> qit( queuedMailIds );

        foreach(QMailId id,queuedMailIds)
        { 
            if ( mailbox->contains(id) ) {
                QMailMessage mail(id,QMailMessage::Header);
                mail.setStatus(QMailMessage::Sent,true);
                QMailStore::instance()->updateMessage(&mail);
                if ( !moveMailToFolder(mail.id(), mailbox, mailboxList()->mailbox(MailboxList::SentString) ) )
                    break;      //no point continuing to move
            }
        }

        if ( !sendSingle ) {
            //loop through, if not all messages sent, start over
            QMailIdList outgoingIds = mailboxList()->mailbox(MailboxList::OutboxString)->messages();
            foreach(QMailId id, outgoingIds)
            {
                QMailMessage mail(id,QMailMessage::Header);
                if ( !(mail.status() & QMailMessage::Sent) ) {
                    // We are still sending
                    sending = true;
                    sendAllQueuedMail();
                    return;
                }
            }
        }

        queuedMailIds.clear();
    }

    transmissionCompleted();
}

void EmailClient::transmissionCompleted()
{
    setActionVisible(cancelButton, false);

    sending = false;
    isSending(false);
}

void EmailClient::addMailToDownloadList(const QMailMessage& mail)
{
    if ( !mailAccount )
	return; // mail check cancelled
    
    if ( mail.status() & QMailMessage::Downloaded 
         || mail.fromAccount() != mailAccount->id() )
        return;

    if ( (mailAccount->maxMailSize() > -1) && (mail.size() > static_cast<uint> ( mailAccount->maxMailSize() * 1024 ) ) )
        return;

    if ( mailAccount->accountType() == MailAccount::IMAP ) {
        Mailbox *box = mailAccount->getMailboxRef( mail.fromMailbox() );
        if ( box ) {
            FolderSyncSetting fs = box->folderSync();
            if ( fs & Sync_OnlyHeaders ) {
                return;
            } else if ( fs & Sync_OnlyNew ) {
                if ( mail.status() & QMailMessage::ReadElsewhere )
                    return;
            }
        }
    }

    mailDownloadList.sizeInsert(mail.serverUid(), mail.size(), mail.id(), mail.fromMailbox() );
}

void EmailClient::getNewMail()
{
    if ( !verifyAccounts(false) )
        return;

    registerTask("transfer");

    receiving = true;
    previewingMail = true;
    updateGetMailButton(false);
    setActionVisible(cancelButton, true);
    selectAccountMenu->setEnabled(false);

    //get any previous mails not downloaded and add to queue
    mailDownloadList.clear();
    QMailIdList incomingIds = mailboxList()->mailbox(MailboxList::InboxString)->messages(QMailMessage::Downloaded,false);
    foreach(QMailId id, incomingIds){
        QMailMessage mail(id,QMailMessage::Header);
            addMailToDownloadList( mail );
    }
    emailHandler->setMailAccount(mailAccount);
    if (!sending)
        queueStatus = Receiving;

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

void EmailClient::getSingleMail(const QMailMessage& message)
{
    if (receiving) {
        QString user = mailAccount->id();
        if ( user == message.fromAccount() ) {
            mailDownloadList.append(message.serverUid(), message.size(), message.id(), message.fromMailbox() );
            setTotalPopSize( mailDownloadList.size() );
        } else {
            qWarning("receiving in progress, no action performed");
        }
        return;
    }
    mailAccount = accountList->getPopRefByAccount( message.fromAccount() );
    if (mailAccount == NULL) {
        QString temp = tr("<qt>Mail was retrieved from account %1<br>Redefine this account to get this mail</qt>").arg(message.fromAccount()) + "</qt>";
        QMessageBox::warning(0, tr("Account not defined"), temp, tr("OK"));
        return;
    }

    registerTask("transfer");

    receiving = true;
    previewingMail = false;
    allAccounts = false;
    updateGetMailButton(false);
    setActionVisible(cancelButton, true);
    selectAccountMenu->setEnabled(false);

    mailDownloadList.clear();
    mailDownloadList.sizeInsert(message.serverUid(), message.size(), message.id(), message.fromMailbox() );
    emailHandler->setMailAccount(mailAccount);
    quitSent = false;
    setTotalPopSize( mailDownloadList.size() );

    isReceiving(true);
    emailHandler->getMailByList(&mailDownloadList, true);
}

void EmailClient::unresolvedUidlArrived(QString &user, QStringList &list)
{
    QString msg = tr("<qt>%1<br>The following messages have been deleted "
                     "from the server by another email client and can not be completed:<br>").arg(user);

    QString mailList = "";

    QMailIdList accountIds = mailboxList()->mailbox(MailboxList::InboxString)->messagesFromAccount(mailAccount->id());
    foreach(QMailId id, accountIds)
    {
        QMailMessage mail(id,QMailMessage::Header);
        if (  !(mail.status() & QMailMessage::Downloaded )) { 
            if ( (list.contains( mail.serverUid() ) ) ) {
                QMailAddress fromAddress(mail.from());
                mailList += fromAddress.name() + " - "  + mail.subject() + "<br>";
            }
        }
    }

    QMessageBox::warning(0, tr("Unresolved mail"), msg + mailList + "</qt>", tr("OK"));
}

void EmailClient::readReplyRequested(const QMailMessage& mail)
{
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
    if ( netCfg.isEmpty() )
        return;

    QWapAccount wapAccount( netCfg );
    if ( wapAccount.mmsDeliveryReport() ) {
        QString msg(tr("<qt>Do you wish to send a Read Reply?</qt>"));
        if (QMessageBox::information(0, tr("Multimedia Message"), msg,
                                     QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
                QMailMessage rrmail;
                rrmail.setMessageType(QMailMessage::Mms);
                rrmail.setTo(mail.from());
                rrmail.setSubject(mail.subject());
                rrmail.setHeaderField("X-Mms-Message-Class", "Auto");
                rrmail.setHeaderField("X-Mms-Delivery-Report", "No");
                rrmail.setHeaderField("X-Mms-Read-Reply", "No");
                QString msg = tr("Sent MMS \"%1\" was read on: %2", "%1 = subject %2 = date");
                msg = msg.arg(mail.subject());
                msg = msg.arg(QDateTime::currentDateTime().toString());
                QMailMessagePart part;
                QMailMessageContentType type("text/plain; charset=ISO-8859-1");
                part.setBody(QMailMessageBody::fromData(msg, type, QMailMessageBody::EightBit));
                rrmail.appendPart(part);
                rrmail.setStatus(QMailMessage::Outgoing, true);
                rrmail.setStatus(QMailMessage::Downloaded, true);
                if ( !mailboxList()->mailbox(MailboxList::OutboxString)->addMail(rrmail) ) {
                    accessError(mailboxList()->mailbox(MailboxList::OutboxString) );
                    return;
                }
                sendSingleMail(rrmail);
            }
    }
# endif
    Q_UNUSED(mail);
}

/*  This function is basically here to ensure the header only
    mail is replaced so that the ListWidget remains the same */
void EmailClient::mailUpdated(const QMailId& id, const QString &mailbox)
{
    QMailMessage message(id,QMailMessage::Header);
    if(readMailWidget()->isVisible() )
        readMailWidget()->mailUpdated( message.id() );
    updateQuery( message, mailbox );
    updateFolderCount( mailbox );
}

void EmailClient::mailRemoved(const QMailId &uuid, const QString &mailbox)
{
    Folder *folder = folderView()->currentFolder();
    if (!folder)
        return;

    if ( folder->mailbox() == mailbox ) {
        EmailListItem *item = messageView()->getRef( uuid );
        if ( item ) {
            EmailListItem *newItem = 0;
            int row = messageView()->row( item );
            if (row > 0) //try below
                newItem = static_cast<EmailListItem *>(messageView()->item( row - 1, 0 ));
            if (newItem == NULL) //try above
                newItem = static_cast<EmailListItem *>(messageView()->item( row + 1, 0 ));

            messageView()->removeRow( row );
            if ( newItem )
                messageView()->setSelectedItem( newItem );

            messageSelectionChanged();
        } else {
            qWarning("Message already removed from view??");
        }
    }

    updateFolderCount( mailbox );       // Need to update count of associated folder in folderlistview
}

/*  Mail arrived from server, treated a bit differently than from disk */
void EmailClient::mailArrived(const QMailMessage& m)
{   
    //make sure mailbox is connect at this point
    openFiles();
    
    QMailMessage mail(m);

    {
        QtopiaIpcEnvelope e(QLatin1String("QPE/TaskBar"), QLatin1String("setLed(int,bool)"));
        e << LED_MAIL << true;
    }

    qLog(Messaging) << "Mail arrived";

#ifndef QTOPIA_NO_MMS
    bool newMessages = false;
    bool getNow = false;
#endif

    /*  Test for get-this-mail activated on mail in trash.  Replace
        mail in trash for consistency sake  */
    if ( mail.status() & QMailMessage::Downloaded ) {
        if ( mailboxList()->mailbox(MailboxList::TrashString)->contains( mail.id() ) ) {
            if ( !mailboxList()->mailbox(MailboxList::TrashString)->addMail(mail) ) {
                accessError( mailboxList()->mailbox(MailboxList::TrashString) );
            }
            return;
        }
#ifndef QTOPIA_NO_MMS
        QString mmsType = mail.headerFieldText("X-Mms-Message-Type");
        if (mmsType.contains("m-delivery-ind")) {
            QString msg;
            QString mmsStatus = mail.headerFieldText("X-Mms-Status");
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
            QString to = mail.headerFieldText("To");
            if (to.isEmpty())
                to = tr("Unspecified", "MMS recipient");
            QMessageBox::information(0, tr("Multimedia message"), msg.arg(to),
                                     QMessageBox::Yes, QMessageBox::NoButton);
            return;
        } else if (mmsType.contains("m-send-req")) {
            if (MailAccount *account = accountList->getAccountById(mail.fromAccount())) {
                // If the user has configured automatic download, we just retrieved this message immediately
                if (account->autoDownload()) {
                    unregisterTask("display");
                    newMessages = true;
                }
            }
        }

#endif
    } else if (!mail.headerFieldText("X-Mms-Message-Type").trimmed().isEmpty()) {
#ifndef QTOPIA_NO_MMS
        // We will simply process this message, and allow the NewMessages code to 
        // deal with the handling
        emailHandler->acceptMail(mail);

        // Update the count of new MMS messages
        QSettings mailconf("Trolltech","qtmail");
        mailconf.beginGroup("MMS");
        int count = mailconf.value("newMmsCount").toInt() + 1;
        mailconf.setValue("newMmsCount", count);
        mailconf.endGroup();

        {
            QtopiaIpcEnvelope e("QPE/System", "newMmsCount(int)");
            e << count;
        }

        if (MailAccount *account = accountList->getAccountById(mail.fromAccount())) {
            // If the user has configured automatic download, we should get this message immediately
            getNow = account->autoDownload();
        }
        newMessages = !getNow;
#endif
    }

    if ( !mailboxList()->mailbox(MailboxList::InboxString)->addMail(mail) ) {
        cancel();
        accessError( mailboxList()->mailbox(MailboxList::InboxString) );
    } else {
        Q_ASSERT(mail.id().isValid());

        if (mail.messageType() == QMailMessage::Sms)
            unreadSmsIds.append(mail.id());
    }

#ifndef QTOPIA_NO_MMS
    if (getNow) {
        registerTask("display");
        getSingleMail(mail);
    } else if (newMessages) {
        clientsSynchronised();
    }
#endif
     
    if ( previewingMail ) {
        addMailToDownloadList( mail );
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
        if (queueStatus == Receiving) {
            progressBar->reset();
            progressBar->setText("");
        }
    } else {
        allAccounts = false;
        receiving = false;
        autoGetMail = false;
        updateGetMailButton(true);
        setActionVisible(cancelButton, false);
        selectAccountMenu->setEnabled(true);

        if (queueStatus == Receiving) {
            progressBar->reset();
            progressBar->setText("");
        }

        Folder *folder = folderView()->currentFolder();
        if ( folder )
            updateFolderCount( folder->mailbox() );
        isReceiving(false);
    }
}

void EmailClient::moveMailFront(const QMailMessage& message)
{
    if ( !(message.status() & QMailMessage::Incoming) 
         || (message.status() & QMailMessage::Downloaded) )
        return;

    if ( (receiving) && (message.fromAccount() == mailAccount->id() ) )
        mailDownloadList.moveFront( message.serverUid() );
}

static EmailClient::ErrorMap smtpErrorInit()
{
    // Create a map of (not-yet translated) error strings - should be stored in ROM data
    static const EmailClient::ErrorEntry map[] = 
    {
        { QAbstractSocket::ConnectionRefusedError, QT_TRANSLATE_NOOP( "EmailClient",  "Connection refused" ) },
        { QAbstractSocket::RemoteHostClosedError, QT_TRANSLATE_NOOP( "EmailClient",  "Remote host closed the connection" ) },
        { QAbstractSocket::HostNotFoundError, QT_TRANSLATE_NOOP( "EmailClient",  "Host not found" ) },
        { QAbstractSocket::SocketAccessError, QT_TRANSLATE_NOOP( "EmailClient",  "Permission denied" ) },
        { QAbstractSocket::SocketResourceError, QT_TRANSLATE_NOOP( "EmailClient",  "Insufficient resources" ) },
        { QAbstractSocket::SocketTimeoutError, QT_TRANSLATE_NOOP( "EmailClient",  "Operation timed out" ) },
        { QAbstractSocket::DatagramTooLargeError, QT_TRANSLATE_NOOP( "EmailClient",  "Datagram too large" ) },
        { QAbstractSocket::NetworkError, QT_TRANSLATE_NOOP( "EmailClient",  "Network error" ) },
        { QAbstractSocket::AddressInUseError, QT_TRANSLATE_NOOP( "EmailClient",  "Address in use" ) },
        { QAbstractSocket::SocketAddressNotAvailableError, QT_TRANSLATE_NOOP( "EmailClient",  "Address not available" ) },
        { QAbstractSocket::UnsupportedSocketOperationError, QT_TRANSLATE_NOOP( "EmailClient",  "Unsupported operation" ) },
        { QAbstractSocket::UnknownSocketError, QT_TRANSLATE_NOOP( "EmailClient",  "Unknown error" ) },
    };

    return qMakePair( static_cast<const EmailClient::ErrorEntry*>(map), ARRAY_SIZE(map) );
}

void EmailClient::smtpError(int code, QString &msg)
{
    // Create a map of error text strings, once only
    static ErrorMap errorMap(smtpErrorInit());

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

    qLog(Messaging) << "smtpError" << code << msg;

    // Add the error text, if configured
    appendErrorText(temp, code, errorMap);

    temp += "</qt>";

    if (code != ErrCancel) {
        QMessageBox::warning(0, tr("Sending error"), temp, tr("OK") );
        progressBar->reset();
    } else {
        progressBar->setText( tr("Aborted by user") );
    }

    sending = false;
    isSending(false);
    setActionVisible(cancelButton, false);
    queuedMailIds.clear();
}

static EmailClient::ErrorMap popErrorInit()
{
    // Create a map of (not-yet translated) error strings - should be stored in ROM data
    static const EmailClient::ErrorEntry map[] = 
    {
        { ErrLoginFailed, QT_TRANSLATE_NOOP( "EmailClient", "Login failed. Check user name and password") },
        { ErrFileSystemFull, QT_TRANSLATE_NOOP( "EmailClient", "Mail check failed.") },
        { ErrNonexistentMessage, QT_TRANSLATE_NOOP( "EmailClient", "Message deleted from server.") },
        { QAbstractSocket::ConnectionRefusedError, QT_TRANSLATE_NOOP( "EmailClient",  "Connection refused" ) },
        { QAbstractSocket::RemoteHostClosedError, QT_TRANSLATE_NOOP( "EmailClient",  "Remote host closed the connection" ) },
        { QAbstractSocket::HostNotFoundError, QT_TRANSLATE_NOOP( "EmailClient",  "Host not found" ) },
        { QAbstractSocket::SocketAccessError, QT_TRANSLATE_NOOP( "EmailClient",  "Permission denied" ) },
        { QAbstractSocket::SocketResourceError, QT_TRANSLATE_NOOP( "EmailClient",  "Insufficient resources" ) },
        { QAbstractSocket::SocketTimeoutError, QT_TRANSLATE_NOOP( "EmailClient",  "Operation timed out" ) },
        { QAbstractSocket::DatagramTooLargeError, QT_TRANSLATE_NOOP( "EmailClient",  "Datagram too large" ) },
        { QAbstractSocket::NetworkError, QT_TRANSLATE_NOOP( "EmailClient",  "Network error" ) },
        { QAbstractSocket::AddressInUseError, QT_TRANSLATE_NOOP( "EmailClient",  "Address in use" ) },
        { QAbstractSocket::SocketAddressNotAvailableError, QT_TRANSLATE_NOOP( "EmailClient",  "Address not available" ) },
        { QAbstractSocket::UnsupportedSocketOperationError, QT_TRANSLATE_NOOP( "EmailClient",  "Unsupported operation" ) },
        { QAbstractSocket::UnknownSocketError, QT_TRANSLATE_NOOP( "EmailClient",  "Unknown error" ) },
    };

    return qMakePair( static_cast<const EmailClient::ErrorEntry*>(map), ARRAY_SIZE(map) );
}

void EmailClient::popError(int code, QString &msg)
{
    static ErrorMap errorMap(popErrorInit());

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

    // Add the error text, if configured
    appendErrorText(temp, code, errorMap);

    if (code == ErrFileSystemFull)
        temp += QLatin1String(" ") + LongStream::errorMessage();

    temp += "</qt>";

    if (code != ErrCancel) {
        if ( !autoGetMail ) {
            QMessageBox::warning(0, tr("Receiving error"), temp, tr("OK") );
        } else {
            progressBar->setText( tr("Automatic fetch failed") );
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
    QMessageBox::warning(0, tr("Sending error"), temp.arg(msg), tr("OK") );

    sending = false;
    isSending(false);
    setActionVisible(cancelButton, false);
    queuedMailIds.clear();
}

void EmailClient::mmsError(int code, QString &msg)
{
#ifndef QTOPIA_NO_MMS
    Q_UNUSED(code)
    if (sending) {
        QString temp(tr("<qt>Failed sending MMS: %1</qt>", "%1 will contain the reason for the failure"));
        QMessageBox::warning(0, tr("Sending error"), temp.arg(msg), tr("OK") );

        sending = false;
        isSending(false);
        queuedMailIds.clear();
    } else if (receiving) {
        QString temp(tr("<qt>Failed receiving MMS: %1</qt>", "%1 will contain the reason for the failure"));
        QMessageBox::warning(0, tr("Receiving error"),
                temp.arg(msg), tr("OK") );
        receiving = false;
        autoGetMail = false;
        isReceiving(false);
        updateGetMailButton(true);
        setActionVisible(cancelButton, false);
        selectAccountMenu->setEnabled(true);
    }
    setActionVisible(cancelButton, false);
#else
    Q_UNUSED(code)
    Q_UNUSED(msg)
#endif
}

QString EmailClient::mailType(const QMailMessage& message)
{
    QString key(QMailComposerFactory::defaultKey(message.messageType()));
    if (!key.isEmpty())
        return QMailComposerFactory::displayName(key);

    return tr("Message");
}

void EmailClient::queryItemSelected()
{
    EmailListItem *item = static_cast<EmailListItem*>(messageView()->currentItem());
    if (item == NULL)
        return;
    if (!messageView()->isItemSelected(item))
        return;

    if (messageView()->currentMailbox() == MailboxList::DraftsString) {
        QMailMessage message = QMailMessage(item->id(),QMailMessage::HeaderAndBody);
        modify(message);
        return;
    }

    showViewer(item->id(), folderView()->currentFolder(), messageView()->showEmailsOnly());

    if (autoDownloadMail) {
        QMailMessage message = QMailMessage(item->id(),QMailMessage::HeaderAndBody);
        if (!(message.status() & QMailMessage::Downloaded))
            getSingleMail(message);
        autoDownloadMail = false;
    }

    QtopiaIpcEnvelope e( "QPE/TaskBar", "setLed(int,bool)" );
    e << LED_MAIL << false;
}

void EmailClient::resetNewMessages()
{
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

            if (mailconf.value("newSystemCount").toInt()) {
                mailconf.setValue("newSystemCount", 0);

                QtopiaIpcEnvelope e("QPE/System", "newSystemCount(int)");
                e << static_cast<int>(0);
            }
        }
    }

    emit messageCountUpdated();
}

void EmailClient::displayNewMessage( const QMailMessage& message )
{
    QString accountId = message.fromAccount();
    MailAccount *account = accountList->getAccountById( accountId );
	
    if (waitingForNewMessage
	    && showMessageType == MailAccount::SMS // can't handle MMS fast yet
	    && account
	    && account->accountType() == showMessageType
	    && (showServerId.isEmpty() || 
	        (showServerId == message.serverUid().right(showServerId.length())))) {
    
        showServerId = QString();
        waitingForNewMessage = false;

        if ( checkMailConflict(
             tr("Should this message be saved in Drafts before viewing the new message?"),
             tr("'View Mail' message will be ignored")) )
            return;
        
        // Return to the inbox after viewing this message
        folderView()->changeToSystemFolder(MailboxList::InboxString);

        showMsgId = message.id();
        showViewer(showMsgId, folderView()->currentFolder(), false);

        updateListViews();
        mReadMail->viewSelectedMail( messageView() );
        static_cast<EmailListItem *>(messageView()->currentItem())->updateState();
    }
}

void EmailClient::mailFromDisk(const QMailId& id, const QString &mailbox)
{
    // Don't create new messageView, i.e. if showing newly arrived message
    if (mMessageView)  {
        // Don't load the message details unless we need them
        if (Folder *folder = folderView()->currentFolder()) {
            if (folder->mailbox() == mailbox) {
                QMailMessage message(id, QMailMessage::Header);
                updateQuery(message, mailbox);
            }
        }
    }
    
    updateFolderCount( mailbox );
}

void EmailClient::mailMoved(const QMailId& id, const QString& sourceBox, const QString& destBox)
{
    mailRemoved(id, sourceBox);

    // Don't load the message details unless we need them
    if (Folder *folder = folderView()->currentFolder()) {
        if (folder->mailbox() == destBox) {
            QMailMessage message(id, QMailMessage::Header);
            updateQuery(message, destBox);
        }
    }

    updateFolderCount(destBox);
}

void EmailClient::mailMoved(const QMailIdList& list, const QString& sourceBox, const QString& destBox)
{
    int count(list.count());
    bool displayProgress(count >= MinimumForProgressIndicator);

    suspendMailCount = true;

    if (displayProgress) {
        QString caption;
        if ( count == 1 )
            caption = (tr("Moving 1 message"));
        else
            caption = tr("Moving %1 messages","number of messages always >=2").arg(count);
                
        progressBar->reset();
        progressBar->setRange(0, count);
        progressBar->setText(caption);
        qApp->processEvents();
    }

    QTime time;
    count = 0;
    foreach (const QMailId& id, list) {
        mailMoved(id, sourceBox, destBox);

        if (displayProgress) {
            ++count;

            // We still need to process events during this loop
            if ((count == 1) || (time.elapsed() > ProgressIndicatorUpdatePeriod)) {
                progressBar->setValue(count);
                time.start();
            }
        }
    }

    if (displayProgress) {
        progressBar->reset();
        progressBar->setText("");
    }

    suspendMailCount = false;

    updateFolderCount(sourceBox);
    updateFolderCount(destBox);
    messageSelectionChanged();
}

void EmailClient::readMail()
{
    mailboxList()->openMailboxes();

    EmailFolderList* outbox = mailboxList()->mailbox(MailboxList::OutboxString);
    if (outbox->mailCount(EmailFolderList::All)) {
        // There are messages ready to be sent
    	QTimer::singleShot( 0, this, SLOT(sendAllQueuedMail()) );
    }

    countList = mailboxList()->mailboxes();
    if (countList.count())
    	QTimer::singleShot( 0, this, SLOT(incrementalFolderCount()) );
}

void EmailClient::incrementalFolderCount()
{
    if (!countList.count())
	return;
    updateFolderCount( countList.first(), true );
    countList.removeFirst();
    if (countList.count())
	QTimer::singleShot( 0, this, SLOT(incrementalFolderCount()) );
}

void EmailClient::accessError(EmailFolderList *box)
{
    QString mailbox = "mailbox"; // No tr

    if ( box )
        mailbox = MailboxList::mailboxTrName( box->mailbox() );

    QString msg = tr("<qt>Cannot access %1. Either there is insufficient space, or another program is accessing the mailbox.</qt>").arg(mailbox);

    QMessageBox::critical( 0, tr("Save error"), msg );
}

void EmailClient::moveError(const EmailFolderList& source, const EmailFolderList& dest)
{
    QString mailbox1 = MailboxList::mailboxTrName( source.mailbox() );
    QString mailbox2 = MailboxList::mailboxTrName( dest.mailbox() );

    QString msg = tr("<qt>Cannot move message from %1 to %2. Either there is insufficient space, or another program is accessing the folders.</qt>").arg(mailbox1).arg(mailbox2);

    QMessageBox::critical( 0, tr("Move error"), msg );
}

void EmailClient::readSettings()
{
    int y;
    QSettings mailconf("Trolltech","qtmail");
    mailconf.beginGroup("qtmailglobal");

    if (( y = mailconf.value("mailidcount", -1).toInt()) != -1) {
        mailIdCount = y;
    }
    mailconf.endGroup();

    mailconf.beginGroup("settings");

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
    mailconf.endGroup();

    mailconf.beginGroup("qtmailglobal");

    if ( mailboxView )
        mailconf.setValue( "currentpage", currentMailboxWidgetId() );

    messageView()->writeConfig( &mailconf );

    EmailListItem *item = static_cast<EmailListItem*>(messageView()->currentItem());
    if ( item ) {
        QMailId id = item->id();
        mailconf.setValue("currentmail", id.toULongLong() );
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
    Q_UNUSED( id );
}

void EmailClient::editAccount(QAction* action)
{
    if (actionMap.contains(action))
        editAccount(actionMap[action]);
}

void EmailClient::deleteAccount(int id)
{
    Q_UNUSED( id );
}

void EmailClient::updateGetMailButton(bool enable)
{
    bool visible(false);

    if (enable) {
        // Enable send mail account if SMTP account exists
        QListIterator<MailAccount*> it = accountList->accountIterator();
        while ( it.hasNext() ) {
            MailAccount *account = it.next();
            if ( account->accountType() < MailAccount::SMS ) {
                // Enable send mail account if POP, IMAP, or Synchronized account exists
                visible = true;
                break;
            }
        }
    }

    setActionVisible(getMailButton, visible);
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
    updateGetMailButton(true);

    // accounts has been changed, update writemailwidget if it's created
    if ( mWriteMail )
        mWriteMail->setAccountList( accountList );
    if ( mReadMail )
        mReadMail->setAccountList( accountList );
}

void EmailClient::messageChanged()
{
    if (suspendMailCount)
        return;

    if (!messageView()->singleColumnMode())
        return;

    if ( currentMailboxWidgetId() != messageId )
        return;

    if (EmailListItem *item = static_cast<EmailListItem*>(messageView()->currentItem())) {
        QMailMessage message(item->id(), QMailMessage::Header);
        QString statusText( EmailListItem::dateToString( message.date().toLocalTime() ) );
        setStatusText( statusText );
    }
}

void EmailClient::deleteMail(const QMailStore::DeletionProperties& properties, bool deleting, EmailFolderList* srcFolder)
{
    static EmailFolderList* const trashFolder = mailboxList()->mailbox( MailboxList::TrashString );

    MailAccount *account = accountList->getAccountById( properties.fromAccount );

    if ( deleting ) {
        // Add it to queue of mails to be deleted from server
        if ( !properties.serverUid.isEmpty() ) {
            if ( account && account->deleteMail() ) {
                account->deleteMsg( properties.serverUid, properties.fromMailbox );
            }
        }

        trashFolder->removeMail( properties.id );
    } else {
        if ( account ) {
            // If the client has "deleteImmediately" set, then do so now.
            if ( Client *client = emailHandler->clientFromAccount(account) ) {
                if ( client->hasDeleteImmediately() )
                    client->deleteImmediately( properties.serverUid );
            }
        }

        // If mail is in queue for download, remove it from queue if possible
        mailDownloadList.remove( properties.serverUid );

        moveMailToFolder( properties.id, srcFolder, trashFolder );
    }
}

/*  handles two primary cases.  When a mail being deleted from inbox/outbox view
        it is transferred to trash, and if from trash it is expunged  */
bool EmailClient::deleteMail(EmailListItem *mailItem)
{
    Folder *folder = folderView()->currentFolder();
    if ( folder == NULL ) {
        qWarning("No folder selected, cannot delete mail");
        return false;
    }

    EmailFolderList* srcFolder = mailboxList()->mailbox( folder->mailbox() );

    const bool deleting(folder->folderType() == FolderTypeSystem && 
                        folder->mailbox() == MailboxList::TrashString);

    QMailMessage message(mailItem->id(), QMailMessage::Header);

    QMailStore::DeletionProperties properties;
    properties.id = message.id();
    properties.serverUid = message.serverUid();
    properties.fromAccount = message.fromAccount();
    properties.fromMailbox = message.fromMailbox();

    deleteMail(properties, deleting, srcFolder);

    return true;
}

bool EmailClient::deleteMailList(QList<EmailListItem*>& deleteList)
{
    Folder *folder = folderView()->currentFolder();
    if ( folder == NULL ) {
        qWarning("No folder selected, cannot delete mail");
        return false;
    }

    EmailFolderList* srcFolder = mailboxList()->mailbox( folder->mailbox() );

    const bool deleting(folder->folderType() == FolderTypeSystem && 
                        folder->mailbox() == MailboxList::TrashString);

    int count(deleteList.count());
    bool displayProgress(count >= MinimumForProgressIndicator);

    suspendMailCount = true;

    if (displayProgress) {
        QString caption;
        if (deleting) {
            if (deleteList.count() == 1 )
                caption = tr("Deleting message");
            else 
                caption = tr("Deleting messages");
        } else {
            if ( deleteList.count() == 1 )
                caption = tr("Moving message");
            else
                caption = tr("Moving messages");
        }

        progressBar->reset();
        progressBar->setRange(0, count);
        progressBar->setText(caption);
        qApp->processEvents();
    }

    QMailIdList ids;
    foreach (EmailListItem* mailItem, deleteList)
        ids.append(mailItem->id());

    QTime time;
    count = 0;
    foreach (const QMailStore::DeletionProperties& properties, QMailStore::instance()->deletionProperties(ids)) {
        deleteMail(properties, deleting, srcFolder);

        if (displayProgress) {
            ++count;

            // We still need to process events during this loop
            if ((count == 1) || (time.elapsed() > ProgressIndicatorUpdatePeriod)) {
                progressBar->setValue(count);
                time.start();
            }
        }
    }

    if (displayProgress) {
        progressBar->reset();
        progressBar->setText("");
    }

    suspendMailCount = false;

    return true;
}

bool EmailClient::moveMailToFolder(const QMailId& id, EmailFolderList *source, EmailFolderList *target)
{
    if ( source == target )
        return false;

    if(!source->moveMail(id,*target))
    {
        moveError(*source,*target);
        return false;
    }

    return true;
}

bool EmailClient::moveMailListToFolder(const QMailIdList& ids, EmailFolderList *source, EmailFolderList *target)
{
    if (source == target)
        return false;

    if (!source->moveMailList(ids,*target))
    {
        moveError(*source,*target);
        return false;
    }

    return true;
}

bool EmailClient::moveMailListToFolder(QList<EmailListItem*>& moveList, EmailFolderList *source, EmailFolderList *target)
{
    QMailIdList ids;
    foreach (EmailListItem* mailItem, moveList)
        ids.append(mailItem->id());
    return moveMailListToFolder( ids, source, target );
}

/*
void EmailClient::showItemMenu(EmailListItem *item)
{
    Q_UNUSED( item );

    Folder *folder = folderView()->currentFolder();

    if ( folder == NULL )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList()->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( MailboxList::OutboxString );

    QMenu *popFolder = new QMenu(this);
    movePop->clear();
    copyPop->clear();
    moveMap.clear();
    QAction *action;
    uint pos = 0;
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if (!pm_folder)
            pm_folder = new QIcon(":icon/folder");

        action = movePop->addAction(*pm_folder, MailboxList::mailboxTrName(*it) );
        moveMap.insert(action, pos);
        action = copyPop->addAction(*pm_folder, MailboxList::mailboxTrName(*it) );
        moveMap.insert(action, pos);

        pos++;
    }
    movePop->setTitle( tr("Move to") );
    copyPop->setTitle( tr("Copy to") );
    popFolder->addMenu( movePop );
    popFolder->addMenu( copyPop );
    if (!pm_trash)
        pm_trash = new QIcon(":icon/folder");
    popFolder->addAction(*pm_trash, tr("Delete message"), this, SLOT(deleteMailItem()) );
    popFolder->popup( QCursor::pos() );
}
*/

bool EmailClient::confirmDeleteWithoutSIM(int deleteCount)
{
    QString text;
    if ( deleteCount == 1 )
         text = tr("The SIM card is not ready. Do you want to delete the message without removal from the SIM card?");
    else
         text = tr("The SIM card is not ready. Do you want to delete the messages without removal from the SIM card?");

    return (QMessageBox::warning(0,
                                 tr("SIM not ready"),
                                 text,
                                 QMessageBox::Yes, 
                                 QMessageBox::No) == QMessageBox::Yes);
}

void EmailClient::deleteMailItem()
{
    Folder *folder = folderView()->currentFolder();
    if (!folder)
        return;

    if ( folder->folderType() == FolderTypeSystem &&
        folder->mailbox() == MailboxList::OutboxString &&
        sending) {
        return; //don't delete when sending
    }

    bool hasSms = false;
    QList<EmailListItem*> deleteList;
    for (int i = 0; i < messageView()->rowCount(); ++i) {
        EmailListItem* item = static_cast<EmailListItem *>(messageView()->item( i, 0 ));
        if ( messageView()->isItemSelected(item) ) {
           deleteList.append( item );
           hasSms |= (item->type() == QMailMessage::Sms);
        }
    }

    int deleteCount = deleteList.count();
    if ( deleteCount == 0)
        return;

    const bool deleting(folder->folderType() == FolderTypeSystem && 
                        folder->mailbox() == MailboxList::TrashString);

    QString action;
    QString actionDetails;
    if ( deleting ) {
        QString item;
        if ( deleteCount == 1  )
            item = tr("1 message");
        else
            item = tr("%1 messages","%1>=2 ->use plural").arg(deleteCount);

        if ( !Qtopia::confirmDelete( this, tr("Email"), item ) )
            return;

        action = tr("Deleting");
        if ( deleteCount == 1 )
            actionDetails = tr("Deleting 1 message");
        else
            actionDetails = tr("Deleting %1 messages", "%1>=2 -> use plural").arg(deleteCount);
    } else {
        // Messages will be removed from SIM on move to Trash
        if (!emailHandler->smsReadyToDelete() && hasSms) {
            if (confirmDeleteWithoutSIM(deleteCount) == false)
                return;
        }

        action = tr("Moving");
        if (deleteCount == 1 )
            actionDetails = tr("Moving 1 message to Trash");
        else
            actionDetails = tr("Moving %1 messages to Trash","%1>=2 ->use plural").arg(deleteCount);
    }

    if (deleteList.count() < MinimumForProgressIndicator) {
        // Tell the user we're doing what they asked for
        AcknowledgmentBox::show(action, actionDetails);
    }

    deleteMailList(deleteList);

    updateFolderCount(folder->mailbox());
    updateFolderCount(MailboxList::TrashString);
}


void EmailClient::moveMailItem(int index)
{
    Folder *folder = folderView()->currentFolder();
    if ( folder == NULL ) 
        return;

    if ( folder->folderType() == FolderTypeSystem &&
         folder->mailbox() == MailboxList::OutboxString &&
         sending) {
        return; //don't delete when sending
    }

    QList<EmailListItem*> moveList;
    EmailListItem *item = 0;
    for (int i = 0; i < messageView()->rowCount(); ++i) {
       item = static_cast<EmailListItem *>(messageView()->item( i, 0 ));
       if ( messageView()->isItemSelected( item ) )
           moveList.append( item );
    }

    if ( moveList.isEmpty() )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList()->mailboxes();
    list.removeAll( mailbox );
    list.removeAll(MailboxList::OutboxString);
    QString target = list[index];

    moveMailListToFolder( moveList, mailboxList()->mailbox(mailbox), mailboxList()->mailbox(target) );

    updateFolderCount(mailbox);
    updateFolderCount(target);
}

void EmailClient::moveMailItem(QAction *action)
{
    if (moveMap.contains(action))
        moveMailItem(moveMap[action]);
}

bool EmailClient::copyMailToFolder(const QMailId& id, EmailFolderList *source, EmailFolderList *target)
{
    if ( source == target )
        return false;

    if (!source->copyMail(id, *target)) {
        accessError(target);
        return false;
    }

    return true;
}

bool EmailClient::copyMailListToFolder(QList<EmailListItem*>& copyList, EmailFolderList *source, EmailFolderList *target)
{
    QString caption;
    if ( copyList.count() == 1 )
        caption = tr("Copying message");
    else 
        caption = tr("Copying messages","2 or more messages");
            
    return foreachListElement(&EmailClient::copyMailToFolder, copyList, source, target, caption);
}

void EmailClient::copyMailItem(int index)
{
    Folder *folder = folderView()->currentFolder();
    QList<EmailListItem*> copyList;
    EmailListItem *item = 0;
    uint size = 0;

    for (int i = 0; i < messageView()->rowCount(); ++i ) {
       item = static_cast<EmailListItem *>(messageView()->item( i, 0 ));
       if ( messageView()->isItemSelected( item ) ) {
           copyList.append( item );
           QMailMessage message(item->id(),QMailMessage::Header);
           size += message.size();
       }
    }

    if (!LongStream::freeSpace( "", size + 1024*10 )) {
        QString title( tr("Copy error") );
        QString msg( "<qt>" + tr("Storage for messages is full.<br><br>Could not copy messages.") + "</qt>" );
        QMessageBox::warning(0, title, msg, tr("OK") );
        return;
    }

    if ( copyList.isEmpty() )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList()->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( MailboxList::OutboxString );
    QString target = list[index];

    EmailFolderList* sourceBox = mailboxList()->mailbox(mailbox);
    EmailFolderList* targetBox = mailboxList()->mailbox(target);

    copyMailListToFolder( copyList, sourceBox, targetBox );

    updateFolderCount(mailbox);
    updateFolderCount(target);
}

void EmailClient::copyMailItem(QAction *action)
{
    if (moveMap.contains(action))
        copyMailItem(moveMap[action]);
}

bool EmailClient::foreachListElement(bool (EmailClient::*func)(const QMailId&, EmailFolderList*, EmailFolderList*), 
                                     QList<EmailListItem*>& list, EmailFolderList *source, EmailFolderList *target, const QString& caption)
{
    bool result(true);

    int count(list.count());
    bool displayProgress(count >= MinimumForProgressIndicator);

    suspendMailCount = true;

    if (displayProgress) {
        progressBar->reset();
        progressBar->setRange(0, count);
        progressBar->setText(caption);
        qApp->processEvents();
    }

    QTime time;
    count = 0;
    foreach (EmailListItem* mailItem, list) {
        result = ( result && (this->*func)( mailItem->id(), source, target ) );

        if (displayProgress) {
            ++count;

            // We still need to process events during this loop
            if ((count == 1) || (time.elapsed() > ProgressIndicatorUpdatePeriod)) {
                progressBar->setValue(count);
                time.start();
            }
        }
    }

    if (displayProgress) {
        progressBar->reset();
        progressBar->setText("");
    }

    suspendMailCount = false;

    return result;
}

void EmailClient::moveMessage()
{
    Folder *folder = folderView()->currentFolder();
    if ( folder == NULL )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList()->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( MailboxList::OutboxString);
    SelectFolderDialog *selectFolderDialog = new SelectFolderDialog(list);
    QtopiaApplication::execDialog( selectFolderDialog );

    if (selectFolderDialog->result() &&
        selectFolderDialog->folder() != -1)
        moveMailItem( selectFolderDialog->folder() );
    delete selectFolderDialog;
}

void EmailClient::copyMessage()
{
    Folder *folder = folderView()->currentFolder();
    if ( folder == NULL )
        return;

    QString mailbox = folder->mailbox();
    QStringList list = mailboxList()->mailboxes();
    list.removeAll( mailbox );
    list.removeAll( MailboxList::OutboxString);
    SelectFolderDialog *selectFolderDialog = new SelectFolderDialog(list);
    QtopiaApplication::execDialog( selectFolderDialog );

    if (selectFolderDialog->result() &&
        selectFolderDialog->folder() != -1)
        copyMailItem( selectFolderDialog->folder() );
    delete selectFolderDialog;
}

/* Select all messages */
void EmailClient::selectAll()
{
    messageView()->selectAll();
}

/*  currently only allowed for trash   */
void EmailClient::emptyTrashFolder()
{
    MailAccount *account;
    EmailFolderList *folderList;

    int type = QMailMessage::AnyType;
    if ( currentMailboxWidgetId() == actionId ) {
        type = QMailMessage::Mms | QMailMessage::Sms | QMailMessage::System;
    } else if (currentMailboxWidgetId() == folderId) {
        type = QMailMessage::Email;
        if (!folderView()->currentFolder())
            return;
    }
    folderList = mailboxList()->mailbox(MailboxList::TrashString);

    QString strName = tr("all messages in the trash");
    if (Qtopia::confirmDelete(this, appTitle, strName)) {

        QMailIdList trashIds = folderList->messages();

        int count(trashIds.count());
        bool displayProgress(count >= MinimumForProgressIndicator);

        suspendMailCount = true;

        if (displayProgress) {
            QString caption;
            if ( count == 1 )
                caption = (tr("Moving 1 message"));
            else
                caption = tr("Moving %1 messages",
                             "number of messages always >=2").arg(count);
                
            progressBar->reset();
            progressBar->setRange(0, count);
            progressBar->setText(caption);
            qApp->processEvents();
        }
        
        QTime time;
        count = 0;
        
        foreach(QMailId id, trashIds)
        {
            QMailMessage mail(id,QMailMessage::Header);
            if ((mail.messageType() & type) && 
                (mail.status() & QMailMessage::Incoming)) {
                account = accountList->getAccountById(mail.fromAccount());
                if (account != NULL) {
                    if (account->deleteMail())
                        account->deleteMsg(mail.serverUid(), mail.fromMailbox());
                }
            }
            if (displayProgress) {
                ++count;

                // We still need to process events during this loop
                if ((count == 1) || 
                    (time.elapsed() > ProgressIndicatorUpdatePeriod)) {
                    progressBar->setValue(count);
                    time.start();
                }
            }
        }
        folderList->empty(type);
        messageView()->clear();

        if (displayProgress) {
            progressBar->reset();
            progressBar->setText("");
        }

        suspendMailCount = false;

        updateFolderCount(MailboxList::TrashString);
    }

    update();
}

void EmailClient::setTotalSmtpSize(int size)
{
    if (queueStatus == Receiving && !receiving)
        queueStatus = Sending;

    if (queueStatus == Sending) {
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
    if (queueStatus == Sending && !sending)
        queueStatus = Receiving;

    if (queueStatus == Receiving) {
        progressBar->reset();
        if(size != 0)
            progressBar->setRange(0,size);
    } else {
        totalSize = size;
    }
}

void EmailClient::setDownloadedSize(int size)
{
    if (queueStatus == Sending && !sending) {
        queueStatus = Receiving;
        setTotalPopSize(totalSize);
    }

    if (queueStatus == Receiving) {
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
    if (queueStatus == Receiving && !receiving) {
        queueStatus = Sending;
        setTotalSmtpSize(totalSize);
    }

    if (queueStatus == Sending) {
        int total = progressBar->maximum();

        if (size > total)
            size = total;

        progressBar->setValue(size);
    } else {
        downloadSize = size;
    }
}

void EmailClient::updateQuery(const QMailMessage& message, const QString &mailbox)
{
    Folder *folder = folderView()->currentFolder();
    if (folder == NULL)
        return;

    if ( folder->mailbox() != mailbox )
        return;

    EmailListItem *item = messageView()->getRef( message.id() );
    if (item != NULL) {
        if ( folder->matchesEmail(message) ) {
            item->setId(message.id());
        } else {
            EmailListItem *newItem = item;

            if ( messageView()->isItemSelected(item) ) {
                int row = messageView()->row( item );
                if (row < messageView()->rowCount() - 1) //try below
                    newItem = static_cast<EmailListItem *>(messageView()->item(row + 1, 0));
                if (!newItem && row) //try above
                    newItem = static_cast<EmailListItem *>(messageView()->item(row - 1, 0));
            } else
                newItem = NULL;

            messageView()->removeRow( messageView()->row( item ) );
            if ( newItem ) {
                messageView()->setSelectedItem( newItem );
            }
            return;
        }
    } else if ( folder->matchesEmail(message) ) {
        if(messageView()->showEmailsOnly())
        {
            if(message.messageType() & QMailMessage::Email)
                messageView()->treeInsert(message.id());
        }
        else
        {
            if(!(message.messageType()  & QMailMessage::Email))
                messageView()->treeInsert(message.id());
        }
    }
}

void EmailClient::updateReceiveStatusLabel(const Client* client, const QString &txt)
{
    if (queueStatus == Receiving) {
        QString status(txt);
        if (!status.isEmpty()) {
            if (MailAccount* account = emailHandler->accountFromClient(client)) 
                if (!account->accountName().isEmpty())
                    status.prepend(account->accountName().append(" - "));
        }
        progressBar->setText(status);
    }
    if (mReadMail && mReadMail->isVisible())
        mReadMail->setProgressText(txt);
}

void EmailClient::updateSendStatusLabel(const Client* client, const QString &txt)
{
    if (queueStatus == Sending) {
        QString status(txt);
        if (!status.isEmpty()) {
            if (MailAccount* account = emailHandler->accountFromClient(client))
                if (!account->accountName().isEmpty())
                    status.prepend(account->accountName().append(" - "));
        }
        progressBar->setText(status);
    }
    if (mReadMail && mReadMail->isVisible())
        mReadMail->setProgressText(txt);
}

void EmailClient::rebuildMoveCopyMenus(const Folder *folder)
{
    //  Rebuild mail move/copy menus as they don't include the currently selected folder
    initActions();
    movePop->clear();
    copyPop->clear();
    QMapIterator<QAction*, int> i(moveMap);
    moveMap.clear();

    QStringList list = mailboxList()->mailboxes();
    QString mailbox = folder->mailbox();
    list.removeAll( mailbox );
    list.removeAll( MailboxList::OutboxString);

    QAction *action;
    uint pos = 0;
    for ( QStringList::Iterator itList = list.begin(); itList != list.end(); ++itList ) {
	if (!pm_folder)
	    pm_folder = new QIcon(":icon/folder");
        action = movePop->addAction(*pm_folder, MailboxList::mailboxTrName(*itList) );
        moveMap.insert(action, pos);
        action = copyPop->addAction(*pm_folder, MailboxList::mailboxTrName(*itList) );
        moveMap.insert(action, pos);

        pos++;
    }
}

void EmailClient::folderSelected(Folder *folder)
{
    if ( !mMessageView ) {
        // No messageview yet just update the statusbar
        if ( !folder )
            return;

        updateFolderCount(folder->mailbox());
        return;
    }


    if ( folder == NULL ) {
        return;
    }

    QMailIdList folderIds;

    unsigned int messageType;

    EmailFolderList* mailFolder = mailboxList()->mailbox(folder->mailbox());
    if(!mailFolder)
        mailFolder = mailboxList()->mailbox(MailboxList::InboxString);

    if(messageView()->showEmailsOnly())
        messageType = QMailMessage::Email;
    else
        messageType = QMailMessage::Mms | QMailMessage::Sms | QMailMessage::System;

    if(folder->folderType() == FolderTypeMailbox)
    {
        Mailbox* mailbox = static_cast<Mailbox*>(folder);
        folderIds = mailFolder->messagesFromMailbox(mailbox->pathName(),
                                                    messageType,
                                                    EmailFolderList::DescendingDate);  
    }
    else if(folder->folderType() == FolderTypeSystem)
    {
        folderIds = mailFolder->messages(messageType,
                                         EmailFolderList::DescendingDate);  
    }
    else if(folder->folderType() == FolderTypeAccount)
    {
        MailAccount* mailbox = static_cast<MailAccount*>(folder);
        QMailMessageKey folderKey(QMailMessageKey::FromAccount,mailbox->id());
        folderIds = mailFolder->messagesFromAccount(mailbox->id(),
                                                    messageType,
                                                    EmailFolderList::DescendingDate);  
    }
    messageView()->treeInsert(folderIds);

    rebuildMoveCopyMenus(folder);
    messageView()->setCurrentMailbox( folder->mailbox() );
    messageView()->setSelectedRow( 0 );
    updateFolderCount( folder->mailbox() );
}

/*  make sure that the currently displayed item in readmail is the same
    after the folder has been deleted and added again
    If this should fail the current item will be the first  */
void EmailClient::imapServerFolders()
{
    EmailListItem *item = static_cast<EmailListItem *>(messageView()->currentItem());
    QMailId selected;

    if ( item && messageView()->isItemSelected(item) )
        selected = item->id();

    folderView()->updateAccountFolder(mailAccount);

    if ( selected.isValid() ) {
        messageView()->setSelectedId( selected );
    }
}

void EmailClient::failedList(QStringList &list)
{
    QMessageBox::warning(0, tr("<qt>The following commands failed:<br>%1</qt>").arg(list.join("<br>")), tr("OK"));
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

void EmailClient::findMatchingMessages()
{
    folderView()->changeToSystemFolder( lastSearch->mailbox() );
    Folder *folder = folderView()->currentFolder();
    if (!folder)
        return;
    
    QMailIdList folderIds;
    unsigned int messageType = 0;
    if(messageView()->showEmailsOnly())
        messageType = QMailMessage::Email;
    else
        messageType = QMailMessage::Mms | QMailMessage::Sms | QMailMessage::System;

    EmailFolderList* mailFolder = mailboxList()->mailbox(folder->mailbox());
    folderIds = mailFolder->messages(messageType, EmailFolderList::DescendingDate);
    
    bool slowFolder = FALSE;
    Search *s = lastSearch;

    if ( s  && !s->getBody().isEmpty() ) {
        if ( !receiving && !sending ) {
            slowFolder = TRUE;
            progressBar->reset();
            progressBar->setRange(0,100);
        }
    }

    int at = 0;
    if (s) {
        messageView()->clear();
        while (at < folderIds.count()) {
            QMailMessage mail(folderIds[at], QMailMessage::HeaderAndBody);
        
            if (!mail.id().isValid()) {
                qWarning("null mailptr in listviewitem, should never occur");
            } else if ( lastSearch->matches(mail) ) {
                messageView()->treeInsert(mail.id());
            }
        
            ++at;
            if (slowFolder)
                progressBar->setValue( at*100/folderIds.count() );
        }
    } else {
        messageView()->treeInsert(folderIds);
    }
    if ( slowFolder )
        progressBar->reset();    

    rebuildMoveCopyMenus(folder);
    messageView()->setCurrentMailbox( folder->mailbox() );
    messageView()->setSelectedRow( 0 );
}

void EmailClient::search()
{
    SearchView searchView(false, this, Qt::Dialog); // No tr
    searchView.setObjectName("search"); // No tr
    searchView.setModal(true);
    searchView.setSearch( lastSearch );

    QtopiaApplication::execDialog(&searchView);

    if (searchView.result() == QDialog::Accepted) {
        // disconnect and reconnect cause we don't know whether the 
        // folderChanged signal will be emitted (worst case two folder 
        // changed events)
        disconnect(folderView(), SIGNAL(folderSelected(Folder*)), 
		   this, SLOT(folderSelected(Folder*)) );
        lastSearch = searchView.getSearch();
        findMatchingMessages();
        showMessageList();
        searchView.hide();

        connect(folderView(), SIGNAL(folderSelected(Folder*)), this, SLOT(folderSelected(Folder*)) );
        QSettings mailconf("Trolltech","qtmail");
        mailconf.beginGroup("lastSearch");
        lastSearch->saveSettings( &mailconf );
        mailconf.endGroup();
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
    folderSelected( folderView()->currentFolder() );

    QString msg = MailboxList::mailboxTrName( mailbox ) + " "; //no tr
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
        int oldId = currentMailboxWidgetId();
        mailboxView->setCurrentIndex( id );
        if (id == folderId) {
            if (oldId == actionId)
                folderView()->restoreCurrentFolder();
            messageView()->setShowEmailsOnly(true);
        } else if (id == messageId) {
            if (QTableWidgetItem* item = messageView()->currentItem())
                item->setSelected(true);
        } else if (id == actionId) {
            if (oldId == folderId)
                folderView()->rememberCurrentFolder();

            messageView()->setShowEmailsOnly(false);
            if (QListWidgetItem* item = mActionView->currentItem())
                item->setSelected(true);
        }

        if (mFolderView) {
            Folder *folder = folderView()->currentFolder();
            if (folder)
                updateFolderCount( folder->mailbox() );
        }
    }
}

void EmailClient::showEmailView()
{
    update();
    showMessageList();
}

void EmailClient::showFolderList(bool recordLocation)
{
    delayedInit();

    setCurrentMailboxWidget( folderId );
    
    // Updates the mailListView
    folderSelected( folderView()->currentFolder() );
    showWidget( this, tr("Email") );    

    if (recordLocation) {
        pushLocation(UILocation(this, folderId));
    }
}

void EmailClient::showMessageList(bool emailList, bool recordLocation)
{
    setCurrentMailboxWidget( messageId );

    if (messageView()->showEmailsOnly() != emailList) {
        messageView()->setShowEmailsOnly(emailList);
    }

    showWidget( this, folderView()->currentFolder()->displayName() );
    if (recordLocation) {
        pushLocation(UILocation(this, messageId, emailList));
    }
}

void EmailClient::showMessageList(bool recordLocation)
{
    showMessageList(messageView()->showEmailsOnly(), recordLocation);
}

void EmailClient::showActionList(bool recordLocation)
{
    setCurrentMailboxWidget( actionId );

    showWidget( this, appTitle );
    if (recordLocation) {
        pushLocation(UILocation(this, actionId));
    }
}

void EmailClient::showComposer(bool recordLocation)
{
    showWidget(mWriteMail);

    if (recordLocation) {
        pushLocation(UILocation(mWriteMail));
    }
}

void EmailClient::showViewer(const QMailId& messageId, Folder* folder, bool email, bool recordLocation)
{
    if ((messageView()->showEmailsOnly() != email) ||
        (folderView()->currentFolder() != folder)) {
        // Update the view for this folder's situation
        messageView()->setShowEmailsOnly(email);
        folderView()->setCurrentFolder(folder);
        folderSelected(folder);
    }

    messageView()->setSelectedId(messageId);
    readMailWidget()->viewSelectedMail(messageView());

    showWidget(mReadMail);

    if (recordLocation) {
        pushLocation(UILocation(mReadMail, messageId, folder, email));
    }
}

void EmailClient::restoreView()
{
    if (!haveLocation()) {
        // If we have never raised the app, we will have no locations
        return;
    }

    popLocation();

    if (!haveLocation()) {
        // We have finished
        closeAfterTransmissionsFinished();
        if (isTransmitting()) // prevents flicker
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
    } else {
        UILocation restoreLocation(currentLocation());

        if (restoreLocation.widget == mWriteMail) {
            showComposer(false);
        } else if (restoreLocation.widget == mReadMail) {
            showViewer(restoreLocation.messageId, restoreLocation.messageFolder, restoreLocation.email, false);
        } else {
            if (restoreLocation.widgetId == actionId)
                showActionList(false);
            else if (restoreLocation.widgetId == folderId)
                showFolderList(false);
            else if (restoreLocation.widgetId == messageId)
                showMessageList(restoreLocation.email, false);
        }

        QStringList mboxList = mailboxList()->mailboxes();
        for (QStringList::Iterator it = mboxList.begin(); it != mboxList.end(); ++it) {
            updateFolderCount( *it, true );
        }
    }
}

bool EmailClient::checkMailConflict(const QString& msg1, const QString& msg2)
{
    if ( writeMailWidget()->isVisible() ) {
        QString message = tr("<qt>You are currently editing a message:<br>%1</qt>").arg(msg1);
        switch( QMessageBox::warning( 0, tr("Messages conflict"), message,
                                      tr("Yes"), tr("No"), 0, 0, 1 ) ) {

            case 0:
            {
                if ( !mWriteMail->saveChangesOnRequest() ) {
                    QMessageBox::warning(0, 
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

void EmailClient::writeMailAction(const QMap<QString, QString> propertyMap )
{
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if ( checkMailConflict(
            tr("Should it be saved in Drafts before writing the new message?"),
            tr("'Write Mail' message will be ignored")) )
        return;

    QMailMessage mail;

    // Set all the properties defined in the supplied map
    EmailPropertySetter setter( mail );
    setter.setProperties( propertyMap );

    modify( mail );

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
#ifndef QTOPIA_NO_SMS
    if (isHidden() || !isVisible())
        closeAfterWrite = true;

    if ( checkMailConflict(
            tr("Should this message be saved in Drafts before writing the new message?"),
            tr("'Write SMS' message will be ignored")) )
        return;

    writeMailWidget()->newMail( QMailComposerFactory::defaultKey( QMailMessage::Sms ), vcard);
    QString recipient;
    if ( number.isEmpty() )
        recipient += "";
    else
        recipient += number;
    writeMailWidget()->setSmsRecipient( recipient );
    if (!body.isNull()) {
        writeMailWidget()->setBody(body,
                                    vcard ? QLatin1String("text/x-vCard")
                                          : QLatin1String("text/plain"));
    }
    mWriteMail->setAccountList( accountList );
    showComposer();

    openFiles();
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
            tr("Should this message be saved in Drafts before writing the new message?"),
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

    writeMailWidget()->newMail( QMailComposerFactory::defaultKey( QMailMessage::Email ) );
    if ( mWriteMail->composer().isEmpty() ) { 
        // failed to create new composer, maybe due to no email account 
        // being present. So hide/quit qtmail.
        if (isTransmitting()) // prevents flicker
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
        return;
    }
    mWriteMail->setRecipient( recipient );
    mWriteMail->setAccountList( accountList );
    showComposer();

    openFiles();
}

void EmailClient::emailVCard( const QDSActionRequest& request )
{
    QtopiaApplication::instance()->showMainWidget();

    QString leafname("email");

    QList<QContact> cardData( QContact::readVCard( request.requestData().data() ) );
    if (!cardData.isEmpty()) {
        const QContact& contact = cardData.first();
        QString name(contact.firstName() + contact.lastName());
        if (!name.isEmpty()) {
            // Remove any non-word chars to ensure we have a valid filename
            leafname = name.remove(QRegExp("\\W"));
        }
    }

    leafname += ".vcf";

    // Save the VCard data to a temporary document
    QString filename = Qtopia::tempDir() + leafname;
    {
        QFile temp( filename );
        if ( !temp.open( QIODevice::WriteOnly ) ) {
            qWarning() << "Unable to open path for write:" << filename;
            return;
        }

        temp.write( request.requestData().data() );
        temp.close();
    }

    QContent doc( filename );
    doc.setName( leafname );
    doc.setRole( QContent::Data );
    doc.commit();

    // This needs to be removed after send
    temporaries.append(doc);

    // write the Email
    writeMessageAction( QString(),
                        QString(),
                        QStringList( doc.fileName() ),
                        QStringList(),
                        QMailMessage::Email );

    // Respond to the request
    QDSActionRequest( request ).respond();
}

void EmailClient::flashSms( const QDSActionRequest& request )
{
#ifndef QTOPIA_NO_SMS
    // Extract the SMS message from the request payload.
    QByteArray data = request.requestData().data();
    QDataStream stream( data );
    QSMSMessage msg;
    stream >> msg;

    // Process the flash SMS message.
    // TODO
#endif

    // Respond to the request
    QDSActionRequest( request ).respond();
}

void EmailClient::collectSysMessages()
{
    QtopiaIpcEnvelope e ( "QPE/SysMessages", "collectMessages()" );
}

void EmailClient::setEnableMessageActions( bool enabled )
{
    if (!enabled)
        messageSelectionChanged();
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
        for( int i = 0, n = static_cast<int>(nsAddresses.count()) ; i < n; ++i )
        {
            QString c = nsAddresses[i];
            QString rc = rawAddresses[i];
            if( c.indexOf( '@' ) != -1 )
                emailAddresses += rc;
            else
            {
                bool hasPhoneChars = false;
                for( int j = 0, l = static_cast<int>(c.length()) ; j < l ; ++j )
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
        if (lnk.type() != "text/plain" && lnk.type() != "text/x-vCalendar" && lnk.type() != "text/x-vCard")
            textOnly = false;
    }
    for (it = fileAttachments.begin(); it != fileAttachments.end(); ++it) {
        int index = (*it).lastIndexOf('.');
        if (index != -1) {
            QString extn = (*it).mid(index);
            if (extn != ".txt" && extn != ".vcs" && extn != ".vcf")
                textOnly = false;
        }
    }

    QMailMessage::MessageType diid = QMailMessage::Mms;
    if (textOnly && emails.trimmed().isEmpty() && type & QMailMessage::Sms) {
        // if we're sending plain text and have no email recipients, use SMS
        diid = QMailMessage::Sms;
    } else if (!(type & QMailMessage::Mms)) {
        diid = QMailMessage::None;
        if (type & QMailMessage::Email)
            diid = QMailMessage::Email;
        if (type & QMailMessage::Sms)
            diid = static_cast<QMailMessage::MessageType>(diid | QMailMessage::Sms);
        if (!diid)
            diid = QMailMessage::Sms;
    }

    writeMailWidget()->newMail( QMailComposerFactory::defaultKey( diid ) );
    if ( mWriteMail->composer().isEmpty() ) { 
        // failed to create new composer, maybe due to no email account 
        // being present. So hide/quit qtmail.
        if (isTransmitting()) // prevents flicker
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
        return;
    }
    writeMailWidget()->setRecipients( emails, numbers );
    mWriteMail->setAccountList( accountList );

    for( int i = 0 ; i < docAttachments.count() ; ++i )
        mWriteMail->attach( QContent( docAttachments[i] ) );

    for( int i = 0 ; i < fileAttachments.count() ; ++i )
        mWriteMail->attach( fileAttachments[i] );

    showComposer();
}

void EmailClient::cleanupMessages( const QDate &removalDate, int removalSize )
{
    bool closeAfterCleanup = isHidden();

    openFiles();
    QStringList mboxList = mailboxList()->mailboxes();
    QStringList::Iterator it;
    for (it = mboxList.begin(); it != mboxList.end(); ++it)
    {
        EmailFolderList *box = mailboxList()->mailbox( *it );
        QMailIdList idList = box->messages();
        foreach(QMailId id, idList)
        {
            QMailMessage mail(id,QMailMessage::Header);
            QDate mailDate( mail.date().toLocalTime().date() );
            uint mailSize( mail.size() );
            if ((mailDate <= removalDate) && (static_cast<int>(mailSize) >= removalSize))
                box->removeMail( mail.id());
        }

    }

    if (closeAfterCleanup) {
        closeAfterTransmissionsFinished();
        if (isTransmitting())
            QTMailWindow::singleton()->hide();
        else
            QTMailWindow::singleton()->close();
    }
}

WriteMail *EmailClient::writeMailWidget()
{
    if ( !mWriteMail ) {
        mWriteMail = new WriteMail( this , "write-mail");
        if ( parentWidget()->inherits("QStackedWidget") )
            static_cast<QStackedWidget*>(parentWidget())->addWidget(mWriteMail);

        connect(mWriteMail, SIGNAL(enqueueMail(QMailMessage)), this,
                SLOT(enqueueMail(QMailMessage)) );
        connect(mWriteMail, SIGNAL(discardMail()), this,
                SLOT(discardMail()) );
        connect(mWriteMail, SIGNAL(saveAsDraft(QMailMessage)), this,
                SLOT(saveAsDraft(QMailMessage)) );
        connect(mWriteMail, SIGNAL(autosaveMail(QMailMessage)), this,
                SLOT(autosaveMail(QMailMessage)) );
        connect(mWriteMail, SIGNAL(noSendAccount(QMailMessage::MessageType)), this,
                SLOT(noSendAccount(QMailMessage::MessageType)) );

        mWriteMail->setAccountList( accountList );
    }

    return mWriteMail;
}

ReadMail *EmailClient::readMailWidget()
{   
    if ( !mReadMail ) {
        mReadMail = new ReadMail( this, "read-mail");
        if ( parentWidget()->inherits("QStackedWidget") )
            static_cast<QStackedWidget*>(parentWidget())->addWidget(mReadMail);

        connect(mReadMail, SIGNAL(cancelView()), this, SLOT(restoreView()) );
        connect(mReadMail, SIGNAL(resendRequested(QMailMessage,int)), this,
                SLOT(resend(QMailMessage,int)) );

        connect(mReadMail, SIGNAL(modifyRequested(QMailMessage)),this,
                SLOT(modify(QMailMessage)));
        connect(mReadMail, SIGNAL(removeItem(EmailListItem*)), this,
                SLOT(deleteMailRequested(EmailListItem*)) );
        connect(mReadMail, SIGNAL(viewingMail(QMailMessage)), this,
                SLOT(moveMailFront(QMailMessage)));
        connect(mReadMail, SIGNAL(getMailRequested(QMailMessage)),this,
                SLOT(getSingleMail(QMailMessage)) );
        connect(mReadMail, SIGNAL(sendMailRequested(QMailMessage)),this,
                SLOT(sendSingleMail(QMailMessage)));
        connect(mReadMail, SIGNAL(mailto(QString)), this,
                SLOT(setDocument(QString)) );
        connect(mReadMail,SIGNAL(readReplyRequested(QMailMessage)),this,
                SLOT(readReplyRequested(QMailMessage)));
        connect(mReadMail, SIGNAL(viewingMail(QMailMessage)),
                emailHandler, SLOT(mailRead(QMailMessage)) );

        mReadMail->setAccountList( accountList );
    }

    return mReadMail;
}

void EmailClient::resend(const QMailMessage& message, int type)
{
    repliedFromMailId = message.id();

    if (type == 1) {
        writeMailWidget()->setAction(WriteMail::Reply);
        repliedFlags = QMailMessage::Replied;
    } else if (type == 2) {
        writeMailWidget()->setAction(WriteMail::ReplyToAll);
        repliedFlags = QMailMessage::RepliedAll;
    } else if (type == 3) {
        writeMailWidget()->setAction(WriteMail::Forward);
        repliedFlags = QMailMessage::Forwarded;
    } else {
        return;
    }

    writeMailWidget()->reply(message, type);
    if ( mWriteMail->composer().isEmpty() ) { 
        // failed to create new composer, maybe due to no email account 
        // being present.
        return;
    }
    showComposer();
}

void EmailClient::modify(const QMailMessage& message)
{
    // Is this type editable?
    QString key(QMailComposerFactory::defaultKey(message.messageType()));
    if (!key.isEmpty()) {
        writeMailWidget()->modify(message);
        if ( mWriteMail->composer().isEmpty() ) { 
            // failed to create new composer, maybe due to no email account 
            // being present.
            return;
        }
        showComposer();
    } else {
        QMessageBox::warning(0,
                             tr("Error"),
                             tr("Cannot edit a message of this type."),
                             tr("OK"));
    }
}


void EmailClient::compose()
{
    delayedInit();

    writeMailWidget()->newMail();
    showComposer();
}

void EmailClient::setDocument(const QString &_address)
{
    // strip leading 'mailto:'
    QString address = _address;
    if (address.startsWith("mailto:"))
        address = address.mid(7);

#ifndef QTOPIA_NO_SMS
    QMailAddress recipient(address);
    if (recipient.isPhoneNumber()) {
        writeMailWidget()->newMail( QMailComposerFactory::defaultKey( QMailMessage::Sms ) );
        writeMailWidget()->setSmsRecipient(address);
    } else
#endif
    {
        writeMailWidget()->newMail( QMailComposerFactory::defaultKey( QMailMessage::Email ) );
        writeMailWidget()->setRecipient(address);
    }

    showComposer();
}

void EmailClient::deleteMailRequested(EmailListItem *item)
{
    if (!item || !item->id().isValid())
        return;

    Folder *folder = folderView()->currentFolder();
    if ( folder->folderType() == FolderTypeSystem &&
         folder->mailbox() == MailboxList::OutboxString &&
         sending) {
        return; //don't delete when sending
    }

    QMailMessage message(item->id(),QMailMessage::Header);
    QString type = mailType(message);

    // Is the SIM card ready/detected?
    if ((message.messageType() == QMailMessage::Sms) && !emailHandler->smsReadyToDelete()) {
        if (confirmDeleteWithoutSIM(1) == false)
            return;
    }

    bool toTrash(true);
    if (folder->folderType() == FolderTypeSystem
        && folder->mailbox() == MailboxList::TrashString ) {
        if (!Qtopia::confirmDelete( this, appTitle, type ))
            return;

        toTrash = false;
    }

    deleteMail( item );

    // Tell the user we're doing what they asked for
    if (toTrash)
        AcknowledgmentBox::show(tr("Moving"), tr("Moving to Trash: %1", "%1=Email/Message/MMS").arg(type));
    else
        AcknowledgmentBox::show(tr("Deleting"), tr("Deleting: %1","%1=Email/Message/MMS").arg(type));

    restoreView();
}

void EmailClient::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);
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

    if (!isTransmitting() && closeAfterTransmissions)
        QTMailWindow::singleton()->close();
    if (!isTransmitting())
        unregisterTask("transfer");
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
    if (!isTransmitting() && closeAfterTransmissions)
        QTMailWindow::singleton()->close();
    if (!isTransmitting())
        unregisterTask("transfer");
}

void EmailClient::suspendOk(bool y)
{
    QtopiaApplication::setPowerConstraint(y ? QtopiaApplication::Enable : QtopiaApplication::DisableSuspend);
}

void EmailClient::moveOutboxMailsToDrafts()
{
//  Move any messages stuck in the outbox to the drafts folder here
    EmailFolderList *outbox = mailboxList()->mailbox(MailboxList::OutboxString);
    EmailFolderList *drafts = mailboxList()->mailbox(MailboxList::DraftsString);
    QMailIdList outboxIds = outbox->messages();
    foreach(QMailId id,outboxIds)
    {
        if ( !moveMailToFolder(id,outbox, drafts ) )
            break;          //no point continuing to move
    }
}

void EmailClient::currentActionViewChanged(const QString &mailbox)
{
    if (mailbox.isEmpty())
        progressBar->setText( "" );
    else
        updateFolderCount( mailbox );
}

void EmailClient::updateFolderCount(const QString &mailbox, bool folderListOnly)
{
    if(suspendMailCount)
        return;

    EmailFolderList* mb = mailboxList()->mailbox(mailbox);
    if (!mb)
        return;

    if (!accountList)
        return;

    bool nHighlight = false;
    bool eHighlight = false;
    IconType nIconType = AllMessages;
    IconType eIconType = AllMessages;
    int nType = QMailMessage::Mms | QMailMessage::Sms | QMailMessage::System;
    int eType = QMailMessage::Email;
    uint nAllCount = mb->mailCount(EmailFolderList::All, nType);
    uint eAllCount = mb->mailCount(EmailFolderList::All, eType);
    uint nAllNewCount = 0;
    uint eAllNewCount = 0;

    if ( mailbox == MailboxList::InboxString ) {
        nAllNewCount = mb->mailCount(EmailFolderList::New, nType);
        eAllNewCount = mb->mailCount(EmailFolderList::New, eType, accountList);
        if ( nAllNewCount ) {
            nHighlight = true;
            nIconType = UnreadMessages;
        }
        if ( eAllNewCount ) {
            eHighlight = true;
            eIconType = UnreadMessages;
        }
        
        mb->mailCount(EmailFolderList::All, eType, accountList);
        QListIterator<MailAccount*> it = accountList->accountIterator();
        while ( it.hasNext() ) {
            MailAccount *account = it.next();

            // Update the account status
            QString countStr;
            int accountCount = account->count();
            int unreadCount = account->unreadCount();
            if ( accountCount ) {
                countStr = QLatin1String(" ");
                if (unreadCount)
                    countStr += QString("%1/%2").arg( unreadCount ).arg( accountCount );
                else
                    countStr += QString("%1").arg( accountCount );
            }
            folderView()->updateAccountStatus( account, countStr, unreadCount, NoIcon );
        }
    } else {
        nAllNewCount = mb->mailCount(EmailFolderList::New, nType);
        if ( nAllNewCount ) {
            nHighlight = true;
            nIconType = UnreadMessages;
        }
        eAllNewCount = mb->mailCount(EmailFolderList::New, eType);
        if ( eAllNewCount ) {
            eHighlight = true;
            eIconType = UnreadMessages;
        }
    }

    QString nS, eS;
    if ( nAllCount ) {
        if (nAllNewCount)
            nS = QString(" %1/%2").arg( nAllNewCount ).arg( nAllCount);
        else
            nS = QString(" %1").arg( nAllCount);
    }
    if ( eAllCount ) {
        if (eAllNewCount)
            eS = QString(" %1/%2").arg( eAllNewCount ).arg( eAllCount);
        else
            eS = QString(" %1").arg( eAllCount);
    }

    // Update the status of the folder
    folderView()->updateFolderStatus( mailbox, eS, eHighlight, NoIcon );
    mActionView->updateFolderStatus( mailbox, nS, NoIcon );

    // If this is the trash folder, then it can be emptied if it has at least 1 mail
    if (emptyTrashAction && mailbox == MailboxList::TrashString) {
        setActionVisible( emptyTrashAction, enableMessageActions );
    }

    // statusbar updates
    if ( receiving || sending || folderListOnly )
        return;
    
    // Update status bar
    if ( currentMailboxWidgetId() == messageId ) {
        messageChanged();
        return;
    }

    uint allCount = nAllCount;
    uint allNewCount = nAllNewCount;
    int type = nType;
	
    if (currentMailboxWidgetId() == folderId) {
        allCount = eAllCount;
        allNewCount = eAllNewCount;
        type = eType;
    }
	
    QString str = QString::number( allCount ); // No tr
    if (( mailbox == MailboxList::InboxString ) ||
        ( mailbox == MailboxList::TrashString )) {
        str += " "; //no tr
        str += tr( "(%1 new)", "%1 = number of new messages" ).arg( allNewCount );
    } else if ( mailbox != MailboxList::SentString ) {
        // Note, the 'unfinished' count is currently always zero...
        int unsent = mb->mailCount(EmailFolderList::Unsent, type);
        int unfinished = mb->mailCount(EmailFolderList::Unfinished, type);

        if ( unsent || unfinished ) {
            str += " (";
            if ( unsent ) {
                str += tr("%1 unsent", "%1 = number of unsent mails" ).arg(unsent);
            } else {    //unfinished only
                str += tr("%1 unfinished", "%1 = number of unfinished mails" ).arg(unfinished);
            }
            str += ")";
        }
    } else {                    //trash folder
    }
    progressBar->setText( str );
}

void EmailClient::settings()
{
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
    QTimer::singleShot(0, this, SLOT(updateAccounts()) );
}

void EmailClient::changedAccount(MailAccount *account)
{
    QTimer::singleShot(0, this, SLOT(updateAccounts()));
    folderView()->updateAccountFolder(account);
    accountList->saveAccounts();
}

void EmailClient::deleteAccount(MailAccount *account)
{
    folderView()->deleteAccountFolder(account);
    accountList->remove(account);
    QTimer::singleShot(0, this, SLOT(updateAccounts()));
    accountList->saveAccounts();
}

FolderListView* EmailClient::folderView()
{
    if ( !mFolderView ) {
        mFolderView = new FolderListView(mailboxList(), mailboxView, "folderView");
        folderId = mailboxView->addWidget( mFolderView );

        connect(mFolderView, SIGNAL(viewMessageList()), this, SLOT(showMessageList()) );
        connect(mFolderView, SIGNAL(folderSelected(Folder*)), this, SLOT(folderSelected(Folder*)) );
        connect(mFolderView, SIGNAL(emptyFolder()), this, SLOT(emptyTrashFolder()) );
        connect(mFolderView, SIGNAL(finished()), this, SLOT(restoreView()) );
        
        /*  Folder and Message View specific init not related to placement  */
        QStringList columns;
        columns << tr( "Folders" );
        mFolderView->setColumnCount( columns.count() );
        mFolderView->setHeaderLabels( columns );
        mFolderView->setRootIsDecorated( false );

        QHeaderView *header = mFolderView->header();
        header->setMovable( false );
        header->setClickable( false );

#ifndef QTOPIA_PHONE
        QAction *fvWhatsThis = QWhatsThis::createAction( mFolderView );
        fvWhatsThis->setText( tr("A list of your folders.  You can tap Outbox "
                     "and then tap the Messages tab to see the "
                     "messages currently in the outbox.") );
#endif

        mFolderView->header()->resizeSection( 0, QApplication::desktop()->availableGeometry().width() );
        
        QSettings mailconf("Trolltech","qtmail");
        QFont font;
        mailconf.beginGroup("settings");
        if ( mailconf.value("font").toString() == "large") {
            font.setPointSize( font.pointSize() + 4 );      // 4 larger than default
        }
        mailconf.endGroup();
        mFolderView->setFont( font );

        QTimer::singleShot(0, this, SLOT(openFiles()) );
    }
    return mFolderView;
}

MailListView* EmailClient::messageView()
{
    if ( !mMessageView ) {
        mMessageView = new MailListView(mailboxView, "messageView" );
        connect(mMessageView, SIGNAL(itemClicked(QTableWidgetItem*)),
                this, SLOT(queryItemSelected()) );
        // Not sure how this is supposed to work - disable until UI is standardised:
        /*
        connect(mMessageView, SIGNAL(itemPressed(EmailListItem*)),
                this, SLOT(showItemMenu(EmailListItem*)) );
        */
        connect(mMessageView, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
                this, SLOT(messageChanged()) );
        connect(mMessageView, SIGNAL(enableMessageActions(bool)),
                this, SLOT(setEnableMessageActions(bool)) );
        connect(mMessageView, SIGNAL(itemSelectionChanged()),
                this, SLOT(messageSelectionChanged()) );
        connect(mMessageView, SIGNAL(backPressed()),
                this, SLOT(restoreView()) );

        messageId = mailboxView->addWidget(mMessageView);

        QSettings mailconf("Trolltech","qtmail");
        mailconf.beginGroup("qtmailglobal");
        messageView()->readConfig( &mailconf );
        mailconf.beginGroup("settings");

        QFont font;
        if ( mailconf.value("font").toString() == "large") {
            font.setPointSize( font.pointSize() + 4 );  // 4 larger than default
        }
        mailconf.endGroup();
        mMessageView->setFont( font );

        Folder* currentFolder = folderView()->currentFolder();
        if (currentFolder)
            folderSelected( folderView()->currentFolder() );

        displayPreviousMail();
    }
    return mMessageView;
}

MailboxList* EmailClient::mailboxList()
{
    if ( !mMailboxList ) {
        mMailboxList = new MailboxList(this);

        connect(mMailboxList, SIGNAL(stringStatus(QString&)), this,
            SLOT(setStatusText(QString&)) );
        
        //connect after mail has been read to speed up reading */
        connect(mMailboxList, SIGNAL(mailAdded(QMailId,QString)), 
            this,SLOT(mailFromDisk(QMailId,QString)) );
        connect(mMailboxList, SIGNAL(mailUpdated(QMailId,QString)), 
            this,SLOT(mailUpdated(QMailId,QString)) );
        connect(mMailboxList, SIGNAL(mailRemoved(QMailId,QString)), 
            this,SLOT(mailRemoved(QMailId,QString)) );
        connect(mMailboxList,SIGNAL(mailMoved(QMailId,QString,QString)),
            this,SLOT(mailMoved(QMailId,QString,QString)));
        connect(mMailboxList,SIGNAL(mailMoved(QMailIdList,QString,QString)),
            this,SLOT(mailMoved(QMailIdList,QString,QString)));
        connect(mMailboxList, SIGNAL(externalEdit(QString)), 
            this,SLOT(externalEdit(QString)) );
    }
    return mMailboxList;
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

        QMailMessage mail;
        mail.setMessageType( QMailMessage::System );
        mail.setStatus( QMailMessage::Downloaded, true );
        mail.setStatus( QMailMessage::Incoming, true );
        mail.setDate( QMailTimeStamp( time ) );
        mail.setSubject( subject );
        QMailMessageContentType type( "text/plain; charset=UTF-8" );
        mail.setBody( QMailMessageBody::fromData( text, type, QMailMessageBody::Base64 ) );
        mail.setFromAccount( "@System" );
        mail.setHeaderField("From", "System");
        mailArrived( mail );

        QtopiaIpcEnvelope e("QPE/SysMessages", "ackMessage(int)");
        e << messageId;
    }
}

void EmailClient::appendErrorText(QString& message, const int code, const ErrorMap& map)
{
    const ErrorEntry *it = map.first, *end = map.first + map.second; // ptr arithmetic!

    for ( ; it != end; ++it)
        if (it->code == code)
        {
            message += tr( it->text );
            return;
        }
}

void EmailClient::planeModeChanged() 
{
    if (planeMode.value().toBool() == false) {
        // We have left airplane mode
        qLog(Messaging) << "Leaving Airplane Safe Mode";

        EmailFolderList* outbox = mailboxList()->mailbox(MailboxList::OutboxString);
        if (outbox->mailCount(EmailFolderList::All)) {
            // Send any queued messages
            sendAllQueuedMail();
        }
    }
}

void EmailClient::messageSelectionChanged()
{
    if (!moveAction)
        return; // initActions hasn't been called yet

    if (suspendMailCount)
        return;

    uint count = messageView()->rowCount();
    uint selected = messageView()->selectedItems().count();
    if (selected > 0) 
    {
        Folder* folder = folderView()->currentFolder();
        if (folder && (folder->folderType() == FolderTypeSystem) && (folder->mailbox() == MailboxList::TrashString)) {
            if (selected == 1 )
                deleteMailAction->setText(tr("Delete message"));
            else 
                deleteMailAction->setText(tr("Delete messages"));
        } else {
            deleteMailAction->setText(tr("Move to Trash"));
        }
        if ( selected == 1 ) {
            moveAction->setText(tr("Move message...", ""));
            copyAction->setText(tr("Copy message...", ""));
        } else {
            moveAction->setText(tr("Move messages...", ">=2 messages"));
            copyAction->setText(tr("Copy messages...", ">=2 messages"));
        }
    } 

    // Ensure that the per-message actions are hidden, if not usable
    setActionVisible(selectAllAction, (count > 0 && count != selected));
    setActionVisible(deleteMailAction, (selected != 0));
    setActionVisible(moveAction, (selected != 0));
    setActionVisible(copyAction, (selected != 0));
}

void EmailClient::showWidget(QWidget* widget, const QString& title)
{
    emit raiseWidget(widget, title);
}

void EmailClient::newMessages(bool userRequest)
{
    if (!QtopiaApplication::instance()->willKeepRunning()) {
        initialAction = IncomingMessages;
    }

    delayedInit();
    openFiles();
    showServerId = QString::null;

    delayedShowMessage(MailAccount::SMS, QMailId(), userRequest);
}

static bool lessThanByTimestamp(const QMailId& lhs, const QMailId& rhs)
{
    QMailMessage lhsMessage(lhs, QMailMessage::Header);
    QMailMessage rhsMessage(rhs, QMailMessage::Header);

    return (lhsMessage.date() < rhsMessage.date());
}

void EmailClient::clientsSynchronised()
{
    // We now have an updated incoming message count
    emit messageCountUpdated();

    // Are these new SMS messages, or were they put on the SIM by a previous phone?
    int newSmsCount = emailHandler->unreadSmsCount();
    if (unreadSmsIds.count() > newSmsCount) {
        // We need to remove the n most recent
        qSort(unreadSmsIds.begin(), unreadSmsIds.end(), lessThanByTimestamp);
        while (newSmsCount-- > 0)
            unreadSmsIds.removeLast();

        foreach (const QMailId& id, unreadSmsIds) {
            // This is not really a new message, mark it as read
            QMailMessage oldMail(id, QMailMessage::Header);
            oldMail.setStatus(QMailMessage::Read, true);
            QMailStore::instance()->updateMessage(&oldMail);
        }
    }
    unreadSmsIds.clear();

    // Are we responding to a raise request from QPE?
    bool respondingToRaise(initialAction == IncomingMessages);

    int newMessageCount = emailHandler->newMessageCount();
    if (newMessageCount != 0 || respondingToRaise) {
        if (newMessagesRequested || newMessageCount == 0) {
            newMessagesRequested = false;

            // Just go to the next stage
            viewNewMessages(respondingToRaise);

            unregisterTask("display");
        } else {
            // Start the message ring
            QtopiaServiceRequest req("Ringtone", "startMessageRingtone()");
            req.send();

            QString text(newMessageCount == 1 ? tr("A new message has arrived. Do you wish to read it now?")
                                              : tr("%1 new messages have arrived. Do you wish to view them now?").arg(newMessageCount) );

            if (newMessagesBox) {
                // Update the text and restart the timer
                newMessagesBox->setText(text);
            } else {
                // Ask the user whether to view the message(s)
                newMessagesBox = new QMessageBox(QMessageBox::Information, tr("New message"), text, QMessageBox::Yes | QMessageBox::No);
                connect(newMessagesBox, SIGNAL(finished(int)), this, SLOT(newMessageAction(int)));
                QtopiaApplication::showDialog(newMessagesBox);

                connect(&newMessageResponseTimer, SIGNAL(timeout()), this, SLOT(abortViewNewMessages()));
            }

            if (NotificationVisualTimeout)
                newMessageResponseTimer.start(NotificationVisualTimeout);
        }
    }
}

void EmailClient::viewNewMessages(bool respondingToRaise)
{
    bool savedAsDraft(false);
    int newMessageCount = emailHandler->newMessageCount();

    // Having chosen to view new messages, we should reset the new message count
    resetNewMessages();

    if (!respondingToRaise) {
        // We were already operating when this new message notification arrived; if we 
        // are composing, we need to save as draft
        if (mWriteMail && 
            (QTMailWindow::singleton()->currentWidget() == mWriteMail)) {
            savedAsDraft = mWriteMail->forcedClosure();
        }
    }

    if (newMessageCount == 1) {
        // Find the newest incoming message and display it
        EmailFolderList* inbox(mailboxList()->mailbox(MailboxList::InboxString));

        unsigned int messageType = QMailMessage::Sms | QMailMessage::Mms | QMailMessage::System;

        QMailIdList unreadList = inbox->messages(QMailMessage::Read,
                                                 false,
                                                 messageType, 
                                                 EmailFolderList::DescendingDate);
        if(!unreadList.isEmpty())
        {
            // We need to change to the correct message view, to control the context menu!
            folderView()->changeToSystemFolder(MailboxList::InboxString);
            showViewer(unreadList.first(), folderView()->currentFolder(), false);
        }
    }
    else {
        viewInbox();
    }

    if (savedAsDraft) {
        // The composer had a partial message, now saved as a draft
        AcknowledgmentBox::show(tr("Saved to Drafts"), tr("Incomplete message has been saved to the Drafts folder"));
    }
}

void EmailClient::viewInbox()
{
    // Show the inbox with the new messages
    messageView()->setShowEmailsOnly(false);
    folderView()->changeToSystemFolder(MailboxList::InboxString);
    messageView()->setSelectedRow(0);

    showMessageList();
}

void EmailClient::newMessageAction(int choice)
{
    newMessageResponseTimer.stop();

    // Stop the message ring, if necessary
    QtopiaServiceRequest req("Ringtone", "stopMessageRingtone()");
    req.send();

    // Are we responding to a raise request from QPE?
    bool respondingToRaise(initialAction == IncomingMessages);
    if (respondingToRaise) {
        // If new messages arrive during this session, do not trigger the ringtone again
        initialAction = None;
    }

    if (choice == QMessageBox::Yes)
        viewNewMessages(respondingToRaise);

    unregisterTask("display");

    newMessagesBox->deleteLater();
    newMessagesBox = 0;
}

void EmailClient::abortViewNewMessages()
{
    newMessagesBox->setResult(QMessageBox::No);
    newMessagesBox->reject();
}

#ifndef QTOPIA_NO_MMS

void EmailClient::mmsMessage(const QDSActionRequest& request)
{
#ifndef QTOPIA_NO_SMS
    if (!QtopiaApplication::instance()->willKeepRunning())
        initialAction = IncomingMessages;

    delayedInit();

    emailHandler->pushMmsMessage(request);
#endif

    // Respond to the request
    QDSActionRequest( request ).respond();
}

#endif

void EmailClient::noSendAccount(QMailMessage::MessageType type)
{
    QString key(QMailComposerFactory::defaultKey(type));
    QString name(QMailComposerFactory::name(key));

    QMessageBox::warning(0,
                         tr("Send Error"), 
                         tr("%1 cannot be sent, because no account has been configured to send with.","%1=MMS/Email/TextMessage").arg(name),
                         QMessageBox::Ok);
}

void EmailClient::nonexistentMessage(const QMailId& id)
{
    // Mark this message as deleted
    QMailMessage deletedMail(id, QMailMessage::Header);
    deletedMail.setStatus(QMailMessage::Removed, true);
    QMailStore::instance()->updateMessage(&deletedMail);

    if (readMailWidget()->isVisible())
        mReadMail->mailUpdated(id);

    QMessageBox::warning(0,
                         tr("Message deleted"), 
                         tr("Message cannot be downloaded, because it has been deleted from the server."),
                         QMessageBox::Ok);
}

void EmailClient::setActionVisible(QAction* action, bool visible)
{
    actionVisibility[action] = visible;
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
    qLog(Messaging) << "EmailService::writeMail()";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
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
    qLog(Messaging) << "EmailService::writeMail(" << name << "," << email << ")";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
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
    qLog(Messaging) << "EmailService::writeMessage(" << name << "," << email << ", ... )";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
    parent->writeMessageAction( name, email, docAttachments, fileAttachments );
}

/*!
    Direct the \i Email service to display the user's message boxes.

    This slot corresponds to the QCop service message
    \c{Email::viewMail()}.
*/
void EmailService::viewMail()
{
    qLog(Messaging) << "EmailService::viewMail()";
    parent->initialAction = EmailClient::View;
    parent->delayedInit();
    QtopiaApplication::instance()->showMainWidget();
}

/*!
    Direct the \i Email service to display the message identified by
    \a id.
    \deprecated

    This slot corresponds to the QCop service message
    \c{Email::viewMail(QMailId)}.
*/
void EmailService::viewMail( const QMailId& id )
{
    qLog(Messaging) << "EmailService::viewMail(" << id.toULongLong() << ")";
    parent->displayMessage(id);
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
    qLog(Messaging) << "EmailService::emailVCard(" << filename << ", )";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
    parent->writeMessageAction( QString(), QString(), QStringList(),
                                QStringList( filename ),
                                QMailMessage::Email);
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
    qLog(Messaging) << "EmailService::emailVCard( QDSActionRequest )";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
    parent->emailVCard( request );
}

/*!
    \internal
*/
void EmailService::emailVCard( const QString&, const QMailId&, const QString& filename, const QString& description )
{
    qLog(Messaging) << "EmailService::emailVCard( , ," << filename << "," << description << ")";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
    // To be removed when the SetValue service is fixed.
    emailVCard( filename, description );
}

/*!
    Direct the \i Email service to purge all messages which
    are older than the given \a date and exceed the minimal mail \a size.
    This is typically called by the cleanup wizard.
    \deprecated

    This slot corresponds to the QCop service message
    \c{Email::cleanupMessages(QDate,int)}.
*/
void EmailService::cleanupMessages( const QDate& date, int size )
{
    qLog(Messaging) << "EmailService::cleanupMessages(" << date << "," << size << ")";
    parent->initialAction = EmailClient::Cleanup;
    parent->delayedInit();
    parent->cleanupMessages( date, size );
}

#ifndef QTOPIA_NO_SMS

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
    qLog(Messaging) << "SMSService::writeSms()";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
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
    qLog(Messaging) << "SMSService::writeSms(" << name << "," << number << ")";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
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
    qLog(Messaging) << "SMSService::writeSms(" << name << "," << number << "," << filename << ")";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
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
    \fn SMSService::newMessages(bool)
    \internal 
*/

/*!
    Show the most recently received SMS message.
    \deprecated

    Clients of this service should instead use \c{Messages::viewNewMessages()}.

    This slot corresponds to the QCop service message
    \c{SMS::viewSms()}.
*/
void SMSService::viewSms()
{
    qLog(Messaging) << "SMSService::viewSms()";

    // Although this requests SMS specifically, we currently have only the 
    // facility to show the newest incoming message, or the combined inbox
    emit newMessages(false);
}

/*!
    \fn SMSService::viewInbox()
    \internal 
*/

/*!
    Show the list of all received SMS messages.
    \deprecated

    This slot corresponds to the QCop service message
    \c{SMS::viewSmsList()}.
*/
void SMSService::viewSmsList()
{
    qLog(Messaging) << "SMSService::viewSmsList";

    // Although this requests SMS specifically, we currently have only the 
    // facility to show the combined inbox
    emit viewInbox();
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message for sending the vcard data in \a filename.  The
    \a description argument provides an optional descriptive text message.

    This slot corresponds to the QCop service message
    \c{SMS::smsVCard(QString,QString)}.
*/
void SMSService::smsVCard( const QString& filename, const QString& description)
{
    qLog(Messaging) << "SMSService::smsVCard(" << filename << ", )";
    parent->initialAction = EmailClient::Compose;
    parent->delayedInit();
    QFile f( filename );
    if (! f.open(QIODevice::ReadOnly) ) {
        qWarning("could not open file: %s", filename.toLatin1().constData() );
    } else {
        QString body =  QString::fromLocal8Bit( f.readAll() );
        parent->writeSmsAction( QString(), QString(), body, true);
    }

    Q_UNUSED(description)
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
    qLog(Messaging) << "SMSService::smsVCard( QDSActionRequest )";
    parent->initialAction = EmailClient::View;
    parent->delayedInit();
    parent->smsVCard( request );
}

/*!
    Direct the \i SMS service to interact with the user to compose a new
    SMS message for sending the vcard data in \a filename.  The
    \a description argument provides an optional descriptive text message.
    The \a channel and \a id are not used.

    \deprecated

    This slot corresponds to the QCop service message
    \c{SMS::smsVCard(QString,QString,QString,QString)}.
*/
void SMSService::smsVCard( const QString& channel, const QString& id, const QString& filename, const QString& description )
{
    qLog(Messaging) << "SMSService::smsVCard( , ," << filename << "," << description << ")";
    parent->initialAction = EmailClient::View;
    parent->delayedInit();
    // To be removed when the SetValue service is fixed.
    smsVCard( filename, description );

    Q_UNUSED(channel)
    Q_UNUSED(id)
}

/*!
    \fn SMSService::mmsMessage(const QDSActionRequest&)
    \internal 
*/

/*!
    Direct the \i SMS services to handle the MMS push notification \a request.

    This slot corresponds to a QDS service with request data containing
    the serialization of an \c MMSMessage, and no response data.

    This slot corresponds to the QCop service message
    \c{SMS::pushMmsMessage(QDSActionRequest)}.
*/
void SMSService::pushMmsMessage( const QDSActionRequest& request )
{
    qLog(Messaging) << "SMSService::pushMmsMessage( QDSActionRequest )";

#ifndef QTOPIA_NO_MMS
    emit mmsMessage(request);
#endif
}

/*!
    Direct the \i SMS service to process the flash SMS message
    within \a request.

    This slot corresponds to a QDS service with request data containing
    the serialization of a QSMSMessage, and no response data.

    This slot corresponds to the QCop service message
    \c{SMS::flashSms(QDSActionRequest)}.
*/
void SMSService::flashSms( const QDSActionRequest& request )
{
    qLog(Messaging) << "SMSService::flashSms( QDSActionRequest )";
    parent->initialAction = EmailClient::View;
    parent->delayedInit();
    parent->flashSms( request );
}

#endif // QTOPIA_NO_SMS

/*!
    \service MessagesService Messages
    \brief Provides the Qtopia Messages viewing service.

    The \i Messages service enables applications to request the display of messages of various types.
*/

/*! \internal */
MessagesService::MessagesService(EmailClient* parent)
    : QtopiaAbstractService( "Messages", parent )
{
    publishAll();
}

/*! \internal */
MessagesService::~MessagesService()
{
}

/*!
    \fn MessagesService::newMessages(bool)
    \internal 
*/

/*!
    Show the newly arrived messages.  If \a userRequest is true, the request will be treated
    as if arising from a direct user action; otherwise, the user will be requested to confirm 
    the action before proceeding.

    This slot corresponds to the QCop service message
    \c{Messages::viewNewMessages()}.
*/
void MessagesService::viewNewMessages(bool userRequest)
{
    qLog(Messaging) << "MessagesService::viewNewMessages(" << userRequest << ")";

    emit newMessages(userRequest);
}

/*!
    \fn MessagesService::message(QMailId)
    \internal 
*/

/*!
    Show the message with the supplied \a id.

    This slot corresponds to the QCop service message
    \c{Messages::viewMessage(QMailId)}.
*/
void MessagesService::viewMessage(QMailId id)
{
    qLog(Messaging) << "MessagesService::viewMessage(" << id << ")";
    if (!id.isValid()) {
        qWarning() << "viewMessage supplied invalid id:" << id.toULongLong();
        return;
    }

    emit message(id);
}

#include "emailclient.moc"

