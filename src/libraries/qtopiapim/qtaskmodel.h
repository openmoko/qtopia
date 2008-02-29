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

#ifndef QTOPIA_TASKMODEL_H
#define QTOPIA_TASKMODEL_H

#include <QStyleOptionViewItem>
#include <QAbstractListModel>
#include <QAbstractItemDelegate>
#include <QSharedDataPointer>

#include <qcategorymanager.h>
#include <qtopia/pim/qtask.h>
#include <qtopia/pim/qpimsource.h>

class QTaskModelData;
class QTOPIAPIM_EXPORT QTaskModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    enum Field {
        Invalid = -1,

        Description,
        Priority,
        Completed,
        PercentCompleted,
        Status,
        DueDate,
        StartedDate,
        CompletedDate,
        Notes,

        Identifier,
        Categories,
    };

    explicit QTaskModel(QObject *parent = 0);
    virtual ~QTaskModel();

    static QString fieldLabel(Field key);
    static QIcon fieldIcon(Field key);

    static QString fieldIdentifier(Field);
    static Field identifierField(const QString &);

    const QList<QTaskContext*> &contexts() const;
    QSet<QPimSource> visibleSources() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> availableSources() const;

    QPimSource source(const QUniqueId &) const;
    QTaskContext *context(const QUniqueId &) const;

    bool sourceExists(const QPimSource &source, const QUniqueId &id) const;
    int rowCount(const QModelIndex & = QModelIndex()) const;
    int columnCount(const QModelIndex & = QModelIndex()) const;

    // overridden so can change later and provide drag-n-drop (via vcard)
    // later without breaking API compatibility.
    QMimeData * mimeData(const QModelIndexList &) const;
    QStringList mimeTypes() const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QVariant data(const QModelIndex &, int) const;
    bool setData(const QModelIndex &, const QVariant &, int);
    QMap<int, QVariant> itemData ( const QModelIndex & index ) const;
    bool setItemData(const QModelIndex &, const QMap<int,QVariant> &);
    QVariant headerData(int, Qt::Orientation orientation, int = Qt::DisplayRole ) const;

    int count() const;
    bool contains(const QModelIndex &) const;
    bool contains(const QUniqueId &) const;
    bool exists(const QUniqueId &) const;

    QModelIndex index(const QUniqueId &) const;
    QUniqueId id(const QModelIndex &) const;
    QUniqueId id(int) const;
    QModelIndex index(int r,int c = 0,const QModelIndex & = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &) const;
    bool hasChildren(const QModelIndex & = QModelIndex()) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QTask task(const QModelIndex &) const;
    QTask task(const QUniqueId &) const;
    QTask task(int) const;

    static QVariant taskField(const QTask &c, QTaskModel::Field k);
    static bool setTaskField(QTask &c, QTaskModel::Field k,  const QVariant &);

    bool updateTask(const QTask&);
    bool removeTask(const QTask &);
    bool removeTask(const QUniqueId &);
    QUniqueId addTask(const QTask& task, const QPimSource &source = QPimSource());

    bool removeList(const QList<QUniqueId> &);


    void setCategoryFilter(const QCategoryFilter &categories);
    QCategoryFilter categoryFilter() const;

    // convienience.
    // does not affect other filter roles.
    void setFilterCompleted(bool);
    bool filterCompleted() const;

    QModelIndexList match(const QModelIndex &start, int role, const QVariant &,
            int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;

    bool flush();
    bool refresh();

private slots:
    void voidCache();

private:
    void setSortField(Field);
    Field sortField() const;

    static void initMaps();
    static QMap<Field, QString> k2t;
    static QMap<Field, QString> k2i;
    static QMap<QString, Field> i2k;

    QTaskModelData *d;
};

#endif // QTOPIA_TASKMODEL_H
