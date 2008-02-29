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

#include "accounteditor.h"
#include "googleaccount.h"
#include "qsoftmenubar.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QProgressBar>
#include <QAppointmentModel>
#include <QVBoxLayout>
#include <QPimContext>
#include <QAction>
#include <QSoftMenuBar>
#include <QMenu>
#include <QDialog>
#include <QTimer>
#include <QtopiaApplication>
#include <QKeyEvent>

#include <qtopia/pim/private/qgooglecontext_p.h>

class AccountWidgetItem : public QListWidgetItem
{
public:
    // constructors based of known types.
    AccountWidgetItem(const QPimSource &source, QGoogleCalendarContext *context, QListWidget *parent = 0)
        : QListWidgetItem(context->icon(),
                context->name(source.identity).isEmpty()
                    ? source.identity : context->name(source.identity),
                parent)
        , mSource(source), mContext(context) {}

    AccountWidgetItem(QPimContext *context, QListWidget *parent = 0)
        : QListWidgetItem(context->icon(), context->title(), parent)
        , mContext(context) {}

    ~AccountWidgetItem() {}

    QPimSource source() const { return mSource; }
    QPimContext *context() const { return mContext; }

private:
    const QPimSource mSource;
    QPimContext *mContext;
};

AccountEditor::AccountEditor( QWidget *parent )
    : QWidget(parent)
        , mModel(0)
{
    mChoices = new QListWidget;
    mProgress = new QProgressBar;

    QVBoxLayout *vbl = new QVBoxLayout(this);
    vbl->addWidget(mChoices);
    vbl->addWidget(mProgress);

    setLayout(vbl);

    connect(mChoices, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem* )),
                this, SLOT(currentAccountChanged(QListWidgetItem *)));
    mChoices->installEventFilter(this);

    actionAdd = new QAction(QIcon(":icon/new"), tr("Add Account"), this);
    actionAdd->setWhatsThis(tr("Create a new account"));
    connect(actionAdd, SIGNAL(triggered()), this, SLOT(addAccount()));

    actionEdit = new QAction(QIcon(":icon/edit"), tr("Edit Account"), this);
    actionEdit->setWhatsThis(tr("Edit the selected account"));
    connect(actionEdit, SIGNAL(triggered()), this, SLOT(editCurrentAccount()));

    actionRemove = new QAction(QIcon(":icon/trash"), tr("Delete Account"), this);
    actionRemove->setWhatsThis(tr("Delete the selected account"));
    connect(actionRemove, SIGNAL(triggered()), this, SLOT(removeCurrentAccount()));

    actionSync = new QAction(QIcon(":icon/sync"), tr("Sync Account"), this);
    actionSync->setWhatsThis(tr("Syncs the selected accounts"));
    connect(actionSync, SIGNAL(triggered()), this, SLOT(syncCurrentAccount()));

    actionSyncAll = new QAction(QIcon(":icon/sync"), tr("Sync All"), this);
    actionSyncAll->setWhatsThis(tr("Syncs all the accounts"));
    connect(actionSyncAll, SIGNAL(triggered()), this, SLOT(syncAllAccounts()));

    QMenu *contextMenu = QSoftMenuBar::menuFor(this);

    contextMenu->addAction(actionAdd);
    contextMenu->addAction(actionEdit);
    contextMenu->addAction(actionRemove);
    contextMenu->addAction(actionSync);
    contextMenu->addAction(actionSyncAll);

    mProgress->hide();
    populate();
    updateActions();

    progressHideTimer = new QTimer(this);
    progressHideTimer->setInterval(2000);
    progressHideTimer->setSingleShot(true);
    connect(progressHideTimer, SIGNAL(timeout()), this, SLOT(hideProgressBar()));
}

AccountEditor::~AccountEditor()
{
}

void AccountEditor::hideProgressBar()
{
    mProgress->hide();
}

/*
   Returns true if the \a model has any contexts that allow adding, editing, removing or
   syncing of accounts that are understood by this widget.   Otherwise returns false.
*/
bool AccountEditor::editableAccounts(const QAppointmentModel *model)
{
    Q_ASSERT(model != 0);

    foreach(QPimContext *c, model->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal)
            return true;
    }
    return false;
}

void AccountEditor::setModel(QAppointmentModel *model)
{
    mModel = model;
    foreach(QPimContext *c, mModel->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal) {
            connect(gcal, SIGNAL(syncProgressChanged(int, int)), this, SLOT(updateProgress()));
            connect(gcal, SIGNAL(finishedSyncing()), progressHideTimer, SLOT(start()));
        }
    }
    populate();
    updateActions();
}

void AccountEditor::addAccount()
{
    QListWidget *accountTypes = new QListWidget;
    QDialog diag;
    QVBoxLayout *vl = new QVBoxLayout;
    vl->addWidget(accountTypes);
    vl->setMargin(7);
    diag.setWindowTitle(tr("Account Type", "window title"));
    diag.setLayout(vl);

    foreach(QPimContext *c, mModel->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal) {
            new AccountWidgetItem(gcal, accountTypes);
        }
    }

    // Select the first item
    accountTypes->setCurrentRow(0);
    if (accountTypes->currentItem())
        accountTypes->currentItem()->setSelected(true);

    connect(accountTypes, SIGNAL(itemActivated(QListWidgetItem *)), &diag, SLOT(accept()));

    // and make this a menu like dialog so we can cancel
    QtopiaApplication::setMenuLike(&diag, true);

    if (QtopiaApplication::execDialog(&diag)) {
        AccountWidgetItem *item = (AccountWidgetItem*)accountTypes->currentItem();
        if (item) {
            // there can be only one....
            QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(item->context());
            if (gcal) {
                GoogleAccount gdiag;
                if (QtopiaApplication::execDialog(&gdiag)) {
                    gcal->addAccount(gdiag.email(), gdiag.password());
                    gcal->setFeedType(gdiag.email(), gdiag.feedType());
                }
            }
        }
    }
    populate();
    updateActions();
}

void AccountEditor::removeCurrentAccount()
{
    AccountWidgetItem *item = (AccountWidgetItem *)mChoices->currentItem();
    if (!item)
        return;
    QGoogleCalendarContext *gcal;
    gcal = qobject_cast<QGoogleCalendarContext *>(item->context());
    if (gcal) {
        gcal->removeAccount(item->source().identity);
        mChoices->takeItem(mChoices->row(item));
        delete item;
    }
    populate();
    updateActions();
}

void AccountEditor::editCurrentAccount()
{
    AccountWidgetItem *item = (AccountWidgetItem *)mChoices->currentItem();
    if (!item)
        return;
    QGoogleCalendarContext *gcal;
    gcal = qobject_cast<QGoogleCalendarContext *>(item->context());
    if (gcal) {
        GoogleAccount diag;
        QString account = item->source().identity;
        diag.setEmail(account);
        diag.setPassword(gcal->password(account));
        diag.setName(gcal->name(account));
        diag.setFeedType(gcal->feedType(account));
        if (QtopiaApplication::execDialog( &diag )) {
            if (account != diag.email()) {
                gcal->removeAccount(account);
                gcal->addAccount(diag.email(), diag.password());
            } else {
                gcal->setPassword(diag.email(), diag.password());
            }
            gcal->setFeedType(diag.email(), diag.feedType());
        }
    }
    populate();
    updateActions();
}

void AccountEditor::syncAllAccounts()
{
    for (int i = 0; i < mChoices->count(); ++i) {
        AccountWidgetItem *accountItem = (AccountWidgetItem *)mChoices->item(i);
        QGoogleCalendarContext *gcal;
        gcal = qobject_cast<QGoogleCalendarContext *>(accountItem->context());
        if (gcal) {
            gcal->syncAccount(accountItem->source().identity);
            continue;
        }
    }
    updateProgress();
}

void AccountEditor::syncCurrentAccount()
{
    AccountWidgetItem *item = (AccountWidgetItem *)mChoices->currentItem();
    if (!item)
        return;
    QGoogleCalendarContext *gcal;
    gcal = qobject_cast<QGoogleCalendarContext *>(item->context());
    if (gcal) {
        gcal->syncAccount(item->source().identity);
        updateProgress();
        return; // redundent, left here for future account types.
    }
}

void AccountEditor::currentAccountChanged(QListWidgetItem *qlwi)
{
    if (qlwi != NULL) {
        qlwi->setSelected(true);
    }
    updateActions();
}

bool AccountEditor::eventFilter(QObject *o, QEvent *e)
{
    if (o) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *kk = (QKeyEvent*) e;
            if( o == mChoices) {
                if (kk->key() == Qt::Key_Select) {
                    if (mChoices->count() == 0)
                        addAccount();
                    else
                        editCurrentAccount();
                    return true;
                }
            }
        }
    }
    return false;
}

void AccountEditor::updateActions()
{
    bool hasCurrent;
    bool currentSyncable;
    bool anySyncable;
    AccountWidgetItem *accountItem = (AccountWidgetItem *)mChoices->currentItem();
    if (accountItem) {
        hasCurrent = true;
        if (accountItem->context()->inherits("QGoogleCalendarContext"))
            currentSyncable = true;
        else
            currentSyncable = false;
    } else {
        hasCurrent = false;
        currentSyncable = false;
    }

    if (currentSyncable) {
        anySyncable = true;
    } else {
        anySyncable = false;
        for (int i = 0; i < mChoices->count(); ++i) {
            AccountWidgetItem *accountItem = (AccountWidgetItem *)mChoices->item(i);
            QGoogleCalendarContext *gcal;
            gcal = qobject_cast<QGoogleCalendarContext *>(accountItem->context());
            if (gcal) {
                anySyncable = true;
                break;
            }
        }
    }

    actionEdit->setVisible(hasCurrent);
    actionRemove->setVisible(hasCurrent);
    actionSync->setVisible(currentSyncable);
    actionSyncAll->setVisible(anySyncable);

    if (hasCurrent)
        QSoftMenuBar::setLabel(mChoices, Qt::Key_Select, QSoftMenuBar::Edit);
    else
        QSoftMenuBar::setLabel(mChoices, Qt::Key_Select, "new", tr("New"));
}

void AccountEditor::updateProgress()
{
    if (mProgress->isHidden())
        mProgress->show();
    int amount = 0;
    int total = 0;
    foreach(QPimContext *c, mModel->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal) {
            int a, t;
            gcal->syncProgress(a, t);
            if (t <= 0 || a > t) {
                mProgress->setValue(0);
                mProgress->setMaximum(0);
                return;
            }
            amount += a;
            amount += t;
        }
    }
    mProgress->setMaximum(total);
    mProgress->setValue(amount);
}

void AccountEditor::populate()
{
    if (!mModel)
        return;
    mChoices->clear();
    foreach(QPimContext *c, mModel->contexts()) {
        QGoogleCalendarContext *gcal = qobject_cast<QGoogleCalendarContext *>(c);
        if (gcal) {
            QStringList accounts = gcal->accounts();
            QPimSource s;
            s.context = gcal->id();
            foreach(QString a, accounts) {
                s.identity = a;
                new AccountWidgetItem(s, gcal, mChoices);
            }
        }
    }
}
