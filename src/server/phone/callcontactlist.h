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

#ifndef HAVE_CALLCONTACTLIST_H
#define HAVE_CALLCONTACTLIST_H

#include <QListView>
#include <qtopia/pim/qcontactmodel.h>
#include <qtopia/pim/qcontactview.h>
#include <qcalllist.h>
#include <qtopia/inputmatch/pkimmatcher.h>

class QMenu;
class QAction;
class QUniqueId;
class PhoneMessageBox;

class CallContactItem : public QObject
{
Q_OBJECT
public:
    enum Types {
        Contact,
        DialedNumber,
        ReceivedCall,
        MissedCall
    };

    static CallContactItem::Types stateToType( QCallListItem::CallType st );
    static QPixmap typePixmap( CallContactItem::Types type );

    CallContactItem( CallContactItem::Types t,
            QCallListItem clItem, QObject * parent = 0);


    QContactModel::Field fieldType() const;
    CallContactItem::Types type() const;

    void setContact( const QContact& cnt, const QString& phoneNumber);
    void setContact( const QContactModel *m, const QUniqueId &);

    QContact contact() const;

    QPixmap decoration() const;
    QPixmap extraDecoration() const;
    QString text() const;
    QString extraInfoText() const;
    QCallListItem callListItem() const;

    QString number() const;
    QUniqueId contactID() const;

private:
    QContactModel::Field contactNumberToFieldType(const QString& number) const;
    QString fieldTypeToContactDetail() const;

    CallContactItem::Types mType;

    mutable QContactModel::Field mFieldType;
    mutable QContact mContact;

    QContactModel const *mModel;
    QUniqueId mId;

    QCallListItem clItem;
};

class CallContactModel : public QAbstractListModel
{
    Q_OBJECT
public:

    CallContactModel( QCallList &callList, QObject *parent = 0);
    virtual ~CallContactModel();

    CallContactItem * itemAt( const QModelIndex & index ) const;

    int rowCount(const QModelIndex & = QModelIndex()) const
    { return callContactItems.count(); }
    QVariant data(const QModelIndex &index, int role) const;

    void resetModel();
    virtual void refresh();

    QString filter () const { return mFilter;};

public slots:
    void setFilter(const QString& filter);

protected:
    QList<CallContactItem*> callContactItems;
    QCallList &mCallList;
    QList<QCallListItem> mRawCallList;
    InputMatcher pk_matcher;
private:
    int findPattern(const QString &content) const;

    QString mFilter;
};

class CallContactDelegate : public QContactDelegate
{
public:
    CallContactDelegate( QObject * parent = 0 );
    virtual ~CallContactDelegate();

    QFont secondaryFont(const QStyleOptionViewItem& o, const QModelIndex& idx) const;
};

class CallContactView : public QListView
{
    Q_OBJECT
public:
    CallContactView(QWidget * parent = 0);
    ~CallContactView();

    void setModel(QAbstractItemModel* model);
    QString number() const
    {
        return mNumber;
    }
    QString numberForIndex(const QModelIndex & idx) const;
    QContact contactForIndex(const QModelIndex & idx) const;

    void setEmptyMessage(const QString& newMessage);

public slots:
    void addItemToContact();
    void openContact();
    void sendMessageToItem();
    virtual void updateMenu(const QModelIndex& current, const QModelIndex& previous);

    virtual void reset();

signals:
    void requestedDial(const QString&, const QUniqueId&);
    void hangupActivated();

protected:
    void keyPressEvent( QKeyEvent *e );
    void paintEvent( QPaintEvent *pe );
    void focusInEvent( QFocusEvent *focusEvent);

    QMenu *mMenu;

    QAction *mOpenContact;
    QAction *mAddContact;
    QAction *mSendMessage;
    PhoneMessageBox *addContactMsg;

    QString mNumber;

    QString m_noResultMessage;

    CallContactModel *cclm;
};

#endif //HAVE_CALLCONTACTLIST_h
