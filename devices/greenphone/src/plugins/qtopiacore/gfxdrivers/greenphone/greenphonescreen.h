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

#ifndef GREENPHONESCREEN_H
#define GREENPHONESCREEN_H

#include <QLinuxFbScreen>

class GreenphoneScreenPrivate;
class GreenphoneScreen : public QLinuxFbScreen
{
public:
    GreenphoneScreen(int displayId);
    ~GreenphoneScreen();

    void exposeRegion(QRegion region, int windowIndex);
    virtual void addReserved(int id, const QRect &rect);
    virtual void removeReserved(int id);

    int cookie;
private:
    GreenphoneScreenPrivate *d;
};

#endif // GREENPHONESCREEN_H


