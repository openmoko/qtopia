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

#ifndef PERFTESTLINUXFBSCREEN_H
#define PERFTESTLINUXFBSCREEN_H

#include <QLinuxFbScreen>

#ifndef QT_NO_QWS_LINUXFB

class PerftestLinuxFbScreen : public QLinuxFbScreen
{
public:
    PerftestLinuxFbScreen(int displayId);
    ~PerftestLinuxFbScreen();

    bool connect(const QString &displaySpec);
    void exposeRegion(QRegion region, int windowIndex);
};

#endif

#endif
