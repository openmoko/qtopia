/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef QAPPOINTMENTVIEW_H
#define QAPPOINTMENTVIEW_H

#include <QStyleOptionViewItem>
#include <QAbstractItemDelegate>

class QFont;
class QTOPIAPIM_EXPORT QAppointmentDelegate : public QAbstractItemDelegate
{
public:
    QAppointmentDelegate( QObject * parent = 0 );
    virtual ~QAppointmentDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem & option,
            const QModelIndex & index ) const;

    virtual QSize sizeHint(const QStyleOptionViewItem & option,
            const QModelIndex &index) const;

    virtual QFont mainFont(const QStyleOptionViewItem &) const;
    virtual QFont secondaryFont(const QStyleOptionViewItem &) const;
private:
    QFont differentFont(const QFont& start, int step) const;

    int iconSize;
};

#endif // QAPPOINTMENTVIEW_H
