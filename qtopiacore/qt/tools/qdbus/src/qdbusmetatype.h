/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDBUSMETATYPE_H
#define QDBUSMETATYPE_H

#include "QtCore/qmetatype.h"
#include <QtDBus/qdbusargument.h>

QT_BEGIN_HEADER

class QDBUS_EXPORT QDBusMetaType
{
public:
    typedef void (*MarshallFunction)(QDBusArgument &, const void *);
    typedef void (*DemarshallFunction)(const QDBusArgument &, void *);

    static void registerMarshallOperators(int typeId, MarshallFunction, DemarshallFunction);
    static bool marshall(QDBusArgument &, int id, const void *data);
    static bool demarshall(const QDBusArgument &, int id, void *data);

    static int signatureToType(const char *signature);
    static const char *typeToSignature(int type);
};

template<typename T>
void qDBusMarshallHelper(QDBusArgument &arg, const T *t)
{ arg << *t; }

template<typename T>
void qDBusDemarshallHelper(const QDBusArgument &arg, T *t)
{ arg >> *t; }

template<typename T>
int qDBusRegisterMetaType(
#ifndef qdoc
    T * /* dummy */ = 0
#endif
)
{
    void (*mf)(QDBusArgument &, const T *) = qDBusMarshallHelper<T>;
    void (*df)(const QDBusArgument &, T *) = qDBusDemarshallHelper<T>;

    int id = qRegisterMetaType<T>(); // make sure it's registered
    QDBusMetaType::registerMarshallOperators(id,
        reinterpret_cast<QDBusMetaType::MarshallFunction>(mf),
        reinterpret_cast<QDBusMetaType::DemarshallFunction>(df));
    return id;
}

QT_END_HEADER

#endif
