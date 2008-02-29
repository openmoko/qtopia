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

#ifndef APPOINTMENT_SQLIO_PRIVATE_H
#define APPOINTMENT_SQLIO_PRIVATE_H

#include <qlist.h>
#include <qdatetime.h>
#include <qtopia/pim/qappointment.h>
#include <qtopiasql.h>
#include <qsqlquery.h>
#include "qappointmentio_p.h"
#include "qpimsqlio_p.h"
#include "qpimsource.h"

class QAppointmentSqlIO;
class QAppointmentDefaultContext : public QAppointmentContext
{
    Q_OBJECT
public:
    // could have constructor protected/private with friends class.
    QAppointmentDefaultContext(QObject *parent, QObject *access);

    QIcon icon() const; // default empty
    QString description() const;
    QString title() const;

    // better to be flags ?
    bool editable() const; // default true

    QPimSource defaultSource() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    bool exists(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateAppointment(const QAppointment &);
    bool removeAppointment(const QUniqueId &);
    QUniqueId addAppointment(const QAppointment &, const QPimSource &);

    bool removeOccurrence(const QUniqueId &original, const QDate &);
    QUniqueId replaceOccurrence(const QUniqueId &original, const QOccurrence &);
    QUniqueId replaceRemaining(const QUniqueId &original, const QAppointment &);
private:
    QAppointmentSqlIO *mAccess;
};

class QAppointmentSqlIO : public QAppointmentIO, public QPimSqlIO {

    Q_OBJECT

public:
    explicit QAppointmentSqlIO(QObject *parent);
    ~QAppointmentSqlIO();

    bool editableByRow() const { return true; }
    bool editableByField() const { return true; }

    bool removeAppointment(const QUniqueId &id);
    bool removeAppointment(int);
    bool updateAppointment(const QAppointment& appointment);
    QUniqueId addAppointment(const QAppointment& appointment, const QPimSource &s)
    { return addAppointment(appointment, s, true); }
    QUniqueId addAppointment(const QAppointment& appointment, const QPimSource &, bool);

    bool removeOccurrence(const QUniqueId &original,
            const QDate &);
    QUniqueId replaceOccurrence(const QUniqueId &original,
            const QOccurrence &);
    QUniqueId replaceRemaining(const QUniqueId &original,
            const QAppointment &);

    QUuid contextId() const;

    void setCategoryFilter(const QCategoryFilter &);
    QCategoryFilter categoryFilter() const;

    void setRangeFilter(const QDateTime &earliest, const QDateTime &latest);
    QDateTime rangeStart() const;
    QDateTime rangeEnd() const;

    void setDurationType(QAppointmentModel::DurationType);

    void setContextFilter(const QSet<int> &);
    QSet<int> contextFilter() const;

    QAppointment appointment(const QUniqueId &) const;
    QAppointment appointment(int row) const;

    int count() const { return QPimSqlIO::count(); }
    bool exists(const QUniqueId & id) const { return !appointment(id).uid().isNull(); }
    bool contains(const QUniqueId & id) const { return row(id) != -1; }

    QVariant key(const QUniqueId &) const;
    QVariant key(int row) const;
    QUniqueId id(int row) const;
    int row(const QUniqueId &) const;

    bool nextAlarm(QDateTime &when, QUniqueId &) const;

    bool updateExtraTables(const QByteArray &, const QPimRecord &);
    bool insertExtraTables(const QByteArray &, const QPimRecord &);
    bool removeExtraTables(const QByteArray &);

    /* subclased from sql io */
    void bindFields(const QPimRecord &r, QSqlQuery &) const;

    QStringList sortColumns() const;

    // forces re-read of data and view reset.
    void refresh();

    void checkAdded(const QAppointment &);
    void checkRemoved(const QUniqueId &);
    void checkRemoved(const QList<QUniqueId> &);
    void checkUpdated(const QAppointment &);
private:
    void invalidateCache();

    QStringList currentFilters() const;

    mutable bool appointmentByRowValid;
    mutable QAppointment lastAppointment;
    mutable QDateTime mAlarmStart;

    QDateTime rStart;
    QDateTime rEnd;
    QAppointmentModel::DurationType rType;

    const QString mainTable;
    const QString catTable;
    const QString customTable;
    const QString exceptionTable;
};

#endif
