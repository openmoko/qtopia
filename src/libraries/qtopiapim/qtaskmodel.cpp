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
#include "qtaskmodel.h"
#include "qtasksqlio_p.h"
#include <QPainter>
#include <QIcon>
#include <QDebug>

#include "qrecordiomerge_p.h"

QMap<QTaskModel::Field, QString> QTaskModel::k2i;
QMap<QString, QTaskModel::Field> QTaskModel::i2k;
QMap<QTaskModel::Field, QString>  QTaskModel::k2t;

class QTaskModelData
{
public:
    QTaskModelData() {}
    ~QTaskModelData() {}

    static QIcon getCachedIcon(const QString& path);
    static QHash<QString, QIcon> cachedIcons;
};

QHash<QString, QIcon> QTaskModelData::cachedIcons;

QIcon QTaskModelData::getCachedIcon(const QString& path)
{
    if (cachedIcons.contains(path))
        return cachedIcons.value(path);

    cachedIcons.insert(path, QIcon(path));
    return cachedIcons.value(path);
}


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

    return QTaskModelData::getCachedIcon(":icon/todolist/" + ident);
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


/*!
  \class QTaskModel
  \module qpepim
  \ingroup pim
  \brief The QTaskModel class provides access to the the Tasks data.

  The QTaskModel is used to access the Task data.  It is a descendant of QAbstractItemModel,
  so it is suitable for use with the Qt View classes such as QListView and QTableView, as well as
  any custom developer Views.

  QTaskModel provides functions for filtering of items.
  For filters or sorting that is not provided by QTaskModel it is recommended that
  QSortFilterProxyModel is used to wrap QTaskModel.

  QTaskModel may merge data from multiple sources.

  QTaskModel will also refresh when changes are made in other instances of QTaskModel or
  from other applications.
*/

/*!
  \enum QTaskModel::Field

  Enumerates the columns when in table mode and columns used for sorting.
  Is a subset of data retrievable from a QTask.

  \omitvalue Invalid
  \omitvalue Description
  \omitvalue Priority
  \omitvalue Completed
  \omitvalue PercentCompleted
  \omitvalue Status
  \omitvalue DueDate
  \omitvalue StartedDate
  \omitvalue CompletedDate
  \omitvalue Notes
  \omitvalue Identifier
  \omitvalue Categories
*/

/*!
  Contstructs a QTaskModel with parent \a parent.
*/
QTaskModel::QTaskModel(QObject *parent)
    : QPimModel(parent)
{
    d = new QTaskModelData;
    QtopiaSql::openDatabase();

    QTaskIO *access = new QTaskSqlIO(this);
    QTaskDefaultContext *context = new QTaskDefaultContext(this, access);

    addAccess(access);
    addContext(context);
}

/*!
  Destructs the QTaskModel
*/
QTaskModel::~QTaskModel()
{
}

/*!
  \overload

    Returns the number of columns for the given \a parent.
*/
int QTaskModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return Categories+1;// last column + 1
}

/*!
  Returns the task for the row specified by \a index.
  The column of \a index is ignored.
*/
QTask QTaskModel::task(const QModelIndex &index) const
{
    return task(index.row());
}

/*!
  Returns the task with the identifier \a id.  The task does
  not have to be in the current filter mode for it to be returned.
*/
QTask QTaskModel::task(const QUniqueId & id) const
{
    const QTaskIO *model = qobject_cast<const QTaskIO *>(access(id));
    if (model)
        return model->task(id);
    return QTask();
}

/*!
  Return the task for the \a row specified.
*/
QTask QTaskModel::task(int row) const
{
    const QTaskIO *model = qobject_cast<const QTaskIO*>(access(row));
    int r = accessRow(row);
    if (model)
        return model->task(r);
    return QTask();
}

/*!
  Updates the task \a task so long as a there is a task in the
  QTaskModel with the same uid as \a task.

  Returns true if the task was successfully updated.  Otherwise return false.
*/
bool QTaskModel::updateTask(const QTask &task)
{
    QTaskContext *c= qobject_cast<QTaskContext *>(context(task.uid()));
    if (c && c->updateTask(task)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Removes the task \a task so long as there is a task in the QTaskModel with
  the same uid as \a task.

  Returns true if the task was successfully removed.  Otherwise return false.
*/
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
    QTaskContext *c = qobject_cast<QTaskContext *>(context(id));
    if (c && c->removeTask(id)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Removes the tasks specified by the list of uids \a ids.

  Returns true if tasks are successfully removed.  Otherwise returns  false.
*/
bool QTaskModel::removeList(const QList<QUniqueId> &ids)
{
    QUniqueId id;
    foreach(id, ids) {
        if (!exists(id))
            return false;
    }
    foreach(id, ids) {
        removeTask(id);
    }
    return true;
}

/*!
  Adds the task \a task to the QTaskModel under the storage source \a source.
  If source is empty will add the task to the default storage source.

  Returns true if the task was successfully added.  Otherwise return false.
*/
QUniqueId QTaskModel::addTask(const QTask& task, const QPimSource &source)
{
    QTaskContext *c = qobject_cast<QTaskContext *>(context(source));

    QUniqueId id;
    if (c && !(id = c->addTask(task, source)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  \overload

  Adds the PIM record encoded in \a bytes to the QTaskModel under the storage source \a source.
  The format of the record in \a bytes is given by \a format.  An empty \a format string will
  cause the record to be read using the data stream operators for the PIM data type of the model.
  If \a source is empty will add the record to the default storage source.

  Returns valid id if the record was successfully added.  Otherwise returns an invalid id.

  Can only add PIM data that is represented by the model.  This means that only task data
  can be added using a QTaskModel.  Valid formats are "vCalendar" or an empty string.

*/
QUniqueId QTaskModel::addRecord(const QByteArray &bytes, const QPimSource &source, const QString &format)
{
    if (format == "vCalendar") {
        QList<QTask> list = QTask::readVCalendar(bytes);
        if (list.count() == 1)
            return addTask(list[0], source);
    } else {
        QTask t;
        QDataStream ds(bytes);
        ds >> t;
        return addTask(t, source);
    }
    return QUniqueId();
}

/*!
  \overload

  Updates the record enoded in \a bytes so long as there is a record in the QTaskModel with
  the same uid as the record.  The format of the record in \a bytes is given by \a format.
  An empty \a format string will cause the record to be read using the data stream operators
  for the PIM data type of the model. If \a id is not null will set the record uid to \a id
  before attempting to update the record.

  Returns true if the record was successfully updated.  Otherwise returns false.
*/
bool QTaskModel::updateRecord(const QUniqueId &id, const QByteArray &bytes, const QString &format)
{
    QTask t;
    if (format == "vCalendar") {
        QList<QTask> list = QTask::readVCalendar(bytes);
        if (list.count() == 1) {
            t = list[0];
        }
    } else {
        QDataStream ds(bytes);
        ds >> t;
    }
    if (!id.isNull())
        t.setUid(id);
    return updateTask(t);
}

/*!
  \fn bool QTaskModel::removeRecord(const QUniqueId &id)
  \overload

  Removes the record that has the uid \a id from the QTaskModel.

  Returns true if the record was successfully removed.  Otherwise returns false.
*/

/*!
  \overload

    Returns the record with the identifier \a id encoded in the format specified by \a format.
    An empty \a format string will cause the record to be written using the data stream
    operators for the PIM data type of the model.
*/
QByteArray QTaskModel::record(const QUniqueId &id, const QString &format) const
{
    QTask t = task(id);
    if (t.uid().isNull())
        return QByteArray();

    QByteArray bytes;
    QDataStream ds(&bytes, QIODevice::WriteOnly);
    if (format == "vCalendar") {
        t.writeVCalendar(&ds);
        return bytes;
    } else {
        ds << t;
        return bytes;
    }
    return QByteArray();
}

/*! \internal */
void QTaskModel::setSortField(Field field)
{
    if (field == sortField())
        return;

    foreach(QRecordIO *model, accessModels()) {
        QTaskIO *taskModel = qobject_cast<QTaskIO *>(model);
        taskModel->setSortKey(field);
    }
}

/*! \internal */
QTaskModel::Field QTaskModel::sortField() const
{
    QList<QRecordIO*> &list = accessModels();
    if (list.count()) {
        QTaskIO *taskModel = qobject_cast<QTaskIO *>(list[0]);
        if (taskModel)
            return taskModel->sortKey();
    }
    return Invalid;
}

/*!
    If \a b is true sets the model to only contain completed tasks. 
    Otherwise will contain both completed and uncompleted tasks.
*/
void QTaskModel::setFilterCompleted(bool b)
{
    if (b == filterCompleted())
        return;

    foreach(QRecordIO *model, accessModels()) {
        QTaskIO *taskModel = qobject_cast<QTaskIO *>(model);
        taskModel->setCompletedFilter(b);
    }
}

/*!
  Returns true if the model only contains completed tasks. Otherwise returns false.
*/
bool QTaskModel::filterCompleted() const
{
    QList<QRecordIO*> &list = accessModels();
    if (list.count()) {
        QTaskIO *taskModel = qobject_cast<QTaskIO *>(list[0]);
        if (taskModel)
            return taskModel->completedFilter();
    }
    return false;
}

/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.
*/
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
    } else if (role == Qt::DecorationRole) {
        QTask t = task(index);
        if (t.isCompleted())
            return d->getCachedIcon(":icon/ok");
        else switch (t.priority()) {
            case 1:
            case 2:
                return d->getCachedIcon(":icon/priority");

            default:
                break;
        }
    }
    return QVariant();
}

/*!
  \overload
  Sets the \a role data for the item at \a index to \a value. Returns true if successful,
  otherwise returns false.
*/
bool QTaskModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    if (!index.isValid())
        return false;

    QTask t = task(index);
    if (!setTaskField(t, (Field)index.column(), value))
        return false;
    return updateTask(t);

#if 0
    /* disabled due to 'notifyUpdated' require whole record.
       While writing whole record is less efficient than partial - at 
       this stage it was the easiest way of fixing the bug where setData
       did not result in cross-model data change from being propogated properly
   */

    int i = index.row();
    const QTaskIO *model = qobject_cast<const QTaskIO*>(d->mio->model(i));
    int r = d->mio->row(i);
    if (model)
        return ((QTaskIO *)model)->setTaskField(r, (Field)index.column(), value);
    return false;
#endif
}

/*!
  \overload

  Returns a map with values for all predefined roles in the model for the item at the
  given \a index.
*/
QMap<int, QVariant> QTaskModel::itemData ( const QModelIndex &index ) const
{
    QMap<int, QVariant> m;
    m.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
    return m;
}

/*!
  \overload
  For every Qt::ItemDataRole in \a roles, sets the role data for the item at \a index to the
  associated value in \a roles. Returns true if successful, otherwise returns false.
*/
bool QTaskModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &roles)
{
    if (roles.count() != 1 || !roles.contains(Qt::EditRole))
        return false;
    return setData(index, roles[Qt::EditRole], Qt::EditRole);
}

/*!
  \overload

  Returns the data for the given \a role and \a section in the header with the
  specified \a orientation.
*/
QVariant QTaskModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole)
            return fieldLabel((Field)section);
        else if (role == Qt::DecorationRole)
            return qvariant_cast<QIcon>(fieldIcon((Field)section));
        else if (role == Qt::EditRole)
            return fieldIdentifier((Field)section);
    }
    return QVariant();
}

/*!
  \overload
    Returns the item flags for the given \a index.
*/
Qt::ItemFlags QTaskModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

/*!
  \overload
    Returns a list of indexes for the items where the data
    matches the specified \a value.  The list that is returned may be empty.

    The search starts from the \a start index.

    The arguments \a role, \a hits and \a flags are currently ignored.
*/
QModelIndexList QTaskModel::match(const QModelIndex &start, int role, const QVariant &value,
            int hits, Qt::MatchFlags flags) const
{
    Q_UNUSED(start)
    Q_UNUSED(value)
    Q_UNUSED(hits);
    Q_UNUSED(role);
    Q_UNUSED(flags);

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

/*!
  Returns the value from \a task that would be returned for
  field \a f as it would from a row in the QTaskModel.
 */
QVariant QTaskModel::taskField(const QTask &task, QTaskModel::Field f)
{
    switch(f) {
        default:
        case Invalid:
            break;
        case Identifier:
            return QVariant(task.uid().toByteArray());
        case Categories:
            return QVariant(task.categories());
        case Description:
            return QVariant(task.description());
        case Priority:
            return QVariant(task.priority());
        case Completed:
            return QVariant(task.isCompleted());
        case PercentCompleted:
            return QVariant(task.percentCompleted());
        case Status:
            return QVariant(task.status());
        case DueDate:
            return QVariant(task.dueDate());
        case StartedDate:
            return QVariant(task.startedDate());
        case CompletedDate:
            return QVariant(task.completedDate());
        case Notes:
            return QVariant(task.notes());
    }
    return QVariant();
}

/*!
  Sets the value in \a task that would be set for field \a f as it would
  if modified for a task in the QTaskModel to \a v.

  Returns true if the task was modified.  Otherwise returns false.
*/
bool QTaskModel::setTaskField(QTask &task, QTaskModel::Field f,  const QVariant &v)
{
    switch (f) {
        default:
        case Invalid:
            return false;
        case Description:
            if (v.canConvert(QVariant::String)) {
                task.setDescription(v.toString());
                return true;
            }
            return false;
        case Priority:
            if (v.canConvert(QVariant::Int)) {
                task.setPriority(v.toInt());
                return true;
            }
            return false;
        case Completed:
            if (v.canConvert(QVariant::Bool)) {
                task.setCompleted(v.toBool());
                return true;
            }
            return false;
        case PercentCompleted:
            if (v.canConvert(QVariant::Int)) {
                task.setPercentCompleted(v.toInt());
                return true;
            }
            return false;
        case Status:
            if (v.canConvert(QVariant::Int)) {
                task.setStatus(v.toInt());
                return true;
            }
            return false;
        case DueDate:
            if (v.canConvert(QVariant::Date)) {
                task.setDueDate(v.toDate());
                return true;
            }
            return false;
        case StartedDate:
            if (v.canConvert(QVariant::Date)) {
                task.setStartedDate(v.toDate());
                return true;
            }
            return false;
        case CompletedDate:
            if (v.canConvert(QVariant::Date)) {
                task.setCompletedDate(v.toDate());
                return true;
            }
            return false;
        case Notes:
            if (v.canConvert(QVariant::String)) {
                task.setNotes(v.toString());
                return true;
            }
            return false;
    }
    return false;
}
