/****************************************************************************
**
** Copyright(C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef EXAMPLESCREEN_H
#define EXAMPLESCREEN_H

#include <QLinuxFbScreen>

class ExampleScreen : public QLinuxFbScreen
{
public:
    ExampleScreen(int displayId);
    ~ExampleScreen();

    void exposeRegion(QRegion region, int windowIndex);

private:
    int fbd;
    QRect dirtyrect;
};

#endif // EXAMPLESCREEN_H


