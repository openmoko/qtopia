/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <QMetaMethod>
#include <QString>
#include <QTimer>
#include <QtopiaIpcAdaptor>
#include <QValueSpaceItem>
#include <qdebug.h>
#include "qmediacontent.h"

#include "qmediaabstractcontrol.h"


class QMediaAbstractControlPrivate
{
public:
    QMediaContent*      mediaContent;
    QString             name;
    QValueSpaceItem*    values;
    QtopiaIpcAdaptor*   send;
    QtopiaIpcAdaptor*   recieve;
};

QMediaAbstractControl::QMediaAbstractControl(QMediaContent* mediaContent, QString const& name):
    QObject(mediaContent),
    d(new QMediaAbstractControlPrivate)
{
    d->mediaContent = mediaContent;
    d->name = name;
    d->values = 0;

    QString baseName = mediaContent->handle().toString() + "/" + name;

    d->send = new QtopiaIpcAdaptor("QPE/Media/Server/Control/" + baseName, this);
    d->recieve = new QtopiaIpcAdaptor("QPE/Media/Library/Control/" + baseName, this);

    connect(mediaContent, SIGNAL(controlAvailable(QString)),
            this, SLOT(controlAvailable(QString)));

    connect(mediaContent, SIGNAL(controlUnavailable(QString)),
            this, SLOT(controlUnavailable(QString)));

    // Check if active server TODO: bit of a hack
    QValueSpaceItem item("/Media/Control/" + d->mediaContent->handle().toString() + "/Session");

    if (item.value("controls").toStringList().contains(name))
    {
        d->values = new QValueSpaceItem("/Media/Control/" + d->mediaContent->handle().toString() + "/" + name);
        QTimer::singleShot(0, this, SIGNAL(valid()));
    }
}

QMediaAbstractControl::~QMediaAbstractControl()
{
    delete d;
}

QVariant QMediaAbstractControl::value(QString const& name, QVariant const& defaultValue) const
{
    if (d->values == 0)
        return defaultValue;

    return d->values->value(name, defaultValue);
}

void QMediaAbstractControl::setValue(QString const& name, QVariant const& value)
{
    if (d->values == 0)
        qWarning("Attempting to call setValue() on an invalid control");

    d->values->setValue(name, value);
}

void QMediaAbstractControl::proxyAll()
{
    QMetaObject const*  mo = metaObject();
    int                 mc = mo->methodCount();
    int                 offset = mo->methodOffset();

    // Connect server signals to client
    for (int i = offset; i < mc; ++i)
    {
        QMetaMethod method = mo->method(i);

        switch (method.methodType())
        {
        case QMetaMethod::Signal:
            QtopiaIpcAdaptor::connect(d->recieve, QByteArray::number(QMESSAGE_CODE) + method.signature(),
                                      this, QByteArray::number(QSIGNAL_CODE) + method.signature());
            break;

        case QMetaMethod::Slot:
        case QMetaMethod::Method:
            break;
        }
    }
}

void QMediaAbstractControl::forward(QString const& slot, SlotArgs const& args)
{
    d->send->send(slot.toLatin1(), args);
}

void QMediaAbstractControl::controlAvailable(const QString& name)
{
    if (name == d->name && d->values == 0)
    {
        // Connect to our valuespace
        d->values = new QValueSpaceItem("/Media/Control/" + d->mediaContent->handle().toString() + "/" + name);

        emit valid();
    }
}

void QMediaAbstractControl::controlUnavailable(const QString& name)
{
    if (name == d->name)
    {
        delete d->values;
        d->values = 0;

        emit invalid();
    }
}



