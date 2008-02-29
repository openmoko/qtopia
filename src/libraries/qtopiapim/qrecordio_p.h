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

#ifndef PIMIO_PRIVATE_H
#define PIMIO_PRIVATE_H

#include <QObject>
#include <QVariant>

#include <QUniqueId>

class QRecordIO : public QObject
{
    Q_OBJECT

public:
    explicit QRecordIO(QObject *parent) : QObject(parent) { }
    virtual int count() const = 0;

    virtual bool exists(const QUniqueId &) const = 0;
    virtual bool contains(const QUniqueId &) const = 0;

    virtual QUuid contextId() const = 0;

    // needs to be comparable?
    virtual QVariant key(int row) const = 0;
    virtual QUniqueId id(int row) const = 0;
    virtual int row(const QUniqueId &) const = 0;

    virtual int predictedRow(const QVariant &, const QUniqueId &) const;

signals:
    void recordsUpdated();
};

#endif // PIMIO_PRIVATE_H
