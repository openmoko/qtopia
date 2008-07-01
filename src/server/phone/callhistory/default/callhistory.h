/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef HAVE_CALLHISTORY_H
#define HAVE_CALLHISTORY_H

#include "callcontactlist.h"

#include <QSoftMenuBar>
#include <qtopia/pim/qcontact.h>
#include <QCallList>
#include <QtopiaAbstractService>

#include <QDialog>

class QLineEdit;
class QTextEntryProxy;
class QToolButton;
class QListWidget;
class QListWidgetItem;

class QAction;
class QAbstractMessageBox;
class QListBox;
class QTabWidget;
class QLabel;
class QKeyEvent;

class CallHistoryModel : public CallContactModel
{
    Q_OBJECT
public:
    CallHistoryModel(QCallList& callList, QObject* parent = 0);
    ~CallHistoryModel();

    QCallList::ListType type() const;
    void setType( QCallList::ListType type );

    void refresh();
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

signals:
    void contactsAboutToBeUpdated();
    void contactsUpdated();

protected slots:
    void updateContacts();
    void reallyUpdateContacts();

private:
    QCallList::ListType mType;
    bool mDirty;
};

class CallHistoryListView : public CallContactListView
{
    friend class CallHistory;
    Q_OBJECT
public:
    CallHistoryListView( QWidget *parent, Qt::WFlags fl = 0 );
    void setModel( QAbstractItemModel* model );

public slots:
    void refreshModel();
    void modelChanged();

protected slots:
    void updateMenu();
    void contactsAboutToChange();
    void contactsChanged();
    
private:
    QAction *mClearList;
    int prevRow;
    QString prevNumber;
    int prevCount;
    bool contactsChanging;
};

class QContact;
class CallHistoryView : public QWidget
{
    Q_OBJECT
public:
    CallHistoryView( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~CallHistoryView();

    QContact contact( QContactModel::Field &phoneType ) const;
    void setContact( const QContact &cnt, QContactModel::Field phoneType );
    void setCallListItem( QCallListItem item );

public slots:
    void clear();

signals:
    void externalLinkActivated();
    void deleteCurrentItem();

protected slots:
    void openContact();
    void addContact();
    void sendMessage();
    void updateMenu();

    void deleteItem();
    void deleteItem(int);
    void contactsChanged();
    void dialNumber();

protected:
    void keyPressEvent( QKeyEvent *e );
    void updateContent();

private:
    bool mHaveFocus;
    bool mHaveContact;
    bool mHaveDialer;
    QLabel *mCallTypePic, *mCallType, *mPortrait, *mName, *mContactTypePic, *mPhoneTypePic, *mNumber, *mStartDate, *mStartTime, *mDuration, *mTimeZone, *mTimeZoneLabel;
    QLabel *mNumberLabel;
    QLabel *mNameLabel;
    QCallListItem mCallListItem;
    QContact mContact;
    QContactModel::Field mPhoneType;
    QAbstractMessageBox *deleteMsg, *addContactMsg;

    QMenu *mMenu;
    QAction *mDeleteAction, *mOpenContact, *mSendMessage, *mAddContact;
    QPushButton *mDialButton;
};

class CallHistoryClearList : public QDialog
{
    Q_OBJECT
public:
    CallHistoryClearList( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~CallHistoryClearList();

    void setSelected(QCallList::ListType);

protected slots:
    void userSelected(QListWidgetItem *);

signals:
    void selected(QCallList::ListType);

private:
    QListWidget *mList;
};

class CallHistory : public QWidget
{
    Q_OBJECT
    friend class CallHistoryService;
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
    void focusFindLE(int);
    void showEvent( QShowEvent *e );
    void viewDetails( const QModelIndex& idx );
    void pageChanged(int);
    void clearList();
    void clearList( QCallList::ListType );
    void setFilterCur(const QString &f);
    void deleteCurrentItem();
    void deleteViewedItem();
    void showList( QCallList::ListType type );
    void viewDetails( QCallListItem, QContact, int );
    void updateTabText( const QString &filterStr );

protected:
    void constructTab( QCallList::ListType type, QAction *clearAction, CallContactDelegate *delegate );
    void cleanup();

private:
    QMap<QObject *, QString> mFilters;
    QTabWidget *mTabs;
    CallHistoryListView *mAllList, *mDialedList, *mReceivedList, *mMissedList;
    CallHistoryView *mView;
    QCallList &mCallList;
    bool mShowMissedCalls;
    bool mAllListShown, mDialedListShown, mReceivedListShown, mMissedListShown;
    CallHistoryClearList *mClearList;

    QLineEdit *mAllFindLE, *mDialedFindLE, *mReceivedFindLE, *mMissedFindLE;
    QTextEntryProxy *mAllFindProxy, *mDialedFindProxy, *mReceivedFindProxy, *mMissedFindProxy;
    QLabel *mAllFindIcon, *mDialedFindIcon, *mReceivedFindIcon, *mMissedFindIcon;
    QCallListItem mViewedItem;
};

class CallHistoryService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class CallHistory;
public:
    CallHistoryService( CallHistory *parent )
        : QtopiaAbstractService( "CallHistory", parent )
        { this->parent = parent; publishAll(); }

public:
    ~CallHistoryService();

public slots:
    void showCallHistory( QCallList::ListType type );
    void viewDetails( QCallListItem, QContact, int );

private:
    CallHistory *parent;
};


#endif
