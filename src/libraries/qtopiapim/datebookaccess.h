/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DATEBOOK_ACCESS_H
#define DATEBOOK_ACCESS_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qpe/pim/event.h>

class EventIO;
class EventIteratorMachine;
class DateBookAccess;

class DateBookIterator 
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


class DateBookAccess : public QObject {
  Q_OBJECT

  friend class DateBookIterator;
public:
  DateBookAccess();
  ~DateBookAccess();
  
  bool editSupported() const;

  void updateEvent(const PimEvent& event);
  void removeEvent(const PimEvent& event);
  void addEvent(const PimEvent& event);

signals:
  void dateBookUpdated();

private:
  EventIO *m_AccessPrivate;
};


#endif

