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

#include "langmodel.h"

#include <QIcon>
#include <QPainter>

QVariant LanguageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    static QVariant pixmapVariant(QPixmap(":image/selectedDict"));

    int pos = index.row();
    if (role == Qt::DisplayRole) {
        //we need to add LRM/RLM at the end of the string because
        //some names end with an ')'. In an RTL env this would move the ')'
        //to the beginning of the line
        if ( list[pos].direction == Qt::LeftToRight )
            return list[pos].langName+QChar(0x200E); //add LRM
        else
            return list[pos].langName+QChar(0x200F); //add RLM
    } else if (role == Qt::DecorationRole) {
        if (list[pos].hasDict)
            return pixmapVariant;
        return QPixmap();
    } else if (role == Qt::FontRole) {
        return QVariant(list[pos].fnt);
    } else if (role == Qt::UserRole) {
        return list[pos].hasDict;
    } else if (role == Qt::UserRole+1) {
        return list[pos].direction;
    }

    return QVariant();
}

int LanguageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return list.count();
}


bool LanguageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::UserRole)
    {
        FontedItem replacement = list[index.row()];
        replacement.hasDict = value.toBool();
        list.replace(index.row(), replacement);
        emit dataChanged(QAbstractListModel::index(0), QAbstractListModel::index(list.count()-1));
        return true;
    }
    return false;
}
