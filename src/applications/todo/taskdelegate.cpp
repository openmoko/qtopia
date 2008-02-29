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

#include "taskdelegate.h"

#include <qtimestring.h>
#include <quniqueid.h>

#include <QListWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QPainter>
#include <QBoxLayout>
#include <QtopiaApplication>

#include "qsoftmenubar.h"
#include "qtaskmodel.h"
#include "qtask.h"


/// Delegate
TaskDelegate::TaskDelegate(QObject *parent) : QPimDelegate(parent)
{

}

TaskDelegate::~TaskDelegate()
{

}

QString TaskDelegate::formatDate(const QDate& date) const
{
    QDate today = QDate::currentDate();
    if (today == date)
        return tr("Today");
    else if (today.year() == date.year())
        return QTimeString::localMD(date);
    else
        return QTimeString::localYMD(date);
}

QList<StringPair> TaskDelegate::subTexts(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option);

    QList< StringPair > subTexts;
    int progress = index.model()->data(index.sibling(index.row(), QTaskModel::PercentCompleted),Qt::DisplayRole).toInt();
    QTask::Status s = (QTask::Status)index.model()->data(index.sibling(index.row(), QTaskModel::Status),Qt::DisplayRole).toInt();

    if (s == QTask::Completed || progress == 100) {
        QDate finishedDate = index.model()->data(index.sibling(index.row(), QTaskModel::CompletedDate),Qt::DisplayRole).toDate();
        // finishedDate should be valid since it is 100% complete..
        subTexts.append(qMakePair(tr("Completed: "), formatDate(finishedDate)));

    } else {
        QDate dueDate = index.model()->data(index.sibling(index.row(), QTaskModel::DueDate),Qt::DisplayRole).toDate();
        if (dueDate.isValid()) {
            if (progress > 0)
                subTexts.append(qMakePair(tr("Due: "), tr("%1 (%2\%)", "Sep 5, 2007 (30%)")
                        .arg(formatDate( dueDate )).arg(progress)));
            else
                subTexts.append(qMakePair(tr("Due: "), formatDate( dueDate )));
        }
        else if (progress > 0 && progress < 100)
            subTexts.append(qMakePair(tr("Progress: "), tr("%1\%", "50%").arg(progress)));
        else if (s != QTask::NotStarted) {
            QDate startDate = index.model()->data(index.sibling(index.row(), QTaskModel::StartedDate),Qt::DisplayRole).toDate();
            if (startDate.isValid())
                subTexts.append(qMakePair(tr("Started: "), formatDate(startDate)));
            // for perc == 0 && s != NotStarted and startDate to be invalid, it means s == Waiting or Deferred
        }
    }
    if (s == QTask::NotStarted)
        subTexts.append(qMakePair(tr("Status: "), tr("Not started")));
    if (s == QTask::Waiting)
        subTexts.append(qMakePair(tr("Status: "), tr("Waiting")));
    if (s == QTask::Deferred)
        subTexts.append(qMakePair(tr("Status: "), tr("Deferred")));

    return subTexts;
}

void TaskDelegate::drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index,
                                  QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const
{
    Q_UNUSED(option);
    Q_UNUSED(trailingFloats);

    int decorationSize = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize) / 2;

    QIcon i = qvariant_cast<QIcon>(index.model()->data(index, Qt::DecorationRole));

    QRect drawRect = option.rect;
    // 8px padding, 4 on either side
    if (rtl) {
        drawRect.setLeft(drawRect.right() - decorationSize - 8);
    } else {
        drawRect.setRight(decorationSize + 8);
    }
    QPoint drawOffset = QPoint(drawRect.left() + ((drawRect.width() - decorationSize)/2), drawRect.top() + ((drawRect.height() - decorationSize) / 2));

    p->drawPixmap(drawOffset, i.pixmap(decorationSize));

    leadingFloats.append(drawRect);
}

QSize TaskDelegate::decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& s) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    int decorationSize = qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize) / 2;
    return QSize(decorationSize + s.width(), qMax(decorationSize + 2, s.height()));
}
