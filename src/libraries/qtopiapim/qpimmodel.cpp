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
#include "qpimmodel.h"
#include "qrecordio_p.h"
#include <QPainter>
#include <QIcon>
#include <QDebug>

#include "qrecordiomerge_p.h"
#include "qrecordio_p.h"

class QPimModelData
{
public:
    QPimModelData() : mio(0), defaultContext(0), defaultModel(0), searchModel(0), filterFlags(0) {}
    ~QPimModelData() {
        if (searchModel)
            delete searchModel;
    }

    QBiasedRecordIOMerge *mio;

    QPimContext *defaultContext;
    QRecordIO *defaultModel;
    QList<QPimContext *> contexts;
    QList<QRecordIO*> models;

    mutable QPimModel *searchModel;
    mutable QString filterText;
    mutable int filterFlags;
};

/*!
  \class QPimModel
  \module qpepim
  \ingroup pim
  \brief The QPimModel class provides an abstract interface to the pim model classes.

  The QPimModel class defines a standard interface that is implemented by all of the pim
  model classes.
*/

/*!
  \fn QUniqueId QPimModel::addRecord(const QByteArray &bytes, const QPimSource &source = QPimSource(), const QString &format = QString())

  Adds the PIM record encoded in \a bytes to the QPimModel under the storage source \a source.
  The format of the record in \a bytes is given by \a format.  An empty \a format string will
  cause the record to be read using the data stream operators for the PIM data type of the model.
  If \a source is empty will add the record to the default storage source.

  Returns valid id if the record was successfully added.  Otherwise returns an invalid id.

  Can only add PIM data that is represented by the model.  This means that only contact data
  can be added using a QContactModel, only appointment data added using QAppointmentModel and
  only task data added using QTaskModel.
*/

/*!
  \fn bool QPimModel::updateRecord(const QUniqueId &id, const QByteArray &bytes, const QString &format = QString())

  Updates the record enoded in \a bytes so long as there is a record in the QPimModel with
  the same uid as the record.  The format of the record in \a bytes is given by \a format.
  An empty \a format string will cause the record to be read using the data stream operators
  for the PIM data type of the model. If \a id is not null will set the record uid to \a id
  before attempting to update the record.

  Returns true if the record was successfully updated.  Otherwise returns false.
*/

/*!
  \fn bool QPimModel::removeRecord(const QUniqueId &id)

  Removes the record that has the uid \a id from the QPimModel.

  Returns true if the record was successfully removed.  Otherwise returns false.
*/

/*!
  \fn QByteArray QPimModel::record(const QUniqueId &id, const QString &format = QString()) const

    Returns the record with the identifier \a id encoded in the format specified by \a format.
    An empty \a format string will cause the record to be written using the data stream
    operators for the PIM data type of the model.
*/

/*!
  Contstructs a QPimModel with parent \a parent.
*/
QPimModel::QPimModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    d = new QPimModelData();
    d->mio = new QBiasedRecordIOMerge(this);

    connect(d->mio, SIGNAL(reset()), this, SLOT(voidCache()));
}

/*!
  Destructs the QPimModel
*/
QPimModel::~QPimModel()
{
}

/*!
  Adds the record \a accessModel to the QPimModel.  If no access models have
  been added previously the \a accessModel is set to be the default model.
*/
void QPimModel::addAccess(QRecordIO *accessModel)
{
    if (!d->defaultModel) {
        d->defaultModel = accessModel;
        d->mio->setPrimaryModel(d->defaultModel);
    }
    d->models.append(accessModel);
}

/*!
  Adds the record \a context to the QPimModel.  If no contexts have
  been added previously the \a context is set to be the default context.
*/
void QPimModel::addContext(QPimContext *context)
{
    if (!d->defaultContext)
        d->defaultContext = context;
    d->contexts.append(context);
}

/*!
  Returns the list of access models added to the pim model
*/
QList<QRecordIO*> &QPimModel::accessModels() const
{ return d->models; }

/*!
  Returns the access model that holds data for the record with identifier \a id.
*/
QRecordIO *QPimModel::access(const QUniqueId &id) const
{
    foreach(QRecordIO *model, d->models) {
        if (model->exists(id))
            return model;
    }
    return 0;
}

/*!
  Returns the access model that holds data for the row \a row in the QPimModel.
*/
QRecordIO *QPimModel::access(int row) const
{
    QRecordIO *model = d->mio->model(row);
    return model;
}

/*!
  Returns the row in the access model that holds data for the row \a row in the QPimModel.
*/
int QPimModel::accessRow(int row) const
{
    return d->mio->row(row);
}

/*!
    \internal
    Causes the model to reset notifying any attached views to update their representation
    of the model.
*/
void QPimModel::voidCache()
{
    reset();
}

/*!
   Returns the contexts of record data that can be shown by the record model.
*/
const QList<QPimContext*> &QPimModel::contexts() const
{
    return d->contexts;
}

/*!
  Returns the list of sources of record data that are currently shown by the
  record model.
*/
QSet<QPimSource> QPimModel::visibleSources() const
{
    QSet<QPimSource> set;
    foreach(QPimContext *c, d->contexts)
        set.unite(c->visibleSources());
    return set;
}

/*!
  Sets the QPimModel to show only records contained in the storage sources specified
  by \a list.

  Also refreshes the model.
*/
void QPimModel::setVisibleSources(const QSet<QPimSource> &list)
{
    foreach (QPimContext *c, d->contexts) {
        QSet<QPimSource> cset = c->sources();
        cset.intersect(list);
        c->setVisibleSources(cset);
    }
    refresh();
}

/*!
  Returns the set of identifiers for storage sources that can be shown.
*/
QSet<QPimSource> QPimModel::availableSources() const
{
    QSet<QPimSource> set;
    foreach(QPimContext *c, d->contexts)
        set.unite(c->sources());
    return set;
}

/*!
  Returns the source identifier that contains the record with identifier \a id.
  If the record does not exist returns a null source.
*/
QPimSource QPimModel::source(const QUniqueId &id) const
{
    foreach(QPimContext *context, d->contexts) {
        if (context->exists(id))
            return context->source(id);
    }
    return QPimSource();
}

/*!
  Returns the context that contains the record with identifier \a id.
  If the record does not exists returns 0.
*/
QPimContext *QPimModel::context(const QUniqueId &id) const
{
    foreach(QPimContext *context, d->contexts) {
        if (context->exists(id))
            return context;
    }
    return 0;
}

/*!
  Prepares the currently visible sources for syncing.  
  All modifications between startSyncTransaction() and commitSyncTransaction()
  will be marked with the same \a timestamp for modification or creation as appropriate.

  Returns true if transaction successfully initiated, otherwise returns false.
  
  Does not abort transaction if unsuccessfully initiated.

  \sa commitSyncTransaction(), abortSyncTransaction()
*/
bool QPimModel::startSyncTransaction(const QDateTime &timestamp)
{
    bool succeeded = true;
    foreach(QRecordIO *io, accessModels()) {
        if (!io->startSyncTransaction(visibleSources(), timestamp)) {
            succeeded = false;
            break;
        }
    }
    return succeeded;
}

/*!
  Aborts the current sync transaction.

  Returns true if transaction successfully aborted, otherwise returns false.

  \sa startSyncTransaction(), commitSyncTransaction()
*/
bool QPimModel::abortSyncTransaction()
{
    bool succeeded = true;
    foreach(QRecordIO *io, accessModels()) {
        if (!io->abortSyncTransaction()) {
            succeeded = false;
            break;
        }
    }
    return succeeded;
}

/*!
  Commits the current sync transaction.

  Returns true if transaction successfully committed, otherwise returns false.

  Does not abort transaction if unsuccessfully committed.

  \sa startSyncTransaction(), abortSyncTransaction()
*/
bool QPimModel::commitSyncTransaction()
{
    bool succeeded = true;
    foreach(QRecordIO *io, accessModels()) {
        if (!io->commitSyncTransaction()) {
            succeeded = false;
            break;
        }
    }
    return succeeded;
}

/*!
  Returns the list of id's for records removed from the current set of visible sources
  on or after the specified \a timestamp.
*/
QList<QUniqueId> QPimModel::removed(const QDateTime &timestamp) const
{
    QList<QUniqueId> result;
    foreach(QRecordIO *io, accessModels()) {
        result += io->removed(visibleSources(), timestamp);
    }
    return result;
}

/*!
  Returns the list of id's for records added the current set of visible sources
  on or after the specified \a timestamp.
*/
QList<QUniqueId> QPimModel::added(const QDateTime &timestamp) const
{
    QList<QUniqueId> result;
    foreach(QRecordIO *io, accessModels()) {
        result += io->added(visibleSources(), timestamp);
    }
    return result;
}

/*!
  Returns the list of id's for records modified in the current set of visible sources
  on or after the specified \a timestamp.
*/
QList<QUniqueId> QPimModel::modified(const QDateTime &timestamp) const
{
    QList<QUniqueId> result;
    foreach(QRecordIO *io, accessModels()) {
        result += io->modified(visibleSources(), timestamp);
    }
    return result;
}

/*!
  Return the number of records visible in the in the current filter mode.
*/
int QPimModel::count() const
{
    return d->mio->count();
}

/*!
  \overload

    Returns the number of rows under the given \a parent.
*/
int QPimModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return count();
}

/*!
  Returns true if the current filter mode of the model contains \a index.
  Otherwise returns false.
*/
bool QPimModel::contains(const QModelIndex &index) const
{
    return (index.row() >= 0 && index.row() < count());
}

/*!
  Returns true if the current filter mode of the model contains the record with the uid \a id.
  Otherwise returns false.
*/
bool QPimModel::contains(const QUniqueId & id) const
{
    return index(id).isValid();
}

/*!
  Returns true if a record with the uid \a id is stored in the record model.  Otherwise
  return false.

  The record with uid \a id does not need to be in the current filter mode.

  \sa contains()
*/
bool QPimModel::exists(const QUniqueId &id) const
{
    foreach(const QRecordIO *model, d->models) {
        if (model->exists(id))
            return true;
    }
    return false;
}

/*!
  Returns true if the record for \a index can be updated or removed.
  Otherwise returns false.
*/
bool QPimModel::editable(const QModelIndex &index) const
{
    return editable(id(index));
}

/*!
  Returns true if the record for \a id can be updated or removed.
  Otherwise returns false.
*/
bool QPimModel::editable(const QUniqueId &id) const
{
    foreach (QPimContext *c, d->contexts) {
        if (c->exists(id))
            return c->editable(id);
    }
    return false;
}

/*!
  Returns true if the record uid \a id is stored in the storage source \a source.
  Otherwise returns false.
*/
bool QPimModel::sourceExists(const QPimSource &source, const QUniqueId &id) const
{
    foreach (QPimContext *c, d->contexts) {
        if (c->sources().contains(source))
            return c->exists(id, source);
    }
    return false;
}

/*!
  If the model contains a record with uid \a id, returns the index of the record.
  Otherwise returns a null QModelIndex

  \sa contains(), exists()
*/
QModelIndex QPimModel::index(const QUniqueId & id) const
{
    int i = d->mio->index(id);
    if (i == -1)
        return QModelIndex();
    return createIndex(i, 0);
}

/*!
  \overload
    Returns the parent of the model item with the given \a index.
*/
QModelIndex QPimModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}


/*!
  \overload
  Returns true if \a parent has any children; otherwise returns false.
  Use rowCount() on the parent to find out the number of children.

  \sa parent(), index()
*/
bool QPimModel::hasChildren(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return false;
}

/*!
  Ensures the data in Records is in a state suitable for syncing.
*/
bool QPimModel::flush() {
    return true;
}

/*!
  Forces a refresh of the Record data.
*/
bool QPimModel::refresh() {
    d->mio->rebuildCache();
    return true;
}

/*!
  Return the id for the record at the row specified by \a index.
  If index is null or out of the range of the model, will return a null id.
*/
QUniqueId QPimModel::id(const QModelIndex &index) const
{
    int i = index.row();
    return id(i);
}

/*!
  Returns the id for record at the \a row specified.
*/
QUniqueId QPimModel::id(int row) const
{
    const QRecordIO *model = qobject_cast<const QRecordIO *>(d->mio->model(row));
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
QModelIndex QPimModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (row < 0 || row >= count() || column < 0 || column >= columnCount())
        return QModelIndex();
    return createIndex(row,column);
}

/*!
  If the \a source exists in the model returns the pim context that provides that
  \a source.  Otherwise returns 0.
*/
QPimContext *QPimModel::context(const QPimSource &source) const
{
    QPimContext *c = 0;
    if (source.isNull()) {
        c = d->defaultContext;
    } else {
        foreach(QPimContext *context, d->contexts) {
            if (context->sources().contains(source)) {
                c = context;
                break;
            }
        }
    }
    return c;
}

/*!
  Set the model to only contain records accepted by the QCategoryFilter \a f.
*/
void QPimModel::setCategoryFilter(const QCategoryFilter &f)
{
    if (f == categoryFilter())
        return;

    foreach(QRecordIO *model, d->models)
        model->setCategoryFilter(f);
    d->mio->rebuildCache();
}

/*!
  Returns the QCategoryFilter that records are tested against for the current filter mode.
*/
QCategoryFilter QPimModel::categoryFilter() const
{
    return d->defaultModel->categoryFilter();
}
