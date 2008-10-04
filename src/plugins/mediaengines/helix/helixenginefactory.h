/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef HELIXENGINEFACTORY_H
#define HELIXENGINEFACTORY_H

#include <qmediaenginefactory.h>

class HelixEngineFactory :
    public QObject,
    public QMediaEngineFactory
{
    Q_OBJECT
    Q_INTERFACES(QMediaEngineFactory)

public:
    HelixEngineFactory();
    ~HelixEngineFactory();

    QMediaEngine* create();
};

#endif
