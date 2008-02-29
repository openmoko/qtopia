/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DATEBOOK_ACCESS_PRIVATE_H
#define DATEBOOK_ACCESS_PRIVATE_H

#include <qlist.h>
#include <qdatetime.h>
#include <qasciidict.h>
#include <qtopia/pim/event.h>
#include <qtopia/pim/private/eventio_p.h>
#include <qtopia/pim/private/xmlio_p.h>

#if defined (QTOPIA_TEMPLATEDLL)
//MOC_SKIP_BEGIN
QTOPIA_TEMPLATEDLL template class QTOPIAPIM_EXPORT QListIterator<PrEvent>;
QTOPIA_TEMPLATEDLL template class QTOPIAPIM_EXPORT QList<PrEvent>;
//MOC_SKIP_END
#endif

class QTOPIAPIM_EXPORT EventXmlIterator : public EventIteratorMachine
{
public:
    EventXmlIterator(const QList<PrEvent>&list) : it(list) {}

    ~EventXmlIterator() {}

    EventXmlIterator &operator=(const EventXmlIterator &o) {
	it = o.it;
	return *this;
    }

    bool atFirst() const { return it.atFirst(); }
    bool atLast() const { return it.atLast(); }
    const PrEvent *toFirst() { return it.toFirst(); }
    const PrEvent *toLast() { return it.toLast(); }

    const PrEvent *next() { return ++it; }
    const PrEvent *prev() { return --it; }
    const PrEvent *current() const { return it.current(); }

private:
    QListIterator<PrEvent>it;
};

class QTOPIAPIM_EXPORT EventXmlIO : public EventIO, public PimXmlIO {

    Q_OBJECT

 public:
  EventXmlIO(AccessMode m,
	    const QString &file = QString::null,
	    const QString &journal = QString::null);
  ~EventXmlIO();

  EventIteratorMachine *begin() const { return new EventXmlIterator(m_PrEvents); }

  const QList<PrEvent>& events();

  PrEvent eventForId( const QUuid &, bool *ok ) const;
  QValueList<Occurrence> getOccurrencesInCurrentTZ(const QDate& from, const QDate& to) const;
  Occurrence getNextOccurrenceInCurrentTZ( const QDateTime& from, 
                                           bool * ok = NULL ) const;
  Occurrence getNextOccurrenceInCurrentTZ( const Occurrence& from, 
                                           bool * ok = NULL ) const;

  bool loadData();
  bool saveData();

  void updateEvent(const PimEvent& event);
  void removeEvent(const PimEvent& event);
  QUuid addEvent(const PimEvent& event, bool assignUid = TRUE );
  void addException(const QDate &d, const PimEvent &p);
  QUuid addException(const QDate &d, const PimEvent &p, const PimEvent& event);

  // find next alarm and set;
  // later will want to be more efficient.
  void addEventAlarm(const PrEvent &);
  void delEventAlarm(const PrEvent &);

  bool nextAlarm(const PrEvent &, QDateTime &when, int &warn);

  void ensureDataCurrent(bool forceReload = false);

protected:
  const char *recordStart() const { return "<event "; } // No tr
  const char *listStart() const
  { return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<!DOCTYPE DATEBOOK>\n<events>\n"; }
  const char *listEnd() const { return "</events>"; }

  PimRecord *createRecord() const { return new PrEvent(); }

  bool internalAddRecord(PimRecord *);
  bool internalUpdateRecord(PimRecord *);
  bool internalRemoveRecord(PimRecord *);

  virtual QString recordToXml(const PimRecord *);

private slots:
  void pimMessage(const QCString &, const QByteArray &);

private:
  QDateTime cNextAlarm;

  QList<PrEvent> m_PrEvents;

  bool needsSave;
};

#endif
