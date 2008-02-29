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

#ifndef __QTOPIA_MEDIA_ABSTRACTCONTROL_H
#define __QTOPIA_MEDIA_ABSTRACTCONTROL_H

#include <QVariant>

#include <qtopiaglobal.h>

class QMediaContent;


class QMediaAbstractControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaAbstractControl : public QObject
{
    Q_OBJECT

public:
    QMediaAbstractControl(QMediaContent* mediaContent,
                          QString const& name);
    ~QMediaAbstractControl();

protected:
    typedef QList<QVariant> SlotArgs;

    QVariant value(QString const& name, QVariant const& defaultValue= QVariant()) const;
    void setValue(QString const& name, QVariant const& value);

    void proxyAll();
    void forward(QString const& slot, SlotArgs const& args = SlotArgs());

private slots:
    void controlAvailable(const QString& name);
    void controlUnavailable(const QString& name);

signals:
    void valid();
    void invalid();

private:
    QMediaAbstractControlPrivate*   d;
};

#endif  // __QTOPIA_MEDIA_ABSTRACTCONTROL_H
