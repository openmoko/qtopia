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
#ifndef CONTACTDETAILS_H
#define CONTACTDETAILS_H

#include <qtopia/pim/qcontact.h>
#include <qtopiaservices.h>
#ifdef QTOPIA_VOIP
#include <qpresence.h>
#endif

#include "qpimdelegate.h"
#include <QWidget>

class QTabWidget;
class ContactBrowser;
class ContactOverview;
#if defined(QTOPIA_TELEPHONY)
class ContactCallHistoryList;
#endif
class ContactMessageHistoryList;
class QContactModel;

class ContactDetails : public QWidget
{
    Q_OBJECT

public:
    ContactDetails( QWidget *parent );
    ~ContactDetails();

    QContact entry() const {return ent;}

public slots:
    void init( const QContact &entry );
    void modelChanged();

signals:
    void externalLinkActivated();
    void backClicked();

    void callContact();
    void textContact();
    void emailContact();
    void editContact();
    void personaliseContact();

    void highlighted(const QString&);

private:
    typedef enum {NoLink = 0, Dialer, Messaging, Email} LinkType;

    QContact ent;
    QString mLink;

    QTabWidget * mTabs;
    ContactOverview * mQuickTab;
    ContactBrowser * mDetailsTab;
#if defined(QTOPIA_TELEPHONY)
    ContactCallHistoryList *mCallHistoryTab;
#endif
    ContactMessageHistoryList *mMessageHistoryTab;
    QContactModel *mModel;
};


// -------------------------------------------------------------
// ContactHistoryDelegate
// -------------------------------------------------------------
class ContactHistoryDelegate : public QPimDelegate
{
    Q_OBJECT

public:
    explicit ContactHistoryDelegate( QObject * parent = 0 );
    virtual ~ContactHistoryDelegate();

    enum ContactHistoryRole {
        SubLabelRole = Qt::UserRole,
        SecondaryDecorationRole = Qt::UserRole+1,
        UserRole = Qt::UserRole+2
    };

    void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSize) const;

    QList<StringPair> subTexts(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    int subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    QSize mPrimarySize;
    QSize mSecondarySize;
};

#endif // CONTACTDETAILS_H

