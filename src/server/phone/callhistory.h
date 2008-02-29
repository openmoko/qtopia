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

#ifndef HAVE_CALLHISTORY_H
#define HAVE_CALLHISTORY_H

#include "callcontactlist.h"

#include <qsoftmenubar.h>
#include <qtopia/pim/qcontact.h>
#include <qcalllist.h>

#include <QTextBrowser>
#include <QStack>
#include <QDialog>

class QLineEdit;
class QToolButton;
class QListWidget;
class QListWidgetItem;

class FriendlyTabs;
class QAction;
class QAbstractMessageBox;
class QListBox;

class CallHistoryModel : public CallContactModel
{
public:
    CallHistoryModel(QCallList& callList, QObject* parent = 0);
    ~CallHistoryModel();

    CallContactItem::Types type() const;
    void setType( CallContactItem::Types type );

    void refresh();
private:
    CallContactItem::Types mType;
};

class CallHistoryList : public CallContactView
{
    friend class CallHistory;
    Q_OBJECT
public:
    CallHistoryList( QWidget *parent, Qt::WFlags fl = 0 );
    void setModel( QAbstractItemModel* model );

public slots:
    void itemDeleted();
    void modelChanged();

protected slots:
    void updateMenu(const QModelIndex& current, const QModelIndex& previous);

private:
    QAction *mClearList;
};

class QKeyEvent;
class CallHistoryBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    CallHistoryBrowser( QWidget *parent = 0 );
    void focusFirstLink();
protected:
    void keyPressEvent( QKeyEvent *e );
};

class QContact;
class CallHistoryView : public QWidget
{
    Q_OBJECT
public:
    CallHistoryView( QCallList &callList, QWidget *parent = 0, Qt::WFlags fl = 0, bool rw = true );
    ~CallHistoryView();

    QContact contact( QContactModel::Field &phoneType ) const;
    void setContact( const QContact &cnt, QContactModel::Field phoneType );
    void setCallListItem( QCallListItem item );

public slots:
    void update();
    void clear();
    void viewRelatedDetails( int callListIndex );

signals:
    void externalLinkActivated();
    void relatedCallClicked( int callListIndex );
    void itemDeleted();

protected slots:
    void openContact();
    void addContact();
    void sendMessage();
    void updateMenu();

    void linkActivated( const QUrl &ahref );
    void deleteItem();
    void deleteItem(int);

protected:
    QString makeDialLink( const QString &number ) const;
    void closeEvent(QCloseEvent *e);

private:
    bool mHaveFocus;
    bool mHaveContact;
    bool mHaveDialer;
    CallHistoryBrowser *mBrowser;
    QCallListItem mCallListItem;
    QContact mContact;
    QContactModel::Field mPhoneType;
    QCallList &mCallList;
    QAbstractMessageBox *deleteMsg, *addContactMsg;
    CallHistoryView *rView;
    QStack<QCallListItem> rStack;

    QMenu *mMenu;
    QAction *mDeleteAction, *mOpenContact, *mSendMessage, *mAddContact;
};

class CallHistoryClearList : public QDialog
{
    Q_OBJECT
public:
    enum ClearType
    {
        Dialed = 0,
        Received,
        Missed,
        All
    };

    CallHistoryClearList( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~CallHistoryClearList();

    void setSelected(CallHistoryClearList::ClearType);

protected slots:
    void userSelected(QListWidgetItem *);

signals:
    void selected(CallHistoryClearList::ClearType);

private:
    QListWidget *mList;
};

class CallHistory : public QWidget
{
    Q_OBJECT
public:
    CallHistory( QCallList &callList, QWidget *parent, Qt::WFlags fl = 0 );
    bool eventFilter( QObject *o, QEvent *e );
    void reset();
    void showMissedCalls();
    void refresh();
    void setFilter( const QString &f );

signals:
    void requestedDial(const QString&, const QUniqueId&);
    void viewedMissedCalls();

protected slots:
    void refreshOnFirstShow(int);
    void showEvent( QShowEvent *e );
    void viewDetails( const QModelIndex& idx );
    void pageChanged(int);
    void clearList();
    void clearList( CallHistoryClearList::ClearType );
    void setFilterCur(const QString &f);
    void cleanup();

protected:
    void closeEvent(QCloseEvent *e);

private:
    QMap<QObject *, QString> mFilters;
    FriendlyTabs *mTabs;
    CallHistoryList *mDialedList, *mReceivedList, *mMissedList;
    QWidget *mDialedTab, *mReceivedTab, *mMissedTab;
    CallHistoryView *mView;
    QCallList &mCallList;
    bool mShowMissedCalls;
    bool mDialedListShown, mReceivedListShown, mMissedListShown;
    CallHistoryClearList *mClearList;

    QLineEdit *mDialedFindLE, *mReceivedFindLE, *mMissedFindLE;
};

#endif
