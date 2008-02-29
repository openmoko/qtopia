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

#ifndef TASKIO_PRIVATE_H
#define TASKIO_PRIVATE_H

#include <qvector.h>
#include <qlist.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qobject.h>
#include <qtopia/pim/qtaskmodel.h>
#include <qtopia/pim/qtask.h>

#include "qrecordio_p.h"

class QTaskIO;
class QPimSource;
class QTaskIO : public QRecordIO {

Q_OBJECT

public:

    explicit QTaskIO(QObject *parent);
    virtual ~QTaskIO();

    virtual bool editableByRow() const { return false; }
    virtual bool editableByField() const { return false; }

    virtual void setSortKey(QTaskModel::Field k) = 0;
    virtual QTaskModel::Field sortKey() const = 0;

    virtual bool completedFilter() const = 0;
    virtual void setCompletedFilter(bool) = 0;

    virtual QTask task(const QUniqueId &) const = 0;
    virtual QTask task(int row) const = 0;
    virtual QUniqueId taskId(int row) const = 0;
    virtual int row(const QUniqueId &) const = 0;

    virtual QVariant taskField(int row, QTaskModel::Field k) const;
    virtual bool setTaskField(int row, QTaskModel::Field k,  const QVariant &);

    virtual bool removeTask(int row) = 0;
    virtual bool removeTask(const QUniqueId & id) = 0;
    virtual bool removeTask(const QTask &) = 0;
    virtual bool removeTasks(const QList<int> &rows) = 0;
    virtual bool removeTasks(const QList<QUniqueId> &ids) = 0;

    virtual bool updateTask(const QTask &) = 0; // not done by row
    virtual QUniqueId addTask(const QTask &, const QPimSource &) = 0; // not done by row

    virtual bool exists(const QUniqueId &) const = 0;
    virtual bool contains(const QUniqueId &) const = 0;
    virtual int count() const = 0;

#ifdef SUPPORT_SYNCML
    virtual bool canProvideDiff() const { return false; }
    virtual void clearJournal() {}
    virtual QList<QUniqueId> addedTasks() const { return QList<QUniqueId>(); }
    virtual QList<QUniqueId> modifiedTasks() const { return QList<QUniqueId>(); }
    virtual QList<QUniqueId> deletedTasks() const { return QList<QUniqueId>(); }
#endif

    // searching.  row based.
    // pda based
    virtual int startSearch(const QString &) = 0;
    virtual int nextSearchItem() = 0;
    virtual void clearSearch() = 0;

    // called by 'models' when they notify each other via qcop or com.
public slots:
    // need implents.  Does not mean add, means has been added by extenral
    // and may need to check or update records.
    virtual void checkAdded(const QUniqueId &);
    virtual void checkRemoved(const QUniqueId &);
    virtual void checkRemoved(const QList<QUniqueId> &);
    virtual void checkUpdated(const QUniqueId &);

protected:
    void notifyAdded(const QUniqueId &);
    void notifyUpdated(const QUniqueId &);
    void notifyRemoved(const QUniqueId &);
    void notifyRemoved(const QList<QUniqueId> &);

    virtual void ensureDataCurrent(bool force = false);
private slots:
    void pimMessage(const QString &message, const QByteArray &data);

private:
    static QList<QTaskIO *> activeTasks;
};

#endif
