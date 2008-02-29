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



#ifndef EMAILCLIENT_H
#define EMAILCLIENT_H

#include "emailhandler.h"
#include "emaillistitem.h"
#include "editaccount.h"
#include "maillist.h"
#include "actionlistview.h"
#include "maillistview.h"
#include "folderlistview.h"
#include "emailfolderlist.h"
#include "readmail.h"
#include "writemail.h"
#include "account.h"

#include <quuid.h>

#include <qlist.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qaction.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qprogressbar.h>
#include <qdir.h>
#include <qtimer.h>
#include <qstackedwidget.h>
#include <qstring.h>
#include <qcontent.h>
#include <QValueSpaceItem>

#include <qtopiaabstractservice.h>
#include <qtopiachannel.h>

class StatusProgressBar;
class Folder;
class AccountList;
class QDSActionRequest;
class QtopiaChannel;
class QListWidget;
class FolderListView;

class EmailClient : public QMainWindow
{
    Q_OBJECT
    friend class EmailService;
    friend class SMSService;
    friend class MessagesService;

    // Whether the initial action for the app was to view incoming messages 
    enum InitialAction { None = 0, IncomingMessages, View, Compose, Cleanup };

public:
    EmailClient(QWidget* parent, const QString name, Qt::WFlags fl = 0 );
    ~EmailClient();

    WriteMail* writeMailWidget();
    ReadMail* readMailWidget();

    bool cleanExit(bool force);
    void closeAfterTransmissionsFinished();
    void update();
    bool isTransmitting();

    void delayedShowMessage(MailAccount::AccountType acct, QMailId id, bool userRequest);

    // Structure to store error strings as plain untranslated strings so they're ROM-able
    struct ErrorEntry { int code; const char* text; };
    typedef QPair<const ErrorEntry*, size_t> ErrorMap;

signals:
    void raiseWidget(QWidget *, const QString &);
    void messageCountUpdated();

public slots:
    void setDocument(const QString &);

    bool deleteMail(EmailListItem *mailItem);
    void mailResponded();

    void openFiles();

    void handleSysMessages(const QString &message,const QByteArray &data);

protected:
    void showEvent(QShowEvent* e);

protected slots:
    void cancel();

    void autosaveMail(const QMailMessage&);
    void enqueueMail(const QMailMessage&);
    void discardMail();
    void saveAsDraft(const QMailMessage&);
    void noSendAccount(QMailMessage::MessageType);

    void sendAllQueuedMail(bool userRequest = false);
    void sendSingleMail(const QMailMessage& message);

    void mailSent(int count);
    void transmissionCompleted();
    void unresolvedUidlArrived(QString &, QStringList &);
    void getSingleMail(const QMailMessage& message);
    void mailArrived(const QMailMessage&);
    void allMailArrived(int);
    void queryItemSelected();
    void resetNewMessages();
    bool moveMailToFolder(const QMailId& id, EmailFolderList *source, EmailFolderList *target);
    bool copyMailToFolder(const QMailId& id, EmailFolderList *source, EmailFolderList *target);
    void emptyTrashFolder();

    // Currently disabled:
    //void showItemMenu(EmailListItem *item);

    bool confirmDeleteWithoutSIM(int);
    void deleteMailItem();
    void moveMailItem(int);
    void copyMailItem(int);
    void moveMailItem(QAction*);
    void copyMailItem(QAction*);
    void moveMessage();
    void copyMessage();
    void selectAll();

    void setTotalPopSize(int);
    void setTotalSmtpSize(int);
    void setTransferredSize(int);
    void setDownloadedSize(int);
    void moveMailFront(const QMailMessage& message);

    void updateSendStatusLabel(const Client*, const QString&);
    void updateReceiveStatusLabel(const Client*, const QString&);

    void rebuildMoveCopyMenus(const Folder *folder);
    void folderSelected(Folder *);
    void imapServerFolders();
    void failedList(QStringList &);

    void displayNewMessage(const QMailMessage& message);
    void mailRemoved(const QMailId &, const QString &);
    void mailUpdated(const QMailId& messageId, const QString &);
    void mailFromDisk(const QMailId& messageId, const QString &);
    void mailMoved(const QMailId& messageId, const QString&, const QString&);
    void mailMoved(const QMailIdList& list, const QString&, const QString&);

    void getAllNewMail();
    void getNewMail();

    void selectAccount(int);
    void selectAccount(QAction*);
    void editAccount(int);
    void editAccount(QAction*);
    void deleteAccount(int);
    void updateGetMailButton(bool enable);
    void updateAccounts();

    void messageChanged();

    void smtpError(int code, QString &);
    void popError(int code, QString &);
    void smsError(int code, QString &);
    void mmsError(int code, QString &);

    void setStatusText(QString &);

    void cornerButtonClicked();
    void findMatchingMessages();
    void search();

    void automaticFetch();

    void externalEdit(const QString &);

    void compose();
    void resend(const QMailMessage& message, int);
    void modify(const QMailMessage& message);
    void deleteMailRequested(EmailListItem *);
    void showEmailView();

    void showFolderList(bool recordLocation = true);
    void showMessageList(bool recordLocation = true);
    void showActionList(bool recordLocation = true);
    void showComposer(bool recordLocation = true);
    void showViewer(const QMailId& messageId, Folder* folder, bool emailFolder, bool recordLocation = true);

    void restoreView();

    void displayRecentMessage();
    void updateListViews();
    void readReplyRequested(const QMailMessage&);

    void settings();
    void changedAccount(MailAccount *);
    void deleteAccount(MailAccount *);

    void selectAccountTimeout();
    FolderListView *folderView();
    MailListView *messageView();
    MailboxList *mailboxList();

    void planeModeChanged();
    void messageSelectionChanged();

    void clientsSynchronised();

private slots:
    void delayedInit();
    void initActions();
    void updateActions();
    void smsVCard( const QDSActionRequest& request );
    void emailVCard( const QDSActionRequest& request );
    void flashSms( const QDSActionRequest& request );
    void collectSysMessages();
    void setEnableMessageActions( bool );
    void incrementalFolderCount();
    void newMessages(bool);
    void abortViewNewMessages();
    void newMessageAction(int);
    void viewInbox();
#ifndef QTOPIA_NO_MMS
    void mmsMessage(const QDSActionRequest&);
#endif
    void nonexistentMessage(const QMailId&);

protected:
    friend class QTMailWindow;
    WriteMail *mWriteMail;
    ReadMail *mReadMail;

private:
    void init();
    void createEmailHandler();

    bool checkMailConflict(const QString& msg1, const QString& msg2);
    void writeMailAction(const QMap<QString, QString> map );
    void writeMailAction(const QString& name, const QString& email);
    void writeSmsAction(const QString& name, const QString& email,
                        const QString& body = QString(), bool vcard = false);
    void writeMessageAction( const QString &name, const QString &addrStr,
            const QStringList &docAttachments,
            const QStringList &fileAttachments,
            int type = QMailMessage::Sms | QMailMessage::Mms | QMailMessage::Email );
    void cleanupMessages( const QDate &removalDate, int removalSize );
    void displayPreviousMail();
    void viewNewMessages(bool terminateOnCompletion);

public slots:
    void displayFolder(const QString &);
    void currentActionViewChanged(const QString &mailbox);
    void displayMessage(const QMailId& id);

private:
    void readMail();
    void readSettings();
    bool saveSettings();

    void displayCachedMail();

    void getNextNewMail();
    void accessError(EmailFolderList *box);
    void moveError(const EmailFolderList& source, const EmailFolderList& destination);
    bool verifyAccounts(bool outgoing);

    void addMailToDownloadList(const QMailMessage& message);
    MailAccount* smtpForMail(QMailMessage& message);

    void updateQuery(const QMailMessage& message, const QString &mailbox);

    void isSending(bool y);
    void isReceiving(bool y);
    void suspendOk(bool y);
    void moveOutboxMailsToDrafts();

    void updateFolderCount(const QString &mailbox, bool folderListOnly = false);
    int currentMailboxWidgetId();
    void setCurrentMailboxWidget(int);
    int currentMessageId() { return messageId; }
    int currentFolderId() { return folderId; }

    void appendErrorText(QString& message, const int code, const ErrorMap& map);
    QString mailType(const QMailMessage& mail);

    void showMessageList(bool email, bool recordLocation);
    void showWidget(QWidget*, const QString& = QString());

    void setActionVisible(QAction*, bool);

    void deleteMail(const QMailStore::DeletionProperties& properties, bool deleting, EmailFolderList* srcFolder);

    bool deleteMailList(QList<EmailListItem*>& deleteList);
    bool moveMailListToFolder(QList<EmailListItem*>& moveList, EmailFolderList *source, EmailFolderList *target);
    bool copyMailListToFolder(QList<EmailListItem*>& copyList, EmailFolderList *source, EmailFolderList *target);

    bool moveMailListToFolder(const QMailIdList& list, EmailFolderList *source, EmailFolderList *target);

    bool foreachListElement(bool (EmailClient::*func)(const QMailId&, EmailFolderList*, EmailFolderList*), 
                            QList<EmailListItem*>& list, EmailFolderList *source, EmailFolderList *target, const QString& caption);

private:
    int folderId, messageId, actionId;

    QMainWindow *mainView;
    QFrame *vbox;
    QVBoxLayout *vboxLayout;

    QString appTitle;
    bool filesRead;
    QMailId cachedDisplayMailId;

    int nosuspend;

    int newAccountId, mailIdCount;
    int accountIdCount;
    int queueStatus, downloadSize, totalSize;
    AccountList *accountList;

    EditAccount *editAccountView;
    EmailHandler *emailHandler;

    MailboxList *mMailboxList;

    QList<QMailId> queuedMailIds;
    MailList mailDownloadList;
    bool sending, receiving, previewingMail, allAccounts, sendSingle;
    bool quitSent;
    MailAccount *mailAccount, *smtpAccount;

    QToolBar *bar;
    StatusProgressBar *progressBar;
    QAction *getMailButton;
    QAction *sendMailButton;
    QAction *composeButton;
    QAction *searchButton;
    QAction *cancelButton;
    QAction *settingsAction;
    QAction *emptyTrashAction;
    QAction *deleteMailAction;
    bool enableMessageActions;

    QMenuBar *mb;
    QMenu *configure;
    QMenu *selectAccountMenu;

    QMenu *movePop, *copyPop;
    QMap< QAction*, int > actionMap;
    QMap< QAction*, int > moveMap;

    QStackedWidget *mailboxView;
    QAction *newAccountAction;
    QAction *editAccountAction;
    QAction *moveAction, *copyAction, *selectAllAction;
    bool closeAfterTransmissions;
    bool closeAfterWrite;
    MailListView *mMessageView;
    FolderListView *mFolderView;
    ActionListView *mActionView;

    QTimer showSmsTimer;
    bool showSmsList;

    QTimer fetchTimer;
    QTimer showMessageTimer;
    bool showMsgList;
    int showMsgRetryCount;
    MailAccount::AccountType showMessageType;
    QMailId showMsgId;
    QString showServerId;
    bool waitingForNewMessage;
    bool newMessagesRequested;
    bool autoGetMail;

    QMailId repliedFromMailId;
    QMailMessage::Status repliedFlags;

    QList<int> queuedAccountIds;
    QTimer checkAccountTimer;

    bool autoDownloadMail;
    bool suspendMailCount;

    QtopiaChannel *sysMessagesChannel;
    QStringList countList;

    QValueSpaceItem planeMode;

    QMessageBox* newMessagesBox;
    QTimer newMessageResponseTimer;
    QtopiaIpcAdaptor messageCountUpdate;

    InitialAction initialAction;

    QList<QContent> temporaries;
    QList<QMailId> unreadSmsIds;

    QMap<QAction*, bool> actionVisibility;

    Search *lastSearch;
};

class EmailService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class EmailClient;
private:
    EmailService( EmailClient *parent )
        : QtopiaAbstractService( "Email", parent )
        { this->parent = parent; publishAll(); }

public:
    ~EmailService();

public slots:
    void writeMail();
    void writeMail( const QString& name, const QString& email );
    void writeMessage( const QString& name, const QString& email,
                       const QStringList& docAttachments,
                       const QStringList& fileAttachments );
    void viewMail();
    void viewMail( const QMailId& id );
    void emailVCard( const QString& filename, const QString& description );
    void emailVCard( const QString& channel,
                     const QMailId& id,
                     const QString& filename,
                     const QString& description );
    void cleanupMessages( const QDate& date, int size );

    // QDS services
    void emailVCard( const QDSActionRequest& request );

private:
    EmailClient *parent;
};

#ifndef QTOPIA_NO_SMS

class SMSService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class EmailClient;
private:
    SMSService( EmailClient *parent )
        : QtopiaAbstractService( "SMS", parent )
        { this->parent = parent; publishAll(); }

    ~SMSService();

signals:
    void newMessages(bool);
    void viewInbox();
#ifndef QTOPIA_NO_MMS
    void mmsMessage(const QDSActionRequest&);
#endif

public slots:
    void writeSms();
    void writeSms( const QString& name, const QString& number );
    void writeSms( const QString& name, const QString& number,
                   const QString& filename );
    void viewSms();
    void viewSmsList();
    void smsVCard( const QString& filename, const QString& description );
    void smsVCard( const QString& channel,
                   const QString& id,
                   const QString& filename,
                   const QString& description );

    // QDS services
    void smsVCard( const QDSActionRequest& request );
    void pushMmsMessage( const QDSActionRequest& request );
    void flashSms( const QDSActionRequest& request );

private:
    EmailClient *parent;
};

#endif

class MessagesService : public QtopiaAbstractService
{
    Q_OBJECT

private:
    friend class EmailClient;

    MessagesService(EmailClient *parent);
    ~MessagesService();

signals:
    void newMessages(bool);
    void message(QMailId);

public slots:
    void viewNewMessages(bool userRequest);
    void viewMessage(QMailId id);
};

#endif // EMAILCLIENT_H
