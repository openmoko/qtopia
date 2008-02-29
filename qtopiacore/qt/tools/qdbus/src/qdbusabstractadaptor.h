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

#ifndef QDBUSABSTRACTADAPTOR_H
#define QDBUSABSTRACTADAPTOR_H

#include <QtCore/qobject.h>
#include <QtDBus/qdbusmacros.h>

QT_BEGIN_HEADER


class QDBusAbstractAdaptorPrivate;
class QDBUS_EXPORT QDBusAbstractAdaptor: public QObject
{
    Q_OBJECT
protected:
    QDBusAbstractAdaptor(QObject *parent);

public:
    ~QDBusAbstractAdaptor();

protected:
    void setAutoRelaySignals(bool enable);
    bool autoRelaySignals() const;

private:
    Q_DECLARE_PRIVATE(QDBusAbstractAdaptor)
};

QT_END_HEADER

#endif
