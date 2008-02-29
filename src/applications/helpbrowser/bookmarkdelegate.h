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

#ifndef BOOKMARKDELEGATE_H
#define BOOKMARKDELEGATE_H

#include <QItemDelegate>
#include <QIcon>

class QPainter;
class QStyleOptionViewItem;
class QModelIndex;


class BookmarkDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    BookmarkDelegate(QObject *parent = 0);

    virtual ~BookmarkDelegate() {}

    void paint (QPainter *painter,const QStyleOptionViewItem &option, const QModelIndex &index) const;

public slots:

    void setDisplayMode(bool displayMode);

private:

    QIcon icon;

    bool displayMode;
};

#endif
