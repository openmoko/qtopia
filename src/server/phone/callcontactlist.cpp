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

#include "callcontactlist.h"

#include <qtopia/pim/qphonenumber.h>

#include <qtimestring.h>
#include <qsoftmenubar.h>
#include <qtopiaservices.h>
#include <quniqueid.h>
#include <qtopiaipcenvelope.h>
#include <qthumbnail.h>
#include <qtopiaapplication.h>

#include <QAction>
#include <QTextDocument>
#include <QKeyEvent>
#include <QPainter>
#include <QDebug>
#include <QSettings>
#include <QMenu>

#include "savetocontacts.h"

CallContactItem::CallContactItem( CallContactItem::Types t,
                                  QCallListItem cli, QObject *parent)
:  QObject(parent), mType(t), mFieldType(QContactModel::Invalid), mModel(0),
   clItem(cli)
{
}

QCallListItem CallContactItem::callListItem() const
{
    return clItem;
}

QContactModel::Field CallContactItem::fieldType() const
{
    if (mFieldType == QContactModel::Invalid && mModel) {
        QString number = contact().defaultPhoneNumber();
        mFieldType = contactNumberToFieldType(number);
    }
    return mFieldType;
}

CallContactItem::Types CallContactItem::type() const
{
    return mType;
}

QUniqueId CallContactItem::contactID() const
{
    return mId;
}

void CallContactItem::setContact( const QContact& cnt, const QString& number)
{
    mContact = cnt;
    mId = cnt.uid();
    mModel = 0;
    mFieldType = contactNumberToFieldType(number.simplified());
}

void CallContactItem::setContact( const QContactModel *m, const QUniqueId &id)
{
    mModel = m;
    mId = id;
    mFieldType = QContactModel::Invalid;
    mContact = QContact();
}

//return value undefined if (mFieldType == CallContactItem::Invalid)
QContact CallContactItem::contact() const
{
    if (mContact.uid() != mId && mModel)
        mContact = mModel->contact(mId);
    return mContact;
}

/*
   Most useful is the type of phone number of a contact
   so show that first.

   Next most useful is a contacts image, so fall back on that

   Finally, the type pixmap should be shown if nothing
   else is available
*/
QPixmap CallContactItem::decoration() const
{
    if (mType == Contact)
        return contact().thumbnail();

    return typePixmap(mType);
}

QPixmap CallContactItem::extraDecoration() const
{
    QIcon icon = QContactModel::fieldIcon(fieldType());
    return icon.isNull() ? QPixmap() : QPixmap(icon.pixmap(QSize(16,16)));
}

QString CallContactItem::text() const
{
    if (mType == CallContactItem::Contact)
        return contact().label();
    return clItem.number();
}

QString CallContactItem::extraInfoText( ) const
{
    if (mType == CallContactItem::Contact && clItem.isNull())
    {
        return fieldTypeToContactDetail();
    }
    else if (!clItem.isNull())
    {
       QString desc;
       QCallListItem::CallType st = clItem.type();
       if ( st == QCallListItem::Dialed )
           desc = tr("Dialed");
       else if ( st == QCallListItem::Received )
           desc = tr("Received");
       else if ( st == QCallListItem::Missed )
           desc = tr("Missed");
       desc += " ";
       QDateTime dt = clItem.start();
       QDate callDate = dt.date();
       QTime callTime = dt.time();
       QString when("%1 %2");
       when = when.arg(QTimeString::localMD(callDate, QTimeString::Short))
           .arg(QTimeString::localHM(callTime, QTimeString::Short));
       return desc + when;
    }
    else
    {
        qWarning("BUG: item is not contact and not in call list index");
        return QString();
    }
}

QString CallContactItem::number() const
{
    if (mType == CallContactItem::Contact && clItem.isNull())
        return fieldTypeToContactDetail();
    else if (!clItem.isNull())
        return clItem.number();
    else
        return QString("");
}

QPixmap CallContactItem::typePixmap( CallContactItem::Types type )
{
    QString typePixFileName;
    switch( type )
    {
        case CallContactItem::Contact:
            typePixFileName = "AddressBook";
            break;
        case CallContactItem::DialedNumber:
            typePixFileName = "phone/outgoingcall";
            break;
        case CallContactItem::ReceivedCall:
            typePixFileName = "phone/incomingcall";
            break;
        case CallContactItem::MissedCall:
            typePixFileName = "phone/missedcall";
            break;
    }

    QIcon icon(":icon/"+typePixFileName);

    return icon.pixmap(QContact::thumbnailSize());
}


QContactModel::Field CallContactItem::contactNumberToFieldType(const QString& number) const
{
    QContact cnt = contact();
    QList<QContactModel::Field> list = QContactModel::phoneFields();
    list.append(QContactModel::Emails);

    int bestMatch = 0;
    QContactModel::Field bestField = QContactModel::Invalid;
    foreach(QContactModel::Field f, list) {
        QString candidate = QContactModel::contactField(cnt, f).toString();
        int match = QPhoneNumber::matchNumbers(number, candidate);
        if (match > bestMatch) {
            bestField = f;
            bestMatch = match;
        }
    }
    return bestField;
}

QString CallContactItem::fieldTypeToContactDetail() const
{
    QContact cnt = contact();
    return QContactModel::contactField(cnt, fieldType()).toString();
}


CallContactItem::Types CallContactItem::stateToType( QCallListItem::CallType st )
{
    if ( st == QCallListItem::Dialed )
        return CallContactItem::DialedNumber;
    else if ( st == QCallListItem::Missed )
        return CallContactItem::MissedCall;
    else if ( st == QCallListItem::Received )
        return CallContactItem::ReceivedCall;
    else {
        qWarning("BUG: Invalid state passed to CallContactItem::stateToType");
        return CallContactItem::DialedNumber;
    }
}

//===================================================================

CallContactModel::CallContactModel( QCallList &callList, QObject *parent)
    :QAbstractListModel(parent), mCallList(callList), pk_matcher("text")
{
    mRawCallList = mCallList.allCalls();
}

CallContactModel::~CallContactModel() {}

CallContactItem * CallContactModel::itemAt( const QModelIndex & index ) const
{
    if (!index.isValid())
        return 0;

    return callContactItems.at(index.row());
}

int CallContactModel::findPattern(const QString &content) const
{
    const QString ctext = content.toLower();
    int idx = ctext.indexOf(mFilter);
    if (idx == -1)
        idx = pk_matcher.collate(ctext).indexOf(mFilter);
    return idx;
}

QVariant CallContactModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < rowCount())
    {
        CallContactItem * item = callContactItems.at(index.row());
        if (!item)
            return QVariant();
        QString text = item->text();
        switch(role)
        {
            case Qt::DecorationRole:
            case QContactModel::PortraitRole:
                return item->decoration();
            case Qt::DisplayRole:
            case QContactModel::LabelRole:
                {
                    QString result;
                    if (text.isEmpty())
                        result = item->extraInfoText();
                    else
                        result = text;
#if 0
                    /* only a guess, and almost impossible to be right withough
                       building the label same as QContact does.

                       TODO have QContact do underlining for a given search term?
                       */
                    if (!mFilter.isEmpty())
                    {
                        int pos = findPattern(result);
                        if (pos != -1)
                            result = Qt::escape(result.left(pos)) +
                                "<u>" + Qt::escape(result.mid(pos, mFilter.length())) +
                                "</u>" + Qt::escape(result.mid(pos + mFilter.length()));

                    }
#endif
                    return result.prepend("<b>").append("</b>");
                }
            case QContactModel::SubLabelRole:
                if (text.isEmpty())
                    return QString();
                else
                    return item->extraInfoText();
            case QContactModel::StatusIconRole:
                return item->extraDecoration();
            default:
                break;
        }
    }
    return QVariant();
}

void CallContactModel::refresh()
{
    mRawCallList = mCallList.allCalls();
}

void CallContactModel::resetModel()
{
    while (!callContactItems.isEmpty())
        delete callContactItems.takeFirst();
}

void CallContactModel::setFilter(const QString& filter)
{
    mFilter = filter;
    refresh();
}

//==================================================================

CallContactDelegate::CallContactDelegate( QObject * parent)
    : QContactDelegate(parent)
{
}

CallContactDelegate::~CallContactDelegate() {}

QFont CallContactDelegate::secondaryFont(const QStyleOptionViewItem& o) const
{
    QFont font = QContactDelegate::secondaryFont(o);
    QFont f(font);
    f.setItalic(true);
    return f;
}


//==============================================================


CallContactView::CallContactView(QWidget * parent)
    :QListView(parent), addContactMsg(0)
{
    setFrameStyle(QFrame::NoFrame);

    mMenu = QSoftMenuBar::menuFor( this );

    QIcon addressbookIcon(":image/addressbook/AddressBook");

    mAddContact = mMenu->addAction(addressbookIcon, tr("Save to Contacts"), this, SLOT(addItemToContact()));
    mAddContact->setVisible(false);
    mOpenContact = mMenu->addAction(addressbookIcon, tr("Open Contact"), this, SLOT(openContact()));
    mOpenContact->setVisible(false);
    mSendMessage = mMenu->addAction(QIcon(":icon/email"), tr("Send Message"), this, SLOT(sendMessageToItem()));
    mSendMessage->setVisible(false);

    m_noResultMessage = tr("No matches.");
    setAlternatingRowColors(true);

    setSelectionMode(QAbstractItemView::SingleSelection);
}

CallContactView::~CallContactView() {}

void CallContactView::paintEvent( QPaintEvent *pe )
{
    QListView::paintEvent(pe);
    if (cclm && !cclm->rowCount())
    {
        QWidget *vp = viewport();
        QPainter p( vp );
        QFont f = p.font();
        f.setBold(true);
        f.setItalic(true);
        p.setFont(f);
        p.drawText( 0, 0, vp->width(), vp->height(), Qt::AlignCenter,
                (cclm->filter().isEmpty() ? tr("No Items"): m_noResultMessage) );
    }
}

void CallContactView::setModel(QAbstractItemModel* model)
{
    cclm  = qobject_cast<CallContactModel*>(model);
    if (!cclm)
    {
        qWarning("CallContactView::setModel(): expecting model of type CallContactModel");
    }
    QListView::setModel(model);
}

void CallContactView::addItemToContact()
{
    QModelIndex idx = selectionModel()->currentIndex();
    CallContactItem * cci = cclm->itemAt(idx);
    if (!cci)
        return;

    QString number = cci->text();
    if (cci->fieldType() == QContactModel::Invalid && !number.isEmpty())
        SavePhoneNumberDialog::savePhoneNumber(number);
}

void CallContactView::openContact()
{
    QModelIndex idx = selectionModel()->currentIndex();
    CallContactItem* cci = cclm->itemAt(idx);
    if (!cci)
        return;

    QUniqueId cntID = cci->contactID();
    if (!cntID.isNull())
    {
        QtopiaServiceRequest req( "Contacts", "showContact(QUniqueId)" );
        req << cntID;
        req.send();
    }
}

void CallContactView::sendMessageToItem()
{
    QModelIndex idx = selectionModel()->currentIndex();
    CallContactItem* cci = cclm->itemAt(idx);
    if (!cci)
        return;

    QString name = cci->text();
    QString number = cci->number();

    if (!number.isEmpty()) {
        QtopiaServiceRequest req( "SMS", "writeSms(QString,QString)");
        req << name << number;
        req.send();
        // XXX what about atachments
    }
}

void CallContactView::updateMenu(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    mSendMessage->setEnabled(false);
    mSendMessage->setVisible(false);
    mAddContact->setEnabled(false);
    mAddContact->setVisible(false);
    mOpenContact->setEnabled(false);
    mOpenContact->setVisible(false);

    CallContactItem* cci = cclm->itemAt(current);
    if (!cci)
        return;

    QContactModel::Field fieldType = cci->fieldType();
    if ( fieldType == QContactModel::HomeMobile ||
         fieldType == QContactModel::BusinessMobile ||
         cci->type() != CallContactItem::Contact )
    {
        mSendMessage->setEnabled(true);
        mSendMessage->setVisible(true);
    }

    switch (cci->type())
    {
        case CallContactItem::Contact:
            mOpenContact->setEnabled(true);
            mOpenContact->setVisible(true);
            break;
        case CallContactItem::DialedNumber:
        case CallContactItem::ReceivedCall:
        case CallContactItem::MissedCall:
            if (fieldType == QContactModel::Invalid) {
                mAddContact->setVisible(true);
                mAddContact->setEnabled(true);
            } else {
                mOpenContact->setEnabled(true);
                mOpenContact->setVisible(true);
            }
            break;
    }
}

QString CallContactView::numberForIndex(const QModelIndex & idx) const
{
    QString number;
    CallContactItem* cci = cclm->itemAt(idx);
    if (!cci)
        return number;
    number = cci->number();
    return number;
}

QContact CallContactView::contactForIndex(const QModelIndex & idx) const
{
    CallContactItem* cci = cclm->itemAt(idx);
    if (!cci)
        return QContact();
    return cci->contact();
}

void CallContactView::reset()
{
    QListView::reset();
    mNumber = QString();
}

void CallContactView::keyPressEvent( QKeyEvent *e )
{
    int key = e->key();
    if (key == Qt::Key_Call || key == Qt::Key_Yes) {
        QModelIndex idx = selectionModel()->currentIndex();
        if( idx.isValid() )
            emit requestedDial( numberForIndex(idx), contactForIndex(idx).uid() );
        else
            emit hangupActivated();
        e->accept();
    } else if (key == Qt::Key_Hangup  || key == Qt::Key_Back || key == Qt::Key_No) {
        emit hangupActivated();
        e->accept();
    } else if (key == Qt::Key_Flip) {
        QSettings cfg("Trolltech","Phone");
        cfg.beginGroup("FlipFunction");
        if (cfg.value("hangup").toBool()) {
            emit hangupActivated();
            e->accept();
        }
    } else {
        QListView::keyPressEvent( e );
    }
}

void CallContactView::focusInEvent( QFocusEvent *focusEvent)
{
    QListView::focusInEvent( focusEvent );
    setEditFocus( true );
}

void CallContactView::setEmptyMessage(const QString& newMessage)
{
    m_noResultMessage = newMessage;
}

