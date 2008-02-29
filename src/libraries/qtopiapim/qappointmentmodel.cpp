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
#include <qtopia/pim/qappointmentmodel.h>
#include "qappointmentsqlio_p.h"
#ifdef GOOGLE_CALENDAR_CONTEXT
#include "qgooglecontext_p.h"
#endif
#include <QSettings>
#include <QMap>
#include <QList>
#include <QSet>
#include <QFile>
#include <QTimer>
#include <QApplication>

#include <QDebug>

#include "qrecordiomerge_p.h"

class QAppointmentModelData
{
public:
    QAppointmentModelData() : durationType(QAppointmentModel::AnyDuration) {}

    QDateTime rStart;
    QDateTime rEnd;
    QAppointmentModel::DurationType durationType;

    static QIcon getCachedIcon(const QString& str);
    static QHash<QString, QIcon> cachedIcons;
};

QHash<QString, QIcon> QAppointmentModelData::cachedIcons;

QIcon QAppointmentModelData::getCachedIcon(const QString &str)
{
    if (cachedIcons.contains(str))
        return cachedIcons.value(str);
    cachedIcons.insert(str, QIcon(str));
    return cachedIcons.value(str);
}

QMap<QAppointmentModel::Field, QString> QAppointmentModel::k2i;
QMap<QString, QAppointmentModel::Field> QAppointmentModel::i2k;
QMap<QAppointmentModel::Field, QString>  QAppointmentModel::k2t;

/*!
  \internal

  Initializes mappings from column enums to translated and non-translated strings.
*/
void QAppointmentModel::initMaps()
{
    if (k2t.count() > 0)
        return;
    struct KeyLookup {
        const char * ident;
        const char * trans;
        Field key;
    };
    static const KeyLookup l[] = {
        { "description", QT_TR_NOOP("Description"), Description },
        { "location", QT_TR_NOOP( "Location" ), Location },
        { "start", QT_TR_NOOP( "Start" ), Start },
        { "end", QT_TR_NOOP( "End" ), End },
        { "allday", QT_TR_NOOP( "All Day" ), AllDay },
        { "timezone", QT_TR_NOOP( "Time Zone" ), TimeZone },

        { "notes", QT_TR_NOOP( "Notes" ), Notes },
        { "alarms", QT_TR_NOOP( "Alarms" ), Alarm },
        { "repeatrule", QT_TR_NOOP( "Repeat Rule" ), RepeatRule },
        { "repeatfrequency", QT_TR_NOOP( "Repeat Frequency" ), RepeatFrequency },
        { "repeatenddate", QT_TR_NOOP( "Repeat End Date" ), RepeatEndDate },
        { "repeatweekflags", QT_TR_NOOP( "Repeat Week Flags" ), RepeatWeekFlags },

        { "identifier", QT_TR_NOOP( "Identifier" ), Identifier},
        { "categories", QT_TR_NOOP( "Categories" ), Categories},
        { 0, 0, Invalid }
    };

    const KeyLookup *k = l;
    while (k->key != 0) {
        k2t.insert(k->key, k->trans);
        k2i.insert(k->key, k->ident);
        i2k.insert(k->ident, k->key);
        ++k;
    }
}

/*!
  Returns a translated string describing the appointment model sort \a field.

  \sa fieldIcon(), fieldIdentifier(), identifierField()
*/
QString QAppointmentModel::fieldLabel(Field field)
{
    if (k2t.count() == 0)
        initMaps();
    if (!k2t.contains(field))
        return QString();
    return k2t[field];
}

/*!
  Returns a icon representing the appointment model sort \a field.

  Returns a null icon if no icon is available.

  \sa fieldLabel(), fieldIdentifier(), identifierField()
*/
QIcon QAppointmentModel::fieldIcon(Field field)
{
    QString ident = fieldIdentifier(field);

    if (ident.isEmpty())
        return QIcon();

    return QAppointmentModelData::getCachedIcon(":image/datebook/" + ident);
}

/*!
  Returns a non-translated string describing the appointment model sort \a field.

  \sa fieldLabel(), fieldIcon(), identifierField()
*/
QString QAppointmentModel::fieldIdentifier(Field field)
{
    if (k2i.count() == 0)
        initMaps();
    if (!k2i.contains(field))
        return QString();
    return k2i[field];
}

/*!
  Returns the appointment model sort field for the non-translated string \a identifier

  \sa fieldLabel(), fieldIcon(), fieldIdentifier()
*/
QAppointmentModel::Field QAppointmentModel::identifierField(const QString &identifier)
{
    if (i2k.count() == 0)
        initMaps();
    if (!i2k.contains(identifier))
        return Invalid;
    return i2k[identifier];
}

/*!
  \class QAppointmentModel
  \module qpepim
  \ingroup pim
  \brief The QAppointmentModel class provides access to the Calendar data.

  The QAppointmentModel is used to access the Calendar data.  It is a descendant of QAbstractItemModel,
  so it is suitable for use with the Qt View classes such as QListView and QTableView, as well as
  any custom developer Views.

  QAppointmentModel provides functions for sorting and some filtering of items.
  For filters or sorting that is not provided by QAppointmentModel it is recommended that
  QSortFilterProxyModel is used to wrap QAppointmentModel.

  QAppointmentModel will refresh when changes are made in other instances of QAppointmentModel or
  from other applications.
*/

/*!
  Constructs a QAppointmentModel with parent \a parent.
*/
QAppointmentModel::QAppointmentModel(QObject *parent)
    : QPimModel(parent)
{

    QtopiaSql::openDatabase();
    d = new QAppointmentModelData();

    QAppointmentSqlIO *access = new QAppointmentSqlIO(this);
    QAppointmentContext *context = new QAppointmentDefaultContext(this, access);

    addAccess(access);
    addContext(context);

#ifdef GOOGLE_CALENDAR_CONTEXT
    addContext(new QGoogleCalendarContext(this, access));
#endif
}

/*!
  Destructs the QAppointmentModel.
*/
QAppointmentModel::~QAppointmentModel()
{
    delete d;
}

/*!
  \enum QAppointmentModel::Field

  Enumerates the columns when in table mode and columns used for sorting.
  This is a subset of data retrievable from a QAppointment.

  \omitvalue Invalid
  \omitvalue Description
  \omitvalue Location
  \omitvalue Start
  \omitvalue End
  \omitvalue AllDay
  \omitvalue TimeZone
  \omitvalue Notes
  \omitvalue Alarm
  \omitvalue RepeatRule
  \omitvalue RepeatFrequency
  \omitvalue RepeatEndDate
  \omitvalue RepeatWeekFlags
  \omitvalue Identifier
  \omitvalue Categories
*/

/*!
  \enum QAppointmentModel::QAppointmentModelRole

  Extends Qt::ItemDataRole

  \value LabelRole
    A short formatted text label of the appointments name.
*/

/*!
  \overload

  Returns an object that contains a serialized description of the specified \a indexes.
  The format used to describe the items corresponding to the \a indexes is obtained from
  the mimeTypes() function.

  If the list of indexes is empty, 0 is returned rather than a serialized empty list.

  Currently returns 0 but may be implemented at a future date.
*/
QMimeData * QAppointmentModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)

    return 0;
}

/*!
  \overload

  Returns a list of MIME types that can be used to describe a list of model indexes.

  Currently returns an empty list but may be implemented at a future date.
*/
QStringList QAppointmentModel::mimeTypes() const
{
    return QStringList();
}

/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.
*/
QVariant QAppointmentModel::data(const QModelIndex &index, int role) const
{
    const QAppointmentIO *model = qobject_cast<const QAppointmentIO *>(access(index.row()));
    int r = accessRow(index.row());
    if (!model)
        return QVariant();

    switch(index.column()) {
        case Description:
            if (index.row() < rowCount()){
                // later, take better advantage of roles.
                switch(role) {
                    default:
                        break;
                    case Qt::DisplayRole:
                        return model->appointmentField(r, Description);
                    case Qt::EditRole:
                        return model->appointmentField(r, Identifier).toByteArray();
                    case LabelRole:
                        {
                            QString l = model->appointmentField(r, Description).toString();
                            return "<b>" + l + "</b>";
                        }
                        break;
                }
            }
            break;
        default:
            if (index.column() > 0 && index.column() < columnCount())
                return model->appointmentField(r, (QAppointmentModel::Field)index.column());
            break;
    }
    return QVariant();
}

/*!
  \overload

  Returns the number of columns for the given \a parent.
*/
int QAppointmentModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return Categories+1;// last column + 1
}

/*!
  \overload
  Sets the \a role data for the item at \a index to \a value. Returns true if successful,
  otherwise returns false.
*/
bool QAppointmentModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    if (!index.isValid())
        return false;

    QAppointment a = appointment(index);
    if (!setAppointmentField(a, (Field)index.column(), value))
        return false;
    return updateAppointment(a);

#if 0
    /* disabled due to 'notifyUpdated' require whole record.
       While writing whole record is less efficient than partial - at 
       this stage it was the easiest way of fixing the bug where setData
       did not result in cross-model data change from being propogated properly
   */

    int i = index.row();
    const QAppointmentIO *model = qobject_cast<const QAppointmentIO*>(d->mio->model(i));
    int r = d->mio->row(i);
    if (model)
        return ((QAppointmentIO *)model)->setAppointmentField(r, (Field)index.column(), value);
    return false;
#endif
}

/*!
  \overload
  For every Qt::ItemDataRole in \a roles, sets the role data for the item at \a index to the
  associated value in \a roles. Returns true if successful, otherwise returns false.
*/
bool QAppointmentModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &roles)
{
    if (roles.count() != 1 || !roles.contains(Qt::EditRole))
        return false;
    return setData(index, roles[Qt::EditRole], Qt::EditRole);
}

/*!
  \overload

  Returns a map with values for all predefined roles in the model for the item at the
  given \a index.
*/
QMap<int,QVariant> QAppointmentModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> result;
    switch (index.column()) {
        case Description:
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            result.insert(Qt::EditRole, data(index, Qt::EditRole));
            break;
        default:
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            break;
    }
    return result;
}

/*!
  \overload

  Returns the data for the given \a role and \a section in the header with the
  specified \a orientation.
*/
QVariant QAppointmentModel::headerData(int section, Qt::Orientation orientation,
        int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    if (section >= 0 && section < columnCount()) {
        if (role == Qt::DisplayRole)
            return fieldLabel((Field)section);
        else if (role == Qt::EditRole)
            return fieldIdentifier((Field)section);
    }
    return QVariant();
}

/*!
  Returns the appointment for the row specified by \a index.
  The column of \a index is ignored.
*/
QAppointment QAppointmentModel::appointment(const QModelIndex &index) const
{
    return appointment(index.row());
}

/*!
  Returns the appointment for the \a row specified.
*/
QAppointment QAppointmentModel::appointment(int row) const
{
    const QAppointmentIO *model = qobject_cast<const QAppointmentIO*>(access(row));
    int r = accessRow(row);
    if (model)
        return model->appointment(r);
    return QAppointment();
}

/*!
  Returns the appointment with the identifier \a id.  The appointment does
  not have to be in the current filter mode for it to be returned.
*/
QAppointment QAppointmentModel::appointment(const QUniqueId & id) const
{
    const QAppointmentIO *model = qobject_cast<const QAppointmentIO*>(access(id));
    if (model)
        return model->appointment(id);
    return QAppointment();
}

/*!
  Returns the value from \a appointment that would be returned for
  sort key \a k as it would from a row in the QAppointmentModel.
*/
QVariant QAppointmentModel::appointmentField(const QAppointment &appointment, QAppointmentModel::Field k)
{
    switch(k) {
        default:
        case QAppointmentModel::Invalid:
            break;
        case QAppointmentModel::Identifier:
            return QVariant(appointment.uid().toByteArray());
        case QAppointmentModel::Categories:
            return QVariant(appointment.categories());
        case QAppointmentModel::Notes:
            return appointment.notes();
        case QAppointmentModel::Description:
            return appointment.description();
        case QAppointmentModel::Location:
            return appointment.location();
        case QAppointmentModel::Start:
            return appointment.start();
        case QAppointmentModel::End:
            return appointment.end();
        case QAppointmentModel::AllDay:
            return appointment.isAllDay();
        case QAppointmentModel::TimeZone:
            return appointment.timeZone().id();
        case QAppointmentModel::Alarm:
            return (int)appointment.alarm();
        case QAppointmentModel::RepeatRule:
            return (int)appointment.repeatRule();
        case QAppointmentModel::RepeatFrequency:
            return appointment.frequency();
        case QAppointmentModel::RepeatEndDate:
            return appointment.repeatUntil();
        case QAppointmentModel::RepeatWeekFlags:
            return (int)appointment.weekFlags();
    }
    return QVariant();
}

/*!
  Sets the value in \a appointment that would be set for sort key \a k as it would
  if modified for a appointment in the QAppointmentModel to \a v.

  Returns true if the appointment was modified.  Otherwise returns false.
*/
bool QAppointmentModel::setAppointmentField(QAppointment &appointment, QAppointmentModel::Field k,  const QVariant &v)
{
    switch(k) {
        default:
        case QAppointmentModel::Invalid:
        case QAppointmentModel::Identifier: // not a settable field
            return false;
        case QAppointmentModel::Categories:
            if (v.canConvert(QVariant::StringList)) {
                appointment.setCategories(v.toStringList());
                return true;
            }
            return false;
        case QAppointmentModel::Notes:
            if (v.canConvert(QVariant::String)) {
                appointment.setNotes(v.toString());
                return true;
            }
            return false;
        case QAppointmentModel::Description:
            if (v.canConvert(QVariant::String)) {
                appointment.setDescription(v.toString());
                return true;
            }
            return false;
        case QAppointmentModel::Location:
            if (v.canConvert(QVariant::String)) {
                appointment.setLocation(v.toString());
                return true;
            }
            return false;
        case QAppointmentModel::Start:
            if (v.canConvert(QVariant::DateTime)) {
                appointment.setStart(v.toDateTime());
                return true;
            }
            return false;
        case QAppointmentModel::End:
            if (v.canConvert(QVariant::DateTime)) {
                appointment.setEnd(v.toDateTime());
                return true;
            }
            return false;
        case QAppointmentModel::AllDay:
            if (v.canConvert(QVariant::Bool)) {
                appointment.setAllDay(v.toBool());
                return true;
            }
            return false;
        case QAppointmentModel::TimeZone:
            if (v.canConvert(QVariant::String)) {
                appointment.setTimeZone(QTimeZone(v.toString().toAscii()));
                return true;
            }
            return false;
        case QAppointmentModel::Alarm:
            if (v.canConvert(QVariant::Int)) {
                appointment.setAlarm(appointment.alarmDelay(), (QAppointment::AlarmFlags)v.toInt());
                return true;
            }
            return false;
        case QAppointmentModel::RepeatRule:
            if (v.canConvert(QVariant::Int)) {
                appointment.setRepeatRule((QAppointment::RepeatRule)v.toInt());
                return true;
            }
            return false;
        case QAppointmentModel::RepeatFrequency:
            if (v.canConvert(QVariant::Int)) {
                appointment.setFrequency(v.toInt());
                return true;
            }
            return false;
        case QAppointmentModel::RepeatEndDate:
            if (v.canConvert(QVariant::Date)) {
                appointment.setRepeatUntil(v.toDate());
                return true;
            }
            return false;
        case QAppointmentModel::RepeatWeekFlags:
            if (v.canConvert(QVariant::Int)) {
                appointment.setWeekFlags((QAppointment::WeekFlags)v.toInt());
                return true;
            }
            return false;
    }
    return false;
}
/*!
  Updates the appointment \a appointment so long as a there is a appointment in the
  QAppointmentModel with the same uid as \a appointment.

  Returns true if the appointment was successfully updated.  Otherwise return false.
*/
bool QAppointmentModel::updateAppointment(const QAppointment& appointment)
{
    QAppointmentContext *c= qobject_cast<QAppointmentContext *>(context(appointment.uid()));
    if (c && c->updateAppointment(appointment)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Removes the appointment \a appointment so long as there is a appointment in the QAppointmentModel with
  the same uid as \a appointment.

  Returns true if the appointment was successfully removed.  Otherwise return false.
*/
bool QAppointmentModel::removeAppointment(const QAppointment& appointment)
{
    return removeAppointment(appointment.uid());
}

/*!
  Removes the appointment that has the uid \a id from the QAppointmentModel;

  Returns true if the appointment was successfully removed.  Otherwise return false.
*/
bool QAppointmentModel::removeAppointment(const QUniqueId& id)
{
    QAppointmentContext *c= qobject_cast<QAppointmentContext *>(context(id));
    if (c && c->removeAppointment(id)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Adds the appointment \a appointment to the QAppointmentModel under the storage source \a source.
  If source is empty will add the appointment to the default storage source.

  Returns true if the appointment was successfully added.  Otherwise return false.
*/
QUniqueId QAppointmentModel::addAppointment(const QAppointment& appointment, const QPimSource &source)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(source));

    QUniqueId id;
    if (c && !(id = c->addAppointment(appointment, source)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  Removes the occurrence \a occurrence from a series of repeating appointments, by creating an exception.

  Returns true if the exception was successfully added. Otherwise returns false.
*/

bool QAppointmentModel::removeOccurrence(const QOccurrence& occurrence)
{
    return removeOccurrence(occurrence.uid(), occurrence.start().date());
}

/*!
  \overload

  Removes the occurrence of appointment \a appointment, that occurs on the date \a date, by creating an exception.

  Returns true if the exception was successfully added. Otherwise returns false.
*/

bool QAppointmentModel::removeOccurrence(const QAppointment& appointment, const QDate &date)
{
    return removeOccurrence(appointment.uid(), date);
}

/*!
  \overload

  Mark the repeating appointment identified by \a id in the this context so as not
  to occur on \a date.  Returns true if the appointment was successfully updated,
  otherwise returns false.
*/

bool QAppointmentModel::removeOccurrence(const QUniqueId &id, const QDate &date)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(id));
    if (c && c->removeOccurrence(id, date)) {
        refresh();
        return true;
    }
    return false;
}

/*!
  Replaces an occurrence of the appointment \a appointment on the date \a date with \a occurrence.
  If \a date is null, the start date of the supplied \a occurrence will be used to create the
  exception.

  Returns the uid of the replacement appointment.
*/

QUniqueId QAppointmentModel::replaceOccurrence(const QAppointment& appointment, const QOccurrence& occurrence, const QDate& date)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(appointment.uid()));
    QUniqueId id;
    if (c && !(id = c->replaceOccurrence(appointment.uid(), occurrence, date)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  Remove all occurrences of the appointment \a appointment with occurrences of the appointment \a replacement that
  occur on or after the date \a date.  If \a date is null, the start date of the \a replacement will be used as
  the transition date.  The original \a appointment will have its repeatUntil date set to the day before the
  transition date.

  Returns the uid of the replacement appointment.
*/

QUniqueId QAppointmentModel::replaceRemaining(const QAppointment& appointment, const QAppointment& replacement, const QDate& date)
{
    QAppointmentContext *c = qobject_cast<QAppointmentContext *>(context(appointment.uid()));
    QUniqueId id;
    if (c && !(id = c->replaceRemaining(appointment.uid(), replacement, date)).isNull()) {
        refresh();
        return id;
    }
    return QUniqueId();
}

/*!
  Removes the appointments specified by the list of uids \a ids.

  Returns true if appointments are successfully removed.  Otherwise returns  false.
*/
bool QAppointmentModel::removeList(const QList<QUniqueId> &ids)
{
    // TODO needs better mixing code.
    // e.g. build list for each model loaded,
    // then remove list at a time, instead of appointment at a time.
    QUniqueId id;
    foreach(id, ids) {
        if (!exists(id))
            return false;
    }
    foreach(id, ids) {
        removeAppointment(id);
    }
    return true;
}

/*!
  \overload

  Adds the PIM record encoded in \a bytes to the QAppointmentModel under the storage source \a source.
  The format of the record in \a bytes is given by \a format.  An empty \a format string will
  cause the record to be read using the data stream operators for the PIM data type of the model.
  If \a source is empty will add the record to the default storage source.

  Returns valid id if the record was successfully added.  Otherwise returns an invalid id.

  Can only add PIM data that is represented by the model.  This means that only appointment data
  can be added using a QAppointmentModel.  Valid formats are "vCalendar" or an empty string.

*/
QUniqueId QAppointmentModel::addRecord(const QByteArray &bytes, const QPimSource &source, const QString &format)
{
    if (format == "vCalendar") {
        QList<QAppointment> list = QAppointment::readVCalendar(bytes);
        if (list.count() == 1)
            return addAppointment(list[0], source);
    } else {
        QAppointment a;
        QDataStream ds(bytes);
        ds >> a;
        return addAppointment(a, source);
    }
    return QUniqueId();
}

/*!
  \overload

  Updates the record enoded in \a bytes so long as there is a record in the QAppointmentModel with
  the same uid as the record.  The format of the record in \a bytes is given by \a format.
  An empty \a format string will cause the record to be read using the data stream operators
  for the PIM data type of the model. If \a id is not null will set the record uid to \a id
  before attempting to update the record.

  Returns true if the record was successfully updated.  Otherwise returns false.
*/
bool QAppointmentModel::updateRecord(const QUniqueId &id, const QByteArray &bytes, const QString &format)
{
    QAppointment a;
    if (format == "vCalendar") {
        QList<QAppointment> list = QAppointment::readVCalendar(bytes);
        if (list.count() == 1) {
            a = list[0];
        }
    } else {
        QDataStream ds(bytes);
        ds >> a;
    }
    if (!id.isNull())
        a.setUid(id);
    return updateAppointment(a);
}

/*!
  \fn bool QAppointmentModel::removeRecord(const QUniqueId &id)
  \overload

  Removes the record that has the uid \a id from the QAppointmentModel.

  Returns true if the record was successfully removed.  Otherwise returns false.
*/

/*!
  \overload

    Returns the record with the identifier \a id encoded in the format specified by \a format.
    An empty \a format string will cause the record to be written using the data stream
    operators for the PIM data type of the model.
*/
QByteArray QAppointmentModel::record(const QUniqueId &id, const QString &format) const
{
    QAppointment a = appointment(id);
    if (a.uid().isNull())
        return QByteArray();

    QByteArray bytes;
    QDataStream ds(&bytes, QIODevice::WriteOnly);
    if (format == "vCalendar") {
        a.writeVCalendar(&ds);
        return bytes;
    } else {
        ds << a;
        return bytes;
    }
    return QByteArray();
}

/*!
   Sets the model to only contain appointments of the specified duration \a type
*/
void QAppointmentModel::setDurationType(DurationType type)
{
    if (d->durationType == type)
        return;
    d->durationType = type;

    foreach(QRecordIO *model, accessModels()) {
        QAppointmentIO *appointmentModel = qobject_cast<QAppointmentIO *>(model);
        appointmentModel->setDurationType(type);
    }
}

/*!
  Returns the duration type of appointments currently included in the model.
*/
QAppointmentModel::DurationType QAppointmentModel::durationType() const
{
    return d->durationType;
}

/*!
  Set the model to only include appointments that end or have occurrences that end on or
  after \a rangeStart and that start before \a rangeEnd.

  Will include all events if \a rangeStart and \a rangeEnd are null.  Likewise will
  include all events that may occur or after \a rangeStart if \a rangeEnd is null and all
  events that may occur on or before \a rangeEnd if \a rangeStart is null.
*/
void QAppointmentModel::setRange(const QDateTime &rangeStart, const QDateTime &rangeEnd)
{
    if (d->rStart == rangeStart && d->rEnd == rangeEnd)
        return;
    d->rStart = rangeStart;
    d->rEnd = rangeEnd;

    foreach(QRecordIO *model, accessModels()) {
        QAppointmentIO *appointmentModel = qobject_cast<QAppointmentIO *>(model);
        appointmentModel->setRangeFilter(rangeStart, rangeEnd);
    }
}

/*!
  Returns the start of the range filter for the model if one is currently applied.

  \sa setRange()
*/
QDateTime QAppointmentModel::rangeStart() const
{
    return d->rStart;
}

/*!
  Returns the end of the range filter for the model if one is currently applied.

  \sa setRange()
 */
QDateTime QAppointmentModel::rangeEnd() const
{
    return d->rEnd;
}

// QOccurrenceModel start

class QOccurrenceModelData
{
public:
    QOccurrenceModelData() : requestedCount(-1) {}

    QAppointmentModel *appointmentModel;

    // cache of ranges
    QDateTime start;
    QDateTime end;
    int requestedCount;

    struct OccurrenceItem
    {
        QDateTime startInTZ;
        QDateTime endInTZ;
        QDate date;
        QUniqueId id;
        int row;
    };

    QVector< OccurrenceItem > cache;
};

/*!
  \class QOccurrenceModel
  \module qpepim
  \ingroup pim
  \brief The QOccurrenceModel class provides access to the Calendar data.

  The QOccurrenceModel is used to access the Calendar data.  It is a descendant of QAbstractItemModel
  so is suitable for use with the Qt View classes such as QListView and QTableView, as well as
  any custom developer Views.

  It differs from the QAppointmentModel in that it will show each occurrence of appointments as a
  separate item in the model.  Since some appointments have infinite occurrences a date
  range must be specified to limit the total set of occurrences included in the model.  Another important difference
  is that the values for the QAppointmentModel::Start and QAppointmentModel::End columns are converted
  to the current time zone of the device.
*/

/*!
  \enum QAppointmentModel::DurationType

  Describes the set of occurrences that should be shown by the model

  \value TimedDuration Occurrences that have times and dates specified for when they start and end.
  \value AllDayDuration Occurrences that only have a date specified for when they start and end.
  \value AnyDuration Occurrences with either date only or date and time specified for when they start and end.
*/

/*!
  \fn QAppointment QOccurrenceModel::appointment(const QModelIndex &index) const

  Returns the appointment for the row specified by \a index.
  The column of \a index is ignored.
*/

/*!
  \fn QAppointment QOccurrenceModel::appointment(int row) const

  Returns the appointment for the \a row specified.
*/

/*!
  Constructs a QOccurrenceModel that contains appointments that occur in the
  range of \a start to \a end.  The model will have the parent \a parent.
  Both \a start and \a end should be in the local time zone.
*/
QOccurrenceModel::QOccurrenceModel(const QDateTime &start, const QDateTime &end, QObject *parent)
    : QAbstractItemModel(parent)
{
    init(new QAppointmentModel(this));
    setRange(start, end);
}

/*!
  Constructs a QOccurrenceModel that contains appointments that
  occur at or after \a start (in the local time zone) for a total
  of \a count occurrences.  The model will have the parent \a parent.
*/
QOccurrenceModel::QOccurrenceModel(const QDateTime &start, int count, QObject *parent)
    : QAbstractItemModel(parent)
{
    init(new QAppointmentModel(this));
    setRange(start, count);
}

void QOccurrenceModel::init(QAppointmentModel *appointmentModel)
{
    od = new QOccurrenceModelData();

    od->appointmentModel = appointmentModel;
    connect(od->appointmentModel, SIGNAL(modelReset()), this, SLOT(rebuildCache()));
}

/*!
  Destroys a QOccurrenceModel.
*/
QOccurrenceModel::~QOccurrenceModel()
{
    delete od;
}

/*!
  \fn int QOccurrenceModel::count() const

  Returns the number of occurrences visible in the current filter mode.
*/

/*!
  Return the number of rows under the given \a parent.
*/
int QOccurrenceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return od->cache.size();
}

/*!
  \overload

  Returns the number of columns for the given \a parent.
*/
int QOccurrenceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return QAppointmentModel::Categories + 1;   // last column + 1
}

/*!
  \overload

  Returns an object that contains a serialized description of the specified \a indexes.
  The format used to describe the items corresponding to the \a indexes is obtained from
  the mimeTypes() function.

  If the list of indexes is empty, 0 is returned rather than a serialized empty list.

  Currently returns 0 but may be implemented at a future date.
*/
QMimeData *QOccurrenceModel::mimeData(const QModelIndexList &indexes) const
{
    Q_UNUSED(indexes)
    return 0;
}

/*!
  \overload

  Returns a list of MIME types that can be used to describe a list of model indexes.

  Currently returns an empty list but may be implemented at a future date.
*/
QStringList QOccurrenceModel::mimeTypes() const
{
    return QStringList();
}

QVariant QOccurrenceModel::appointmentData(int row, int column) const
{
    return od->appointmentModel->data(od->appointmentModel->index(row, column), Qt::DisplayRole);
}

/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.
*/
QVariant QOccurrenceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount())
        return QVariant();

    QOccurrenceModelData::OccurrenceItem item = od->cache.at(index.row());

    switch(index.column()) {
        case QAppointmentModel::Description:
            switch(role) {
                default:
                    break;
                case Qt::DisplayRole:
                    return appointmentData(item.row, QAppointmentModel::Description);
                case Qt::EditRole:
                    return item.id.toByteArray();
                case QAppointmentModel::LabelRole:
                    {
                        QString l = appointmentData(item.row, QAppointmentModel::Description).toString();
                        return "<b>" + l + "</b>";
                    }
                case Qt::BackgroundColorRole:
                    if( appointmentData(item.row, QAppointmentModel::RepeatRule)
                            != QAppointment::NoRepeat )
                        return QVariant( QColor( 0, 50, 255 ) );
                    else
                        return QVariant( QColor( 255, 50, 0 ) );
                case Qt::DecorationRole:
                    {
                        QOccurrence o = occurrence(index);
                        QList<QVariant> icons;
                        if( o.appointment().hasRepeat() )
                            icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/repeat" ) ) );
                        if( o.appointment().isException() )
                            icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/repeatException" ) ) );
                        if( o.appointment().timeZone() != QTimeZone() && o.appointment().timeZone() != QTimeZone::current() )
                            icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/globe" ) ) );
                        switch( o.alarm() ) {
                            case QAppointment::Audible:
                                icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/audible" ) ) );
                                break;
                            case QAppointment::Visible:
                                icons.append( QVariant( QAppointmentModelData::getCachedIcon( ":icon/silent" ) ) );
                                break;
                            default:
                                break;
                        }
                        return icons;
                    }
            }
            break;
        case QAppointmentModel::Start:
            return od->cache.at(index.row()).startInTZ;
        case QAppointmentModel::End:
            return od->cache.at(index.row()).endInTZ;
        default:
            break;
    }
    return od->appointmentModel->data(od->appointmentModel->index(item.row, index.column()), role);
}

/*!
  \overload
  Returns false since QOccurrenceModel does not allow editing.  \a index, \a value and \a role
  are ignored.
*/
bool QOccurrenceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false; // not settable in occurrence model.
}

/*!
  \overload
  Returns false since QOccurrenceModel does not allow editing.  \a index and \a values
  are ignored
*/
bool QOccurrenceModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &values)
{
    Q_UNUSED(index)
    Q_UNUSED(values)
    return false;
}

/*!
  \overload

  Returns a map with values for all predefined roles in the model for the item at the
  given \a index.
*/
QMap<int,QVariant> QOccurrenceModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> result;
    switch (index.column()) {
        case QAppointmentModel::Description:
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            result.insert(Qt::EditRole, data(index, Qt::EditRole));
            break;
        default:
            result.insert(Qt::DisplayRole, data(index, Qt::DisplayRole));
            break;
    }
    return result;
}

/*!
  Returns true if the current filter mode of the model contains \a index.
  Otherwise returns false.
*/
bool QOccurrenceModel::contains(const QModelIndex &index) const
{
    return (index.row() >= 0 && index.row() < rowCount());
}

/*!
  Returns true if the current filter mode of the model contains an occurrence
  for the appointment with uid \a id.
  Otherwise returns false.
*/
bool QOccurrenceModel::contains(const QUniqueId &id) const
{
    for (int i = 0; i < od->cache.size(); ++i) {
        if (od->cache.at(i).id == id)
            return true;
    }
    return false;
}

/*!
  If the model contains an occurrence representing an appointment with uid \a id returns
  the index of the first occurrence.
  Otherwise returns a null QModelIndex

  \sa contains()
*/
QModelIndex QOccurrenceModel::index(const QUniqueId &id) const
{
    for (int i = 0; i < od->cache.size(); ++i) {
        if (od->cache.at(i).id == id)
            return createIndex(i, 0);
    }
    return QModelIndex();
}

/*!
  Return the id for the appointment represented by the occurrence
  at the row specified by \a index.  If index is null or out of the range of
  the model will return a null id.
*/
QUniqueId QOccurrenceModel::id(const QModelIndex &index) const
{
    return od->cache.at(index.row()).id;
}

/*!
  If the model contains an occurrence \a o returns the index of that occurrence.
  Otherwise returns a null QModelIndex
*/
QModelIndex QOccurrenceModel::index(const QOccurrence &o) const
{
    if (od->cache.count() < 1)
        return QModelIndex();
    for (int i = 0; i < od->cache.size(); ++i) {
        if (od->cache.at(i).id == o.uid() && od->cache.at(i).date == o.date())
            return createIndex(i, 0);
    }
    return QModelIndex();
}

/*!
  Returns the occurrence for the row specified by \a index.  The column of \a index
 is ignored.
*/
QOccurrence QOccurrenceModel::occurrence(const QModelIndex &index) const
{
    return occurrence(index.row());
}

/*!
  Returns the occurrence at the \a row specified.
*/
QOccurrence QOccurrenceModel::occurrence(int row) const
{
    QOccurrenceModelData::OccurrenceItem value = od->cache[row];
    return QOccurrence(value.date, od->appointmentModel->appointment(value.id));
}

/*!
   Returns the occurrence that occurs on the \a date specified for the appointment with
   uid \a id.  If the appointment does not exists or does not occur on the \a date,
   returns a null occurrence.
*/
QOccurrence QOccurrenceModel::occurrence(const QUniqueId &id, const QDate &date) const
{
    QOccurrence o(date, appointment(id));
    if (index(o).isValid())
        return o;
    return QOccurrence();
}

/*!
  Returns the appointment with the identifier \a id.  The appointment does
  not have to be in the current filter mode for it to be returned.
*/
QAppointment QOccurrenceModel::appointment(const QUniqueId &id) const
{
    return od->appointmentModel->appointment(id);
}

/*!
  \overload
  Returns the index of the item in the model specified by the given \a row, \a column
  and \a parent index.
*/
QModelIndex QOccurrenceModel::index(int row,int column,const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || row >= rowCount() || column < 0 || column >= columnCount())
        return QModelIndex();
    return createIndex(row,column);
}

/*!
  \overload
    Returns the parent of the model item with the given \a index.
*/
QModelIndex QOccurrenceModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

/*!
  Sets the model to only contain occurrences of the specified duration \a type.
*/
void QOccurrenceModel::setDurationType(QAppointmentModel::DurationType type)
{
    od->appointmentModel->setDurationType(type);
}

/*!
  Returns the duration type of occurrences currently included in the model.
*/
QAppointmentModel::DurationType QOccurrenceModel::durationType() const
{
    return od->appointmentModel->durationType();
}

// no update or edit functions.
// get the appointment and edit that.

/*!
  Sets the model to only contain the first \a count occurrences that occur at or after the
  \a start (in the local time zone) of the specified range.
*/
void QOccurrenceModel::setRange(const QDateTime &start, int count)
{
    if (!start.isValid() || count < 1)
        return;
    od->appointmentModel->setRange(start, QDateTime());
    od->start = start;
    od->end = QDateTime();
    od->requestedCount = count;
}

/*!
  Sets the model to only contain occurrences that start before the \a end of the range,
  and end at or after the \a start of the range.  Both \a start and \a end should be
  in the local time zone.
*/
void QOccurrenceModel::setRange(const QDateTime &start, const QDateTime &end)
{
    if (!start.isValid() || !end.isValid() || start >= end)
        return;
    od->appointmentModel->setRange(start.addDays(-1), end.addDays(1)); // account for TZ
    od->start = start;
    od->end = end;
    od->requestedCount = -1;
}

/*!
  Returns the start of the range specified that occurrences must end at or after to be
  included in the model, in the local time zone.
*/
QDateTime QOccurrenceModel::rangeStart() const
{
    return od->start;
}

/*!
  Returns the end of the range specified that occurrences must start before to be included
  in the model, in the local time zone.

  If the range was specified by a start and a count, this will return a null QDateTime.
*/
QDateTime QOccurrenceModel::rangeEnd() const
{
    return od->end;
}

/*!
  Set the model to only contain occurrences accepted by the QCategoryFilter \a f.
*/
void QOccurrenceModel::setCategoryFilter(const QCategoryFilter &f)
{
    od->appointmentModel->setCategoryFilter(f);
}

/*!
  Returns the QCategoryFilter that occurrences are tested against for the current filter mode.
*/
QCategoryFilter QOccurrenceModel::categoryFilter() const
{
    return od->appointmentModel->categoryFilter();
}

/*!
  Returns the list of sources of occurrence data that are currently shown by the
  occurrence model.
*/
QSet<QPimSource> QOccurrenceModel::visibleSources() const
{
    return od->appointmentModel->visibleSources();
}

/*!
  Sets the QAppointmentModel to show only appointments contained in the storage sources specified
  by \a list.

  Also refreshes the model.
*/
void QOccurrenceModel::setVisibleSources(const QSet<QPimSource> &list)
{
    od->appointmentModel->setVisibleSources(list);
}

/*!
  Returns the set of identifiers for storage sources that can be shown.
*/
QSet<QPimSource> QOccurrenceModel::availableSources() const
{
    return od->appointmentModel->availableSources();
}

/*!
  Returns true if the appointment uid \a id is stored in the storage source \a source.
  Otherwise returns false.
*/
bool QOccurrenceModel::sourceExists(const QPimSource &source, const QUniqueId &id) const
{
    return od->appointmentModel->sourceExists(source, id);
}

/*!
  Returns true if the occurrence for \a index can be updated or removed.
  Otherwise returns false.
*/
bool QOccurrenceModel::editable(const QModelIndex &index) const
{
    return editable(id(index));
}

/*!
  Returns true if the appointment for \a id can be updated or removed.
  Otherwise returns false.
*/
bool QOccurrenceModel::editable(const QUniqueId &id) const
{
    return od->appointmentModel->editable(id);
}

/*!
  Returns true if the occurrence model has not yet updated the list of occurrence
  due to a change in stored appointments.

  \sa fetchCompleted(), completeFetch()
*/
bool QOccurrenceModel::fetching() const
{
    return false;
}

/*!
  \fn void QOccurrenceModel::fetchCompleted()

  This signal is emitted when the occurrence model finished caching changes
  resulting from a change to the stored appointments.

  \sa completeFetch(), fetching()
*/

/*!
  Forces the occurrence model to finish updating its internal cache now.
  This can be expensive and should only be called if no further modifications
  to appointments is expected.  The occurrence model will update its cache in
  the next Qt event loop if this function is not called.

  \sa fetchCompleted(), fetching()
*/
void QOccurrenceModel::completeFetch()
{
    od->appointmentModel->refresh();
}

/*!
  Rebuilds the cache of repeating occurrences that fit within the range.

  Ensures any recently updates to the model will be reflected in queries against the model.

  This can be an expensive operation.
*/
void QOccurrenceModel::rebuildCache()
{
    /*
       for each appointment, build a list of occurrences.
   */
    if( (!od->end.isNull() || od->requestedCount > 0) && !od->start.isNull()) {
        QMultiMap< QDateTime, QOccurrenceModelData::OccurrenceItem > result;

        int c = od->appointmentModel->rowCount(); // count can be expensive for some types of requests.
        for (int i = 0; i < c; i++) {
            QAppointment a = od->appointmentModel->appointment(i);

            // not first occurrence, that may be far too early.
            QOccurrence o = a.nextOccurrence(od->start.date().addDays(-1));
            int individualCount = 0;
            while(o.isValid()) {
                individualCount++;
                // work out if in range.
                QDateTime start = o.startInCurrentTZ();
                QDateTime end = o.endInCurrentTZ().addSecs(-1);

                if ((od->end.isNull() || start < od->end) && end >= od->start) {
                    QOccurrenceModelData::OccurrenceItem item;
                    item.startInTZ = start;
                    item.endInTZ = end.addSecs(1);
                    item.date = o.date();
                    item.id = o.uid();
                    item.row = i;
                    result.insert(start, item);
                }
                // escape for forever events going past end range.
                else if (!od->end.isNull() && start > od->end)
                    break;
                // escape for forever events going past max count.
                else if (od->requestedCount > 0 && individualCount >= od->requestedCount)
                    break;
                o = o.nextOccurrence();
            }
        }
        // sort
        od->cache = result.values().toVector();
        // truncate
        if (od->requestedCount > 0 && od->cache.size() > od->requestedCount)
            od->cache.resize(od->requestedCount);

        reset();
    }
    // and whether the cache changed, didn't change, or didn't even update....
    emit fetchCompleted();
}

/*!
  Forces a refresh of the Occurrence data.
*/
void QOccurrenceModel::refresh()
{
    reset();
}

