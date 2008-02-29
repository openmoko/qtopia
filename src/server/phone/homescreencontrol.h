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

#ifndef _HOMESCREENCONTROL_H_
#define _HOMESCREENCONTROL_H_

#include "qabstracthomescreen.h"

class HomeScreenControl
{

public:

    HomeScreenControl()
    : m_homeScreen(0)
    {
    }

    static HomeScreenControl *instance()
    {
        static HomeScreenControl *homeScreenControl = 0;

        if (!homeScreenControl)
            homeScreenControl = new HomeScreenControl;
        return homeScreenControl;
    }

    void setHomeScreen(QAbstractHomeScreen *homeScreen)
    {
        m_homeScreen = homeScreen;
    }

    QAbstractHomeScreen *homeScreen() const
    {
        return m_homeScreen;
    }

private:
    QAbstractHomeScreen *m_homeScreen;

};

#endif
