/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

/*********************************************************************
** Asteroids(R) is a registered trademark of Atari Corporation.  Please
** note that the Asteroids(R) games included in Qtopia was not developed
** by nor is endorsed by Atari Corporation. Trolltech respectfully
** acknowledges Atari Corporation's ownership of the trademark."
**********************************************************************/

/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#ifndef __KAST_TOPLEVEL_H__
#define __KAST_TOPLEVEL_H__

#include <qtopia/sound.h>

#include <qmainwindow.h>
#include <qdict.h>
#include <qmap.h>

#include "view.h"

#ifdef QTOPIA_PHONE
# include <qtopia/contextmenu.h>
#endif

class KALedMeter;
class QLCDNumber;

class KAstTopLevel : public QMainWindow
{
    Q_OBJECT
public:
    KAstTopLevel( QWidget *parent=0, const char *name=0, WFlags fl=0 );
    virtual ~KAstTopLevel();

private:
    void playSound( const char *snd );
    void readSoundMapping();
    void doStats();
#ifdef QTOPIA_PHONE
    void updateContext1();
#endif

protected:
    virtual void showEvent( QShowEvent * );
    virtual void hideEvent( QHideEvent * );
    virtual void keyPressEvent( QKeyEvent *event );
    virtual void keyReleaseEvent( QKeyEvent *event );
    virtual void focusInEvent( QFocusEvent *event );
    virtual void focusOutEvent( QFocusEvent *event );

private slots:
    void slotNewGame();

    void slotMissileFired();
    void slotShipKilled();
    void slotRockHit( int size );
    void slotRocksRemoved();

    void slotUpdateVitals();

private:
    KAsteroidsView *view;
    QLCDNumber *scoreLCD;
    QLCDNumber *levelLCD;
    QLCDNumber *shipsLCD;

    QLCDNumber *teleportsLCD;
//    QLCDNumber *bombsLCD;
    QLCDNumber *brakesLCD;
    QLCDNumber *shieldLCD;
    QLCDNumber *shootLCD;
    KALedMeter *powerMeter;

    Sound shipDestroyed;
    Sound rockDestroyed;
    Sound missileFired;

    // waiting for user to press Enter to launch a ship
    bool waitShip;
    bool isPaused;
    bool havePress;

    int shipsRemain;
    int score;
    int level;
    bool showHiscores;


    enum Action { Launch, Thrust, RotateLeft, RotateRight, Shoot, Teleport,
                    Brake, Shield, Pause, NewGame  };

    QMap<int,Action> actions;
    QString launchButtonText;

#ifdef QTOPIA_PHONE
    ContextMenu *contextMenu;
#endif
};

#endif

