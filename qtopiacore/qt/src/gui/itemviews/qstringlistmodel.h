/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
