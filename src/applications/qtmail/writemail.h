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

#ifndef WRITEMAIL_H
#define WRITEMAIL_H

#include <QMainWindow>
#include <QString>
#include <QMailMessage>

class QAction;
class QContent;
class QMailComposerInterface;
class QStackedWidget;

class AccountList;
class DetailsPage;
class SelectListWidget;

class WriteMail : public QMainWindow
{
    Q_OBJECT

public:
    enum ComposeAction { Create = 0, Reply = 1, ReplyToAll = 2, Forward = 3 };

    WriteMail( QWidget* parent, const char* name, Qt::WFlags fl = 0 );
    ~WriteMail();
    void reply(const QMailMessage& replyMail, int type);
    void modify(const QMailMessage& previousMessage);
    void setRecipients(const QString &emails, const QString &numbers);
    void setRecipient(const QString &recipient);
    bool readyToSend() const;
    void setBody(const QString &text, const QString &type);
    bool hasContent();
#ifndef QTOPIA_NO_SMS
    void setSmsRecipient(const QString &recipient);
#endif
    void setAccountList(AccountList *list);

    QString composer() const;
    void setComposer( const QString &id );

    bool isComplete() const;
    bool changed() const;
    bool canClose();

    void setAction(ComposeAction action);

    bool forcedClosure();

public slots:
    bool saveChangesOnRequest();
    void reset();
    void discard();
    bool draft();
    void composerSelected(const QString &key);
    void selectionCanceled();

signals:
    void autosaveMail(const QMailMessage&);
    void saveAsDraft(const QMailMessage&);
    void enqueueMail(const QMailMessage&);
    void discardMail();
    void noSendAccount(QMailMessage::MessageType);

public slots:
    void newMail( const QString &cmpsr = QString(), bool detailsOnly = false );
    void attach( const QContent &dl );
    void attach( const QString &fileName );

protected slots:
    void previousStage();
    void nextStage();
    void composeStage();
    void detailsStage();
    void sendStage();

    void enqueue();

    void messageChanged();
    void detailsChanged();

private:
    bool largeAttachments();
    uint largeAttachmentsLimit() const;

    bool buildMail();
    void init();

    QMailMessage mail;

    QMailComposerInterface *m_composerInterface;

    QWidget *m_composerWidget;
    DetailsPage *m_detailsPage;
    QAction *m_previousAction;
    QAction *m_cancelAction, *m_draftAction;

    QStackedWidget* widgetStack;

    QMainWindow *m_mainWindow;
    AccountList *accountList;
    bool hasMessageChanged;
    bool _detailsOnly;
    ComposeAction _action;
    QWidget *_selectComposer;
    SelectListWidget *_composerList;
};

#endif // WRITEMAIL_H
