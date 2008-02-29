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

#ifndef TASKDELEGATE_H
#define TASKDELEGATE_H

#include <QAbstractItemDelegate>
#include <qtopia/pim/qtaskmodel.h>

class TaskTableDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    TaskTableDelegate(QObject * = 0);
    ~TaskTableDelegate();

    static QRect checkBoxGeom(const QRect &cell);

   // drawing
    void paint(QPainter *p, const QStyleOptionViewItem &o, const QModelIndex &i) const
    {
        const QTaskModel *tm = qobject_cast<const QTaskModel *>(i.model());
        if (tm)
            paint(p, o, tm, i);
    }

    void paint(QPainter *, const QStyleOptionViewItem &, const QTaskModel *c, const QModelIndex &) const;

    // editing
    QWidget *createEditor(QWidget *parent,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const;

    // non widget editors (e.g. check boxes.
    bool editorEvent(QEvent *event,
                             QAbstractItemModel *model,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index);

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;
    QSize sizeHint(const QStyleOptionViewItem &o, int column) const;

    void updateEditorGeometry(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;

private slots:
    void setPriority(int);
    void cancelEdit();

signals:
    void showItem(const QModelIndex &);

private:
};
#endif
