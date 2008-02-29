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

#ifndef __QTOPIA_MEDIALIBRARY_MEDIAHANDLE_H
#define __QTOPIA_MEDIALIBRARY_MEDIAHANDLE_H

#include <quuid.h>


class QMediaHandle
{
public:
    QMediaHandle() {}
    explicit QMediaHandle(QUuid const& id):
        m_id(id) {}
    QMediaHandle(QMediaHandle const& c):
        m_id(c.m_id) {}

    QMediaHandle& operator=(QMediaHandle const& rhs)
    {
        m_id = rhs.m_id;
        return *this;
    }

    QUuid const& id() const
    {
        return m_id;
    }

private:
    QUuid   m_id;
};


#endif  // __QTOPIA_MEDIALIBRARY_MEDIAHANDLE_H

