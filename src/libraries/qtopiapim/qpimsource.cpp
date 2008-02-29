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

#include "qpimsource.h"
#include "qrecordio_p.h"
#include <QIcon>

/*!
  Returns a hash value for the source.
*/
QTOPIAPIM_EXPORT uint qHash(const QPimSource &s)
{
    return qHash(s.context) + qHash(s.identity);
}


/*!
  \class QPimSource
  \module qpepim
  \ingroup pim
  \brief The QPimSource class holds identifying information for a storage source of PIM data.

  The QPimSource class include a universal identifier representing the QPimContext
  that created the source and an identity string representing the source within
  the context it represents.
*/

/*!
  \variable QPimSource::context
  \brief the unique id representing the QPimContext that controls the source.
*/

/*!
  \variable QPimSource::identity
  \brief the context string to identify multiple sources from a single QPimContext.
*/

/*!
  \fn bool QPimSource::operator==(const QPimSource &other) const

  Returns true if this source is equal to \a other, otherwise returns false.
*/

/*!
  \fn bool QPimSource::operator!=(const QPimSource &other) const

  Returns true if this source is not equal to \a other, otherwise returns false.
*/

/*!
  \fn bool QPimSource::operator<(const QPimSource &other) const
  Returns true if this source is less than \a other, otherwise returns false.
*/

/*!
  \fn bool QPimSource::isNull() const
  Returns true if the source is null, otherwise returns false.
*/


/*!
  \class QPimContext
  \module qpepim
  \ingroup pim
  \brief The QPimContext class represents a storage context of PIM data.

  The QPimContext class represents a storage context of PIM data, such as SIM Card
  contacts or contacts stored on the device in Qtopia's native format.  The
  class can be used to perform operations that relate to a specific context of
  PIM data.

  QPimContext should not be subclassed directly.  Instead, one of the data type
  specific contexts should be used.  These are QContactContext, QTaskContext
  and QAppointmentContext.

  Currently there is no way for applications to implement their own contexts.
  This feature is being considered for future versions of Qtopia.
*/

/*!
  Constructs a QPimContext with parent \a parent.
*/
QPimContext::QPimContext(QObject *parent)
    : QObject(parent)
{
}

/*!
  Returns an icon associated with the context.  The default implementation
  returns a null icon.
*/
QIcon QPimContext::icon() const { return QIcon(); }

/*!
  Returns true if any PIM records stored by the context can be edited
  The default implementation returns false.
*/
bool QPimContext::editable() const { return false; }

/*!
  \fn QString QPimContext::description() const

  Returns text describing the PIM context suitable for displaying to the user.
*/

/*!
  \fn QString QPimContext::title() const

  Returns the title of the PIM context suitable for displaying to the user.
*/

/*!
  \fn QString QPimContext::title(const QPimSource &source) const

  Returns the title of the PIM data \a source suitable for displaying to the user.
  By default returns the title for the context.
*/


/*!
  \fn bool QPimContext::editable(const QUniqueId &id) const

  Returns whether the PIM record identified by \a id can be edited by this context,
  otherwise returns false.
*/

/*!
  \fn void QPimContext::setVisibleSources(const QSet<QPimSource> &visible)

  Filters the model that created this context to only show records for the source
  that is contained in \a visible.  Does not affect data from other contexts.
*/

/*!
  \fn QSet<QPimSource> QPimContext::visibleSources() const

  Returns the set of PIM data sources that are controlled by this context and are
  visible in the model that created the context.
*/

/*!
  \fn QSet<QPimSource> QPimContext::sources() const

  Returns the list of PIM data sources that are controlled by this context.
*/

/*!
  \fn QUuid QPimContext::id() const

  Returns a unique identifier for this context.
*/

/*!
  \fn bool QPimContext::exists(const QUniqueId &id) const

  Returns true if the PIM record identified by \a id exists in a PIM data source
  controlled by this context.  Otherwise returns false.
*/

/*!
  \fn bool QPimContext::exists(const QUniqueId &id, const QPimSource &source)

  Returns true if the contact identified by \a id exists in the PIM data \a source
  and the \a source is controlled by this context.  Otherwise returns false.
*/

/*!
  \fn QPimSource QPimContext::source(const QUniqueId &id) const

  Provided that the PIM record identified by \a id exists for this context,
  returns the PIM data source where the record is stored.  Otherwise
  returns a null PIM data source.
*/

/*!
  \fn void QPimSource::serialize(Stream &value) const
  \internal

  Serializes the QPimSource out to a template
  type \c{Stream} \a stream.
*/
template <typename Stream> void QPimSource::serialize(Stream &out) const
{
    out << context;
    out << identity;
    return out;
}

/*!
  \fn void QPimSource::deserialize(Stream &value)
  \internal

  Deserializes the QPimSource out to a template
  type \c{Stream} \a stream.
*/
template <typename Stream> void QPimSource::deserialize(Stream &in)
{
    in >> context;
    in >> identity;
    return in;
}

/*!
  \class QContactContext
  \module qpepim
  \ingroup pim
  \brief The QContactContext class represents a storage context of contact data.

  The QContactContext class represents a storage context of contact data,
  such as SIM Card contacts or contacts stored on the phone in Qtopia's native
  format.  The class can be used to perform operations that relate to a
  specific context of PIM data.

  Currently there is no way for applications to implement their own contexts.
  This feature is being considered for future versions of Qtopia.
*/

/*!
  \fn bool QContactContext::updateContact(const QContact &contact)

  Updates the contact with the same identifier as \a contact if it exists in this context.
*/

/*!
  \fn bool QContactContext::removeContact(const QUniqueId &id)

  Removes the contact with the identifier \a id if it exists in this context.
*/

/*!
  \fn QUniqueId QContactContext::addContact(const QContact &contact, const QPimSource &source)

  Adds the \a contact to the PIM data \a source if it is controlled by this context.
  If the contact is successfully added, returns the new unique id for this contact,
  otherwise returns a null id.
*/

/*!
  \fn QList<QContact> QContactContext::exportContacts(const QPimSource &source, bool &ok) const

  Exports the contacts stored in the PIM data \a source and returns them as a list.
  The source must be controlled by this context.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  There isn't necessarily going to be a one to one match to contacts in the PIM
  data source.  Contacts may be merged or split over multiple contacts to form
  the list.
*/

/*!
  \fn bool QContactContext::importContacts(const QPimSource &source, const QList<QContact> &contacts)

  Imports the \a contacts and merges them with the contacts listed in to the
  PIM data \a source.  The source must be controlled by this context.
  If successful returns true, otherwise returns false.

  There isn't necessarily going to be a one to one match to contacts in the PIM
  data source.  Contacts may be merged or split over multiple contacts.  Also
  contacts that match in name will be updated from the list rather than additional
  contacts created.
*/

/*!
  \fn QContact QContactContext::exportContact(const QUniqueId &id, bool &ok) const

  Exports a single contact identified by \a id.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  \sa exportContacts()
*/

/*!
  \fn bool QContactContext::importContact(const QPimSource &source, const QContact &contact)

  Imports a single \a contact to the PIM data \a source.  If successful returns
  true, otherwise returns false.

  \sa importContacts()
*/

/*!
  \fn QContactContext::QContactContext(QObject *parent)

  Constructs a QContactContext with parent \a parent.
*/

/*!
  \class QAppointmentContext
  \module qpepim
  \ingroup pim
  \brief The QAppointmentContext class represents a storage context of appointment data.

  The QAppointmentContext class represents a storage context of appointment data.
  The class can be used to perform operations that relate to a specific context of PIM data.

  Currently there is no way for applications to implement their own contexts.
  This feature is being considered for future versions of Qtopia.
*/

/*!
  \fn bool QAppointmentContext::updateAppointment(const QAppointment &appointment)

  Updates the appointment with the same identifier as \a appointment if it exists in this context.
*/

/*!
  \fn bool QAppointmentContext::removeAppointment(const QUniqueId &id)

  Removes the appointment with the identifier \a id if it exists in this context.
*/

/*!
  \fn QUniqueId QAppointmentContext::addAppointment(const QAppointment &appointment, const QPimSource &source)

  Adds the \a appointment to the PIM data \a source if it is controlled by this context.
  If the appointment is successfully added, returns the new unique id for this appointment,
  otherwise returns a null id.
*/

/*!
  \fn bool QAppointmentContext::removeOccurrence(const QUniqueId &id, const QDate &date)

  Mark the repeating appointment identified by \a id in this context so as not
  to occur on \a date.  Returns true if the appointment was successfully updated,
  otherwise returns false.
*/

/*!
  \fn QUniqueId QAppointmentContext::replaceOccurrence(const QUniqueId &id, const QOccurrence &occurrence)

  Replaces an occurrence of the appointment identified by \a id with \a occurrence.
  Returns true if the appointment was successfully updated, otherwise returns false.
*/

/*!
  \fn QUniqueId QAppointmentContext::replaceRemaining(const QUniqueId &id, const QAppointment &appointment)

  Modifies the appointment identified by \a id to not repeat after the first
  occurrence of \a appointment and adds \a appointment do the PIM data source
  that stores the appointment identified by \a id.  Returns the unique id for
  the new appointment if successful, otherwise returns a null id.
*/

/*!
  \fn QList<QAppointment> QAppointmentContext::exportAppointments(const QPimSource &source, bool &ok) const

  Exports the appointments stored in the PIM data \a source and returns them as a list.
  The source must be controlled by this context.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  There isn't necessarily going to be a one to one match to appointments in the PIM
  data source.  Appointments may be merged or split over multiple appointments to form
  the list.
*/

/*!
  \fn bool QAppointmentContext::importAppointments(const QPimSource &source, const QList<QAppointment> &appointments)

  Imports the \a appointments and merges them with the appointments listed in to the
  PIM data \a source.  The source must be controlled by this context.
  If successful returns true, otherwise returns false.

  There isn't necessarily going to be a one to one match to appointments in the PIM
  data source.  Appointments may be merged or split over multiple appointments.  Also
  appointments that match in name will be updated from the list rather than additional
  appointments created.
*/

/*!
  \fn QAppointment QAppointmentContext::exportAppointment(const QUniqueId &id, bool &ok) const

  Exports a single appointment identified by \a id.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  \sa exportAppointments()
*/

/*!
  \fn bool QAppointmentContext::importAppointment(const QPimSource &source, const QAppointment &appointment)

  Imports a single \a appointment to the PIM data \a source.  If successful returns
  true, otherwise returns false.

  \sa importAppointments()
*/

/*!
  \fn QAppointmentContext::QAppointmentContext(QObject *parent)

  Constructs a QAppointmentContext with parent \a parent.
*/

/*!
  \class QTaskContext
  \module qpepim
  \ingroup pim
  \brief The QTaskContext class represents a storage context of task data.

  The QTaskContext class represents a storage context of task data.
  The class can be used to perform operations that relate to a specific context of PIM data.

  Currently there is no way for applications to implement their own contexts.
  This feature is being considered for future versions of Qtopia.
*/

/*!
  \fn bool QTaskContext::updateTask(const QTask &task)

  Updates the task with the same identifier as \a task if it exists in this context.
*/

/*!
  \fn bool QTaskContext::removeTask(const QUniqueId &id)

  Removes the task with the identifier \a id if it exists in this context.
*/

/*!
  \fn QUniqueId QTaskContext::addTask(const QTask &task, const QPimSource &source)

  Adds the \a task to the PIM data \a source if it is controlled by this context.
  If the task is successfully added, returns the new unique id for this task,
  otherwise returns a null id.
*/

/*!
  \fn QList<QTask> QTaskContext::exportTasks(const QPimSource &source, bool &ok) const

  Exports the tasks stored in the PIM data \a source and returns them as a list.
  The source must be controlled by this context.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  There isn't necessarily going to be a one to one match to tasks in the PIM
  data source.  Tasks may be merged or split over multiple tasks to form
  the list.
*/

/*!
  \fn bool QTaskContext::importTasks(const QPimSource &source, const QList<QTask> &tasks)

  Imports the \a tasks and merges them with the tasks listed in to the
  PIM data \a source.  The source must be controlled by this context.
  If successful returns true, otherwise returns false.

  There isn't necessarily going to be a one to one match to tasks in the PIM
  data source.  Tasks may be merged or split over multiple tasks.  Also
  tasks that match in name will be updated from the list rather than additional
  tasks created.
*/

/*!
  \fn QTask QTaskContext::exportTask(const QUniqueId &id, bool &ok) const

  Exports a single task identified by \a id.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  \sa exportTasks()
*/

/*!
  \fn bool QTaskContext::importTask(const QPimSource &source, const QTask &task)

  Imports a single \a task to the PIM data \a source.  If successful returns
  true, otherwise returns false.

  \sa importTasks()
*/

/*!
  \fn QTaskContext::QTaskContext(QObject *parent)

  Constructs a QTaskContext with parent \a parent.
*/
