/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include <QSharedDataPointer>
#include <QPimModel>

#include <qcategorymanager.h>
#include <qtopia/pim/qtask.h>
#include <qtopia/pim/qpimsource.h>

class QTaskModelData;
class QTOPIAPIM_EXPORT QTaskModel : public QPimModel
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

        Alarm,

        RepeatRule,
        RepeatEndDate,
        RepeatFrequency,
        RepeatWeekFlags
    };

    explicit QTaskModel(QObject *parent = 0);
    virtual ~QTaskModel();

    static QString fieldLabel(Field key);
    static QIcon fieldIcon(Field key);

    static QString fieldIdentifier(Field);
    static Field identifierField(const QString &);

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

    QUniqueId addRecord(const QByteArray &, const QPimSource &, const QString &format = QString());
    bool updateRecord(const QUniqueId &id, const QByteArray &, const QString &format = QString());
    QByteArray record(const QUniqueId &id, const QString &format = QString()) const;

    bool removeRecord(const QUniqueId &id) { return removeTask(id); }

    // convienience.
    // does not affect other filter roles.
    void setFilterCompleted(bool);
    bool filterCompleted() const;

    bool updateRecurringTasks();

    QModelIndexList match(const QModelIndex &start, int role, const QVariant &,
            int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;

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
