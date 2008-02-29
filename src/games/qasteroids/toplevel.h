/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
 * KAsteroids - Copyright (c) Martin R. Jones 1997
 *
 * Part of the KDE project
 */

#ifndef __KAST_TOPLEVEL_H__
#define __KAST_TOPLEVEL_H__

#include <qsound.h>

#include <qmainwindow.h>
#include <qmap.h>

#include "view.h"

#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif

class KALedMeter;
class QLCDNumber;
class QHBoxLayout;

class KAstTopLevel : public QMainWindow
{
    Q_OBJECT
 public:
    KAstTopLevel(QWidget* parent=0, Qt::WFlags fl=0);
    virtual ~KAstTopLevel();

 private:
    void playSound(const char* snd);
    void readSoundMapping();
    void doStats();
#ifdef QTOPIA_PHONE
    void updateContext1();
#endif
    QWidget* buildTopRow(QWidget* parent);
    QWidget* buildBottomRow(QWidget* parent);
    KAsteroidsView* buildAsteroidsView(QWidget* parent);
    QPalette buildPalette();
    bool eventConsumed(QKeyEvent* e) const;

 protected:
    virtual void showEvent(QShowEvent* );
    virtual void hideEvent(QHideEvent* );
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);

 private slots:
    void slotNewGame();
    void slotMissileFired();
    void slotShipKilled();
    void slotRockHit(int size);
    void slotRocksRemoved();
    void slotUpdateVitals();

 private:
    KAsteroidsView* view_;
    QLCDNumber* scoreLCD_;
    QLCDNumber* levelLCD_;
    QLCDNumber* shipsLCD_;

    QLCDNumber* teleportsLCD_;
    // QLCDNumber* bombsLCD_;
    QLCDNumber* brakesLCD_;
    QLCDNumber* shieldLCD_;
    QLCDNumber* shootLCD_;
    KALedMeter* powerMeter_;

    QSound shipDestroyed;
    QSound rockDestroyed;
    QSound missileFired;

    bool waitForNewShip_; // Wait for user to launch new ship.
    bool keyIsPressed_; // true means a key is currently pressed.

    int shipCount_;
    int score_;
    int currentLevel_;

    enum Action {
        Launch,
        Thrust,
        RotateLeft,
        RotateRight,
        Shoot,
        Teleport,
        Brake,
        Shield,
        Pause,
        NewGame
    };

    QMap<int,Action>    actions_;

#ifdef QTOPIA_PHONE
    QMenu*              contextMenu_;
#endif
};

#endif

