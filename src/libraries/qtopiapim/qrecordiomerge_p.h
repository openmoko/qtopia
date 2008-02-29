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

#ifndef MERGEIO_PRIVATE_H
#define MERGEIO_PRIVATE_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QMap>
#include <QList>

#include <quniqueid.h>

#include "qrecordio_p.h"

class QTimer;

/* a class that takes multiple models, and generates or caches
   infomration to provide a unified index.

   // (from what? XmlIO, QRecordIO etc?
   expect sub ios to provide.

   QVariant key(int row) const;
   QUniqueId id(int row) const;

   and also be sorted and filtered the same way.
   Since filtering and such is model dependent.. timer 0 on changes in case of multiple changes.

   cachingis approximately n*(lookup of row in model), but is spread out so as to not disrupt
   gui (if thats becomes a problem.  changing a tuning value at the top of the .cpp
   file will change how much work is done on any one cycle).
*/

// should be a QObject, so can be auto updating.
class QRecordIOMerge : public QObject
{
    Q_OBJECT
public:
    explicit QRecordIOMerge(QObject *parent);
    ~QRecordIOMerge();

    void addModel(const QRecordIO *);
    void removeModel(const QRecordIO *);
    // models expected to filter themselves.

    QList<const QRecordIO*> models() const { return mModelIndexed.keys(); }

    int count() const { return mCount; }

    /*
       Incremental loading.  e.g. on big change might take a while to rebuild
       cache... so this function indicates how far the cache is through.
    */
    int cachedTo() const { return mIndexed; }

    const QRecordIO *model(int index) const;
    int row(int index) const;

    int index(const QRecordIO *, int row) const;
    int index(const QUniqueId &) const;

signals:
    void reset();
    void rowsChanged(int, int);

protected slots:

    void indexNext();
    void clearIndex();
private:
    struct ModelRow {
        const QRecordIO *model;
        int row;
    };


    int mCount;
    int mIndexed;

    QTimer *cachingTimer;

    QVector<ModelRow> indToRow;
    QMap<const QRecordIO*, QVector<int> > rowToIndex;
    QMap<const QRecordIO*, int > mModelIndexed;
};

// different from above in that it assumes
// main list is v.large (10000) and other lists
// v.small (<200).  Since the only current concrete use case
// is merging with sim card which is < 200, this is appropriate.
// will have to change when add new models, but thats what abstraction is for.
class QBiasedRecordIOMerge : public QObject
{
    Q_OBJECT
public:
    explicit QBiasedRecordIOMerge(QObject *parent);
    ~QBiasedRecordIOMerge();

    void setPrimaryModel(const QRecordIO *);
    void removePrimaryModel();

    void addSecondaryModel(const QRecordIO *);
    void removeSecondaryModel(const QRecordIO *);
    // models expected to filter themselves.

    QList<const QRecordIO*> models() const;

    int count() const;

    const QRecordIO *model(int index) const;
    int row(int index) const;

    int index(const QRecordIO *, int row) const;
    int index(const QUniqueId &) const;

    static bool compare(const QVariant &, const QVariant &);

signals:
    void reset();

public slots:
    void rebuildCache();
    void updateCache();

private:
    void lookupRowModel(int index) const;
    int findPrimaryIndex(const QVariant &key, const QUniqueId &id);

    const QRecordIO *mPrimary;
    QMap<const QRecordIO*, QVector<int> > mSecondaries;
    QList<int> mSkips;
    mutable int lastIndex;
    mutable int lastRow;
    mutable const QRecordIO *lastModel;
    QTimer *rebuildTimer;
};

#endif //MERGEIO_PRIVATE_H
