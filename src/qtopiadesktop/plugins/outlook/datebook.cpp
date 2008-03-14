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
#include "outlooksync.h"
#include "trace.h"

#include <QBuffer>
#include <QXmlStreamReader>
#include <QFile>
#include <QUuid>

class OutlookDatebookSync : public OutlookSyncPlugin
{
    Q_OBJECT
    QD_CONSTRUCT_PLUGIN(OutlookDatebookSync,OutlookSyncPlugin)
public:
    QString displayName() { return tr("Outlook Appointments"); }

    QString dataset() { return "calendar"; }
    QByteArray referenceSchema() { return "<Appointment><Identifier/><Description/><Location/><TimeZone/><When><Start/><End/></When><Alarm><Type/><Delay/></Alarm><Repeat><Type/><Frequency/><Until/><Nearest/><WeekMask/><Exception><OriginalDate/><Appointment><Identifier/><Description/><Location/><TimeZone/><When><Start/><End/></When><Alarm><Type/><Delay/></Alarm><Notes/><Categories/></Appointment></Exception></Repeat><Notes/><Categories/></Appointment>"; }

    Outlook::OlDefaultFolders folderEnum() { return Outlook::olFolderCalendar; }
    Outlook::OlItemType itemEnum() { return Outlook::olAppointmentItem; }
    bool isValidObject( IDispatchPtr dispatch )
    {
        TRACE(OutlookSyncPlugin) << "OutlookDatebookSync::isValidObject";
        Outlook::_AppointmentItemPtr item( dispatch );
        return ( item->GetClass() == Outlook::olAppointment );
    }

    void getProperties( IDispatchPtr dispatch, QString &entryid, QDateTime &lastModified )
    {
        TRACE(OutlookSyncPlugin) << "OutlookDatebookSync::getProperties";
        Outlook::_AppointmentItemPtr item( dispatch );
        entryid = bstr_to_qstring(item->GetEntryID());
        lastModified = date_to_qdatetime(item->GetLastModificationTime());
    }

    void dump_item( IDispatchPtr dispatch, QTextStream &stream )
    {
        Q_ASSERT( dispatch );
        Outlook::_AppointmentItemPtr item( dispatch );
        dump_item( item, stream, false );
    }

    void dump_item( const Outlook::_AppointmentItemPtr &item, QTextStream &stream, bool dump_exception )
    {
        TRACE(OutlookSyncPlugin) << "OutlookDatebookSync::dump_item";

        Outlook::OlRecurrenceState recstate = item->GetRecurrenceState();
        if ( recstate == Outlook::olApptOccurrence ) {
            WARNING() << "Cannot sync occurrences!";
            return;
        }
        if ( (dump_exception && recstate != Outlook::olApptException) ||
             (!dump_exception && recstate == Outlook::olApptException) ) {
            WARNING() << "Found" << (recstate == Outlook::olApptException?"Exception":"non-Exception")
                      << "when syncing" << (dump_exception?"Exceptions":"non-Exceptions");
            return;
        }

        PREPARE_CUSTOM_MAP(qtopiaUserProps);
        PREPARE_MAPI(Appointment);

        stream << "<Appointment>\n";
        DUMP_STRING(Identifier,EntryID);
        DUMP_STRING(Description,Subject);
        DUMP_STRING(Location,Location);
        QString timezone;
        {
            Outlook::UserPropertyPtr up = props->Find("Qtopia Timezone");
            if ( up ) {
                timezone = variant_to_qstring(up->GetValue());
            }
        }
        DUMP_EXPR(TimeZone,timezone);
        stream << "<When>\n";
        // For some reason item->GetAllDayEvent() is always false when pulling data from Outlook.
        // To work around this we check if the start and end times are 00:00:00
        // (which is what Outlook sets for all day events). This breaks timed
        // events that start and stop at midnight... perhaps such events are rare enough to ignore.
        //if ( item->GetAllDayEvent() ) {
        if ( date_to_qdatetime(item->GetStart()).time() == QTime(0, 0, 0) && date_to_qdatetime(item->GetEnd()).time() == QTime(0, 0, 0) ) {
            DUMP_DATE(StartDate,Start);
            DUMP_DATE(EndDate,End);
        } else {
            QDateTime dt = date_to_qdatetime(item->GetStart());
            bool utc = !timezone.isEmpty();
            if ( utc )
                dt = dt.toUTC();
            DUMP_EXPR(Start,escape(dateTimeToString(dt, utc)));
            dt = date_to_qdatetime(item->GetEnd());
            if ( utc )
                dt = dt.toUTC();
            DUMP_EXPR(End,escape(dateTimeToString(dt, utc)));
        }
        stream << "</When>\n";
        stream << "<Alarm>\n";
        if ( item->GetReminderSet() ) {
            DUMP_EXPR(Type,item->GetReminderPlaySound()?"Audible":"Visible");
            DUMP_INT(Delay,ReminderMinutesBeforeStart);
        }
        stream << "</Alarm>\n";
        if ( !dump_exception ) {
            stream << "<Repeat>\n";
            if ( recstate != Outlook::olApptNotRecurring && item->GetIsRecurring() ) {
                Q_ASSERT(recstate == Outlook::olApptMaster);
                Outlook::RecurrencePatternPtr recpat = item->GetRecurrencePattern();
                Q_ASSERT(recpat);

                Outlook::OlRecurrenceType rectype = recpat->GetRecurrenceType();
                LOG() << "recpat->RecurrenceType" << rectype;

                QString type;
                if ( rectype == Outlook::olRecursDaily && !recpat->GetDayOfWeekMask() )
                    type = "Daily";
                else if ( ( rectype == Outlook::olRecursDaily && recpat->GetDayOfWeekMask() ) || rectype == Outlook::olRecursWeekly )
                    type = "Weekly";
                else if ( rectype == Outlook::olRecursMonthly )
                    type = "MonthlyDate";
                else if ( ( rectype == Outlook::olRecursMonthNth || rectype == Outlook::olRecursYearNth ) && recpat->GetInstance() != 5 )
                    type = "MonthlyDay";
                else if ( ( rectype == Outlook::olRecursMonthNth || rectype == Outlook::olRecursYearNth ) && recpat->GetInstance() == 5 )
                    type = "MonthlyEndDay";
                else if ( rectype == Outlook::olRecursYearly )
                    type = "Yearly";
                Q_ASSERT(!type.isEmpty());
                LOG() << "Type" << type << "Instance" << recpat->GetInstance();
                stream << "<Type>" << type << "</Type>\n";

                int frequency = recpat->GetInterval();
                LOG() << "recpat->GetInterval" << frequency;
                if ( rectype == Outlook::olRecursDaily && recpat->GetDayOfWeekMask() )
                    frequency = 1;
                stream << "<Frequency>" << frequency << "</Frequency>\n";

                stream << "<Until>";
                if ( !recpat->GetNoEndDate() ) {
                    LOG() << "recpat->GetPatternEndDate" << date_to_qdatetime(recpat->GetPatternEndDate()).date();
                    stream << dateToString(date_to_qdatetime(recpat->GetPatternEndDate()).date());
                }
                stream << "</Until>\n";

                if ( type == "Weekly" || type == "MonthlyDay" || type == "MonthlyEndDay" ) {
                    stream << "<WeekMask>";
                    if ( recpat->GetDayOfWeekMask() ) {
                        int mask = recpat->GetDayOfWeekMask();
                        LOG() << "recpat->GetDayOfWeekMask" << mask;
                        QStringList list;
                        if ( mask & Outlook::olMonday )
                            list << "Monday";
                        if ( mask & Outlook::olTuesday )
                            list << "Tuesday";
                        if ( mask & Outlook::olWednesday )
                            list << "Wednesday";
                        if ( mask & Outlook::olThursday )
                            list << "Thursday";
                        if ( mask & Outlook::olFriday )
                            list << "Friday";
                        if ( mask & Outlook::olSaturday )
                            list << "Saturday";
                        if ( mask & Outlook::olSunday )
                            list << "Sunday";
                        stream << list.join(" ");
                    } else {
                        LOG() << "recpat->GetDayOfWeekMask" << 0;
                    }
                    stream << "</WeekMask>\n";
                }

                Outlook::ExceptionsPtr exceptions = recpat->GetExceptions();
                if ( exceptions ) {
                    int expcount = exceptions->GetCount();
                    for ( int i = 0; i < expcount; i++ ) {
                        stream << "<Exception>\n";

                        long item_to_get = i+1;
                        Outlook::ExceptionPtr exception = exceptions->Item(item_to_get);
                        Q_ASSERT(exception);

                        DUMP_DATE_ITEM(OriginalDate,OriginalDate,exception);
                        if ( exception->GetDeleted() ) {
                            stream << "<Appointment></Appointment>\n";
                        } else {
                            Outlook::_AppointmentItemPtr exceptionItem = exception->GetAppointmentItem();
                            dump_item( exceptionItem, stream, true );
                        }

                        stream << "</Exception>\n";
                    }
                }
            }
            stream << "</Repeat>\n";
        }
        DUMP_MAPI(Notes,Body);
        stream << "<Categories>\n";
        foreach ( const QString &category, bstr_to_qstring(item->GetCategories()).split(", ", QString::SkipEmptyParts) )
            DUMP_EXPR(Category,category);
        stream << "</Categories>\n";
        //DUMP_CUSTOM_MAP(qtopiaUserProps);
        stream << "</Appointment>\n";
    }

    QString read_item( IDispatchPtr dispatch, const QByteArray &record )
    {
        Q_ASSERT( dispatch );
        Outlook::_AppointmentItemPtr item( dispatch );
        QXmlStreamReader reader(record);
        return read_item( item, reader, false );
    }

    QString read_item( const Outlook::_AppointmentItemPtr &item, QXmlStreamReader &reader, bool dump_exception )
    {
        TRACE(OutlookSyncPlugin) << "OutlookDatebookSync::read_item";

        PREPARE_CUSTOM_MAP(qtopiaUserProps);

        enum State {
            Idle, When, Alarm, Repeat, Exception, Categories
        };
        State state = Idle;
        Outlook::RecurrencePatternPtr recpat = 0;

        QString key;
        QXmlStreamAttributes attributes;
        QString value;
        QStringList categories;
        bool utc = false;
        bool allday = false;
        while (!reader.atEnd()) {
            bool loop = true;
            switch(reader.readNext()) {
                case QXmlStreamReader::StartElement:
                    key = reader.qualifiedName().toString();
                    value = QString();
                    attributes = reader.attributes();
                    if ( key == "When" )
                        state = When;
                    if ( state == When && key == "StartDate" ) {
                        allday = true;
                    }
                    if ( state == When && key == "Start" ) {
                        allday = false;
                    }
                    if ( key == "Alarm" ) {
                        state = Alarm;
                        LOG() << "item->PutReminderSet" << false;
                        item->PutReminderSet( false );
                    }
                    if ( state == Alarm && key == "Type" || key == "Delay" ) {
                        // Outlook only wants to see alarms set on events in the future
                        // If we sync an event in the past with an alarm it will go off
                        // immediately, something that can be annoying when you do an
                        // initial sync with lots of events with alarms.
                        if ( date_to_qdatetime(item->GetStart()) > QDateTime::currentDateTime() ) {
                            LOG() << "item->PutReminderSet" << true;
                            item->PutReminderSet( true );
                        }
                    }
                    if ( !dump_exception ) {
                        if ( key == "Repeat" ) {
                            state = Repeat;
                            item->ClearRecurrencePattern();
                        }
                        if ( state == Repeat && key == "Type" ) {
                            recpat = item->GetRecurrencePattern();
                            recpat->PutPatternStartDate( item->GetStart() );
                        }
                        if ( state == Repeat && key == "Exception" ) {
                            state = Exception;
                        }
                    }
                    if ( key == "Categories" )
                        state = Categories;
                    break;
                case QXmlStreamReader::Characters:
                    value = unescape(reader.text().toString());
                    break;
                case QXmlStreamReader::EndElement:
                    key = reader.qualifiedName().toString();
                    //LOG() << "key" << key << "value" << value;
                    READ_STRING(Description,Subject);
                    READ_STRING(Location,Location);
                    if ( key == "TimeZone" ) {
                        utc = ( !value.isEmpty() );
                    }
                    if ( state == When ) {
                        if ( allday ) {
                            item->PutAllDayEvent( true );
                            READ_DATE(StartDate,Start);
                            READ_DATE(EndDate,End);
                        } else {
                            item->PutAllDayEvent( false );
                            if ( key == "Start" ) {
                                QDateTime dt = stringToDateTime(value, utc);
                                if ( utc ) {
                                    dt.setTimeSpec( Qt::UTC );
                                    dt = dt.toLocalTime();
                                }
                                LOG() << "item->PutStart" << dt;
                                item->PutStart( qdatetime_to_date(dt) );
                            }
                            if ( key == "End" ) {
                                QDateTime dt = stringToDateTime(value, utc);
                                if ( utc ) {
                                    dt.setTimeSpec( Qt::UTC );
                                    dt = dt.toLocalTime();
                                }
                                LOG() << "item->PutEnd" << dt;
                                item->PutEnd( qdatetime_to_date(dt) );
                            }
                        }
                        if ( key == "When" )
                            state = Idle;
                    }
                    if ( state == Alarm ) {
                        READ_ENUM(Type,ReminderPlaySound,true,Audible);
                        READ_ENUM(Type,ReminderPlaySound,false,Visible);
                        READ_INT(Delay,ReminderMinutesBeforeStart);
                        if ( key == "Alarm" )
                            state = Idle;
                    }
                    if ( dump_exception == false && state == Repeat ) {
                        READ_ENUM_ITEM(Type,RecurrenceType,Outlook::olRecursDaily,Daily,recpat);
                        READ_ENUM_ITEM(Type,RecurrenceType,Outlook::olRecursWeekly,Weekly,recpat);
                        READ_ENUM_ITEM(Type,RecurrenceType,Outlook::olRecursMonthly,MonthlyDate,recpat);
                        READ_ENUM_ITEM(Type,RecurrenceType,Outlook::olRecursMonthNth,MonthlyDay,recpat);
                        READ_ENUM_ITEM(Type,RecurrenceType,Outlook::olRecursMonthNth,MonthlyEndDay,recpat);
                        READ_ENUM_ITEM(Type,RecurrenceType,Outlook::olRecursYearly,Yearly,recpat);
                        if ( key == "Type" ) {
                            if ( value == "MonthlyEndDay" ) {
                                LOG() << "recpat->PutInstance" << 5;
                                recpat->PutInstance( 5 );
                            }
                        }

                        if ( key == "Frequency" ) {
                            int interval = QVariant(value).toInt();
                            if ( interval >= 12 && interval % 12 == 0 ) {
                                // since interval is bigger than 12 yet divisible by 12 this is more
                                // likely to be a YearNth which Qtopia Sync Agent sends down as a
                                // MonthNth with interval *= 12
                                recpat->PutRecurrenceType( Outlook::olRecursYearNth );
                            }
                            LOG() << "recpat->PutInterval" << interval;
                            recpat->PutInterval( interval );
                        }

                        if ( key == "Until" ) {
                            if ( value.isEmpty() ) {
                                LOG() << "recpat->PutNoEndDate" << true;
                                recpat->PutNoEndDate( true );
                            } else {
                                LOG() << "recpat->PutPatternEndDate" << QDateTime( stringToDate(value), QTime(0,0,0) );
                                recpat->PutPatternEndDate( qdatetime_to_date(QDateTime( stringToDate(value), QTime(0,0,0) )) );
                            }
                        }

                        // Outlook doesn't seem to support Nearest == false (so ignore it)

                        if ( key == "WeekMask" ) {
                            int mask = 0;
                            foreach( const QString &v, value.split(" ") ) {
                                if ( v == "Monday" ) mask |= Outlook::olMonday;
                                else if ( v == "Tuesday" ) mask |= Outlook::olTuesday;
                                else if ( v == "Wednesday" ) mask |= Outlook::olWednesday;
                                else if ( v == "Thursday" ) mask |= Outlook::olThursday;
                                else if ( v == "Friday" ) mask |= Outlook::olFriday;
                                else if ( v == "Saturday" ) mask |= Outlook::olSaturday;
                                else if ( v == "Sunday" ) mask |= Outlook::olSunday;
                            }
                            LOG() << "recpat->PutDayOfWeekMask" << mask;
                            recpat->PutDayOfWeekMask( (Outlook::OlDaysOfWeek)mask );
                        }

                        if ( key == "Repeat" )
                            state = Idle;
                    }

                    if ( dump_exception == false && state == Exception ) {
                        if ( key == "OriginalDate" ) {
                            QDate exceptionDate = stringToDate(value);
                            Outlook::_AppointmentItemPtr eitem = recpat->GetOccurrence( qdatetime_to_date(QDateTime( exceptionDate, QTime(0,0,0) )) );
                            QString entryid = read_item( eitem, reader, true );
                            if ( entryid.isEmpty() )
                                state = Repeat; // the delete case eats the closing Exception tag
                        }
                        if ( key == "Exception" ) {
                            state = Repeat;
                        }
                    }

                    READ_STRING(Notes,Body);
                    if ( state == Categories ) {
                        if ( key == "Category" )
                            categories << value;
                        if ( key == "Categories" ) {
                            LOG() << "item->PutCategories" << categories;
                            item->PutCategories( qstring_to_bstr(categories.join(", ")) );
                            state = Idle;
                        }
                    }
                    READ_CUSTOM(TimeZone,Qtopia Timezone);

                    if ( dump_exception && key == "Appointment" )
                        loop = false;
                    if ( dump_exception && key == "Exception" ) {
                        // Oops... no Appointment tag in an Exception tag
                        // That means we need to delete the existing exception
                        item->Delete();
                        return QString();
                    }
            }
            if ( !loop )
                break;
        }

        item->Save();
        return bstr_to_qstring(item->GetEntryID());
    }

    void delete_item( IDispatchPtr dispatch )
    {
        TRACE(OutlookSyncPlugin) << "OutlookDatebookSync::delete_item";
        Q_ASSERT( dispatch );
        Outlook::_AppointmentItemPtr item( dispatch );
        item->Delete();
    }

    void OutlookDatebookSync::init_item( IDispatchPtr dispatch )
    {
        TRACE(OutlookSyncPlugin) << "OutlookDatebookSync::init_item";
        Outlook::_AppointmentItemPtr item( dispatch );
        item->PutReminderSet( false );
        item->ClearRecurrencePattern();
        item->Save();
    }
};

QD_REGISTER_PLUGIN(OutlookDatebookSync);

#include "datebook.moc"
