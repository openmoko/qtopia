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



#ifndef EMAILCLIENT_H
#define EMAILCLIENT_H

#include "emailhandler.h"
#include "emaillistitem.h"
#include "editaccount.h"
#include "maillist.h"
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

#include <qtopiaabstractservice.h>
#include <qtopiachannel.h>

class StatusProgressBar;
class Folder;
class AccountList;
class QDSActionRequest;
class QtopiaChannel;

class EmailClient : public QMainWindow
{
    Q_OBJECT
    friend class EmailService;
    friend class SMSService;

public:
    EmailClient(QWidget* parent, const QString name, Qt::WFlags fl = 0 );
    ~EmailClient();

    WriteMail* writeMailWidget();
    ReadMail* readMailWidget();

    bool cleanExit(bool force);
    void closeAfterTransmissionsFinished();
    void update();
    bool isTransmitting();
    void delayedShowMessage(MailAccount::AccountType acct, QUuid id, bool showList);

signals:
    void raiseWidget(QWidget *, const QString &);

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

    void autosaveMail(const Email &);
    void enqueueMail(const Email &);
    void discardMail();
    void saveAsDraft(const Email &);

    void sendAllQueuedMail();
    void sendSingleMail(Email *);

    void mailSent(int count);
    void unresolvedUidlArrived(QString &, QStringList &);
    void getSingleMail(Email *);
    void mailArrived(const Email &);
    void allMailArrived(int);
    void queryItemSelected();
    bool moveMailToFolder(Email *mail, EmailFolderList *source, EmailFolderList *target);
    void emptyFolder();

    void showItemMenu(EmailListItem *item);
    void showDeleteNotReadyWarning();
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
    void moveMailFront(Email *mailPtr);

    void updateSendingStatusLabel(const QString &);
    void updatePopStatusLabel(const QString &);
    void folderSelected(Folder *);
    void imapServerFolders();
    void failedList(QStringList &);

    void mailRemoved(const QUuid &, const QString &);
    void mailUpdated(Email *, const QString &);
    void mailFromDisk(Email *, const QString &);
    void mailMoved(Email*, const QString&, const QString&);

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
    void search();

    void automaticFetch();

    void externalEdit(const QString &);

    void compose();
    void resend(const Email &, int);
    void modify(Email *);
    void deleteMailRequested(EmailListItem *);
    void showEmailView();
    void showFolderList();
    void showMessageList();
    void displayRecentMessage();
    void readReplyRequested(Email*);

    void settings();
    void changedAccount(MailAccount *);
    void deleteAccount(MailAccount *);

    void selectAccountTimeout();

    MailListView *messageView();

private slots:
    void smsVCard( const QDSActionRequest& request );
    void emailVCard( const QDSActionRequest& request );
    void collectSysMessages();
    void setEnableMessageActions( bool );

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
            int type = MailMessage::SMS | MailMessage::MMS | MailMessage::Email );
    void cleanupMessages( const QDate &removalDate, int removalSize );
    void displayPreviousMail();
    void displayMail(const QString &, const QUuid &);

    void readMail();
    void readSettings();
    bool saveSettings();

    void getNextNewMail();
    void accessError(EmailFolderList *box);
    void moveError(const EmailFolderList& source, const EmailFolderList& destination);
    bool verifyAccounts(bool outgoing);

    void addMailToDownloadList(Email *mail);
    MailAccount* smtpForMail(Email *mail);

    void updateQuery(Email *mail, const QString &mailbox);

    void isSending(bool y);
    void isReceiving(bool y);
    void suspendOk(bool y);
    void moveOutboxMailsToDrafts();

    void updateFolderCount(const QString &mailbox, bool folderListOnly = false);
    int currentMailboxWidgetId();
    void setCurrentMailboxWidget(int);
    int currentMessageId() { return messageId; }
    int currentFolderId() { return folderId; }

private:
    int invalidId, folderId, messageId;

    QMainWindow *mainView;
    QFrame *vbox;
    QVBoxLayout *vboxLayout;

    QString appTitle;
    bool filesRead;
    QString cachedDisplayMailbox;
    QUuid cachedDisplayMailUuid;

    int nosuspend;

    int newAccountId, mailIdCount;
    int accountIdCount;
    int queueStatus, downloadSize, totalSize;
    AccountList *accountList;

    EditAccount *editAccountView;
    EmailHandler *emailHandler;

    MailboxList *mailboxList;

    QList<QUuid> queuedUuids;
    MailList mailDownloadList;
    bool sending, receiving, previewingMail, allAccounts, sendSingle;
    bool quitSent;
    MailAccount *mailAccount, *smtpAccount;

    QToolBar *bar;
    StatusProgressBar *progressBar;
    QString statusLabelHeader;
    QAction *getMailButton;
    QAction *sendMailButton;
    QAction *composeButton;
    QAction *searchButton;
    QAction *cancelButton;
    QAction *settingsAction;
    QAction *emptyTrashAction;
    QAction *deleteMailAction;

    QMenuBar *mb;
    QMenu *configure;
    QMenu *selectAccountMenu;

    QMenu *movePop, *copyPop;
    QMap< QAction*, int > actionMap;
    QMap< QAction*, int > moveMap;

#ifdef QTOPIA_PHONE
    QStackedWidget *mailboxView;
    QAction *newAccountAction;
    QAction *editAccountAction;
    QAction *moveAction, *copyAction, *selectAllAction;
#else
    QTabWidget *mailboxView;
#endif
    bool closeAfterTransmissions;
    bool closeAfterWrite;
    bool closeAfterView;
    QWidget *widget_3;
    QGridLayout *gridQuery;
    MailListView *mMessageView;
    FolderListView *folderView;

    QTimer showSmsTimer;
    bool showSmsList;

    QTimer fetchTimer;
    QTimer showMessageTimer;
    bool showMsgList;
    int showMsgRetryCount;
    MailAccount::AccountType showMessageType;
    QUuid showMsgId;
    bool autoGetMail;

    QUuid repliedFromUuid;
    uint repliedFlags;

    QList<int> queuedAccountIds;
    QTimer checkAccountTimer;

    bool autoDownloadMail;
    bool suspendMailCount;

    QtopiaChannel *sysMessagesChannel;
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
    void viewMail( const QString& mailbox, const QString& id );
    void emailVCard( const QString& filename, const QString& description );
    void emailVCard( const QString& channel,
                     const QString& id,
                     const QString& filename,
                     const QString& description );
    void emailVCard( const QDSActionRequest& request );
    void cleanupMessages( const QDate& date, int size );

private:
    EmailClient *parent;
};

#ifdef QTOPIA_PHONE

class SMSService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class EmailClient;
private:
    SMSService( EmailClient *parent )
        : QtopiaAbstractService( "SMS", parent )
        { this->parent = parent; publishAll(); }

public:
    ~SMSService();

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
    void smsVCard( const QDSActionRequest& request );

    void viewSysSms();
private:
    EmailClient *parent;
};

#endif

#endif // EMAILCLIENT_H
