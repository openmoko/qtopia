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

#ifndef TASKDELEGATE_H
#define TASKDELEGATE_H

#include "qpimdelegate.h"

class TaskDelegate : public QPimDelegate
{
    Q_OBJECT

public:
    TaskDelegate(QObject * = 0);
    ~TaskDelegate();

    void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSize) const;

    QList<StringPair> subTexts(const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    QString formatDate(const QDate& date) const;
};

#endif
