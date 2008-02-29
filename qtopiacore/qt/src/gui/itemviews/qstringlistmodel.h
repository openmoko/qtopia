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

#ifndef QSTRINGLISTMODEL_H
#define QSTRINGLISTMODEL_H

#include <QtCore/qstringlist.h>
#include <QtGui/qabstractitemview.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_STRINGLISTMODEL

class Q_GUI_EXPORT QStringListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QStringListModel(QObject *parent = 0);
    QStringListModel(const QStringList &strings, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    Qt::ItemFlags flags(const QModelIndex &index) const;
    
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QStringList stringList() const;
    void setStringList(const QStringList &strings);

private:
    Q_DISABLE_COPY(QStringListModel)
    QStringList lst;
};

#endif // QT_NO_STRINGLISTMODEL

QT_END_HEADER

#endif // QSTRINGLISTMODEL_H
