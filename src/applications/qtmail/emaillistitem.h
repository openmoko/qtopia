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



#ifndef EMAILLISTITEM_H
#define EMAILLISTITEM_H

#include <qtablewidget.h>
#include <qitemdelegate.h>
#include <qtopia/pim/qcontactmodel.h>

#include "email.h"

class MailListView;
class QFont;
class QPixmap;
class QObject;
class QPainter;

class EmailListItem: public QTableWidgetItem
{
public:
    EmailListItem(MailListView *parent, Email *mailIn, int col);
    virtual ~EmailListItem();

    Email* mail();
    void setMail(Email *newMail);
    QUuid id();
    QPixmap *pixmap() { return typePm; }
    virtual bool operator<(const QTableWidgetItem &other) const;

    QString key(int c) const;

    bool stateUpdated() { return columnsSet; }
    void updateState();
    static void deletePixmaps();
    static QString dateToString( QDateTime dateTime );

protected:
    void setColumns();

private:
    Email *_mail;
    MailListView *parentView;
    bool columnsSet;
    QPixmap *typePm;
    bool alt;
    int mCol;
};

class EmailListItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    EmailListItemDelegate(MailListView *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

private:
    MailListView *mParent;
};

#endif
