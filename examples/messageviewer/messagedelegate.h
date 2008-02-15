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

#ifndef MESSAGEDELEGATE_H
#define MESSAGEDELEGATE_H

#include <QAbstractItemDelegate>
#include <QSize>

class QModelIndex;
class QPainter;
class QStyleOptionViewItem;

class MessageDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    explicit MessageDelegate(QObject* parent = 0);
    virtual ~MessageDelegate();

    enum Role
    {
        SubLabelRole = Qt::UserRole,
        SecondaryDecorationRole
    };

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // MESSAGEDELEGATE_H
