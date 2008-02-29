/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef ACCOUNTEDITOR_H
#define ACCOUNTEDITOR_H

#include <QWidget>
#include <QList>
#include <QSet>
#include <QAbstractListModel>

#include <qpimsource.h>

class QPimSource;
class QPimContext;
class QPimSourceModelData;
class QTOPIAPIM_EXPORT QPimSourceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit QPimSourceModel(QWidget *parent = 0);
    ~QPimSourceModel();

    // sets the internal data
    void setContexts(const QList<QPimContext *> &);
    // for convenience
    void setContexts(const QList<QContactContext *> &);
    void setContexts(const QList<QAppointmentContext *> &);
    void setContexts(const QList<QTaskContext *> &);

    void setCheckedSources(const QSet<QPimSource> &);
    QSet<QPimSource> checkedSources() const;

    QPimSource source(const QModelIndex &) const;
    QPimContext *context(const QModelIndex &) const;
    using QAbstractListModel::index;
    QModelIndex index(const QPimSource &) const;

    // for normal model work.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &, const QVariant &, int role = Qt::EditRole);

    Qt::ItemFlags flags(const QModelIndex &) const;

    void addSource(const QPimSource &);
    void removeSource(const QPimSource &);
    void updateSource(const QModelIndex &index, const QPimSource&);
private:
    QPimSourceModelData *d;
};

#endif // ACCOUNTEDITOR_H
