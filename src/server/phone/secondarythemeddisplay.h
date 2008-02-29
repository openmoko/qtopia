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

#ifndef _SECONDARYTHEMEDDISPLAY_H_
#define _SECONDARYTHEMEDDISPLAY_H_

#include "qabstractsecondarydisplay.h"

class SecondaryHomeScreen;
class SecondaryTitle;

class ThemedSecondaryDisplay : public QAbstractSecondaryDisplay
{
Q_OBJECT
public:
    ThemedSecondaryDisplay(QWidget *parent = 0, Qt::WFlags f = 0);

    void applyBackgroundImage();

    // Persist the new background, after applying it.
    void updateBackground();

private:
    SecondaryHomeScreen *home;
    SecondaryTitle *title;
};

#endif // _SECONDARYTHEMEDDISPLAY_H_

