/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPROXYMODEL_P_H
#define QPROXYMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QAbstractItemModel*.  This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//
//

#include "QtCore/qabstractitemmodel.h"
#include "private/qabstractitemmodel_p.h"

#ifndef QT_NO_PROXYMODEL

class QEmptyModel : public QAbstractItemModel
{
public:
    explicit QEmptyModel(QObject *parent = 0) : QAbstractItemModel(parent) {}
    QModelIndex index(int, int, const QModelIndex &) const { return QModelIndex(); }
    QModelIndex parent(const QModelIndex &) const { return QModelIndex(); }
    int rowCount(const QModelIndex &) const { return 0; }
    int columnCount(const QModelIndex &) const { return 0; }
    bool hasChildren(const QModelIndex &) const { return false; }
    QVariant data(const QModelIndex &, int) const { return QVariant(); }
};

class QProxyModelPrivate : private QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QProxyModel)

public:
    void _q_sourceDataChanged(const QModelIndex &tl,const QModelIndex &br);
    void _q_sourceRowsAboutToBeInserted(const QModelIndex &parent, int first ,int last);
    void _q_sourceRowsInserted(const QModelIndex &parent, int first ,int last);
    void _q_sourceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void _q_sourceRowsRemoved(const QModelIndex &parent, int first, int last);
    void _q_sourceColumnsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void _q_sourceColumnsInserted(const QModelIndex &parent, int first, int last);
    void _q_sourceColumnsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void _q_sourceColumnsRemoved(const QModelIndex &parent, int first, int last);

    QProxyModelPrivate() : QAbstractItemModelPrivate(), model(0) {}
    QAbstractItemModel *model;
    QEmptyModel empty;
};

#endif // QT_NO_PROXYMODEL

#endif // QPROXYMODEL_P_H
