/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef DATEBOOK_ACCESS_PRIVATE_H
#define DATEBOOK_ACCESS_PRIVATE_H

#include <qlist.h>
#include <qdatetime.h>
#include <qtopia/pim/qappointment.h>
#include "qappointmentio_p.h"
#include "qpimxmlio_p.h"

#if defined (QTOPIA_TEMPLATEDLL)
//MOC_SKIP_BEGIN
QTOPIA_TEMPLATEDLL template class QListIterator<QAppointment>;
QTOPIA_TEMPLATEDLL template class QList<QAppointment*>;
//MOC_SKIP_END
#endif

class EventXmlIO : public EventIO, public QPimXmlIO {

    Q_OBJECT

 public:

    enum EventFields {
        Description,
        Location,
        StartTimeZone,
        Notes,
        StartDateTime,
        EndDateTime,
        DatebookType,
        HasAlarm,
        AlarmDelay,
        SoundType,

        RepeatPattern,
        RepeatFrequency,
        RepeatWeekdays,
        RepeatHasEndDate,
        RepeatEndDate,

        RecordParent,
        RecordChildren,
        Exceptions
    };

    explicit EventXmlIO(AccessMode m,
            const QString &file = QString(),
            const QString &journal = QString());
  ~EventXmlIO();

  EventIteratorMachine *begin() const { return new EventXmlIterator(m_QAppointments); }

  const QList<QAppointment*>& events();

  QAppointment eventForId( const QUniqueId &, bool *ok ) const;
  QList<Occurrence> getOccurrencesInCurrentTZ(const QDate& from, const QDate& to) const;
  Occurrence getNextOccurrenceInCurrentTZ( const QDateTime& from,
                                           bool * ok = NULL ) const;
  Occurrence getNextOccurrenceInCurrentTZ( const Occurrence& from,
                                           bool * ok = NULL ) const;

  bool loadData();
  bool saveData();

  void updateEvent(const QAppointment& event);
  void removeEvent(const QAppointment& event);
  QUniqueId addEvent(const QAppointment& event, bool assignUid = true );
  void addException(const QDate &d, const QAppointment &p);
  QUniqueId addException(const QDate &d, const QAppointment &p, const QAppointment& event);

  // find next alarm and set;
  // later will want to be more efficient.
  void addEventAlarm(const QAppointment &);
  void delEventAlarm(const QAppointment &);

  bool nextAlarm(const QAppointment &, QDateTime &when, int &warn);

  void ensureDataCurrent(bool forceReload = false);

protected:
  const char *recordStart() const { return "<event "; } // No tr
  const char *listStart() const
  { return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<!DOCTYPE DATEBOOK>\n<events>\n"; }
  const char *listEnd() const { return "</events>"; }

  QPimRecord *createRecord() const { return new QAppointment(); }

  bool internalAddRecord(QPimRecord *);
  bool internalUpdateRecord(QPimRecord *);
  bool internalRemoveRecord(QPimRecord *);

  virtual void setFields(QPimRecord *, const QMap<QString, QString> &) const;
  virtual QMap<QString, QString> fields(const QPimRecord *) const;

private slots:
  void pimMessage(const QString &, const QByteArray &);

private:
  QDateTime cNextAlarm;

  QList<QAppointment*> m_QAppointments;

  bool needsSave;
};

#endif
