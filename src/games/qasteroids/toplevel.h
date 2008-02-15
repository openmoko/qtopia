/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

class KALedMeter;
class QLCDNumber;
class QHBoxLayout;
class QMenu;

class KAstTopLevel : public QMainWindow
{
    Q_OBJECT
 public:
    KAstTopLevel(QWidget* parent=0, Qt::WFlags fl=0);
    virtual ~KAstTopLevel();

 private:
    void startNewGame();
    bool gameEnded() const;
    void endGame();
    void playSound(const char* snd);
    void readSoundMapping();
    void reportStatistics();

    QWidget* buildTopRow(QWidget* parent);
    QWidget* buildBottomRow(QWidget* parent);
    KAsteroidsView* buildAsteroidsView(QWidget* parent);
    QPalette buildPalette();
    bool eventConsumed(QKeyEvent* e) const;
    void populateRocks();
    void populatePowerups();

 protected:
    virtual void showEvent(QShowEvent* );
    virtual void hideEvent(QHideEvent* );
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void keyReleaseEvent(QKeyEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);

 private slots:
    void slotNewGameLevel();
    void slotMissileFired();
    void slotShipKilled();
    void slotUpdateScore(int key);
    void slotUpdateVitals();

 private:
    KAsteroidsView* view_;
    QLCDNumber* scoreLCD_;
    QLCDNumber* levelLCD_;
    QLCDNumber* shipsLCD_;

    QLCDNumber* teleportsLCD_;
    QLCDNumber* brakesLCD_;
    QLCDNumber* shieldLCD_;
    QLCDNumber* shootLCD_;
    KALedMeter* powerMeter_;

    QSound shipDestroyed;
    QSound rockDestroyed;
    QSound missileFired;

    bool	gameEnded_;
    int	shipCount_;
    int	score_;
    int	currentLevel_;

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
	NewGame,
	Populate_Powerups,
	Populate_Rocks
    };

    QMap<int,Action>    actions_;

    QMenu*              contextMenu_;
};

#endif

