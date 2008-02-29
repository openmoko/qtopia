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

#ifndef PIMSOURCE_H
#define PIMSOURCE_H

#include <QUuid>
#include <QString>
#include <QSet>
#include <QUniqueId>

#include <QList>
#include <QContact>
#include <QTask>
#include <QAppointment>

#include <qtopiaipcmarshal.h>

struct QTOPIAPIM_EXPORT QPimSource
{
    QUuid context;
    QString identity;
    bool operator==(const QPimSource &other) const
    { return context == other.context && identity == other.identity; }
    bool operator!=(const QPimSource &other) const
    { return context != other.context || identity != other.identity; }
    bool operator<(const QPimSource &other) const
    {
        if (context < other.context)
            return true;
        return identity < other.identity;
    }
    bool isNull() const
    { return context.isNull() && identity.isNull(); }

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
};

Q_DECLARE_USER_METATYPE(QPimSource)

uint qHash(const QPimSource &s);

class QTOPIAPIM_EXPORT QPimContext : public QObject
{
    Q_OBJECT
public:
    virtual QIcon icon() const; // default empty
    virtual QString description() const = 0;
    virtual QString title() const = 0;
    virtual QString title(const QPimSource &) const { return title(); }

    // better to be flags ?
    virtual bool editable() const; // default true
    virtual bool editable(const QUniqueId &) const { return editable(); } // default true

    virtual void setVisibleSources(const QSet<QPimSource> &) {}
    virtual QSet<QPimSource> visibleSources() const { return sources(); }
    virtual QSet<QPimSource> sources() const = 0;
    virtual QUuid id() const = 0;

    virtual bool exists(const QUniqueId &) const = 0;
    virtual bool exists(const QUniqueId &id, const QPimSource &source)
    {
        Q_UNUSED(source);
        return exists(id);
    }
    virtual QPimSource source(const QUniqueId &) const = 0;

    // assumption is that the record is of they type for the pim contact...

protected:
    explicit QPimContext(QObject *parent = 0);
};

class QContact;
class QAppointment;
class QOccurrence;
class QTask;

class QTOPIAPIM_EXPORT QContactContext : public QPimContext
{
public:
    virtual bool updateContact(const QContact &) = 0;
    virtual bool removeContact(const QUniqueId &) = 0;
    virtual QUniqueId addContact(const QContact &, const QPimSource &) = 0;

    virtual QList<QContact> exportContacts(const QPimSource &, bool &ok) const { ok = false; return QList<QContact>(); }
    virtual bool importContacts(const QPimSource &, const QList<QContact> &) { return false; }

    virtual QContact exportContact(const QUniqueId &, bool &ok) const { ok = false; return QContact(); }
    virtual bool importContact(const QPimSource &s, const QContact &c)
    { QList<QContact> l; l << c; return importContacts(s, l); }

protected:
    explicit QContactContext(QObject *parent = 0)
        : QPimContext(parent) {}
};

class QTOPIAPIM_EXPORT QAppointmentContext : public QPimContext
{
public:
    virtual bool updateAppointment(const QAppointment &) = 0;
    virtual bool removeAppointment(const QUniqueId &) = 0;
    virtual QUniqueId addAppointment(const QAppointment &, const QPimSource &) = 0;

    virtual bool removeOccurrence(const QUniqueId &original, const QDate &) = 0;
    virtual QUniqueId replaceOccurrence(const QUniqueId &original, const QOccurrence &) = 0;
    virtual QUniqueId replaceRemaining(const QUniqueId &original, const QAppointment &) = 0;

    virtual QList<QAppointment> exportAppointments(const QPimSource &, bool &ok) const { ok = false; return QList<QAppointment>(); }
    virtual bool importAppointments(const QPimSource &, const QList<QAppointment> &) { return false; }

    virtual QAppointment exportAppointment(const QUniqueId &, bool &ok) const { ok = false; return QAppointment(); }
    virtual bool importAppointment(const QPimSource &s, const QAppointment &c)
    { QList<QAppointment> l; l << c; return importAppointments(s, l); }

protected:
    explicit QAppointmentContext(QObject *parent = 0)
        : QPimContext(parent) {}
};

class QTOPIAPIM_EXPORT QTaskContext : public QPimContext
{
public:
    virtual bool updateTask(const QTask &) = 0;
    virtual bool removeTask(const QUniqueId &) = 0;
    virtual QUniqueId addTask(const QTask &, const QPimSource &) = 0;

    virtual QList<QTask> exportTasks(const QPimSource &, bool &ok) const { ok = false; return QList<QTask>(); }
    virtual bool importTasks(const QPimSource &, const QList<QTask> &) { return false; }

    virtual QTask exportTask(const QUniqueId &, bool &ok) const { ok = false; return QTask(); }
    virtual bool importTask(const QPimSource &s, const QTask &c)
    { QList<QTask> l; l << c; return importTasks(s, l); }
protected:
    explicit QTaskContext(QObject *parent = 0)
        : QPimContext(parent) {}
};

#endif // PIMSOURCE_H
