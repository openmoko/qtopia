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

#ifndef __QTOPIA_MEDIASERVER_QMEDIAHELIXSETTINGSSERVER_H
#define __QTOPIA_MEDIASERVER_QMEDIAHELIXSETTINGSSERVER_H

#include <QAbstractIpcInterface>

class IHXClientEngine;

namespace qtopia_helix
{

class QMediaHelixSettingsServerPrivate;

class QMediaHelixSettingsServer : public QAbstractIpcInterface
{
    Q_OBJECT

public:
    QMediaHelixSettingsServer(IHXClientEngine* engine);
    ~QMediaHelixSettingsServer();

public slots:
    void setOption(QString const& value, QVariant const& value);

signals:
    void optionChanged(QString name, QVariant value);

private:
    QMediaHelixSettingsServerPrivate*   d;
};

}   // ns qtopia_helix

#endif  // __QTOPIA_MEDIASERVER_QMEDIAHELIXSETTINGSSERVER_H
