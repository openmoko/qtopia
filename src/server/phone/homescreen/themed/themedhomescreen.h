/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef _THEMEDHOMESCREEN_H
#define _THEMEDHOMESCREEN_H

#include "qabstracthomescreen.h"

class PhoneThemedView;
class ThemedItemPlugin;
class ThemeItem;
class ThemeBackground;
class QValueSpaceObject;

class ThemedHomeScreen : public QAbstractHomeScreen
{
    Q_OBJECT

public:
    ThemedHomeScreen(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~ThemedHomeScreen();

    void updateBackground();
    void updateHomeScreenInfo();
    void updateInformation();
    void activatePinbox(bool enable);
    void showPinboxInformation(const QString &pix, const QString &text);

protected:
//    bool eventFilter(QObject *, QEvent *);

public Q_SLOTS:
    void applyHomeScreenImage();

private Q_SLOTS:
    void themeLoaded();
    void themeItemClicked(ThemeItem *item);

private:
    PhoneThemedView     *themedView;
    ThemedItemPlugin    *bgIface;
    ThemeBackground     *themeBackground;
    QValueSpaceObject   *vsObject;
};

#endif // _THEMEDHOMESCREEN_H
