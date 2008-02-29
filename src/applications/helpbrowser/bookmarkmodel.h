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

#ifndef BOOKMARKMODEL_H
#define BOOKMARKMODEL_H

#include <QAbstractListModel>
#include <QVector>

class QModelIndex;
class QVariant;
class Bookmark;

class BookmarkModel : public QAbstractListModel
{
public:
    BookmarkModel(QObject *parent = 0);

    virtual ~BookmarkModel() {}

    // Required
    int rowCount(const QModelIndex &parent = QModelIndex() ) const;
    // Required
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    bool contains(const Bookmark &);

    bool addItem(const Bookmark &,bool refresh = false);

    void insert(int row,const Bookmark &,bool refresh = false);

    bool remove(int row,bool refresh = false);

    Bookmark getItem(int row) const;

    QModelIndex createIdx(int row);

    void load();

    void write();

private:

    // Number of spaces of indentation for the Bookmarks data file.
    static const int DATAFILE_INDENT = 4;

    QVector<QVariant> dat;
};

#endif
