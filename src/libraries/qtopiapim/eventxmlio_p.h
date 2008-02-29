/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info\@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DATEBOOK_ACCESS_PRIVATE_H
#define DATEBOOK_ACCESS_PRIVATE_H

#include <qlist.h>
#include <qdatetime.h>
#include <qasciidict.h>
#include <qtopia/pim/event.h>
#include "eventio_p.h"
#include "xmlio_p.h"

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
