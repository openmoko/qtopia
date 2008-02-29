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
#include <QPainter>
#include <QMap>
#include <QList>
#include <QSet>
#include <QPixmap>
#include <QFile>
#include <QTextDocument>
#include <QTimer>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QStyle>

#include <QDebug>

#include "qrecordiomerge_p.h"

/*!
   \class QAppointmentModel
   \brief QAppointmentModel represents the appointments in the database.

   This class allows you to view, update and delete appointments.
*/

class QAppointmentModelData
{
public:
    QAppointmentModelData() : mio(0), defaultmodel(0), durationType(QAppointmentModel::AnyDuration) {}
    QBiasedRecordIOMerge *mio;

    QAppointmentIO *defaultmodel;
    QAppointmentContext *defaultContext;
    QList<QAppointmentContext *> contexts;
    QList<QAppointmentIO*> models;

    QDateTime rStart;
    QDateTime rEnd;
    QAppointmentModel::DurationType durationType;
};

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

    return QIcon(":image/datebook/" + ident);
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
    : QAbstractItemModel(parent)
{
    QtopiaSql::openDatabase();
    d = new QAppointmentModelData();
    d->mio = new QBiasedRecordIOMerge(this);

    d->defaultmodel = new QAppointmentSqlIO(this);

    QAppointmentDefaultContext *dcon = new QAppointmentDefaultContext(this, d->defaultmodel);

    d->defaultContext = dcon;

    d->models.append(d->defaultmodel);
    d->contexts.append(dcon);

#ifdef GOOGLE_CALENDAR_CONTEXT
    QGoogleCalendarContext *gcon = new QGoogleCalendarContext(this, d->defaultmodel);
    d->contexts.append(gcon);
#endif

    d->mio->setPrimaryModel(d->defaultmodel);

    // XXX BiasedRecordIOMerge doesn't emit reset until after a timer event
    // is dispatched, which leaves us inconsistent for a short while after
    // a change.  This mostly affects QOccurrenceModel, which has a higher
    // level cache.  There's no API for querying whether a IOMerge or a
    // QAppointmentModel is pending a cache rebuild, unfortunately.
    // Thus we monitor the models directly.
    foreach(const QAppointmentIO *model, d->models) {
        connect(model, SIGNAL(recordsUpdated()), this, SLOT(voidCache()));
    }
    // This might not be necessary any more
    connect(d->mio, SIGNAL(reset()), this, SLOT(voidCache()));
}

/*!
  Destructs the QAppointmentModel.
*/
QAppointmentModel::~QAppointmentModel()
{
    delete d;
}

void QAppointmentModel::voidCache()
{
    reset();
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
    //generic = QPixmap(":image/datebook/generic-appointment");
    QAppointment c(appointment(index));
    switch(index.column()) {
        case Description:
            if (index.row() < rowCount()){
                // later, take better advantage of roles.
                switch(role) {
                    default:
                        break;
                    case Qt::DisplayRole:
                        return QVariant(c.description());
                    case Qt::EditRole:
                        return QVariant(c.uid().toByteArray());
                    case LabelRole:
                        {
                            QString l = c.description();
                            return "<b>" + l + "</b>";
                        }
                        break;
                }
            }
            break;
        default:
            if (index.column() > 0 && index.column() < columnCount())
                return appointmentField(c, (Field)index.column());
            break;
    }
    return QVariant();
}

/*!
  \fn int QAppointmentModel::count() const

  Returns the number of appointments visible in the current filter mode.
*/

/*!
  \overload

  Returns the number of rows under the given \a parent.
*/
int QAppointmentModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->mio->count();
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
bool QAppointmentModel::setData(const QModelIndex &index, const QVariant &value, int role) const
{
    if (role != Qt::EditRole)
        return false;
    if (!index.isValid())
        return false;

    int i = index.row();
    const QAppointmentIO *model = qobject_cast<const QAppointmentIO*>(d->mio->model(i));
    int r = d->mio->row(i);
    if (model)
        return ((QAppointmentIO *)model)->setAppointmentField(r, (Field)index.column(), value);
    return false;
}

/*!
  \overload
  For every Qt::ItemDataRole in \a roles, sets the role data for the item at \a index to the
  associated value in \a roles. Returns true if successful, otherwise returns false.
*/
bool QAppointmentModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &roles) const
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
  \overload
    Returns the parent of the model item with the given \a index.
*/
QModelIndex QAppointmentModel::parent(const QModelIndex &index) const
{
    // TODO depend on view style.
    Q_UNUSED(index);
    return QModelIndex();
}

/*!
  \overload
  Returns true if \a parent has any children; otherwise returns false.
  Use rowCount() on the parent to find out the number of children.

  \sa parent(), index()
*/
bool QAppointmentModel::hasChildren(const QModelIndex &parent) const
{
    // TODO maybe?  after all, the concept of children does exist.
    Q_UNUSED(parent)
    return false;
}

/*!
  Ensures the data in Appointments is in a state suitable for syncing.
*/
bool QAppointmentModel::flush() { return true; }

/*!
  Forces a refresh of the Appointment data.
*/
bool QAppointmentModel::refresh() { reset(); return true; }

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
    const QAppointmentIO *model = qobject_cast<const QAppointmentIO*>(d->mio->model(row));
    int r = d->mio->row(row);
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
    foreach(const QAppointmentIO *model, d->models) {
        if (model->exists(id))
            return model->appointment(id);
    }
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
    foreach(QAppointmentContext *context, d->contexts) {
        if (context->exists(appointment.uid())) {
            return context->updateAppointment(appointment);
        }
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
    foreach(QAppointmentContext *context, d->contexts) {
        if (context->exists(id)) {
            return context->removeAppointment(id);
        }
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
    if (source.isNull()) {
        return d->defaultContext->addAppointment(appointment, source);
    } else {
        foreach(QAppointmentContext *context, d->contexts) {
            if (context->sources().contains(source))
                return context->addAppointment(appointment, source);
        }
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
    foreach(QAppointmentContext *context, d->contexts) {
        if (context->exists(id))
            return context->removeOccurrence(id, date);
    }
    return false;
}

/*!
  Replaces an occurrence of the appointment \a appointment with an \a occurrence that begins on the same date.

  Returns the uid of the replacement appointment.
*/

QUniqueId QAppointmentModel::replaceOccurrence(const QAppointment& appointment, const QOccurrence& occurrence)
{
    foreach(QAppointmentContext *context, d->contexts) {
        if (context->exists(appointment.uid()))
            return context->replaceOccurrence(appointment.uid(), occurrence);
    }
    return QUniqueId();
}

/*!
  Remove all occurrences of the appointment \a appointment with occurrences of the appointment \a replacement that
  occur on or after the start date of the \a replacement.

  Returns the uid of the replacement appointment.
*/

QUniqueId QAppointmentModel::replaceRemaining(const QAppointment& appointment, const QAppointment& replacement)
{
    foreach(QAppointmentContext *context, d->contexts) {
        if (context->exists(appointment.uid()))
            return context->replaceRemaining(appointment.uid(), replacement);
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
  Returns true if a appointment with the uid \a id is stored in the appointment model.  Otherwise
  return false.

  The appointment with uid id does not need to be in the current filter mode.
*/
bool QAppointmentModel::exists(const QUniqueId &id) const
{
    foreach(const QAppointmentIO *model, d->models) {
        if (model->exists(id))
            return true;
    }
    return false;
}

/*!
   Sets the model to only contain appointments of the specified duration \a type
*/
void QAppointmentModel::setDurationType(DurationType type)
{
    if (d->durationType == type)
        return;
    d->durationType = type;

    foreach(QAppointmentIO *model, d->models)
        model->setDurationType(type);
    d->mio->rebuildCache();
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

    foreach(QAppointmentIO *model, d->models)
        model->setRangeFilter(rangeStart, rangeEnd);
    d->mio->rebuildCache();
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

/*!
  Set the model to only contain appointments accepted by the QCategoryFilter \a f.
*/
void QAppointmentModel::setCategoryFilter(const QCategoryFilter &f)
{
    if (f == categoryFilter())
        return;

    foreach(QAppointmentIO *model, d->models)
        model->setCategoryFilter(f);
    d->mio->rebuildCache();
}

/*!
  Returns the QCategoryFilter that appointments are tested against for the current filter mode.
*/
QCategoryFilter QAppointmentModel::categoryFilter() const
{
    // assumed others are the same.
    return d->defaultmodel->categoryFilter();
}

/*!
  \overload
  Returns the index of the item in the model specified by the given \a row, \a column
  and \a parent index.
*/
QModelIndex QAppointmentModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (row < 0 || row >= rowCount() || column < 0 || column >= columnCount())
        return QModelIndex();
    return createIndex(row,column);
}

/*!
  If the model contains a appointment with uid \a id returns the index of the appointment.
  Otherwise returns a null QModelIndex

  \sa exists()
*/
QModelIndex QAppointmentModel::index(const QUniqueId & id) const
{
    int i = d->mio->index(id);
    if (i == -1)
        return QModelIndex();
    return createIndex(i, 0);
}

/*!
  Return the id for the appointment at the row specified by \a index.
  if index is null or out of the range of the model will return a null id.
*/
QUniqueId QAppointmentModel::id(const QModelIndex &index) const
{
    int i = index.row();
    const QAppointmentIO *model = qobject_cast<const QAppointmentIO *>(d->mio->model(i));
    int r = d->mio->row(i);
    if (model)
        return model->id(r);
    return QUniqueId();
}

/*!
  Returns the list of sources of appointment data that are currently shown by the
  appointment model.
*/
QSet<QPimSource> QAppointmentModel::visibleSources() const
{
    QSet<QPimSource> set;
    foreach(QPimContext *c, d->contexts)
        set.unite(c->visibleSources());
    return set;
}

/*!
   Returns the contexts of appointment data that can be shown by the appointment model.
*/
const QList<QAppointmentContext*> &QAppointmentModel::contexts() const
{
    return d->contexts;
}

/*!
  Returns the context that contains the appointment with identifier \a id.
  If the contact does not exists returns 0.
*/
QAppointmentContext *QAppointmentModel::context(const QUniqueId &id) const
{
    foreach(QAppointmentContext *context, d->contexts) {
        if (context->exists(id))
            return context;
    }
    return 0;
}

/*!
  Returns the source identifier that contains the appointment with identifier \a id.
  If the appointment does not exist returns a null source.
*/
QPimSource QAppointmentModel::source(const QUniqueId &id) const
{
    foreach(QPimContext *context, d->contexts) {
        if (context->exists(id))
            return context->source(id);
    }
    return QPimSource();
}

/*!
  Sets the QAppointmentModel to show only appointments contained in the storage sources specified
  by \a list.

  Also refreshes the model.
*/
void QAppointmentModel::setVisibleSources(const QSet<QPimSource> &list)
{
    foreach (QPimContext *c, d->contexts) {
        QSet<QPimSource> cset = c->sources();
        cset.intersect(list);
        c->setVisibleSources(cset);
    }
    refresh();
}

/*!
  Returns true if the appointment uid \a id is stored in the storage source \a source.
  Otherwise returns false.
*/
bool QAppointmentModel::sourceExists(const QPimSource &source, const QUniqueId &id) const
{
    foreach (QPimContext *c, d->contexts) {
        if (c->sources().contains(source))
            return c->exists(id, source);
    }
    return false;
}

/*!
  Returns the set of identifiers for storage sources that can be shown.
*/
QSet<QPimSource> QAppointmentModel::availableSources() const
{
    QSet<QPimSource> set;
    foreach(QPimContext *c, d->contexts)
        set.unite(c->sources());
    return set;
}

/*!
  Returns true if the appointment for \a index can be updated or removed.
  Otherwise returns false.
*/
bool QAppointmentModel::editable(const QModelIndex &index) const
{
    return editable(id(index));
}

/*!
  Returns true if the appointment for \a id can be updated or removed.
  Otherwise returns false.
*/
bool QAppointmentModel::editable(const QUniqueId &id) const
{
    foreach (QPimContext *c, d->contexts) {
        if (c->exists(id))
            return c->editable();
    }
    return false;
}

// QOccurrenceModel start

class QOccurrenceModelData
{
public:
    QOccurrenceModelData() : requestedCount(-1), rebuildCacheTimer(0) {}

    QAppointmentModel *appointmentModel;

    // cache of ranges
    QDateTime start;
    QDateTime end;
    int requestedCount;

    QTimer *rebuildCacheTimer;

    QVector< QPair<QDate, QUniqueId> > cache;
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
  range must be specified to limit the total set of occurrences included in the model.
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
*/
QOccurrenceModel::QOccurrenceModel(const QDateTime &start, const QDateTime &end, QObject *parent)
    : QAbstractItemModel(parent)
{
    init(new QAppointmentModel(this));
    setRange(start, end);
}

/*!
  Constructs a QOccurrenceModel that contains appointments that
  occur in the range of \a start for a total of \a count
  occurrences.  The model will have the parent \a parent.
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
    od->rebuildCacheTimer = new QTimer(this);
    od->rebuildCacheTimer->setSingleShot(true);
    od->rebuildCacheTimer->setInterval(0);
    connect(od->rebuildCacheTimer, SIGNAL(timeout()), this, SLOT(rebuildCache()));

    od->appointmentModel = appointmentModel;
    connect(od->appointmentModel, SIGNAL(modelReset()), this, SLOT(voidCache()));
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

/*!
  \overload

  Returns the data stored under the given \a role for the item referred to by the \a index.
*/
QVariant QOccurrenceModel::data(const QModelIndex &index, int role) const
{
    // should have different roles?
    //generic = QPixmap(":image/datebook/generic-appointment");
    QOccurrence o = occurrence(index);
    switch(index.column()) {
        case QAppointmentModel::Description:
            if (index.row() < rowCount()){
                // later, take better advantage of roles.
                switch(role) {
                    default:
                        break;
                    case Qt::DisplayRole:
                        return QVariant(o.description());
                    case Qt::EditRole:
                        return QVariant(o.uid().toByteArray());
                    case QAppointmentModel::LabelRole:
                        {
                            QString l = o.description();
                            return "<b>" + l + "</b>";
                        }
                    case Qt::BackgroundColorRole:
                        if( o.appointment().hasRepeat() )
                            return QVariant( QColor( 0, 50, 255 ) );
                        else
                            return QVariant( QColor( 255, 50, 0 ) );
                    case Qt::DecorationRole:
                        {
                            QList<QVariant> icons;
                            if( o.appointment().hasRepeat() )
                                icons.append( QVariant( QIcon( ":icon/repeat" ) ) );
                            if( o.appointment().isException() )
                                icons.append( QVariant( QIcon( ":icon/repeatException" ) ) );
                            if( o.appointment().timeZone() != QTimeZone() && o.appointment().timeZone() != QTimeZone::current() )
                                icons.append( QVariant( QIcon( ":icon/globe" ) ) );
                            switch( o.alarm() ) {
                                case QAppointment::Audible:
                                    icons.append( QVariant( QIcon( ":icon/audible" ) ) );
                                    break;
                                case QAppointment::Visible:
                                    icons.append( QVariant( QIcon( ":icon/silent" ) ) );
                                    break;
                                default:
                                    break;
                            }
                            return icons;
                        }
                }
            }
            break;
        case QAppointmentModel::Start:
            return o.start();
        case QAppointmentModel::End:
            return o.end();
        default:
            return QAppointmentModel::appointmentField(o.appointment(), (QAppointmentModel::Field)index.column());
            break;
    }
    return QVariant();
}

/*!
  \overload
  Returns false since QOccurrenceModel does not allow editing.  \a index, \a value and \a role
  are ignored.
*/
bool QOccurrenceModel::setData(const QModelIndex &index, const QVariant &value, int role) const
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
bool QOccurrenceModel::setItemData(const QModelIndex &index, const QMap<int,QVariant> &values) const
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
        if (od->cache.at(i).second == id)
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
        if (od->cache.at(i).second == id)
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
    return od->cache.at(index.row()).second;
}

/*!
  If the model contains an occurrence \a o returns the index of that occurrence.
  Otherwise returns a null QModelIndex
*/
QModelIndex QOccurrenceModel::index(const QOccurrence &o) const
{
    if (od->cache.count() < 1)
        return QModelIndex();
    QPair<QDate, QUniqueId> value(o.date(), o.uid());
    int index = od->cache.indexOf(value);
    if (index > -1)
        return createIndex(index, 0);
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
    QPair<QDate, QUniqueId> value = od->cache[row];
    return QOccurrence(value.first, od->appointmentModel->appointment(value.second));
}

/*!
   Returns the occurrence that occurs on the \a date specified for the appointment with
   uid \a id.  If the appointment does not exists or does not occur on the \a date,
   returns a null occurrence.
*/
QOccurrence QOccurrenceModel::occurrence(const QUniqueId &id, const QDate &date) const
{
    QPair<QDate, QUniqueId> value(date, id);
    int index = od->cache.indexOf(value);
    if (index > -1)
        return QOccurrence(date, appointment(id));
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
  Sets the model to only contain the first \a count occurrences that occur after the
  \a start of the specified range
*/
void QOccurrenceModel::setRange(const QDateTime &start, int count)
{
    if (!start.isValid() || count < 1)
        return;
    od->appointmentModel->setRange(start, QDateTime());
    od->start = start;
    od->end = QDateTime();
    od->requestedCount = count;
    voidCache();
}

/*!
  Sets the model to only contain occurrences that occur between the \a start and
  \a end of the specified range.
*/
void QOccurrenceModel::setRange(const QDateTime &start, const QDateTime &end)
{
    if (!start.isValid() || !end.isValid() || start >= end)
        return;
    od->appointmentModel->setRange(start, end);
    od->start = start;
    od->end = end;
    od->requestedCount = -1;
    voidCache();
}

/*!
  Returns the start of the range specified that occurrences must end after to be
  included in the model
*/
QDateTime QOccurrenceModel::rangeStart() const
{
    return od->start;
}

/*!
  Returns the end of the range specified that occurrences must start before to be included
  in the model.

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

void QOccurrenceModel::voidCache()
{
    od->rebuildCacheTimer->start();
}

/*!
  Returns true if the occurrence model has not yet updated the list of occurrence
  due to a change in stored appointments.

  \sa fetchCompleted(), completeFetch()
*/
bool QOccurrenceModel::fetching() const
{
    return od->rebuildCacheTimer->isActive();
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
    if (fetching())
        rebuildCache();
}

/*!
  Rebuilds the cache of repeating occurrences that fit within the range.

  Ensures any recently updates to the model will be reflected in queries against the model.

  This can be an expensive operation.
*/
void QOccurrenceModel::rebuildCache()
{
    od->rebuildCacheTimer->stop();
    /*
       for each appointment, build a list of occurrences.
   */
    if( (!od->end.isNull() || od->requestedCount > 0) && !od->start.isNull()) {
        QMultiMap< QDateTime, QPair<QDate, QUniqueId> > result;

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

                if ((od->end.isNull() || start <= od->end) && end >= od->start)
                    result.insert(start, QPair<QDate, QUniqueId>(o.date(), o.appointment().uid()));
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

/*!
  Constructs a QAppointmentModelDelegate with parent \a parent.
*/
QAppointmentDelegate::QAppointmentDelegate( QObject * parent )
    : QAbstractItemDelegate(parent)
{
    iconSize = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize);
}

/*!
  Destroys a QAppointmentModelDelegate.
*/
QAppointmentDelegate::~QAppointmentDelegate() {}

/*!
  \internal
  Provides an alternate font based of the \a start font.  Reduces the size of the returned font
  by at least step point sizes.  Will attempt a total of six point sizes beyond the request
  point size until a valid font size that differs from the starting font size is found.
*/
QFont QAppointmentDelegate::differentFont(const QFont& start, int step) const
{
    int osize = QFontMetrics(start).lineSpacing();
    QFont f = start;
    for (int t=1; t<6; t++) {
        int newSize = f.pointSize() + step;
        if ( newSize > 0 )
            f.setPointSize(f.pointSize()+step);
        else
            return start; // we cannot find a font -> return old one
        step += step < 0 ? -1 : +1;
        QFontMetrics fm(f);
        if ( fm.lineSpacing() != osize )
            break;
    }
    return f;
}

/*!
  Returns the font to use for painting the main label text of the item.
  Due to the nature of rich text painting in Qt 4.0 attributes such as bold and italic will be
  ignored.  These attributes can be set by the text returned for
  QAbstractItemModel::data() where role is QAppointmentModel::LabelRole.

  By default returns the font of the style option \a o.
*/
QFont QAppointmentDelegate::mainFont(const QStyleOptionViewItem &o) const
{
    return o.font;
}

/*!
  Returns the font to use for painting the sub label text of the item.
  Due to the nature of rich text painting in Qt 4.0 attributes such as bold and italic will be
  ignored.  These attributes can be set by the text returned for
  QAbstractItemModel::data() where role is QAppointmentModel::SubLabelRole.

  By default returns a font at least two point sizes smaller of the font of the
  style option \a o.
*/
QFont QAppointmentDelegate::secondaryFont(const QStyleOptionViewItem &o) const
{
    return differentFont(o.font, -2);
}

/*!
  \overload

  Paints the element at \a index using \a painter with style options \a option.
*/
void QAppointmentDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option,
        const QModelIndex & index) const
{
    //  Prepare brush + pen and draw in background rectangle

    QRect border;
    QPen pen(Qt::black);
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.highlight());
        border = option.rect.adjusted(1, 1, -1, -1);
        pen.setWidth(2);
    } else {
        painter->setBrush(qvariant_cast<QColor>(index.model()->data(index, Qt::BackgroundColorRole)).light(170));
        border = option.rect.adjusted(0, 0, -1, -1);
    }

    painter->setPen(pen);
    painter->drawRect(border);

    //  Draw in the relevant event icons

    QRect contentRect = option.rect.adjusted(2, 2, -2, -2);
    int iconRow = 0;
    int drawnIconSize = qMin(qMin(contentRect.height(), contentRect.width()), iconSize);
    QList<QVariant> icons = index.model()->data( index, Qt::DecorationRole ).toList();
    for (QList<QVariant>::Iterator it = icons.begin(); it != icons.end(); ++it) {
        QIcon icon = qvariant_cast<QIcon>(*it);
        icon.paint(painter, contentRect.right() - drawnIconSize, contentRect.top() + iconRow, drawnIconSize, drawnIconSize);
        if(contentRect.height() > iconRow + drawnIconSize + 2)
            iconRow += drawnIconSize + 2;
        else
            contentRect.setRight(contentRect.right() - (drawnIconSize + 2));
    }
    if (iconRow > 0)
        contentRect.setRight(contentRect.right() - (drawnIconSize + 2));

    //  Prepare pen and draw in text

    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    else
        painter->setPen(option.palette.color(QPalette::Text));

    static QTextOption o(Qt::AlignLeft);
    o.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    painter->drawText(contentRect, index.model()->data(index, Qt::DisplayRole).toString());
}

/*!
   \overload

   Returns the size hint for objects drawn with the delegate with style options \a option for item at \a index.
*/
QSize QAppointmentDelegate::sizeHint(const QStyleOptionViewItem & option,
        const QModelIndex &index) const
{
    Q_UNUSED(index);

    QFontMetrics fm(mainFont(option));

#ifndef QTOPIA_PHONE
    QFontMetrics sfm(secondaryFont(option));
    return QSize(fm.width("M") * 10, fm.height() + sfm.height() + 4);
#else
    return QSize(fm.width("M") * 10, fm.height() + 4);  //  Make Qtopia phone more compact
#endif
}
