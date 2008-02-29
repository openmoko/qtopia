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

#ifndef WRITEMAIL_H
#define WRITEMAIL_H

#include <QMainWindow>
#include <QAction>
#include <QLabel>
#include <QToolButton>
#include <QLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QListView>
#include <QComboBox>
#include <QSize>
#include <QStringList>
#include <QList>
#include <QListWidget>
//#include <qtopia/pim/private/contactfieldselector_p.h>

#include "email.h"
#include "addatt.h"
#ifdef QTOPIA_PHONE
#include "composer.h"
#else
#include "pdacomposer.h"
#endif

class AccountList;
class FileSelector;
class QLineEdit;
class QStackedWidget;
class QHBox;
#ifdef QTOPIA_PHONE
class DetailsPage;
#endif

class SelectListWidget : public QListWidget
{
    Q_OBJECT

public:
    SelectListWidget( QDialog* parent );

    bool keyBackPressed();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    QDialog *mParent;
    bool mKeyBackPressed;
};

class WriteMail : public QMainWindow
{
    Q_OBJECT

public:
    WriteMail( QWidget* parent, const char* name, Qt::WFlags fl = 0 );
    ~WriteMail();
    void reply(const Email &, int type);
    void modify(Email *previousMail);
    void setRecipients(const QString &emails, const QString &numbers);
    void setRecipient(const QString &recipient);
    bool hasRecipients() const;
    void setBody(const QString &text);
    bool hasContent();
#ifdef QTOPIA_PHONE
    void setSmsRecipient(const QString &recipient);
#endif
    void setAccountList(AccountList *list);

#ifdef QTOPIA_PHONE
    void setComposer( const QString &id );
    void setComposerFocus();
    QString composer() const;
#endif

    bool isComplete() const;
    bool changed() const;
#ifdef QTOPIA_PHONE
    bool keyPressAccepted();
#endif

public slots:
    bool tryAccept();
    void reset();
    void discard();

signals:
    void autosaveMail(const Email &);
    void saveAsDraft(const Email &);
    void enqueueMail(const Email &);
    void discardMail();

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

    void updateUI();
    bool selectComposer();
    void enqueue();
    void draft();

    void messageChanged();

protected:
    void keyPressEvent(QKeyEvent *);

private:
    bool buildMail();
    void init();
    Email mail;

#ifdef QTOPIA_PHONE
    ComposerInterface
#else
    PDAComposer
#endif
    *m_composerInterface;

#ifdef QTOPIA_PHONE
    QWidget *m_composerWidget;
    DetailsPage *m_detailsPage;
    QAction *m_previousAction;
#else
    QAction *m_ccAction, *m_bccAction, *m_fromAction, *m_wrapAction;
#endif
    QAction *m_cancelAction, *m_draftAction;

    QStackedWidget* widgetStack;

    QMainWindow *m_mainWindow;
    AccountList *accountList;
#ifdef QTOPIA_PHONE
    bool keyEventAccepted;
#endif
    bool hasMessageChanged;
    bool _detailsOnly;
};

#endif // WRITEMAIL_H
