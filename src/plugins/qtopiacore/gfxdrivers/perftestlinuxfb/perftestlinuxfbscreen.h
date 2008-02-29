/****************************************************************************
**
** Copyright(C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#endif // PERFTESTLINUXFBSCREEN_H

