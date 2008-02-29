/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef KEYFILTER_H
#define KEYFILTER_H

#include <QtCore>

class KeyFilter : public QObject
{
public:
    KeyFilter( QObject* subject, QObject* target, QObject* parent = 0 );

    void addKey( int key );

    bool eventFilter( QObject* o, QEvent* e );

private:
    QObject *m_target;
    QSet<int> m_keys;
};

#endif // KEYFILTER_H
