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

#ifndef DATEBOOK_ACCESS_H
#define DATEBOOK_ACCESS_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qtopia/pim/event.h>

class EventIO;
class EventIteratorMachine;
class DateBookAccess;

class QTOPIAPIM_EXPORT DateBookIterator 
{
    friend class DateBookAccess;
public:
    DateBookIterator(const EventIO &);
    DateBookIterator(EventIteratorMachine *);
    DateBookIterator(const DateBookAccess &);
    DateBookIterator(const DateBookIterator &);
    DateBookIterator &operator=(const DateBookIterator &);
    ~DateBookIterator();

    bool atFirst() const;
    bool atLast() const;
    const PimEvent *toFirst();
    const PimEvent *toLast();

    const PimEvent *operator++();
    const PimEvent *operator*() const;

    const PimEvent *current() const;

private:
    EventIteratorMachine *machine;
};


class QTOPIAPIM_EXPORT DateBookAccess : public QObject {
  Q_OBJECT

  friend class DateBookIterator;
public:
  DateBookAccess();
  ~DateBookAccess();
  
  bool editSupported() const;

  void updateEvent(const PimEvent& event);
  void removeEvent(const PimEvent& event);
  void addEvent(const PimEvent& event);

  PimEvent eventForId(const QUuid &, bool *ok = 0) const;

signals:
  void dateBookUpdated();

private:
  EventIO *m_AccessPrivate;
};


#endif

