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

#ifndef __QTOPIA_MEDIALIBRARY_HELIXSETTINGSCONTROL_H
#define __QTOPIA_MEDIALIBRARY_HELIXSETTINGSCONTROL_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <qtopiaglobal.h>


class QMediaHelixSettingsControlPrivate;

class QTOPIAMEDIA_EXPORT QMediaHelixSettingsControl : public QObject
{
    Q_OBJECT

public:
    QMediaHelixSettingsControl(QObject* parent = 0);
    ~QMediaHelixSettingsControl();

    QStringList availableOptions();

    void setOption(QString const& name, QVariant const& value);
    QVariant optionValue(QString const& name);

    static QString name();

signals:
    void optionChanged(QString name, QVariant value);

private:
    Q_DISABLE_COPY(QMediaHelixSettingsControl);

    QMediaHelixSettingsControlPrivate*   d;
};


#endif  // __QTOPIA_MEDIALIBRARY_HELIXSETTINGSCONTROL_H
