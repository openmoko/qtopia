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

#include <qfile.h>
#include <qsettings.h>
#include <qtopianamespace.h>
#include <quuid.h>
#include <qfileinfo.h>
#include <qtopiaipcenvelope.h>
#include <qtopianamespace.h>
#include <qapplication.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include "qappointmentxmlio_p.h"
#include "qappointment.h"

#ifdef Q_OS_WIN32
#include <process.h>
#include <time.h>
#else
#include <unistd.h>
#endif



static const XmlIOField datebookentries[] = {
    { "description", EventXmlIO::Description },
    { "location", EventXmlIO::Location },
    { "timezone", EventXmlIO::StartTimeZone },
    { "note", EventXmlIO::Notes },
    { "start", EventXmlIO::StartDateTime },
    { "end", EventXmlIO::EndDateTime },
    { "type", EventXmlIO::DatebookType },
    { "alarm", EventXmlIO::AlarmDelay },
    { "balarm", EventXmlIO::HasAlarm },
    { "sound", EventXmlIO::SoundType },

    { "rtype", EventXmlIO::RepeatPattern },
    { "rfreq", EventXmlIO::RepeatFrequency },
    { "rweekdays", EventXmlIO::RepeatWeekdays },
    { "rhasenddate", EventXmlIO::RepeatHasEndDate },
    { "enddt", EventXmlIO::RepeatEndDate },

    { "recparent", EventXmlIO::RecordParent },
    { "recchildren", EventXmlIO::RecordChildren },
    { "exceptions", EventXmlIO::Exceptions },

    { 0, 0 }
};

// from time_t to QDateTime, but no timezone conversion.
QDateTime asQDateTime( time_t time )
{
    struct tm *lt;
#if defined(Q_WS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif
    lt = gmtime( &time );
    QDateTime dt(
            QDate( lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday ),
            QTime( lt->tm_hour, lt->tm_min, lt->tm_sec ) );
    return dt;
}

time_t asTimeT( const QDateTime& dt )
{
    time_t tmp;
    struct tm *lt;

#if defined(Q_WS_WIN32) || defined (Q_OS_WIN64)
    _tzset();
#else
    tzset();
#endif

    // get a tm structure from the system to get the correct tz_name
    tmp = time( 0 );
    lt = gmtime( &tmp );

    lt->tm_sec = dt.time().second();
    lt->tm_min = dt.time().minute();
    lt->tm_hour = dt.time().hour();
    lt->tm_mday = dt.date().day();
    lt->tm_mon = dt.date().month() - 1; // 0-11 instead of 1-12
    lt->tm_year = dt.date().year() - 1900; // year - 1900
    //lt->tm_wday = dt.date().dayOfWeek(); ignored anyway
    //lt->tm_yday = dt.date().dayOfYear(); ignored anyway
    lt->tm_wday = -1;
    lt->tm_yday = -1;
    // tm_isdst negative -> mktime will find out about DST
    lt->tm_isdst = -1;
    // keep tm_zone and tm_gmtoff
    tmp = mktime( lt );
    return tmp;
}

EventXmlIO::EventXmlIO(AccessMode m,
                     const QString &file,
                     const QString &journal )
    : EventIO(m), QPimXmlIO(), needsSave(false)
{
    if ( file != QString() )
        setDataFilename( file );
    else setDataFilename( Qtopia::applicationFileName( "datebook", "datebook.xml" ) );
    if ( journal != QString() )
        setJournalFilename( journal );
    else setJournalFilename( Qtopia::applicationFileName( "datebook", "datebook.journal" ) );

    loadData();

    if (m == ReadOnly) {
        QtopiaChannel *channel = new QtopiaChannel( "QPE/PIM",  this );

        connect( channel, SIGNAL(received(const QString&,const QByteArray&)),
                this, SLOT(pimMessage(const QString&,const QByteArray&)) );
    }
}

void EventXmlIO::pimMessage(const QString &message, const QByteArray &data)
{
    QDataStream ds(data);
    if (message == "addedEvent(int,QAppointment)") {
        int pid;
        QAppointment event;
        ds >> pid;
        ds >> event;
        if (pid != getpid()) {
            internalAddRecord(new QAppointment(event));
            emit eventsUpdated();
        }
    } else if (message == "removedEvent(int,QAppointment)") {
        int pid;
        QAppointment event;
        ds >> pid;
        ds >> event;
        if (pid != getpid()) {
            internalRemoveRecord(new QAppointment(event));
            emit eventsUpdated();
        }
    } else if (message == "updatedEvent(int,QAppointment)") {
        int pid;
        QAppointment event;
        ds >> pid;
        ds >> event;
        if (pid != getpid()) {
            internalUpdateRecord( new QAppointment(event) );
            emit eventsUpdated();
        }
    } else if (message == "reloadEvents()") {
        ensureDataCurrent();
    } else if ( message == "reload(int)" ) {
        int force;
        ds >> force;
        ensureDataCurrent(force);
    }
}

EventXmlIO::~EventXmlIO() {
}

bool EventXmlIO::loadData()
{
    if (QPimXmlIO::loadData()) {
        emit eventsUpdated();
        return true;
    }
    return false;
}

bool EventXmlIO::internalAddRecord(QPimRecord *rec)
{
    QAppointment *ev = (QAppointment *)rec;

    m_QAppointments.append( ev );
    return true;
}

bool EventXmlIO::internalRemoveRecord(QPimRecord *rec)
{
    QAppointment *ev = (QAppointment *)rec;

    QMutableListIterator<QAppointment*> it(m_QAppointments);
    while(it.hasNext()) {
        QAppointment *current = it.next();
        if (current->uid() == ev->uid()) {
            it.remove();
            delete ev;
            return true;
        }
    }
    delete ev;
    return false;
}

bool EventXmlIO::internalUpdateRecord(QPimRecord *rec)
{
    QAppointment *ev = (QAppointment *)rec;

    QMutableListIterator<QAppointment*> it(m_QAppointments);
    while(it.hasNext()) {
        QAppointment *current = it.next();
        if (current->uid() == ev->uid()) {
            if ( current != ev ) {
                *current = *ev;
            }
            delete ev;
            return true;
        }
    }
    delete ev;
    return false;
}

bool EventXmlIO::saveData()
{
    if ( !QFile::exists( dataFilename() ) || QFile::exists( journalFilename() ) )
        needsSave = true;

    if (!needsSave)
        return true;

    if (accessMode() != ReadOnly ) {
        if (QPimXmlIO::saveData((QList<QPimRecord*> &)m_QAppointments)) {
            needsSave = false;
            return true;
        }
    }
    return false;
}

void EventXmlIO::setFields(QPimRecord *r, const QMap<QString,QString> &recMap) const
{
    QAppointment *rec = (QAppointment *)r;
    QMap<QString,QString> m = recMap;
    time_t startUtc = 0;
    time_t endUtc = 0;
    time_t endDateUtc = 0;
    bool hasTimeZone = false;

    bool mHasAlarm = false;
    int mAlarmDelay = 0;
    QAppointment::SoundTypeChoice mAlarmSound;

    QMap<QString, QString>::Iterator it = m.begin();
    while (it != m.end()) {
        QString a = it.key();
        QString value = it.value();
        const XmlIOField *t = datebookentries;
        int key = 0;
        while (t->fieldId) {
            if (a == t->text) {
                key = t->fieldId;
                break;
            }
            ++t;
        }
        switch( key ) {
            case Description:
                rec->setDescription( value );
                break;
            case Location:
                rec->setLocation( value );
                break;
            case StartTimeZone:
                if (value != "None") // No tr
                    rec->setTimeZone( QTimeZone(value.toLocal8Bit()) );
                // hasTimeZone in that one is set, even if it is none.
                hasTimeZone = true;
                break;
            case Notes:
                rec->setNotes( value );
                break;
            case StartDateTime:
                startUtc = (time_t) value.toLong();
                break;
            case EndDateTime:
                endUtc = (time_t) value.toLong();
                break;
            case DatebookType:
                rec->setAllDay( value == "AllDay" );
                break;
            case HasAlarm:
                if ( value.contains("true", Qt::CaseInsensitive)) { // No tr
                    mHasAlarm = true;
                } else {
                    mHasAlarm = false;
                }
                break;
            case AlarmDelay:
                mAlarmDelay = value.toInt();
                break;
            case SoundType:
                mAlarmSound = value == "loud" ? QAppointment::Loud : QAppointment::Silent; // No tr
                break;
            case RepeatPattern:
                if ( value == "Daily" ) // No tr
                    rec->setRepeatType(QAppointment::Daily);
                else if ( value == "Weekly" ) // No tr
                    rec->setRepeatType(QAppointment::Weekly);
                else if ( value == "MonthlyDay" )
                    rec->setRepeatType(QAppointment::MonthlyDay);
                else if ( value == "MonthlyDate" )
                    rec->setRepeatType(QAppointment::MonthlyDate);
                else if ( value == "MonthlyEndDay" )
                    rec->setRepeatType(QAppointment::MonthlyEndDay);
                else if ( value == "Yearly" ) // No tr
                    rec->setRepeatType(QAppointment::Yearly);
                else
                    rec->setRepeatType(QAppointment::NoRepeat);
                break;
            case RepeatFrequency:
                if (value.toInt() != 0) {
                    rec->setFrequency( value.toInt() );
                }
                break;
            case RepeatHasEndDate:
                if ( value.toInt() != 0 ) {
                    rec->setRepeatForever( false );
                } else {
                    rec->setRepeatForever( true );
                }
                break;
            case RepeatWeekdays:
                rec->p_setWeekMask(value.toInt());
                break;
            case RepeatEndDate:
                endDateUtc = (time_t) value.toLong();
                break;

            case RecordParent:
                rec->setParentUid( value.toInt() );
                break;
            case RecordChildren:
                {
                    QStringList list = value.split(" ");
                    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
                        rec->addChildUid( (*it).toInt() );
                    }
                }
                break;
            case Exceptions:
                {
                    QStringList list = value.split(" ");
                    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
                        int tday = 1, tmonth = 1, tyear = 2000;
                        switch( (*it).length() ) {
                            case 8:
                                tday = (*it).right( 2 ).toInt();
                            case 6:
                                tmonth = (*it).mid( 4, 2 ).toInt();
                            case 4:
                                tyear = (*it).left( 4 ).toInt();
                                break;
                            default:
                                break;
                        }
                        //QDate date = TimeConversion::fromISO8601( QString(*it) ).date();
                        QDate date(tyear, tmonth, tday);
                        rec->addException( date );
                    }
                }
                break;
            default:
                break;
        }
    }

    if (mHasAlarm)
        rec->setAlarm(mAlarmDelay, mAlarmSound);
    else
        rec->clearAlarm();

    if (!hasTimeZone) {
        // make one up.  Can't be "None" because in old datebook all
        // events were in UTC.  So make it the current locale.
        rec->setTimeZone( QTimeZone::current() );
    }

    // if there was a timezone, it would be set by now.
    rec->setStartAsUTC( startUtc );
    rec->setEndAsUTC( endUtc );
    if (rec->hasRepeat() && endDateUtc != 0 && !rec->repeatForever())
        rec->setRepeatTillAsUTC( endDateUtc );

    {
        const XmlIOField *t = datebookentries;
        while (t->fieldId) {
            m.remove(t->text);
            ++t;
        }
    }

    // pass remaining fields to parent.
    QPimXmlIO::setFields(rec, m);
}

QMap<QString, QString> EventXmlIO::fields(const QPimRecord *r) const
{
    QAppointment *rec = (QAppointment *)r;
    QMap<QString,QString> m = QPimXmlIO::fields(r);

    const XmlIOField *t = datebookentries;
    while (t->fieldId) {
        int key = t->fieldId;
        switch( key ) {
            case Description:
                if (!rec->description().isEmpty())
                m.insert(t->text, rec->description());
                break;
            case Location:
                if (!rec->location().isEmpty())
                m.insert(t->text, rec->location());
                break;
            case Notes:
                if (!rec->notes().isEmpty())
                m.insert(t->text, rec->notes());
                break;
            case StartTimeZone:
                if (rec->timeZone().isValid())
                    m.insert(t->text, rec->timeZone().id());
                else
                    m.insert(t->text, "None"); // No tr
                break;
            case StartDateTime:
                m.insert(t->text, QString::number( rec->startAsUTC() ) );
                break;
            case EndDateTime:
                m.insert(t->text, QString::number( rec->endAsUTC() ) );
                break;
            case DatebookType:
                if ( rec->isAllDay() )
                    m.insert(t->text, "AllDay"); // No tr
                else
                    m.insert(t->text, "Timed"); // No tr
                break;
            case HasAlarm:
                m.insert(t->text, rec->hasAlarm() ? "true" : "false"); // No tr
                break;
            case AlarmDelay:
                if (rec->hasAlarm())
                    m.insert(t->text, QString::number( rec->alarmDelay() ));
                break;
            case SoundType:
                if (rec->hasAlarm()) {
                    if ( rec->alarmSound() == QAppointment::Loud )
                        m.insert(t->text, "loud"); // No tr
                    else
                        m.insert(t->text, "silent"); // No tr
                }
                break;
            case RepeatPattern:
                switch ( rec->repeatType() ) {
                    case QAppointment::Daily:
                        m.insert(t->text, "Daily"); // No tr
                        break;
                    case QAppointment::Weekly:
                        m.insert(t->text, "Weekly"); // No tr
                        break;
                    case QAppointment::MonthlyDay:
                        m.insert(t->text, "MonthlyDay"); // No tr
                        break;
                    case QAppointment::MonthlyEndDay:
                        m.insert(t->text, "MonthlyEndDay"); // No tr
                        break;
                    case QAppointment::MonthlyDate:
                        m.insert(t->text, "MonthlyDate"); // No tr
                        break;
                    case QAppointment::Yearly:
                        m.insert(t->text, "Yearly"); // No tr
                        break;
                    default:
                        m.insert(t->text, "NoRepeat"); // No tr
                        break;
                }
                break;
            case RepeatFrequency:
                if (rec->hasRepeat())
                    m.insert(t->text, QString::number( rec->frequency() ));
                break;
            case RepeatHasEndDate:
                if (rec->hasRepeat())
                    m.insert(t->text, QString::number( !rec->repeatForever() ));
                break;
            case RepeatEndDate:
                if (rec->hasRepeat() && !rec->repeatForever())
                m.insert(t->text, QString::number( rec->repeatTillAsUTC() ));
                break;
            case RepeatWeekdays:
                if (rec->repeatType() == QAppointment::Weekly)
                    m.insert(t->text, QString::number( rec->p_weekMask() ));
                break;
            case RecordParent:
                if (rec->isException())
                    m.insert(t->text, rec->seriesUid().toString() );
                break;
            case RecordChildren:
                {
                    const QList<QUniqueId> &vlc = rec->childUids();
                    if (vlc.count() > 0) {
                        QList<QUniqueId>::ConstIterator cit;
                        QString out;
                        for( cit = vlc.begin(); cit != vlc.end(); ++cit ) {
                            if (cit != vlc.begin())
                                out += " ";
                            out += (*cit).toString();
                        }

                        m.insert(t->text, out);
                    }
                }
                break;
            case Exceptions:
                {
                    const QList<QDate> &vle = rec->exceptions();
                    if (vle.count() > 0) {
                        QList<QDate>::ConstIterator eit;
                        QString out;
                        for( eit = vle.begin(); eit != vle.end(); ++eit ) {
                            QDate date = *eit;
                            if (eit != vle.begin())
                                out += " ";

                            QString str;
                            str.sprintf("%04d%02d%02d", date.year(), date.month(), date.day());
                            out += str;
                        }
                        m.insert(t->text, out);
                    }
                }
                break;
            default:
                break;
        }
        ++t;
    }
    return m;
}

const QList<QAppointment*>& EventXmlIO::events()
{
    ensureDataCurrent();
    return m_QAppointments;
}

QAppointment EventXmlIO::eventForId(const QUniqueId & u, bool *ok) const
{
    QListIterator<QAppointment*> it(m_QAppointments);

    QAppointment *p;
    while(it.hasNext()) {
        p = it.next();
        if (u == p->uid()) {
            if (ok)
                *ok = true;
            return *p;
        }
    }

    if (ok)
        *ok = false;
    return QAppointment();
}

/*!
 Returns the events between \a from and \a to where \a from and \a to are in local time.
 */
QList<Occurrence> EventXmlIO::getOccurrencesInCurrentTZ(const QDate& from, const QDate& to) const
{
    QList<Occurrence> results;

    QListIterator<QAppointment*> it(m_QAppointments);
    while(it.hasNext()) {
        QAppointment e(*(it.next()));
        QDate date = from.addDays(-1);

        int duration = e.start().date().daysTo(e.end().date()) + 1;

        bool ok;
        date = e.nextOccurrence(date, &ok);
        while (ok && date <= to.addDays(1)) {
            // loose check succeed, tight check on TZ now.
            Occurrence o(date, e);
            if (o.startInCurrentTZ().date() <= to && o.endInCurrentTZ().addSecs(-1).date() >= from)
                results.append(o);
            date = e.nextOccurrence(date.addDays(duration), &ok);
        }
    }
    return results;
}

Occurrence EventXmlIO::getNextOccurrenceInCurrentTZ( const QDateTime& from,
                                                     bool * ok ) const
{
    if( ok ) *ok = false;

    bool closestOccurValid = false;
    Occurrence closestOccur;

    QListIterator<QAppointment*> it(m_QAppointments);
    while(it.hasNext()) {
        QAppointment e(*(it.next()));

        bool ocok = false;
        QDate date = e.nextOccurrence( from.date(), &ocok );
        if( ocok ) {
            Occurrence o(date, e);
            if( o.startInCurrentTZ() >= from ) {

                if( false == closestOccurValid ||
                    o.startInCurrentTZ() < closestOccur.startInCurrentTZ() ||
                    (o.startInCurrentTZ() == closestOccur.startInCurrentTZ() &&
                     o.event().uid() < closestOccur.event().uid()) ) {

                    closestOccur = o;
                    closestOccurValid = true;

                }

            }
        }
    }

    if( ok ) *ok = closestOccurValid;
    return closestOccur;
}

Occurrence EventXmlIO::getNextOccurrenceInCurrentTZ( const Occurrence& from,
                                                     bool * ok ) const
{
    if( ok ) *ok = false;

    bool closestOccurValid = false;
    Occurrence closestOccur;

    QListIterator<QAppointment*> it(m_QAppointments);
    while(it.hasNext()) {
        QAppointment e(*(it.next()));

        bool ocok = false;
        QDate date = e.nextOccurrence( from.date(), &ocok );
        if( ocok ) {
            Occurrence o(date, e);

            if( o.startInCurrentTZ() == from.startInCurrentTZ() &&
                o.event().uid() <= from.event().uid() )
                continue;

            if( o.startInCurrentTZ() >= from.startInCurrentTZ() &&
                (false == closestOccurValid ||
                 o.startInCurrentTZ() < closestOccur.startInCurrentTZ()) ) {
                closestOccur = o;
                closestOccurValid = true;
            }
        }
    }

    if( ok ) *ok = closestOccurValid;
    return closestOccur;
}

QUniqueId EventXmlIO::addEvent(const QAppointment &event, bool assignUid )
{
    QUniqueId u;
    if (accessMode() == ReadOnly)
        return u;

    // Don't add events that will never occur.
    if (!event.isValid())
        return u;

    ensureDataCurrent();

    QAppointment *ev = new QAppointment((const QAppointment &)event);

    if ( assignUid || ev->uid().isNull() )
        assignNewUid(ev);

    u = ev->uid();

    if (internalAddRecord(ev)) {
        needsSave = true;
        if (ev->hasAlarm())
            addEventAlarm(*ev);

        updateJournal(*ev, ACTION_ADD);

        {
            QtopiaIpcEnvelope e("QPE/PIM", "addedEvent(int,QAppointment)");
            e << getpid();
            e << *ev;
        }
    }
    return u;
}

void EventXmlIO::addException(const QDate &d, const QAppointment &p)
{
    if (accessMode() == ReadOnly)
        return;

    ensureDataCurrent();
    QAppointment parent((const QAppointment &)p);
    parent.addException(d);
    // don't add empty uid to parent.
    parent.addChildUid(0);

    if (!parent.isValid()) {
        // apperently this is really a delete of the event.
        removeEvent(p);
        return;
    }

    if (internalUpdateRecord(new QAppointment(parent))) {
        needsSave = true;

        updateJournal(parent, ACTION_REPLACE);

        {
            QtopiaIpcEnvelope e("QPE/PIM", "updatedEvent(int,QAppointment)");
            e << getpid();
            e << parent;
        }
    }
}

QUniqueId EventXmlIO::addException(const QDate &d, const QAppointment &p, const QAppointment& event)
{
    QUniqueId u;
    if (accessMode() == ReadOnly)
        return u;

    ensureDataCurrent();
    // don't need to check is valid as can't disapear by adding an exception.
    QAppointment parent((const QAppointment &)p);
    QAppointment *ev = new QAppointment((const QAppointment &)event);

    assignNewUid(ev);
    u = ev->uid();

    ev->setParentUid(parent.uid());
    parent.addException(d);
    parent.addChildUid(ev->uid());

    if (internalAddRecord(ev) && internalUpdateRecord(new QAppointment(parent))) {
        needsSave = true;
        if (ev->hasAlarm())
            addEventAlarm(*ev);

        updateJournal(*ev, ACTION_ADD);
        updateJournal(parent, ACTION_REPLACE);

        {
            QtopiaIpcEnvelope e("QPE/PIM", "addedEvent(int,QAppointment)");
            e << getpid();
            e << *ev;
        }
        {
            QtopiaIpcEnvelope e("QPE/PIM", "updatedEvent(int,QAppointment)");
            e << getpid();
            e << parent;
        }
    }
    return u;
}

void EventXmlIO::removeEvent(const QAppointment &event)
{

    if (accessMode() == ReadOnly)
        return;

    ensureDataCurrent();
    // in case parent becomes invalid.
    QAppointment parent;

    QAppointment *ev = new QAppointment((const QAppointment &)event);

    if (event.isException()) {
        QListIterator<QAppointment*> it(m_QAppointments);
        while(it.hasNext()) {
            QAppointment *current = (it.next());
            if (current->uid() == ev->seriesUid()) {

                QList<QDate>::ConstIterator eit = current->exceptions().begin();
                QList<QUniqueId>::ConstIterator cit = current->childUids().begin();

                QDate tdate;
                for(; eit != current->exceptions().end() && cit != current->childUids().end();
                        ++eit, ++cit)
                {
                    if (*cit == ev->uid()) {
                        tdate = *eit;
                    }
                }
                current->removeException(ev->uid());
                current->QAppointment::addException(tdate, 0);

                if (!current->isValid()) {
                    parent = *(current);
                }
            }
        }
    }

    QListIterator<QAppointment*> it(m_QAppointments);
    while(it.hasNext()) {
        QAppointment *current = (it.next());
        if (current->uid() == ev->uid()) {
            if (current->hasAlarm())
                delEventAlarm(*current);
        }
    }


    if (internalRemoveRecord(ev)) {
        needsSave = true;
        updateJournal(event, ACTION_REMOVE);
        {
            QtopiaIpcEnvelope e("QPE/PIM", "removedEvent(int,QAppointment)");
            e << getpid();
            e << event;
        }
    }

    if (!parent.uid().isNull()) {
        // parent is invalid,
        removeEvent(parent);
    }
}

void EventXmlIO::updateEvent(const QAppointment &event)
{
    if (accessMode() == ReadOnly)
        return;

    ensureDataCurrent();

    if (!event.isValid()) {
        // really a remove.
        removeEvent(event);
        return;
    }

    QAppointment *ev = new QAppointment((const QAppointment&)event);

    QListIterator<QAppointment*> it(m_QAppointments);
    while(it.hasNext()) {
        QAppointment *current = it.next();
        if (current->uid() == ev->uid()) {
            if (current->hasAlarm())
                delEventAlarm(*current);
        }
    }

    if (internalUpdateRecord(ev)) {
        needsSave = true;
        if (event.hasAlarm())
            addEventAlarm((const QAppointment &)event);

        updateJournal(event, ACTION_REPLACE);
        {
            QtopiaIpcEnvelope e("QPE/PIM", "updatedEvent(int,QAppointment)");
            e << getpid();
            e << event;
        }
    }
}

void EventXmlIO::ensureDataCurrent(bool forceReload)
{
    if (accessMode() == WriteOnly || ( isDataCurrent() && !forceReload) )
        return;

    // get rid of all events first
    QListIterator<QAppointment*> it(m_QAppointments);
    while(it.hasNext()) {
        QAppointment *current = it.next();
        if ( current->hasAlarm() )
            delEventAlarm( *current );
    }

    m_QAppointments.clear();
    loadData();
}

bool EventXmlIO::nextAlarm( const QAppointment &ev, QDateTime& when, int& warn)
{
    QDateTime now = QDateTime::currentDateTime();
    bool ok;
    warn = ev.alarmDelay();

    // -1 days to make room for timezone shift.
    QDate next = ev.nextOccurrence(now.date().addDays(-1), &ok);
    while (ok) {
        Occurrence o(next, ev);
        // only need to check in TZ... want to shift in orig time
        if (now <= o.startInCurrentTZ().addSecs(-60*ev.alarmDelay())) {
            when = o.startInCurrentTZ().addSecs(-60*ev.alarmDelay());
            return true;
        }
        next = ev.nextOccurrence(
                next.addDays(ev.start().daysTo(ev.end()) + 1), &ok);
    }
    return false;
}

void EventXmlIO::addEventAlarm(const QAppointment &ev)
{
#ifdef Q_WS_QWS
    QDateTime when;
    int warn;
    if (nextAlarm(ev, when, warn)) {
        Qtopia::addAlarm(when, "Calendar", "alarm(QDateTime,int)", warn);
    }
#else
    Q_UNUSED( ev );
#endif
}

void EventXmlIO::delEventAlarm(const QAppointment &ev)
{
#ifdef Q_WS_QWS
    QDateTime when;
    int warn;
    if ( nextAlarm(ev,when,warn) ) {
        Qtopia::deleteAlarm( when, "Calendar", "alarm(QDateTime,int)", warn);
    }
#else
    Q_UNUSED( ev );
#endif
}
