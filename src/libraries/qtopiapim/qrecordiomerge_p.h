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

#ifndef MERGEIO_PRIVATE_H
#define MERGEIO_PRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QMap>
#include <QList>

#include <quniqueid.h>

#include "qrecordio_p.h"

class QTimer;

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

    void setPrimaryModel(QRecordIO *);
    void removePrimaryModel();

    void addSecondaryModel(QRecordIO *);
    void removeSecondaryModel(QRecordIO *);
    // models expected to filter themselves.

    QList<QRecordIO*> models() const;

    int count() const;

    QRecordIO *model(int index) const;
    int row(int index) const;

    int index(QRecordIO *, int row) const;
    int index(const QUniqueId &) const;

    static bool compare(const QVariant &, const QVariant &);

signals:
    void reset();

public slots:
    void rebuildCache();
    void updateCache();
    void updateCacheNow();

private:
    void lookupRowModel(int index) const;
    int findPrimaryIndex(const QVariant &key, const QUniqueId &id);

    QRecordIO *mPrimary;
    QMap<QRecordIO*, QVector<int> > mSecondaries;
    QList<int> mSkips;
    mutable int lastIndex;
    mutable int lastRow;
    mutable QRecordIO *lastModel;
    QTimer *rebuildTimer;
    bool modelChanged;
};

#endif //MERGEIO_PRIVATE_H
