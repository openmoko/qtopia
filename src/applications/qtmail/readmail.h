/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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



#ifndef READMAIL_H
#define READMAIL_H

#include <qaction.h>
#include <qmainwindow.h>
#include <qmenudata.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qmime.h>

#include <quuid.h>
#include <qurl.h>
#include <qmap.h>

#include "emaillistitem.h"
#include "viewatt.h"
#include <QMailMessage>

class QLabel;
class MailListView;
class QMenu;
class QStackedWidget;
class QContactModel;
class QMailViewerInterface;
class AccountList;
class QContactModel;

class ReadMail : public QMainWindow
{
    Q_OBJECT

public:
    enum ResendAction { Reply = 1, ReplyToAll = 2, Forward = 3 };

    ReadMail( QWidget* parent = 0, const QString name = QString(), Qt::WFlags fl = 0 );
    ~ReadMail();

    void setAccountList(AccountList* list);

    void viewSelectedMail(MailListView *view);
    void mailUpdated(const QMailId& message);

private slots:
    void updateView();

signals:
    void resendRequested(const QMailMessage&, int);
    void mailto(const QString &);
    void modifyRequested(const QMailMessage&);
    void removeItem(EmailListItem *);
    void viewingMail(const QMailMessage&);
    void getMailRequested(const QMailMessage&);
    void sendMailRequested(const QMailMessage&);
    void readReplyRequested(const QMailMessage&);
    void cancelView();

public slots:
    void cleanup();
    void isSending(bool);
    void isReceiving(bool);

protected slots:
    void linkClicked(const QUrl &lnk);
    void closeView();

    void next();
    void previous();

    void deleteItem();
    void viewAttachments();

    void reply();
    void replyAll();
    void forward();
    void modify();

    void setStatus(int);
    void getThisMail();
    void sendThisMail();

    void storeContact();

protected:
    void keyPressEvent(QKeyEvent *);

private:
    void viewMms();

    void init();
    void showMail(const QMailId& message);
    void updateButtons();
    void buildMenu(const QString &mailbox);
    void initImages(QMailViewerInterface* view);

    bool hasGet(const QString &mailbox);
    bool hasSend(const QString &mailbox);
    bool hasEdit(const QString &mailbox);
    bool hasReply(const QString &mailbox);
    bool hasDelete(const QString &mailbox);

    void dialNumber(const QString&);

    void switchView(QWidget* widget, const QString& title);
    QMailAddress extractAddress() const;

private slots:
    void mmsFinished();
    void contactModelReset();

private:
    QStackedWidget *views;
    MailListView *mailView;
    bool sending, receiving;
    QMailMessage mail;
    QMailId lastMailId;
    ViewAtt *viewAtt;
    bool isMms;
    bool isSmil;
    bool firstRead;

    QMenu *context;

    QAction *deleteButton;
    bool initialized;
    QAction *nextButton;
    QMailViewerInterface *emailView;
    QMailViewerInterface *smilView;
    QAction *attachmentsButton;
    QAction *previousButton;
    QAction *replyButton;
    QAction *replyAllAction;
    QAction *forwardAction;
    QAction *getThisMailButton;
    QAction *sendThisMailButton;
    QAction *modifyButton;
    QAction *storeButton;
    AccountList *accountList;
    QContactModel *contactModel;
    bool modelUpdatePending;
};

#endif // READMAIL_H
