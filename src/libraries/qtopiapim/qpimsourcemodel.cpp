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

#include <qpimsourcemodel.h>
#include <QIcon>
#include <QDebug>

class QPimSourceModelData
{
public:
    QList<QPimSource> mSourceList;
    QList<QPimContext*> mContextList;
    QSet<QPimSource> mCheckedSources;

    bool mCheckable;
};

/*!
  \class QPimSourceModel
  \module qpepim
  \ingroup qpepim

  QPimSourceModel is an aid to creating widget for applications to manage PIM
  data from multiple sources.  It provides sorting of the sources, accessing
  translated strings for source titles, and the ability to check and uncheck
  items for use with setting the set of visible sources for one of the pim
  models, QContactModel, QTaskModel or QAppointmentModel.
*/

/*!
  Constructs a QPimSourceModel with parent \a parent.
*/
QPimSourceModel::QPimSourceModel(QWidget *parent)
    : QAbstractListModel(parent)
{
    d = new QPimSourceModelData();
    d->mCheckable = false;
}

/*!
  Destroys the QPimSourceModel.
*/
QPimSourceModel::~QPimSourceModel()
{
    delete d;
}

/*!
  Sets the contexts defining the PIM data sources to display to \a contexts
  Available sources for each context is build into the list of PIM
  data sources included in the model.
*/
void QPimSourceModel::setContexts(const QList<QPimContext *> &contexts)
{
    /*
       build list of sources
       sort list of sources
       emit update, reset style fine in this case.
   */
    d->mContextList = contexts;
    d->mSourceList.clear();
    foreach(QPimContext *con, contexts)
        d->mSourceList += con->sources().toList();

    qSort(d->mSourceList);

    reset();
}

/*!
  Sets the contexts defining the PIM data sources to display to \a contexts
  Available sources for each context is build into the list of PIM
  data sources included in the model.
*/

void QPimSourceModel::setContexts(const QList<QContactContext *> &contexts)
{
    d->mContextList.clear();
    d->mSourceList.clear();
    foreach(QContactContext *con, contexts) {
        d->mContextList.append(con);
        d->mSourceList += con->sources().toList();
    }

    qSort(d->mSourceList);

    reset();
}

/*!
  Sets the contexts defining the PIM data sources to display to \a contexts
  Available sources for each context is build into the list of PIM
  data sources included in the model.
*/
void QPimSourceModel::setContexts(const QList<QAppointmentContext *> &contexts)
{
    d->mContextList.clear();
    d->mSourceList.clear();
    foreach(QAppointmentContext *con, contexts) {
        d->mContextList.append(con);
        d->mSourceList += con->sources().toList();
    }

    qSort(d->mSourceList);

    reset();
}

/*!
  Sets the contexts defining the PIM data sources to display to \a contexts
  Available sources for each context is build into the list of PIM
  data sources included in the model.
*/
void QPimSourceModel::setContexts(const QList<QTaskContext *> &contexts)
{
    d->mContextList.clear();
    d->mSourceList.clear();
    foreach(QTaskContext *con, contexts) {
        d->mContextList.append(con);
        d->mSourceList += con->sources().toList();
    }

    qSort(d->mSourceList);

    reset();
}

/*!
  Sets the sources that should be marked as checked to those contained in
  \a set.  Will also mark items as checkable.
*/
void QPimSourceModel::setCheckedSources(const QSet<QPimSource> &set)
{
    d->mCheckable = true;
    d->mCheckedSources = set;

    reset();
}

/*!
  Returns the set of checked sources.
*/
QSet<QPimSource> QPimSourceModel::checkedSources() const
{
    return d->mCheckedSources;
}

/*!
  Returns the PIM data source at \a index.
*/
QPimSource QPimSourceModel::source(const QModelIndex &index) const
{
    return d->mSourceList[index.row()];
}

/*!
  Returns the context that controls the PIM data source at \a index.
*/
QPimContext *QPimSourceModel::context(const QModelIndex &index) const
{
    QPimSource s = source(index);
    foreach(QPimContext *c, d->mContextList) {
        if (c->sources().contains(s))
            return c;
    }
    return 0;
}

/*!
  Returns the index where the PIM data \a source is positioned in the model.
  Returns a null index if the source is not listed in the model.
*/
QModelIndex QPimSourceModel::index(const QPimSource &source) const
{
    int i = d->mSourceList.indexOf(source);
    if (i < 0)
        return QModelIndex();
    return createIndex(i, 0);
}

/*!
  Returns the number of rows in the model.  The parameter \a parent is not used.
*/
int QPimSourceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->mSourceList.count();
}

/*!
  Returns the item flags for the given \a index.
*/
Qt::ItemFlags QPimSourceModel::flags(const QModelIndex &index) const
{
    if (d->mCheckable)
        return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable;
    else
        return QAbstractListModel::flags(index);
}

/*!
  Returns the data stored under the given \a role for the item referred to by the
  \a index.
*/
QVariant QPimSourceModel::data(const QModelIndex &index, int role) const
{
    if (!context(index))
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            return context(index)->title(source(index));
        case Qt::DecorationRole:
            return context(index)->icon();
        case Qt::CheckStateRole:
            if (d->mCheckedSources.contains(source(index)))
                return Qt::Checked;
            else
                return Qt::Unchecked;

        default:
            return QVariant();
    }
}

/*!
  Sets the \a role data for the item at \a index to \a value.
  Returns true if successful, otherwise returns false.

  In this implementation only the dat for the Qt::CheckStateRole can be modified
  this way.
*/
bool QPimSourceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::CheckStateRole)
        return false;
    if (value.toInt() == Qt::Checked)
        d->mCheckedSources.insert(source(index));
    else
        d->mCheckedSources.remove(source(index));
    dataChanged(index, index);
    return true;
}

/*!
  Adds the PIM data \a source to the model
*/
void QPimSourceModel::addSource(const QPimSource &source)
{
    // first iterate through list to find target row, then insert
    int pos = 0;
    foreach(QPimSource s, d->mSourceList) {
        if (source < s)
            break;
        pos++;
    }
    beginInsertRows(QModelIndex(), pos, pos);
    d->mSourceList.append(source);
    qSort(d->mSourceList);
    endInsertRows();
}

/*!
  Removes the PIM data \a source from the model.
*/
void QPimSourceModel::removeSource(const QPimSource &source)
{
    int pos = d->mSourceList.indexOf(source);
    if (pos < 0)
        return;
    beginRemoveRows(QModelIndex(), pos, pos);
    d->mSourceList.removeAt(pos);
    endRemoveRows();
}

/*!
  Updates the PIM data source at \a index to \a source.  Because the model maintains
  a sort order the new \a source may not appear at \a index.
*/
void QPimSourceModel::updateSource(const QModelIndex &index, const QPimSource&source)
{
    removeSource(QPimSourceModel::source(index));
    addSource(source);
}
