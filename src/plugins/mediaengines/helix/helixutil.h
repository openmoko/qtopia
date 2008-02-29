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

#ifndef HELIX_UTIL_H
#define HELIX_UTIL_H

#include <QtCore>

#include <config.h>
#include <hxcom.h>

// Return path to Helix libraries
QString helix_library_path();

class GenericContext : public IUnknown
{
public:
    GenericContext( const QList<IUnknown*>& classes = QList<IUnknown*>() );
    virtual ~GenericContext();

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

private:
    INT32 m_refCount;

    QList<IUnknown*> m_classes;
};

#endif // HELIX_UTIL_H
