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
#include <qtopiasql.h>
#include "qappointmentsqlio_p.h"
#include <QString>

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QTimer>

#include <qtopianamespace.h>
#include <qtopiaipcenvelope.h>
#ifdef Q_OS_WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

static const char *contextsource = "default";
/*
data fields

description location start end allday starttimezone alarm alarmdelay repeatrule repeatfrequency repeatenddate repeatweekflags

:d, :l, :s, :e, :a, :stz, :n, :at, :ad, :rr, :rf, :re, :rw

*/

QAppointmentSqlIO::QAppointmentSqlIO(QObject *parent)
    : QAppointmentIO(parent),
    QPimSqlIO( contextId(),
            "appointments", "appointmentcategories", "appointmentcustom",
            "description = :d, location = :l, start = :s, end = :e, "
            "allday = :a, starttimezone = :stz, "
            "alarm = :at, alarmdelay = :ad, "
            "repeatrule = :rr, repeatfrequency = :rf, "
            "repeatenddate = :re, "
            "repeatweekflags = :rw",
            "(recid, context, description, location, start, end, allday, "
            "starttimezone, alarm, alarmdelay, repeatrule, "
            "repeatfrequency, repeatenddate, "
            "repeatweekflags) VALUES (:i, :context, :d, :l, :s, :e, :a, :stz, "
            ":at, :ad, :rr, :rf, :re, :rw)"),
            appointmentByRowValid(false),
            rType(QAppointmentModel::AnyDuration),
            mainTable("appointments"),
            catTable("appointmentcategories"),
            customTable("appointmentcustom"),
            exceptionTable("appointmentexceptions")
{
    QStringList tables;
    tables << "sqlsources";
    tables << "changelog";
    tables << "categories";
    tables << mainTable;
    tables << catTable;
    tables << customTable;
    tables << exceptionTable;
    QtopiaSql::ensureSchema(tables, QtopiaSql::systemDatabase());
    QStringList sort;
    sort << "start";
    sort << "end";
    sort << "repeatenddate";
    QPimSqlIO::setOrderBy(sort);
}


QAppointmentSqlIO::~QAppointmentSqlIO()
{
}

bool QAppointmentSqlIO::removeAppointment(const QUniqueId &id)
{
    if (id.isNull())
        return false;

    QAppointment original = appointment(id);
    if (original.isValid() && original.hasAlarm())
        removeAlarm(original);

    if (QPimSqlIO::removeRecord(id)) {
        notifyRemoved(id);
        invalidateCache();
        emit recordsUpdated();
        return true;
    }
    return false;
}

bool QAppointmentSqlIO::removeAppointment(int row)
{
    return removeAppointment(recordId(row));
}

bool QAppointmentSqlIO::updateAppointment(const QAppointment& appointment)
{
    QAppointment original = this->appointment(appointment.uid());
    if (original.isValid() && original.hasAlarm())
        removeAlarm(original);

    if (QPimSqlIO::updateRecord(appointment)) {
        addAlarm(appointment);

        notifyUpdated(appointment);
        invalidateCache();
        emit recordsUpdated();
        return true;
    }
    return false;
}

void QAppointmentSqlIO::refresh()
{
    invalidateCache();
    emit recordsUpdated();
}

void QAppointmentSqlIO::checkAdded(const QAppointment &) { refresh(); }
void QAppointmentSqlIO::checkRemoved(const QUniqueId &) { refresh(); }
void QAppointmentSqlIO::checkRemoved(const QList<QUniqueId> &) { refresh(); }
void QAppointmentSqlIO::checkUpdated(const QAppointment &) { refresh(); }

QUniqueId QAppointmentSqlIO::addAppointment(const QAppointment& appointment, const QPimSource &source, bool createuid)
{
    QPimSource s;
    s.identity = contextsource;
    s.context = contextId();
    QUniqueId i = addRecord(appointment, source.isNull() ? s : source, createuid);
    if (!i.isNull()) {
        QAppointment added = this->appointment(i);
        if (added.isValid() && added.hasAlarm())
            addAlarm(added);

	notifyAdded(added);
        invalidateCache();
        emit recordsUpdated();
    }
    return i;
}

bool QAppointmentSqlIO::updateExtraTables(const QByteArray &, const QPimRecord &)
{
    return true; // not actually adding exceptions directly at the moment.
}

bool QAppointmentSqlIO::insertExtraTables(const QByteArray &, const QPimRecord &)
{
    return true; // not actually adding exceptions directly at the moment.
}

bool QAppointmentSqlIO::removeExtraTables(const QByteArray &uid)
{
    QSqlQuery q;
    if (!q.prepare("DELETE FROM " + exceptionTable + " WHERE recid=:ida OR alternateid=:idb")) {
        qWarning("Failed to prepare query %s: %s",
                q.lastQuery().toLocal8Bit().constData(), q.lastError().text().toLocal8Bit().constData());
    }
    q.bindValue(":ida", uid);
    q.bindValue(":idb", uid);
    if (!q.exec()) {
        qWarning("Failed to remove exceptional events for  record: %s",
                q.lastError().text().toLocal8Bit().constData());
        return false;
    }
    return true;
}

bool QAppointmentSqlIO::removeOccurrence(const QUniqueId &original,
        const QDate &date)
{
    const QLocalUniqueId &lid = (const QLocalUniqueId &)original;

    // TODO should so some checking to make sure request is reasonable.

    QSqlDatabase::database().transaction();

    QSqlQuery q;
    if( !q.prepare("INSERT INTO " + exceptionTable + " (recid, edate) VALUES (:i, :ed)") )
        return false;

    q.bindValue(":i", lid.toByteArray());
    q.bindValue(":ed", date);
    if( !q.exec() || !QSqlDatabase::database().commit() )
    {
        QSqlDatabase::database().rollback();
        return false;
    }

    invalidateCache();
    emit recordsUpdated();
    return true;
}

QUniqueId QAppointmentSqlIO::replaceOccurrence(const QUniqueId &original,
        const QOccurrence &occurrence)
{
    // TODO should do some checking to make sure request is reasonable.

    const QLocalUniqueId &lid = (const QLocalUniqueId &)original;

    QDate date = occurrence.date();
    QAppointment a = occurrence.appointment();
    a.setRepeatRule(QAppointment::NoRepeat);
    int c = context(original);

    QLocalUniqueId u = addRecord(a, c, true);
    if (!u.isNull()) {
        QAppointment added = appointment(u);
        if (added.isValid() && added.hasAlarm())
            addAlarm(added);

        notifyAdded(added);
        invalidateCache();
        emit recordsUpdated();
    }

    //  Begin transaction after add, because SQLite does not support nested transactions
    QSqlDatabase::database().transaction();

    QSqlQuery q;
    q.prepare("INSERT INTO " + exceptionTable + " (recid, edate, alternateid)"
            " VALUES (:i, :ed, :aid)");
    q.bindValue(":i", lid.toByteArray());
    q.bindValue(":ed", date);
    q.bindValue(":aid", u.toByteArray());

    QtopiaSql::logQuery(q);
    if( !q.exec() )
    {
        qWarning() << "Failed to add exception: " << q.lastError().text();
        QSqlDatabase::database().rollback();
        return QUniqueId();
    }

    if( !QSqlDatabase::database().commit() )
    {
        qWarning() << "Failed to commit changes: " << QSqlDatabase::database().lastError().text();
        QSqlDatabase::database().rollback();
        return QUniqueId();
    }

    invalidateCache();
    emit recordsUpdated();
    return u;
}

QUniqueId QAppointmentSqlIO::replaceRemaining(const QUniqueId &original,
        const QAppointment &remaining)
{
    // TODO check sanity of using remaining as a alternate end to original.
    // also make sure that exceptions transferred to the replacement are sane for possible change in repeat days

    QAppointment ao = appointment(original);

    ao.setRepeatUntil(remaining.start().date().addDays(-1));
    updateAppointment(ao);
    int c = context(original);
    QUniqueId u = addRecord(remaining, c, true);
    if (!u.isNull()) {
        QAppointment added = appointment(u);
        if (added.isValid() && added.hasAlarm())
            addAlarm(added);

        notifyAdded(added);
        invalidateCache();
        emit recordsUpdated();
    }

    //  Begin transaction after add, because SQLite does not support nested transactions
    QSqlDatabase::database().transaction();

    //  Transfer any exceptions after the switchover date to the replacement appointment
    QSqlQuery q;
    q.prepare("UPDATE " + exceptionTable + " SET recid=:newid WHERE recid=:oldid AND edate >= :d");

    const QLocalUniqueId &newlid = (const QLocalUniqueId &)u;
    QByteArray newuid = newlid.toByteArray();
    q.bindValue(":newid", newuid);

    const QLocalUniqueId &oldlid = (const QLocalUniqueId &)original;
    QByteArray olduid = oldlid.toByteArray();
    q.bindValue(":oldid", olduid);

    q.bindValue(":d", remaining.start().date());

    QtopiaSql::logQuery(q);
    if (!q.exec())
    {
        qWarning() << "Failed to transfer exceptions: " << q.lastError().text();
        QSqlDatabase::database().rollback();
        return QUniqueId();
    }

    if( !QSqlDatabase::database().commit() )
    {
        qWarning() << "Failed to commit changes: " << QSqlDatabase::database().lastError().text();
        QSqlDatabase::database().rollback();
        return QUniqueId();
    }

    invalidateCache();
    emit recordsUpdated();
    return u;
}

QUuid QAppointmentSqlIO::contextId() const
{
    // generated with uuidgen
    static QUuid u("5ecdd517-9aed-4e4b-b248-1970c56eb49a");
    return u;
}

void QAppointmentSqlIO::setCategoryFilter(const QCategoryFilter &f)
{
    if (f != categoryFilter()) {
        QPimSqlIO::setCategoryFilter(f);
        invalidateCache();
        emit recordsUpdated();
    }
}

QCategoryFilter QAppointmentSqlIO::categoryFilter() const
{
    return QPimSqlIO::categoryFilter();
}

void QAppointmentSqlIO::setRangeFilter(const QDateTime &earliest, const QDateTime &latest)
{
    if (rStart == earliest && rEnd == latest)
        return;
    rStart = earliest;
    rEnd = latest;
    setFilters(currentFilters());
    invalidateCache();
    emit recordsUpdated();
}

QStringList QAppointmentSqlIO::currentFilters() const
{
    QStringList f;
    if (rStart.isValid())
        f.append("((repeatenddate IS NULL OR repeatenddate >= '" + rStart.date().toString(Qt::ISODate)
            + "') OR end >= '" + rStart.toString(Qt::ISODate) + "')");

    if (rEnd.isValid())
        f.append("start < '" + rEnd.toString(Qt::ISODate) + "'");

    switch(rType){
        case QAppointmentModel::AnyDuration:
            break;
        case QAppointmentModel::AllDayDuration:
            f.append("allday = 'true'");
            break;
        case QAppointmentModel::TimedDuration:
            f.append("allday = 'false'");
            break;
    }
    return f;
}

void QAppointmentSqlIO::setDurationType(QAppointmentModel::DurationType type)
{
    if (type == rType)
        return;
    rType = type;
    setFilters(currentFilters());
    invalidateCache();
    emit recordsUpdated();
}

void QAppointmentSqlIO::setContextFilter(const QSet<int> &list)
{
    if (list != contextFilter()) {
        QPimSqlIO::setContextFilter(list, ExcludeContexts);
        invalidateCache();
        emit recordsUpdated();
    }
}

QSet<int> QAppointmentSqlIO::contextFilter() const
{
    return QPimSqlIO::contextFilter();
}

QAppointment QAppointmentSqlIO::appointment(const QUniqueId &u) const
{
    if (appointmentByRowValid && u == lastAppointment.uid())
        return lastAppointment;


    QSqlQuery q;
    q.prepare("SELECT recid, description, location, start, end, allday, "
            "starttimezone, alarm, alarmdelay, repeatrule, "
            "repeatfrequency, repeatenddate, repeatweekflags "
            "FROM " + mainTable + " WHERE recid = :i");
    const QLocalUniqueId &lid = (const QLocalUniqueId &)u;
    QByteArray uid = lid.toByteArray();
    q.bindValue(":i", uid);

    QAppointment t;
    retrieveRecord(uid, t);
    q.setForwardOnly(true);

    QtopiaSql::logQuery(q);
    if (!q.exec()) {
        appointmentByRowValid = false;
        qWarning("failed to select appointment: %s", (const char *)q.lastError().text().toLocal8Bit());
        return t;
    }

    if ( q.next() ) {
        // categories for this appointment;
        QSqlQuery q2;
        q2.prepare("SELECT edate, alternateid FROM " + exceptionTable + " WHERE recid=:id");
        q2.bindValue(":id", uid);
        QtopiaSql::logQuery(q2);
        if (!q2.exec())
            qWarning("select appointment exceptions failed: %s",
                    (const char *)q2.lastError().text().toLocal8Bit());
        QList<QAppointment::Exception> elist;
        while(q2.next()) {
            QAppointment::Exception ae;
            ae.date = q2.value(0).toDate();
            if (!q2.value(1).isNull())
                ae.alternative = QUniqueId(q2.value(1).toByteArray());
            elist.append(ae);
        }
        t.setExceptions(elist);

        q2.prepare("SELECT recid FROM " + exceptionTable + " WHERE alternateid=:id");
        q2.bindValue(":id", uid);
        QtopiaSql::logQuery(q2);
        if (!q2.exec())
            qWarning("check for exception parent failed: %s",
                        (const char *)q2.lastError().text().toLocal8Bit());
        if(q2.next())
            t.setExceptionParent(QUniqueId(q2.value(0).toByteArray()));

        t.setUid(QUniqueId(q.value(0).toByteArray()));
        t.setDescription(q.value(1).toString());
        t.setLocation(q.value(2).toString());
        t.setStart(q.value(3).toDateTime());
        t.setEnd(q.value(4).toDateTime());
        t.setAllDay(q.value(5).toBool());
        t.setTimeZone(QTimeZone(q.value(6).toString().toLocal8Bit().constData()));

        QAppointment::AlarmFlags af = (QAppointment::AlarmFlags)q.value(7).toInt();
        if (af != QAppointment::NoAlarm)
            t.setAlarm(q.value(8).toInt(), af);

        t.setRepeatRule((QAppointment::RepeatRule)q.value(9).toInt());
        t.setFrequency(q.value(10).toInt());
        t.setRepeatUntil(q.value(11).toDate());
        t.setWeekFlags((QAppointment::WeekFlags)q.value(12).toInt());
        lastAppointment = t;
        appointmentByRowValid = true;

        // custom fields for this appointment
        q2.prepare("SELECT fieldname, fieldvalue from " + customTable + " where recid=:id");
        q2.bindValue(":id", uid);
        QtopiaSql::logQuery(q2);
        if (!q2.exec()) {
            qWarning("select fieldname, fieldvalue failed: %s",
                     (const char *)q2.lastError().text().toLocal8Bit());
        }
        QMap<QString, QString> tMap;
        while (q2.next())
            t.setCustomField(q2.value(0).toString(), q2.value(1).toString());

    } else {
        appointmentByRowValid = false;
        qWarning("Did not find selected appointment: %s", u.toString().toLocal8Bit().constData());
    }
    return t;
}

QAppointment QAppointmentSqlIO::appointment(int row) const
{
    return appointment(recordId(row));
}

// needs to be comparable?
QVariant QAppointmentSqlIO::key(int row) const
{
    return appointment(row).start();
}

QVariant QAppointmentSqlIO::key(const QUniqueId &id) const
{
    const QLocalUniqueId &lid = (const QLocalUniqueId &)id;
    QSqlQuery q;
    if (!q.prepare("SELECT start FROM appointments WHERE recid = :id"))
        qWarning("select record key failed: %s", (const char *)q.lastError().text().toLocal8Bit());
    q.bindValue(":id", lid.toByteArray());

    if (q.exec() && q.next())
        return q.value(0);
    return QVariant();
}

QUniqueId QAppointmentSqlIO::id(int row) const
{
    return QPimSqlIO::recordId(row);
}

int QAppointmentSqlIO::row(const QUniqueId &id) const
{
    return QPimSqlIO::row(id);
}

bool QAppointmentSqlIO::nextAlarm(QDateTime &when, QUniqueId &) const
{
    mAlarmStart = when;
    return false;
}

void QAppointmentSqlIO::bindFields(const QPimRecord &r, QSqlQuery &query) const
{
    const QAppointment &a = (const QAppointment &)r;
    query.bindValue(":d", a.description());
    query.bindValue(":l", a.location());
    query.bindValue(":s", a.start());
    query.bindValue(":e", a.end());
    query.bindValue(":a", a.isAllDay());
    query.bindValue(":stz", a.timeZone().id());
    query.bindValue(":at", (int)a.alarm());
    query.bindValue(":ad", a.alarmDelay());
    query.bindValue(":rr", (int)a.repeatRule());
    query.bindValue(":rf", a.frequency());
    query.bindValue(":re", a.repeatUntil());
    query.bindValue(":rw", (int)a.weekFlags());
}

QStringList QAppointmentSqlIO::sortColumns() const
{
    return QStringList() << "start";
}

void QAppointmentSqlIO::invalidateCache()
{
    QPimSqlIO::invalidateCache();
    appointmentByRowValid = false;
}

/***************
 * CONTEXT
 **************/
QAppointmentDefaultContext::QAppointmentDefaultContext(QObject *parent, QObject *access)
    : QAppointmentContext(parent)
{
    mAccess = qobject_cast<QAppointmentSqlIO *>(access);
    Q_ASSERT(mAccess);
}

QIcon QAppointmentDefaultContext::icon() const
{
    return QPimContext::icon(); // redundent, but will do for now.
}

QString QAppointmentDefaultContext::description() const
{
    return tr("Default appointment storage");
}

QString QAppointmentDefaultContext::title() const
{
    return tr("Appointments");
}

bool QAppointmentDefaultContext::editable() const
{
    return true;
}

QSet<QPimSource> QAppointmentDefaultContext::sources() const
{
    QSet<QPimSource> list;
    list.insert(defaultSource());
    return list;
}

QUuid QAppointmentDefaultContext::id() const
{
    return mAccess->contextId();
}

void QAppointmentDefaultContext::setVisibleSources(const QSet<QPimSource> &set)
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = mAccess->contextFilter();

    if (set.contains(defaultSource()))
        filter.remove(context);
    else
        filter.insert(context);

    mAccess->setContextFilter(filter);
}

QSet<QPimSource> QAppointmentDefaultContext::visibleSources() const
{
    int context = QPimSqlIO::sourceContext(defaultSource());

    QSet<int> filter = mAccess->contextFilter();
    if (!filter.contains(context))
        return sources();
    return QSet<QPimSource>();
}

bool QAppointmentDefaultContext::exists(const QUniqueId &id) const
{
    int context = QPimSqlIO::sourceContext(defaultSource());
    return mAccess->exists(id) && mAccess->context(id) == context;
}

QPimSource QAppointmentDefaultContext::defaultSource() const
{
    QPimSource s;
    s.context = mAccess->contextId();
    s.identity = contextsource;
    return s;
}

QPimSource QAppointmentDefaultContext::source(const QUniqueId &id) const
{
    if (exists(id))
        return defaultSource();
    return QPimSource();
}

bool QAppointmentDefaultContext::updateAppointment(const QAppointment &appointment)
{
    return mAccess->updateAppointment(appointment);
}

bool QAppointmentDefaultContext::removeAppointment(const QUniqueId &id)
{
    return mAccess->removeAppointment(id);
}

QUniqueId QAppointmentDefaultContext::addAppointment(const QAppointment &appointment, const QPimSource &source)
{
    if (source.isNull() || source == defaultSource())
        return mAccess->addAppointment(appointment, defaultSource());
    return QUniqueId();
}

bool QAppointmentDefaultContext::removeOccurrence(const QUniqueId &original, const QDate &date)
{
    return mAccess->removeOccurrence(original, date);
}

QUniqueId QAppointmentDefaultContext::replaceOccurrence(const QUniqueId &original, const QOccurrence &occurrence)
{
    return mAccess->replaceOccurrence(original, occurrence);
}

QUniqueId QAppointmentDefaultContext::replaceRemaining(const QUniqueId &original, const QAppointment &r)
{
    return mAccess->replaceRemaining(original, r);
}
