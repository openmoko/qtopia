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

#include "custom.h"
#include "plugin.h"
#include "webcams.h"
#include <QCameraDevice>
Plugin::Plugin(QObject* parent)
:QCameraDevicePlugin(parent)
{}

Plugin::~Plugin()
{ }

QStringList Plugin::keys() const
{
    QStringList cameras;
    cameras <<  QString("v4l2webcam");
    return cameras;
}

QCameraDevice* Plugin::create(QString const& key)
{
    if( key == "v4l2webcam" )
        return new V4L2Webcam;
    return 0;
}

QTOPIA_EXPORT_PLUGIN(Plugin);
