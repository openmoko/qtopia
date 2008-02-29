/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3SHARED_H
#define Q3SHARED_H

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

struct Q_COMPAT_EXPORT Q3Shared
{
    Q3Shared() : count( 1 ) { }
    void ref()		{ count++; }
    bool deref()	{ return !--count; }
    uint count;
};

QT_END_HEADER

#endif // Q3SHARED_H
