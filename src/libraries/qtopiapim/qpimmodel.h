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

#ifndef QTOPIA_PIMMODEL_H
#define QTOPIA_PIMMODEL_H

#include <QAbstractItemModel>
#include <QPimRecord>
#include <QCategoryManager>

#include <qpimsource.h>
#include <qtopiaglobal.h>

class QRecordIO;
class QPimModelData;
class QTOPIAPIM_EXPORT QPimModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QPimModel(QObject *parent = 0);
    virtual ~QPimModel();

    // acts on visible sources.
    bool startSyncTransaction(const QDateTime &syncTime);
    bool abortSyncTransaction();
    bool commitSyncTransaction();

    // acts on visible sources.
    QList<QUniqueId> removed(const QDateTime &) const;
    QList<QUniqueId> added(const QDateTime &) const;
    QList<QUniqueId> modified(const QDateTime &) const;

    const QList<QPimContext*> &contexts() const;
    QSet<QPimSource> visibleSources() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> availableSources() const;

    QPimSource source(const QUniqueId &) const;
    QPimContext *context(const QUniqueId &) const;
    QPimContext *context(const QPimSource &) const;

    bool sourceExists(const QPimSource &source, const QUniqueId &id) const;
    int rowCount(const QModelIndex & = QModelIndex()) const;

    int count() const;
    bool contains(const QModelIndex &) const;
    bool contains(const QUniqueId &) const;
    bool exists(const QUniqueId &) const;

    bool editable(const QModelIndex &index) const;
    bool editable(const QUniqueId &) const;

    QModelIndex index(const QUniqueId &) const;
    QUniqueId id(const QModelIndex &) const;
    QUniqueId id(int) const;
    QModelIndex index(int r,int c = 0,const QModelIndex & = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &) const;
    bool hasChildren(const QModelIndex & = QModelIndex()) const;

    void setCategoryFilter(const QCategoryFilter &categories);
    QCategoryFilter categoryFilter() const;

    virtual QUniqueId addRecord(const QByteArray &, const QPimSource &, const QString & = QString()) = 0;
    virtual bool updateRecord(const QUniqueId &, const QByteArray &, const QString & = QString()) = 0;
    virtual bool removeRecord(const QUniqueId &) = 0;
    virtual QByteArray record(const QUniqueId &, const QString & = QString()) const = 0;

    bool flush();
    bool refresh();

private slots:
    void voidCache();

protected:
    void addAccess(QRecordIO *);
    void addContext(QPimContext *);
    QRecordIO *access(const QUniqueId &) const;
    QRecordIO *access(int row) const;
    int accessRow(int row) const;
    QList<QRecordIO*> &accessModels() const;

private:

    QPimModelData *d;
};

#endif // QTOPIA_PIMMODEL_H
