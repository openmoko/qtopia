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
#include "qtaskmodel.h"
#include "qtasksqlio_p.h"
#include <QPainter>
#include <QIcon>
#include <QDebug>

#include "qrecordiomerge_p.h"

QMap<QTaskModel::Field, QString> QTaskModel::k2i;
QMap<QString, QTaskModel::Field> QTaskModel::i2k;
QMap<QTaskModel::Field, QString>  QTaskModel::k2t;

/*!
  \internal

  Initializes mappings from column enums to translated and non-translated strings.
*/
void QTaskModel::initMaps()
{
    if (k2t.count() > 0)
        return;
    struct KeyLookup {
        const char* ident;
        const char* trans;
        Field key;
    };
    static const KeyLookup l[] = {
        { "completed", QT_TR_NOOP( "Completed" ), Completed },
        { "priority", QT_TR_NOOP( "Priority" ), Priority },
        { "description", QT_TR_NOOP("Description"), Description },
        { "percentcompleted", QT_TR_NOOP( "Percent Completed" ), PercentCompleted },
        { "status", QT_TR_NOOP( "Status" ), Status },
        { "duedate", QT_TR_NOOP( "Due Date" ), DueDate },
        { "starteddate", QT_TR_NOOP( "Started Date" ), StartedDate },
        { "completeddate", QT_TR_NOOP( "Completed Date" ), CompletedDate },
        { "notes", QT_TR_NOOP( "Notes" ), Notes },

        { "identifier", QT_TR_NOOP( "Identifier" ), Identifier},
        { "categories", QT_TR_NOOP( "Categories" ), Categories},
        { 0, 0, Invalid }
    };

    const KeyLookup *k = l;
    while (k->key != Invalid) {
        k2t.insert(k->key, tr(k->trans));
        k2i.insert(k->key, k->ident);
        i2k.insert(k->ident, k->key);
        ++k;
    }
}

/*!
  Returns a translated string describing the task model field \a k.

  \sa fieldIcon(), fieldIdentifier(), identifierField()
*/
QString QTaskModel::fieldLabel(Field k)
{
    if (k2t.count() == 0)
        initMaps();
    if (!k2t.contains(k))
        return QString();
    return k2t[k];
}

/*!
  Returns a icon representing the task model field \a k.

  Returns a null icon if no icon is available.

  \sa fieldLabel(), fieldIdentifier(), identifierField()
*/
QIcon QTaskModel::fieldIcon(Field k)
{
    QString ident = fieldIdentifier(k);

    if (ident.isEmpty() || !QFile::exists(":icon/todolist/" + ident))
        return QIcon();

    return QIcon(":icon/todolist/" + ident);
}


/*!
  Returns a non-translated string describing the task model field \a k.

  \sa fieldLabel(), fieldIcon(), identifierField()
*/
QString QTaskModel::fieldIdentifier(Field k)
{
    if (k2i.count() == 0)
        initMaps();
    if (!k2i.contains(k))
        return QString();
    return k2i[k];
}

/*!
  Returns the task model field for the non-translated string identifier \a i.

  \sa fieldLabel(), fieldIcon(), fieldIdentifier()
*/
QTaskModel::Field QTaskModel::identifierField(const QString &i)
{
    if (i2k.count() == 0)
        initMaps();
    if (!i2k.contains(i))
        return Invalid;
    return i2k[i];
}


class QTaskModelData
{
public:
    QTaskModelData() : mio(0), searchModel(0), filterFlags(0) {}
    ~QTaskModelData() {
        if (searchModel)
            delete searchModel;
    }

    QBiasedRecordIOMerge *mio;

    QTaskContext *defaultContext;
    QTaskIO *defaultmodel;
    QList<QTaskContext *> contexts;
    QList<QTaskIO*> models;

    mutable QTaskModel *searchModel;
    mutable QString filterText;
    mutable int filterFlags;
};

QTaskModel::QTaskModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    d = new QTaskModelData();
    d->mio = new QBiasedRecordIOMerge(this);

    d->defaultmodel = new QTaskSqlIO(this);
    QTaskDefaultContext *dcon = new QTaskDefaultContext(this, d->defaultmodel);
    d->defaultContext = dcon;

    d->models.append(d->defaultmodel);
    d->contexts.append(dcon);
    d->mio->setPrimaryModel(d->defaultmodel);

    connect(d->mio, SIGNAL(reset()), this, SLOT(voidCache()));
}

QTaskModel::~QTaskModel()
{
}

void QTaskModel::voidCache()
{
    reset();
}

const QList<QTaskContext*> &QTaskModel::contexts() const
{
    return d->contexts;
}

QSet<QPimSource> QTaskModel::visibleSources() const
{
    QSet<QPimSource> set;
    foreach(QTaskContext *c, d->contexts)
        set.unite(c->visibleSources());
    return set;
}

void QTaskModel::setVisibleSources(const QSet<QPimSource> &list)
{
    foreach (QTaskContext *c, d->contexts) {
        QSet<QPimSource> cset = c->sources();
        cset.intersect(list);
        c->setVisibleSources(cset);
    }
    refresh();
}

QSet<QPimSource> QTaskModel::availableSources() const
{
    QSet<QPimSource> set;
    foreach(QTaskContext *c, d->contexts)
        set.unite(c->sources());
    return set;
}

QPimSource QTaskModel::source(const QUniqueId &id) const
{
    foreach(QTaskContext *context, d->contexts) {
        if (context->exists(id))
            return context->source(id);
    }
    return QPimSource();
}

QTaskContext *QTaskModel::context(const QUniqueId &id) const
{
    foreach(QTaskContext *context, d->contexts) {
        if (context->exists(id))
            return context;
    }
    return 0;
}

int QTaskModel::count() const
{
    return d->mio->count();
}

int QTaskModel::rowCount(const QModelIndex &) const
{
    return count();
}

int QTaskModel::columnCount(const QModelIndex &) const
{
    return Categories+1;// last column + 1
}

bool QTaskModel::contains(const QModelIndex &index) const
{
    return (index.row() >= 0 && index.row() < count());
}

bool QTaskModel::contains(const QUniqueId & id) const
{
    return index(id).isValid();
}

bool QTaskModel::exists(const QUniqueId &id) const
{
    foreach(const QTaskIO *model, d->models) {
        if (model->exists(id))
            return true;
    }
    return false;
}

QModelIndex QTaskModel::index(const QUniqueId & id) const
{
    int i = d->mio->index(id);
    if (i == -1)
        return QModelIndex();
    return createIndex(i, 0);
}

QModelIndex QTaskModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

bool QTaskModel::hasChildren(const QModelIndex &) const
{
    return false;
}

/*!
  Ensures the data in Tasks is in a state suitable for syncing.
*/
bool QTaskModel::flush() { return true; }

/*!
  Forces a refresh of the Task data.
*/
bool QTaskModel::refresh() { reset(); return true; }

QUniqueId QTaskModel::id(const QModelIndex &index) const
{
    int i = index.row();
    return id(i);
}

/*!
  Returns the id for task at the \a row specified.
*/
QUniqueId QTaskModel::id(int row) const
{
    const QTaskIO *model = qobject_cast<const QTaskIO *>(d->mio->model(row));
    int r = d->mio->row(row);
    if (model)
        return model->id(r);
    return QUniqueId();
}

/*!
  \overload
  Returns the index of the item in the model specified by the given \a row, \a column
  and \a parent index.
*/
QModelIndex QTaskModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (row < 0 || row >= count() || column < 0 || column >= columnCount())
        return QModelIndex();
    return createIndex(row,column);
}

QTask QTaskModel::task(const QModelIndex &index) const
{
    return task(index.row());
}

QTask QTaskModel::task(const QUniqueId & id) const
{
    foreach(const QTaskIO *model, d->models) {
        if (model->exists(id))
            return model->task(id);
    }
    return QTask();
}

/*!
  Return the task for the \a row specified.
*/
QTask QTaskModel::task(int row) const
{
    const QTaskIO *model = qobject_cast<const QTaskIO*>(d->mio->model(row));
    int r = d->mio->row(row);
    if (model)
        return model->task(r);
    return QTask();
}

bool QTaskModel::updateTask(const QTask &task)
{
    foreach(QTaskContext *context, d->contexts) {
        if (context->exists(task.uid())) {
            return context->updateTask(task);
        }
    }
    return false;
}

bool QTaskModel::removeTask(const QTask &task)
{
    return removeTask(task.uid());
}

/*!
  Removes the task that has the uid \a id from the TaskModel;

  Returns true if the task was successfully removed.  Otherwise return false.
*/
bool QTaskModel::removeTask(const QUniqueId& id)
{
    foreach(QTaskContext *context, d->contexts) {
        if (context->exists(id)) {
            return context->removeTask(id);
        }
    }
    return false;
}

bool QTaskModel::removeList(const QList<QUniqueId> &list)
{
    QUniqueId id;
    foreach(id, list) {
        if (!exists(id))
            return false;
    }
    foreach(id, list) {
        removeTask(id);
    }
    return true;
}

QUniqueId QTaskModel::addTask(const QTask& task, const QPimSource &source)
{
    if (source.isNull()) {
        return d->defaultContext->addTask(task, source);
    } else {
        foreach(QTaskContext *context, d->contexts) {
            if (context->sources().contains(source))
                return context->addTask(task, source);
        }
    }
    return QUniqueId();
}

void QTaskModel::setSortField(Field field)
{
    if (field == sortField())
        return;

    foreach(QTaskIO *model, d->models)
        model->setSortKey(field);
    d->mio->rebuildCache();
}

QTaskModel::Field QTaskModel::sortField() const
{
    return d->defaultmodel->sortKey();
}

void QTaskModel::setCategoryFilter(const QCategoryFilter &f)
{
    if (f == categoryFilter())
        return;

    foreach(QTaskIO *model, d->models)
        model->setCategoryFilter(f);
    d->mio->rebuildCache();
}

QCategoryFilter QTaskModel::categoryFilter() const
{
    return d->defaultmodel->categoryFilter();
}

void QTaskModel::setFilterCompleted(bool b)
{
    if (b == filterCompleted())
        return;

    foreach(QTaskIO *model, d->models)
        model->setCompletedFilter(b);
    d->mio->rebuildCache();
}

bool QTaskModel::filterCompleted() const
{
    return d->defaultmodel->completedFilter();
}

QVariant QTaskModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            default:
            case Invalid:
                return QVariant();
            case Description:
                return task(index).description();
            case Priority:
                return task(index).priority();
            case Completed:
                return task(index).isCompleted();
            case PercentCompleted:
                return task(index).percentCompleted();
            case Status:
                return task(index).status();
            case DueDate:
                return task(index).dueDate();
            case StartedDate:
                return task(index).startedDate();
            case CompletedDate:
                return task(index).completedDate();
            case Notes:
                return task(index).notes();
        }
    }
    return QVariant();
}

bool QTaskModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QTask t = task(index);
    if (role == Qt::EditRole) {
        switch (index.column()) {
            default:
            case Invalid:
                return false;
            case Description:
                t.setDescription(value.toString());
                return updateTask(t);
            case Priority:
                t.setPriority(value.toInt());
                return updateTask(t);
            case Completed:
                t.setCompleted(value.toBool());
                return updateTask(t);
            case PercentCompleted:
                t.setPercentCompleted(value.toInt());
                return updateTask(t);
            case Status:
                t.setStatus(value.toInt());
                return updateTask(t);
            case DueDate:
                t.setDueDate(value.toDate());
                return updateTask(t);
            case StartedDate:
                t.setStartedDate(value.toDate());
                return updateTask(t);
            case CompletedDate:
                t.setCompletedDate(value.toDate());
                return updateTask(t);
            case Notes:
                t.setNotes(value.toString());
                return updateTask(t);
        }
    }
    return false;
}

QMap<int, QVariant> QTaskModel::itemData ( const QModelIndex &index ) const
{
    QMap<int, QVariant> m;
    m.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
    return m;
}

bool QTaskModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &roles)
{
    if (roles.count() != 1 || !roles.contains(Qt::EditRole))
        return false;
    return setData(index, roles[Qt::EditRole], Qt::EditRole);
}

QVariant QTaskModel::headerData(int section, Qt::Orientation o, int role) const
{
    if (o == Qt::Horizontal) {
        if (role == Qt::DisplayRole)
            return fieldLabel((Field)section);
        else if (role == Qt::DecorationRole)
            return qvariant_cast<QIcon>(fieldIcon((Field)section));
        else if (role == Qt::EditRole)
            return fieldIdentifier((Field)section);
    }
    return QVariant();
}

Qt::ItemFlags QTaskModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QModelIndexList QTaskModel::match(const QModelIndex &start, int, const QVariant &value,
            int /*count*/, Qt::MatchFlags) const
{
    Q_UNUSED(start)
    Q_UNUSED(value)

    return QModelIndexList();
}

/*!
  \overload

  Returns an object that contains a serialized description of the specified \a indexes.
  The format used to describe the items corresponding to the \a indexes is obtained from
  the mimeTypes() function.

  If the list of indexes is empty, 0 is returned rather than a serialized empty list.

  Currently returns 0 but may be implemented at a future date.
*/
QMimeData * QTaskModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)

    return 0;
}

/*!
  \overload

  Returns a list of MIME types that can be used to describe a list of model indexes.

  Currently returns an empty list but may be implemented at a future date.
*/
QStringList QTaskModel::mimeTypes() const
{
    return QStringList();
}

/*!
  \overload

  Sorts the model by \a column in ascending order

  Currently \a order is ignored but may be implemented at a future date.
*/
void QTaskModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(order)

    if (column >= 0 && column < columnCount())
        setSortField((Field)column);
}
