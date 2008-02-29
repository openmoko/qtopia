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

#ifndef QDBUSMACROS_H
#define QDBUSMACROS_H

#include <QtCore/qglobal.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qvariant.h>

QT_BEGIN_HEADER

#if defined(QDBUS_MAKEDLL)
# define QDBUS_EXPORT Q_DECL_EXPORT
#else
# define QDBUS_EXPORT Q_DECL_IMPORT
#endif

#ifndef Q_MOC_RUN
# define Q_NOREPLY
#endif

#ifdef Q_CC_MSVC
#include <QtCore/qlist.h>
#include <QtCore/qset.h>
#include <QtCore/qhash.h>
#include <QtCore/qvector.h>
#endif

QT_END_HEADER

#endif
