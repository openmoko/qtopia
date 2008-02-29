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

#ifndef __AST_VIEW_H__
#define __AST_VIEW_H__

#include <qwidget.h>
#include <qlist.h>
#include <QHash>
#include <qtimer.h>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "sprites.h"

#define MAX_SHIP_POWER_LEVEL          100

class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
 public:
    MyGraphicsView(QGraphicsScene* scene, QWidget* parent = 0)
        : QGraphicsView(scene,parent) { }

 protected:
    void resizeEvent(QResizeEvent* event);
};

class KAsteroidsView : public QWidget
{
    Q_OBJECT
 public:
    KAsteroidsView(QWidget* parent = 0);
    virtual ~KAsteroidsView();

    int refreshRate;

    void reset();
    void setRockSpeed(double rs) { rockSpeed_ = rs; }
    void addRocks(int count);
    void newGame();
    void endGame();
    bool gameOver() const { return gameOver_; }
    void newShip();

    void decrementNextShotDelay() { if (nextShotDelay_) --nextShotDelay_; }
    void delayShooting(int count) { nextShotDelay_ = count; }
    void enableShooting() { nextShotDelay_ = 0; }

    bool engineIsOn() const { return engineIsOn_; }
    void startEngine();
    void stopEngine();

#if 0
    void startEngine() { engineIsOn_ = (shipPowerLevel_ > 0); }
    void stopEngine() { engineIsOn_ = false; }
#endif

    void rotateShipLeft(bool r) { rotateShipLeft_ = r; rotatingSlowly_ = 5; }
    void rotateShipRight(bool r) { rotateShipRight_ = r; rotatingSlowly_ = 5; }
    void stopShipRotation();

    bool shipIsBraking() const { return shipIsBraking_; }
    void startBraking();
    void stopBraking();

 private:
    void applyBrakes() { shipIsBraking_ = true; }
    void releaseBrakes() { shipIsBraking_ = false; }
    KPowerup* createPowerup(int type);

 public:
    bool shipCanShoot() const { return !nextShotDelay_; }
    bool shipIsShooting() const { return shipIsShooting_; }
    void startShooting() { shipIsShooting_ = true; enableShooting(); }
    void stopShooting() { shipIsShooting_ = false; enableShooting(); }

    void raiseShield();
    void teleport();
    void pause(bool p);

    void showText(const QString &text, bool scroll=true);
    void hideText();

    int shotsFired() const { return shotsFired_; }
    void incrementShotsFired() { ++shotsFired_; }
    void clearShotsFired() { shotsFired_ = 0; }
    int rocksHit() const { return rocksHit_; }
    void incrementRocksHit() { ++rocksHit_; }
    void clearRocksHit() { rocksHit_ = 0; }
    int shipPowerLevel() const { return shipPowerLevel_; }

    int teleportCount() const { return teleportCount_; }
    int brakeForce() const { return brakeForce_; }
    void incrementBrakeForce();
    int shieldStrength() const { return shieldStrength_; }
    int firePower() const { return firePower_; }

    void constructMessages(const QString& t);
    void reportStartGame();
    void reportShipKilled();
    void reportGameOver();
    int checksum() const;

 signals:
    void missileFired();
    void shipKilled();
    void rockHit(int size);
    void rocksRemoved();
    void updateVitals();

 private slots:
    void dropShield();

 protected:
    void readSprites();
    void destroyRock(MyAnimation* );
    void reducePower(int val);
    void addExhaust(double x, double y, double dx, double dy, int count);
    void processFragments();
    void processMissiles();
    void processShip();
    void processPowerups();
    void processShield();
    double randDouble();
    int randInt(int range);

    virtual void timerEvent(QTimerEvent* );

    bool textAboveScreen() const;

 private:
    QGraphicsScene*             scene_;
    MyGraphicsView*             view_;
    QHash<int,QList<QPixmap> >  animation_;
    QList<MyAnimation*>         rocks_;
    QList<KMissile*>            missiles_;
    QList<KFragment*>           ship_fragments_;
    QList<KExhaust*>            exhaust_;
    QList<KPowerup*>            powerups_;
    KShield*                    shield_;
    MyAnimation*                ship_;
    QGraphicsTextItem*  textSprite_;

    bool        started_;
    bool        rotateShipLeft_;
    bool        rotateShipRight_;
    bool        engineIsOn_;
    bool        shipIsShooting_;
    bool        teleportShip_;
    bool        shipIsBraking_;
    bool        pauseShip;
    bool        shieldIsUp_;

    bool        vitalsChanged_;
    bool        gameOver_;

    int         shipAngleIndex_;
    int         rotatingSlowly_;
    int         rotationRate_;
    int         shipPowerLevel_;

    int         shotsFired_;
    int         rocksHit_;
    int         nextShotDelay_;

    int         brakeForce_;
    int         shieldStrength_;
    int         teleportCount_;
    int         firePower_;

    double      shipDx_;
    double      shipDy_;

    int         textDy_;
    int         timerEventCount_;
    bool        paused_;
    int         masterTimerId_;

    double      rockSpeed_;
    QTimer*     shieldTimer_;

    QString     startGameMessage_;
    QString     shipKilledMessage_;
    QString     gameOverMessage_;
};

#endif
