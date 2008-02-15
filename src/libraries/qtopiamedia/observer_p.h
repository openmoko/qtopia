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

#ifndef OBSERVER_H
#define OBSERVER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore>

class Subject;

class Observer
{
public:
    virtual ~Observer() { };
    virtual void update( Subject* subject ) = 0;
};

class Subject
{
public:
    virtual ~Subject() { };

    virtual void attach( Observer* observer );
    virtual void detach( Observer* observer );
    virtual void notify();

protected:
    Subject() { };

private:
    QList<Observer*> m_observers;
};

inline void Subject::attach( Observer* observer )
{
    m_observers.append( observer );
}

inline void Subject::detach( Observer* observer )
{
    m_observers.removeAll( observer );
}

inline void Subject::notify()
{
    foreach( Observer *observer, m_observers ) {
        observer->update( this );
    }
}

#endif // OBSERVER_H
