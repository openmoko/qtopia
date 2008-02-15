/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef QPRIVATEIMPLEMENTATIONDEF_H
#define QPRIVATEIMPLEMENTATIONDEF_H

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

#include "qprivateimplementation_p.h"


template<typename ImplementationType>
QTOPIAMAIL_EXPORT QPrivatelyImplemented<ImplementationType>::QPrivatelyImplemented(ImplementationType* p)
    : d(p)
{
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT QPrivatelyImplemented<ImplementationType>::QPrivatelyImplemented(const QPrivatelyImplemented& other)
    : d(other.d)
{
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT QPrivatelyImplemented<ImplementationType>::~QPrivatelyImplemented()
{
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT const QPrivatelyImplemented<ImplementationType>& QPrivatelyImplemented<ImplementationType>::operator=(const QPrivatelyImplemented<ImplementationType>& other)
{
    d = other.d;
    return *this;
}

template<typename ImplementationType>
template<typename ImplementationSubclass>
QTOPIAMAIL_EXPORT ImplementationSubclass* QPrivatelyImplemented<ImplementationType>::impl()
{
    return static_cast<ImplementationSubclass*>(static_cast<ImplementationType*>(d));
}

template<typename ImplementationType>
template<typename InterfaceType>
QTOPIAMAIL_EXPORT typename InterfaceType::ImplementationType* QPrivatelyImplemented<ImplementationType>::impl(InterfaceType*)
{
    return impl<typename InterfaceType::ImplementationType>();
}

template<typename ImplementationType>
template<typename ImplementationSubclass>
QTOPIAMAIL_EXPORT const ImplementationSubclass* QPrivatelyImplemented<ImplementationType>::impl() const
{
    return static_cast<const ImplementationSubclass*>(static_cast<const ImplementationType*>(d));
}

template<typename ImplementationType>
template<typename InterfaceType>
QTOPIAMAIL_EXPORT const typename InterfaceType::ImplementationType* QPrivatelyImplemented<ImplementationType>::impl(const InterfaceType*) const
{
    return impl<const typename InterfaceType::ImplementationType>();
}

/* We could probably use SFINAE to make these work, but I won't try now...
template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyImplemented<ImplementationType>::operator== (const QPrivatelyImplemented<ImplementationType>& other) const
{
    return ((d == other.d) || 
            (*(impl<ImplementationType>()) == *(other.impl<ImplementationType>())));
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyImplemented<ImplementationType>::operator!= (const QPrivatelyImplemented<ImplementationType>& other) const
{
    return ((d != other.d) &&
            !(*(impl<ImplementationType>()) == *(other.impl<ImplementationType>())));
}

template<typename ImplementationType>
QTOPIAMAIL_EXPORT bool QPrivatelyImplemented<ImplementationType>::operator< (const QPrivatelyImplemented<ImplementationType>& other) const
{
    return ((d != other.d) &&
            (*impl<ImplementationType>() < *other.impl<ImplementationType>()));
}
*/

#endif // QPRIVATEIMPLEMENTATIONDEF_H
