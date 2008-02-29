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
#include <QDateTime>
#include <QString>
#include <QByteArray>

#include <QContactModel>
#include <QTaskModel>
#include <QAppointmentModel>
#include <QCategoryManager>
#include <QSqlQuery>
#include <QSqlError>
#include <QtopiaSql>
#include <qtopialog.h>
QTOPIA_LOG_OPTION(QPimSync)

#include "qpimsyncstorage.h"

QPimSyncStorage::QPimSyncStorage(QObject *parent) : QObject(parent)
{
}

void QPimSyncStorage::setModel(QPimModel *model)
{
    m = model;
    // set editable sources only for model
    QSet<QPimSource> sources = m->availableSources();
    QMutableSetIterator<QPimSource> it(sources);
    while(it.hasNext()) {
        QPimSource s = it.next();
        if (!m->context(s)->editable()) 
            it.remove();
    }
    m->setVisibleSources(sources);
}

QPimSyncStorage::~QPimSyncStorage(){}

bool QPimSyncStorage::startSyncTransaction(const QDateTime &time)
{
    mUnmappedCategories.clear();
    return m->startSyncTransaction(time);
}

bool QPimSyncStorage::abortSyncTransaction()
{
    return m->abortSyncTransaction();
}

bool QPimSyncStorage::commitSyncTransaction()
{
    return m->commitSyncTransaction();
}

// appointments
QAppointmentSyncStorage::QAppointmentSyncStorage()
    : QPimSyncStorage(0), model(new QAppointmentModel(this))
{
    setModel(model);
}

QAppointmentSyncStorage::~QAppointmentSyncStorage()
{
}

bool QAppointmentSyncStorage::commitSyncTransaction()
{
    if (QPimSyncStorage::commitSyncTransaction()) {
        QCategoryManager c("Calendar");
        foreach(const QString &category, unmappedCategories()) {
            QString id = c.add(category);
            if (id != category) {
                QSqlQuery q(QtopiaSql::instance()->systemDatabase());
                if (!q.prepare("UPDATE appointmentcategories SET categoryid = :i WHERE categoryid = :c"))
                    qWarning() << "Failed to prepare category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();

                q.bindValue(":c", category);
                q.bindValue(":i", id);
                if (!q.exec())
                    qWarning() << "Failed to execute category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();
            }
        }
        return true;
    }
    return false;
}

void QAppointmentSyncStorage::addServerRecord(const QByteArray &record)
{
    QAppointment appointment;
    // exceptions need to be applied after the appointment is added.
    QList<QPimXmlException> exceptions;

    QString serverId;
    QPimXmlStreamReader h(record);
    appointment = h.readAppointment(serverId, exceptions, model);
    if (h.hasError())
        return;

    mergeUnmappedCategories(h.missedLabels());

    QUniqueId clientId = model->addAppointment(appointment);
    appointment.setUid(clientId);

    emit mappedId(serverId, clientId.toString());

    foreach(const QPimXmlException &e, exceptions) {
        if (e.replacement) {
            QOccurrence o(e.originalDate, e.appointment);
            QUniqueId eId = model->replaceOccurrence(appointment, o);
            emit mappedId(e.serverId, eId.toString());
        } else {
            model->removeOccurrence(appointment, e.originalDate);
        }
    }
}

static bool exists(const QList<QAppointment::Exception> &list, const QPimXmlException &e)
{
    foreach(const QAppointment::Exception &o, list) {
        if (e.replacement && e.appointment.uid() == o.alternative)
            return true;
        if (!e.replacement && e.originalDate == o.date)
            return true;
    }
    return false;
}

static bool exists(const QList<QPimXmlException> &list, const QAppointment::Exception &o)
{
    foreach(const QPimXmlException &e, list) {
        if (e.replacement && e.appointment.uid() == o.alternative)
            return true;
        if (!e.replacement && e.originalDate == o.date)
            return true;
    }
    return false;
}

void QAppointmentSyncStorage::replaceServerRecord(const QByteArray &record)
{
    QAppointment appointment;
    // exceptions need to be applied after the appointment is added.
    QList<QPimXmlException> exceptions;

    QString serverId;
    QPimXmlStreamReader h(record);
    appointment = h.readAppointment(serverId, exceptions, model);
    if (h.hasError())
        return;

    mergeUnmappedCategories(h.missedLabels());

    // update all exceptions as well.
    model->updateAppointment(appointment);
    // refresh to get exception info from db.
    appointment = model->appointment(appointment.uid());

    QList<QAppointment::Exception> origList = appointment.exceptions();
    foreach(const QPimXmlException &e, exceptions) {
        if (!exists(origList, e)) {
            if (e.replacement) {
                QOccurrence o(e.originalDate, e.appointment);
                model->replaceOccurrence(appointment, o);
            } else 
                model->removeOccurrence(appointment, e.originalDate);
        } else if (e.replacement) {
            model->updateAppointment(e.appointment);
        }
    }
    foreach(const QAppointment::Exception &oe, origList) {
        if (!exists(exceptions, oe)) {
            model->restoreOccurrence(appointment.uid(), oe.date);
        }
    }
}

void QAppointmentSyncStorage::removeServerRecord(const QString &localId)
{
    QUniqueId id(localId);
    model->removeAppointment(id);
}

QList<QPimXmlException> QAppointmentSyncStorage::convertExceptions(const QList<QAppointment::Exception> origList) const
{
    QList<QPimXmlException> newList;
    foreach(const QAppointment::Exception &e, origList)
    {
        QPimXmlException n;
        n.originalDate = e.date;
        n.replacement = !e.alternative.isNull();
        if (n.replacement)
            n.appointment = model->appointment(e.alternative);
        newList.append(n);
    }
    return newList;
}

void QAppointmentSyncStorage::performSync(const QDateTime &since)
{
    // if since null, slow sync, send all added.
    // else two way sync, use added, removed and modified.

    QList<QUniqueId> changes = model->added(since);
    QMap<QUniqueId, int> exceptions;

    // created
    foreach(const QUniqueId &id, changes) {
        QAppointment a = model->appointment(id);
        if (a.isException()) {
            if (!exceptions.contains(a.exceptionParent()))
                exceptions.insert(a.exceptionParent(), 0);
            continue;
        } else if (a.hasExceptions()) {
            exceptions.insert(a.uid(), 1);
        }

        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeAppointment(a, convertExceptions(a.exceptions()));
        emit addClientRecord(record);
    }
    if (since.isNull()) {
        // slow sync, modified and removed make no sense, create for
        // exceptions would have been caught anyway;
        return;
    }

    changes = model->removed(since);

    // removed. (removing can lead to modified)
    foreach(const QUniqueId &id, changes) {
        QAppointment a = model->appointment(id);
        if (a.isException()) {
            if (!exceptions.contains(a.exceptionParent()))
                exceptions.insert(a.exceptionParent(), 0);
            continue;
        } else if (a.hasExceptions()) {
            exceptions.insert(a.uid(), 3);
        }
        emit removeClientRecord(id.toString());
    }

    // modified
    changes = model->modified(since);
    foreach(const QUniqueId &id, changes) {
        QAppointment a = model->appointment(id);
        if (a.isException()) {
            if (!exceptions.contains(a.exceptionParent()))
                exceptions.insert(a.exceptionParent(), 0);
            continue;
        } else if (a.hasExceptions()) {
            exceptions.insert(a.uid(), 2);
        }
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeAppointment(a, convertExceptions(a.exceptions()));
        emit replaceClientRecord(record);
    }
    // now for the exceptions where the parent not already changed
    QMapIterator<QUniqueId, int> it(exceptions);
    while(it.hasNext()) {
        it.next();
        if (it.value() == 0) {
            // changed exception, parent not already 1(added) 2(modified) or 3(removed);
            QAppointment a = model->appointment(it.key());
            QByteArray record;
            QPimXmlStreamWriter h(&record);
            h.writeAppointment(a, convertExceptions(a.exceptions()));
            emit replaceClientRecord(record);

        }

    }
}

QContactSyncStorage::QContactSyncStorage()
    : QPimSyncStorage(0), model(new QContactModel(this))
{
    setModel(model);
}

QContactSyncStorage::~QContactSyncStorage()
{
}

bool QContactSyncStorage::commitSyncTransaction()
{
    qLog(QPimSync) << "task sync transaction.";
    if (QPimSyncStorage::commitSyncTransaction()) {
        QCategoryManager c("Address Book");
        foreach(const QString &category, unmappedCategories()) {
            QString id = c.add(category);
            qLog(QPimSync) << "add category" << category << "with id" << id;
            if (id != category) {
                QSqlQuery q(QtopiaSql::instance()->systemDatabase());
                if (!q.prepare("UPDATE contactcategories SET categoryid = :i WHERE categoryid = :c"))
                    qWarning() << "Failed to prepare category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();

                q.bindValue(":c", category);
                q.bindValue(":i", id);
                if (!q.exec())
                    qWarning() << "Failed to execute category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();
            }
        }
        return true;
    }
    return false;
}

void QContactSyncStorage::addServerRecord(const QByteArray &record)
{
    QPimXmlStreamReader h(record);
    QString serverId;
    QContact c = h.readContact(serverId, model);
    if (!h.hasError()) {
        c.setUid(model->addContact(c));
        mergeUnmappedCategories(h.missedLabels());
        emit mappedId(serverId, c.uid().toString());
    } else {
        qLog(QPimSync) << "failed to parse:" << int(h.error()) << h.errorString();
    }
}

void QContactSyncStorage::replaceServerRecord(const QByteArray &record)
{
    QPimXmlStreamReader h(record);
    QString serverId;
    QContact c = h.readContact(serverId, model);
    if (!h.hasError()) {
        model->updateContact(c);
        mergeUnmappedCategories(h.missedLabels());
    } else {
        qLog(QPimSync) << "failed to parse:" << int(h.error()) << h.errorString();
    }
}

void QContactSyncStorage::removeServerRecord(const QString &localId)
{
    model->removeContact(QUniqueId(localId));
}

void QContactSyncStorage::performSync(const QDateTime &since)
{
    QList<QUniqueId> changes = model->added(since);
    qLog(QPimSync) << "added" << changes.count();

    foreach(const QUniqueId &id, changes) {
        QContact c = model->contact(id);
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeContact(c);
        emit addClientRecord(record);
    }

    changes = model->removed(since);

    qLog(QPimSync) << "removed" << changes.count();

    foreach(const QUniqueId &id, changes) {
        emit removeClientRecord(id.toString());
    }

    changes = model->modified(since);

    qLog(QPimSync) << "modified" << changes.count();

    foreach(const QUniqueId &id, changes) {
        QContact c = model->contact(id);
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeContact(c);
        emit replaceClientRecord(record);
    }
}

QTaskSyncStorage::QTaskSyncStorage()
    : QPimSyncStorage(0), model(new QTaskModel(this))
{
    setModel(model);
}

QTaskSyncStorage::~QTaskSyncStorage()
{
}

bool QTaskSyncStorage::commitSyncTransaction()
{
    if (QPimSyncStorage::commitSyncTransaction()) {
        QCategoryManager c("Todo List");
        foreach(const QString &category, unmappedCategories()) {
            QString id = c.add(category);
            if (id != category) {
                QSqlQuery q(QtopiaSql::instance()->systemDatabase());
                if (!q.prepare("UPDATE taskcategories SET categoryid = :i WHERE categoryid = :c"))
                    qWarning() << "Failed to prepare category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();

                q.bindValue(":c", category);
                q.bindValue(":i", id);
                if (!q.exec())
                    qWarning() << "Failed to execute category update" << __FILE__ << __LINE__ << ":" << q.lastError().text();
            }
        }
        return true;
    }
    return false;
}

void QTaskSyncStorage::addServerRecord(const QByteArray &record)
{
    QPimXmlStreamReader h(record);
    QString serverId;
    QTask t = h.readTask(serverId, model);
    if (!h.hasError()) {
        t.setUid(model->addTask(t));
        mergeUnmappedCategories(h.missedLabels());
        emit mappedId(serverId, t.uid().toString());
    }
}

void QTaskSyncStorage::replaceServerRecord(const QByteArray &record)
{
    QString serverId;
    QPimXmlStreamReader h(record);
    QTask t = h.readTask(serverId, model);
    if (h.hasError()) {
        model->updateTask(t);
        mergeUnmappedCategories(h.missedLabels());
    }
}

void QTaskSyncStorage::removeServerRecord(const QString &localId)
{
    model->removeTask(QUniqueId(localId));
}

void QTaskSyncStorage::performSync(const QDateTime &since)
{
    QList<QUniqueId> changes = model->added(since);

    foreach(const QUniqueId &id, changes) {
        QTask t = model->task(id);
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeTask(t);
        emit addClientRecord(record);
    }

    changes = model->removed(since);
    foreach(const QUniqueId &id, changes) {
        emit removeClientRecord(id.toString());
    }

    changes = model->modified(since);
    foreach(const QUniqueId &id, changes) {
        QTask t = model->task(id);
        QByteArray record;
        QPimXmlStreamWriter h(&record);
        h.writeTask(t);
        emit replaceClientRecord(record);
    }
}

