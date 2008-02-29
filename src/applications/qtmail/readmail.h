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



#ifndef READMAIL_H
#define READMAIL_H

#include <qaction.h>
#include <qmainwindow.h>
#include <qmenudata.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qtextbrowser.h>
#include <qtoolbutton.h>
#include <qmime.h>

#include <quuid.h>
#include <qurl.h>
#include <qmap.h>
#include <qtopia/pim/qcontactmodel.h>

#include "email.h"
#include "emaillistitem.h"
#include "viewatt.h"

class QLabel;
class MailListView;
class QMenu;
class SmilView;
class QStackedWidget;
class SmilDataSource;
class SmilElement;

/*  Browser (impl. to catch setSource)  */
class Browser: public QTextBrowser
{
    Q_OBJECT

public:
    Browser(QWidget *parent = 0);
    virtual ~Browser();
    void scrollBy(int dx, int dy);
    virtual QVariant loadResource( int, const QUrl& );
    void setResource( const QUrl& name, QVariant var );

signals:
    void linkClicked(const QString &href);

public slots:
    virtual void setSource(const QUrl &name);

private:
    QMap<QUrl, QVariant> resourceMap;
};

class ReadMail : public QMainWindow
{
    Q_OBJECT

public:
    ReadMail( QWidget* parent = 0, const QString name = QString(), Qt::WFlags fl = 0 );
    ~ReadMail();
    void update(MailListView *view);
    void updateView();
    void mailUpdated(Email *mailIn);
#ifdef QTOPIA_PHONE
    bool eventFilter( QObject *, QEvent * );
#endif
signals:
    void resendRequested(const Email &, int);
    void mailto(const QString &);
    void modifyRequested(Email *);
    void removeItem(EmailListItem *);
    void viewingMail(Email *);
    void getMailRequested(Email *);
    void sendMailRequested(Email *);
    void readReplyRequested(Email*);
    void cancelView();

public slots:
    void cleanup();
    void isSending(bool);
    void isReceiving(bool);
    void setProgressText(const QString &txt);

protected slots:
    void resizeEvent(QResizeEvent *);

    void linkClicked(const QUrl &lnk);

    void next();
    void previous();

    void deleteItem();
    void shiftText();
    void viewAttachments();

    void reply();
    void replyAll();
    void forward();
    void modify();

    void setStatus(int);
    void getThisMail();
    void sendThisMail();
    void print();

protected:
    void keyPressEvent(QKeyEvent *);
    void closeEvent( QCloseEvent *e );
private:
    QString fancyText(QString mailSize);
    QString normalText(QString mailSize);

    QString formatText(QString txt);
    QString buildParagraph(QString txt, QString prepend, bool preserveWs = false);
    QString handleReplies(QString txt);
    QString noBreakReplies(QString txt);
    QString smsBreakReplies(QString txt);

    QString encodeUrlAndMail(QString txt);
    QString listRefMailTo(QStringList list);
    QString refMailTo(QString adr);
    void viewMms();
#ifdef QTOPIA_PHONE
#ifndef QTOPIA_NO_MMS
    void tweakSmil(SmilElement *smil);
    void advanceMmsSlide();
#endif
#endif

    int pWidth();
    int pHeight();

    void init();
    void updateButtons();
    void buildMenu(const QString &mailbox);
    void initImages();

    bool hasGet(const QString &mailbox);
    bool hasSend(const QString &mailbox);
    bool hasEdit(const QString &mailbox);
    bool hasReply(const QString &mailbox);

    void addDialActions();

private slots:
    void mmsFinished();
    void requestTransfer(SmilDataSource *dataSource, const QString &src);
    void cancelTransfer(SmilDataSource *dataSource, const QString &src);
    void dialEmbeddedNumber();

private:
    QStackedWidget *views;
    MailListView *mailView;
    bool plainTxt;
    bool sending, receiving;
    Email *mail;
    QUuid lastMailUuid;
    ViewAtt *viewAtt;
    bool isMms;
    bool isSmil;
    bool firstRead;

    int _pWidth, _pHeight;

#ifdef QTOPIA_PHONE
    QMenu *context;
    SmilView *smilView;
#else
    QToolBar *bar;
    QMenuBar *menu;
    QMenu *viewMenu, *mailMenu, *statusMenu;
#endif

    QAction *deleteButton;
    bool initialized;
    QAction *plainTextButton;
    QAction *nextButton;
    Browser *emailView;
    QAction *attachmentsButton;
    QAction *previousButton;
    QAction *replyButton;
    QAction *replyAllAction;
    QAction *forwardAction;
    QAction *getThisMailButton;
    QAction *sendThisMailButton;
    QAction *modifyButton;
    QAction *printButton;
    QLabel *progressLabel;
    QContactModel *mContactModel;
    QList<QAction *> dialActions;
};

#endif // READMAIL_H
