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

#ifndef QFACTORYINTERFACE_H
#define QFACTORYINTERFACE_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_MODULE(Core)

struct Q_CORE_EXPORT QFactoryInterface
{
    virtual ~QFactoryInterface() {}
    virtual QStringList keys() const = 0;
};

Q_DECLARE_INTERFACE(QFactoryInterface, "com.trolltech.Qt.QFactoryInterface")

QT_END_HEADER

#endif // QFACTORYINTERFACE_H
