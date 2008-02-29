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

#ifndef PHRASEMODEL_H
#define PHRASEMODEL_H

#include "phrase.h"

#include <QList>
#include <QAbstractItemModel>

class PhraseModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    PhraseModel(QObject *parent = 0)
        : QAbstractTableModel(parent), sortColumn(-1) {}

    void removePhrases();
    QList<Phrase> phraseList() const {return plist;}

    QModelIndex addPhrase(Phrase p);
    void removePhrase(const QModelIndex &index);

    Phrase phrase(const QModelIndex &index) const;
    void setPhrase(const QModelIndex &indx, Phrase ph);
    QModelIndex index(const Phrase phr) const;

    static bool compare(const Phrase &left, const Phrase &right);
    bool sortParameters(Qt::SortOrder &so, int &sc) const;
    void resort();

    // from qabstracttablemodel
    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

private:
    Qt::SortOrder sortOrder;
    int sortColumn;

    QList<Phrase> plist;
};

#endif // PHRASEMODEL_H
