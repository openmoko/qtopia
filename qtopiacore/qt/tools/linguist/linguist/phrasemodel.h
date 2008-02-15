/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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
