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

  /* datebookaccess1.cpp functions */
  void addException(const PimEvent &event, const QDate &, const PimEvent &exception);
  void addException(const PimEvent &event, const QDate &);
  void removeException(const PimEvent &event, const PimEvent &exception);
  void removeException(const PimEvent &event, const QDate &);
  void removeAllExceptions(const PimEvent &event);
  /* end datebookaccess1.cpp functions */

  PimEvent eventForId(const QUuid &, bool *ok = 0) const;

signals:
  void dateBookUpdated();

private:
  EventIO *m_AccessPrivate;
};


#endif

