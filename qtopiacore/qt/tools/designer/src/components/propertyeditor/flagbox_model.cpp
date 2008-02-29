/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "flagbox_model_p.h"
#include <QtCore/qdebug.h>

using namespace qdesigner_internal;

FlagBoxModel::FlagBoxModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

FlagBoxModel::~FlagBoxModel()
{
}

void FlagBoxModel::setItems(const QList<FlagBoxModelItem> &items)
{
    m_items = items;
    emit reset();
}

int FlagBoxModel::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? m_items.count() : 0;
}

int FlagBoxModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex FlagBoxModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

QModelIndex FlagBoxModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column, 0);
}

QVariant FlagBoxModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.row() != -1);

    const FlagBoxModelItem &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return item.name();

    case Qt::CheckStateRole:
        return item.isChecked() ? Qt::Checked : Qt::Unchecked;

    default:
        return QVariant();
    } // end switch
}

bool FlagBoxModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_ASSERT(index.row() != -1);

    FlagBoxModelItem &item = m_items[index.row()];

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole: {
        item.setName(value.toString());
    } return true;

    case Qt::CheckStateRole: {
        Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
        item.setChecked(state == Qt::Unchecked ? false : true);
        emit dataChanged(index, index);
    } return true;

    default: break;
    } // end switch

    return false;
}

Qt::ItemFlags FlagBoxModel::flags(const QModelIndex &index) const
{
    Q_ASSERT(index.row() != -1);

    const FlagBoxModelItem &thisItem = m_items[index.row()];
    if (thisItem.value() == 0) {
        // Disabled if checked
        if (thisItem.isChecked())
            return 0;
    } else if (bitcount(thisItem.value()) > 1) {
        // Disabled if all flags contained in the mask are checked
        unsigned int currentMask = 0;
        for (int i = 0; i < m_items.size(); ++i) {
            const FlagBoxModelItem &item = m_items[i];
            if (bitcount(item.value()) == 1)
                currentMask |= item.isChecked() ? item.value() : 0;
        }
        if ((currentMask & thisItem.value()) == thisItem.value())
            return 0;
    }
    return QAbstractItemModel::flags(index);
}

// Helper function that counts the number of 1 bits in argument
int FlagBoxModel::bitcount(int mask)
{
    int count = 0;
    for (int i = 31; i >= 0; --i)
        count += ((mask >> i) & 1) ? 1 : 0;
    return count;
}
