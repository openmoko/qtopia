/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WIDGETINFO_H
#define WIDGETINFO_H

#include <QObject>

class QString;
struct QMetaObject;
class QMetaEnum;

class WidgetInfo: public QObject
{
protected:
    WidgetInfo();

public:
    static bool isValidProperty(const QString &className, const QString &name);
    static bool isValidEnumerator(const QString &className, const QString &name);
    static bool isValidSignal(const QString &className, const QString &name);
    static bool isValidSlot(const QString &className, const QString &name);

    static QString resolveEnumerator(const QString &className, const QString &name);

private:
    static const QMetaObject *metaObject(const QString &widgetName);
    static bool checkEnumerator(const QMetaObject *meta, const QString &name);
    static bool checkEnumerator(const QMetaEnum &metaEnum, const QString &name);

    static QString resolveEnumerator(const QMetaObject *meta, const QString &name);
    static QString resolveEnumerator(const QMetaEnum &metaEnum, const QString &name);
};

#endif // WIDGETINFO_H
