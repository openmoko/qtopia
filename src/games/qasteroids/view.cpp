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

#include "view.h"

#include <qapplication.h>
#include <QVBoxLayout>
#include <QtDebug>

#include <stdlib.h>
#include <math.h>

#define IMG_BACKGROUND ":image/qasteroids/bg"

#define REFRESH_DELAY          33
#define MISSILE_SPEED          10.0
#define SHIP_STEPS             64
#define ROTATION_RATE           2
#define SHIELD_ON_COST          1
#define SHIELD_HIT_COST        30
#define BRAKE_ON_COST           4

#define ROCK_SPEED_MULTIPLIER   2.5
#define MAX_SHIP_SPEED          8
#define MAX_BRAKING_FORCE       5
#define MAX_SHIELD_STRENGTH     5
#define MAX_FIREPOWER           5

#define TEXT_SPEED              2

#define PI_X_2                  6.283185307
#ifndef M_PI
#define M_PI                    3.141592654
#endif

#define ROCK_IMAGE_COUNT   32
#define SHIP_IMAGE_COUNT   32
#define FRAG_IMAGE_COUNT   16
#define SHIELD_IMAGE_COUNT  6

static struct
{
    int id;
    const char *path;
    int frames;
} kas_animations[] =
{
//  { ID_ROCK_LARGE,       "rock1/rock1\%1.png",    ROCK_IMAGE_COUNT },
    { ID_ROCK_MEDIUM,      "rock2/rock2\%1.png",    ROCK_IMAGE_COUNT },
    { ID_ROCK_SMALL,       "rock3/rock3\%1.png",    ROCK_IMAGE_COUNT },
    { ID_SHIP,             "ship/ship\%1.png",      SHIP_IMAGE_COUNT },
    { ID_MISSILE,          "missile/missile.png",   0 },
    { ID_FRAGMENT,         "bits/bits\%1.png",      FRAG_IMAGE_COUNT },
    { ID_EXHAUST,          "exhaust/exhaust.png",   0 },
    { ID_ENERGY_POWERUP,   "powerups/energy.png",   0 },
    { ID_TELEPORT_POWERUP, "powerups/teleport.png", 0 },
    { ID_BRAKE_POWERUP,    "powerups/brake.png",    0 },
    { ID_SHIELD_POWERUP,   "powerups/shield.png",   0 },
    { ID_SHOOT_POWERUP,    "powerups/shoot.png",    0 },
    { ID_SHIELD,           "shield/shield\%1.png",  SHIELD_IMAGE_COUNT },
    { 0,                   0,                       0 }
};

/* mws
  Apparently, the teleport sprite used to be animated.
  This was the line in the table above for the teleport
  icon:

  { ID_TELEPORT_POWERUP, "powerups/teleport%1.png",  12 },

  But there is a teleport.png file there, not a sequence
  of them, so it must be a single png, ie frames = 0.
 */

/*!
  \externalpage http://doc.trolltech.com/4.2/graphicsview.html
  \title Qt Graphics View Framework
 */

/*!
  \class MyGraphicsView
  \internal

  \brief The MyGraphicsView class is a subclass of QGraphicsView,
         which was created for the sole purpose of allowing us to
         implement our own resizeEvent() function.

  We need to re-implement that function to get the widget's new
  geometry once it has been resized to fit the space available. We
  do this by calling the base class resizeEvent(), which processes
  the resize event generated when the widget was resized to fit
  the available space.

  After processing that event, we get the width() and height() of
  the resized widget from the base class and use them to construct
  a rectangle at (0,0), which we use to set the scene geometry of
  our QGraphicsScene.
 */


/*!
  \internal

  We need to re-implement this virtual function here to get the
  widget's new geometry once it has been resized to fit the space
  available. We do this by calling the base class resizeEvent(),
  which processes the resize event generated when the widget was
  resized to fit the available space.

  After processing that event, we get the width() and height() of
  the resized widget from the base class and use them to construct
  a rectangle at (0,0), which we use to set the scene geometry of
  our QGraphicsScene.
 */
void MyGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    scene()->setSceneRect(0,0,width(),height());
}

/*!
  \class KAsteroidsView
  \brief The KAsteroidsView class contains the asteroids game's
         use of the \l {Qt Graphics View Framework} API.
  \internal

  The class creates a \l {QGraphicsScene} {scene} and a
  \l {MyGraphicsView} {view} for the scene.
 */

/*!
  \internal
 */
KAsteroidsView::KAsteroidsView(QWidget* parent)
    : QWidget(parent)
{
    scene_ = new QGraphicsScene();
    view_ = new MyGraphicsView(scene_,this);
    view_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scene_->setBackgroundBrush(QPixmap(IMG_BACKGROUND));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(view_);
    layout->setSpacing(0);
    layout->setMargin(0);

    textSprite_ = new QGraphicsTextItem(0,scene_);

    shield_ = 0;
    shieldIsUp_ = false;
    refreshRate = REFRESH_DELAY;

    readSprites();

    shieldTimer_ = new QTimer(this);
    connect(shieldTimer_, SIGNAL(timeout()), this, SLOT(dropShield()));
    masterTimerId_ = -1;

    shipPowerLevel_ = MAX_SHIP_POWER_LEVEL;
    vitalsChanged_ = false;

    paused_ = true;
    gameOver_ = true;
    started_ = false;
}

/*!
  \internal

  The destructor resets the game. It does not delete the scene
  or the view. The view has this KAsteroidsView as its parent,
  so we assume it will be deleted in the normal Qt hierarchical
  destruction on exit. The scene has no parent, but we assume
  it is deleted during the same process.
 */
KAsteroidsView::~KAsteroidsView()
{
    reset();
}

/*!
  \internal

  This function is called on exit, but, more importantly, it
  is called when a new game starts. It deletes any existing
  rocks, missiles, ship fragments, and powerups and resets
  several global variables. It also hides the ship and the
  shield.
 */
void KAsteroidsView::reset()
{
    qDeleteAll(rocks_);
    rocks_.clear();
    qDeleteAll(missiles_);
    missiles_.clear();
    qDeleteAll(ship_fragments_);
    ship_fragments_.clear();
    qDeleteAll(powerups_);
    powerups_.clear();
    KPowerup::clearCounts();
    qDeleteAll(exhaust_);
    exhaust_.clear();

    clearShotsFired();
    clearRocksHit();

    rockSpeed_ = 1.0;
    timerEventCount_ = 0;
    paused_ = false;

    ship_->hide();
    shield_->hide();
/*
    if (masterTimerId_ >= 0) {
        killTimer(masterTimerId_);
        masterTimerId_ = -1;
    }
*/
}

/*!
  \internal

  Stop the ship's rotation to left or right, and reset the
  rotation rate variables to their initial values.
 */
void KAsteroidsView::stopShipRotation()
{
    rotateShipLeft_ = false;
    rotateShipRight_ = false;
    rotationRate_ = ROTATION_RATE;
    rotatingSlowly_ = 0;
}

/*!
  \internal

  If the braking force is not yet at its maximum, increment it.
 */
void KAsteroidsView::incrementBrakeForce()
{
    if (brakeForce() < MAX_BRAKING_FORCE)
        brakeForce_++;
}

/*!
  \internal

  Start a new game. Calls reset() and emits updateVitals();
 */
void KAsteroidsView::newGame()
{
    gameOver_ = false;
    reset();
    if (masterTimerId_ < 0)
        masterTimerId_ = startTimer(REFRESH_DELAY);
    emit updateVitals();
}

/*!
  \internal

  Set a flag indicating the game has ended.
 */
void KAsteroidsView::endGame()
{
    gameOver_ = true;
}

/*!
  \internal

  Put the game in its paused state if \a p is true,
  and take it out of its paused state if \a p is false.

  This is a bit misleading, because the actual pause is
  maintained by a QMessageBox waiting for the user to
  press the "OK" button it puts up. As soon as "OK" is
  pressed, this function is called again with \a p set
  to false.
 */
void KAsteroidsView::pause(bool p)
{
    if (!paused_ && p) {
        if (masterTimerId_ >= 0) {
            killTimer(masterTimerId_);
            masterTimerId_ = -1;
        }
    }
    else if (paused_ && !p)
        masterTimerId_ = startTimer(REFRESH_DELAY);
    paused_ = p;
}

/*!
  \internal

  Create a new ship and show it. The ship is created in the
  center of the screen with its shield up to prevent it from
  being destroyed immediately if there is also a rock there.
 */
void KAsteroidsView::newShip()
{
    ship_->setPos(scene_->width()/2, scene_->height()/2);
    ship_->resetImage();
    shield_->setPos(scene_->width()/2, scene_->height()/2);
    shield_->resetImage();
    ship_->setVelocity(0.0, 0.0);
    shipDx_ = 0;
    shipDy_ = 0;
    shipAngleIndex_ = 0;
    stopEngine();
    stopShipRotation();
    stopShooting();
    releaseBrakes();
    teleportShip_ = false;
    shieldIsUp_ = true;
    shipPowerLevel_ = MAX_SHIP_POWER_LEVEL;

    brakeForce_ = 0;
    teleportCount_ = 0;
    firePower_ = 0;

    ship_->show();
    shield_->show();
    shieldStrength_ = 1;   // just in case the ship appears on a rock.
    raiseShield();
}

/*!
  \internal

  If the ship's shield strength is greater than 0,
  raise the shield.
 */
void KAsteroidsView::raiseShield()
{
    shieldIsUp_ = (shieldStrength_ > 0);
    if (shieldIsUp_) {
        shieldTimer_->setSingleShot(true);
        shieldTimer_->start(1000);
        --shieldStrength_;
        reducePower(SHIELD_ON_COST);
    }
}

/*!
  \internal

  Drop the shield. The shield's hide() function is called,
  and the "shield is up" flag is turned off.
 */
void KAsteroidsView::dropShield()
{
    shield_->hide();
    shieldIsUp_ = false;
}

/*!
  \internal

  If the ship has any braking force available, start applying
  the brakes to slow down the ship.
 */
void KAsteroidsView::startBraking()
{
    if (brakeForce())
        applyBrakes();
}

/*!
  \internal

  If the ship has any braking force available, release the
  brakes, ie stop applying the breaks. Also, if the ship is
  braking, stop the ship movement, and stop its rotation. I
  don't know if this is actually correct, but this is how it
  was programmed when I ported it to Qtopia 4.2.
 */
void KAsteroidsView::stopBraking()
{
    if (brakeForce()) {
        if (shipIsBraking()) {
            stopEngine();
            stopShipRotation();
        }
        releaseBrakes();
    }
}

/*!
  \internal

  Read the sprite files and generate pixmaps for use in the
  animations required for the game.
 */
void KAsteroidsView::readSprites()
{
    QString sprites_prefix = IMG_BACKGROUND;
    int sep = sprites_prefix.lastIndexOf("/");

    sprites_prefix.truncate(sep);

    int i = 0;
    QString fn;
    QString base = sprites_prefix + "/";
    QList<QPixmap> pixmap_list;
    while (kas_animations[i].id) {
        pixmap_list.clear();
        if (kas_animations[i].frames) {
            for (int j=0; j<kas_animations[i].frames; ++j) {
                QString s(kas_animations[i].path);
                fn = base + s.arg(j,4,10,QLatin1Char('0'));
                QPixmap pixmap(fn);
                pixmap_list.insert(j,pixmap);
            }
        }
        else {
            fn = base + QString(kas_animations[i].path);
            QPixmap pixmap(fn);
            pixmap_list.insert(0,pixmap);
        }
        animation_.insert(kas_animations[i].id,pixmap_list);
/*
  The stuff below was replaced by the stuff above. I don't know if it's right.
  mws 13/10/2006 ...and it's Friday the 13th today.
        animation_.insert(kas_animations[i].id,
            new QCanvasPixmapArray(sprites_prefix+"/"+kas_animations[i].path,
                                   kas_animations[i].frames));
*/
        i++;
    }

    ship_ = new MyAnimation(&animation_[ID_SHIP],scene_);
    ship_->hide();

    shield_ = new KShield(&animation_[ID_SHIELD],scene_);
    shield_->hide();
}

/*!
  \internal

  Adds \a count new rocks to the scene.
 */
void KAsteroidsView::addRocks(int count)
{
    for (int i=0; i<count; i++) {
        KRock *rock = new KRock(&animation_[ID_ROCK_MEDIUM],
                                scene_,
                                ID_ROCK_MEDIUM,
                                randInt(2),
                                randInt(2) ? -1 : 1);
        double dx = (2.0 - randDouble()*4.0) * rockSpeed_;
        double dy = (2.0 - randDouble()*4.0) * rockSpeed_;
        rock->setVelocity(dx,dy);
        rock->setImage(randInt(ROCK_IMAGE_COUNT));
        if (dx > 0) {
            if (dy > 0)
                rock->setPos(5,5);
            else
                rock->setPos(5,scene_->height() - 25);
        }
        else {
            if (dy > 0)
                rock->setPos(scene_->width() - 25,5);
            else
                rock->setPos(scene_->width() - 25,scene_->height() - 25);

        }
        rock->show();
        rocks_.append(rock);
    }
}

/*!
  \internal

  Displays a \a text message in a certain \a color, which
  then scrolls off the screen if \a scroll is true. In the
  QTopia phone version, \a scroll is ignored.
 */
void KAsteroidsView::showText(const QString& text, bool scroll)
{
    textSprite_->setPlainText(text);
    textSprite_->setDefaultTextColor(Qt::yellow);

    qreal x = (scene_->width()-textSprite_->boundingRect().width())/2;
    qreal y = 0.0;

#ifdef QTOPIA_PHONE
    scroll = false;
#endif
    if (scroll) {
        y = -textSprite_->boundingRect().height();
        textDy_ = TEXT_SPEED;
    }
    else {
        y = (scene_->height()-textSprite_->boundingRect().height())/2;
        textDy_ = 0;
    }
    textSprite_->setPos(x,y);
    textSprite_->show();
}

/*!
  \internal

  Starts the text scrolling up so that it scrolls off the
  screen.
 */
void KAsteroidsView::hideText()
{
    textDy_ = -TEXT_SPEED;
}

/*!
  \internal

  Returns true if the text message has scrolled completely
  off the top of the screen.
 */
bool KAsteroidsView::textAboveScreen() const
{
    if (textDy_ >= 0)
        return false;
    QRectF br = textSprite_->boundingRect();
    if (br.y() > -br.height())
        return false;
    return true;
}

/*!
  \internal

  Construct a bunch of messages for communicating instructions
  to the user.
 */
void KAsteroidsView::constructMessages(const QString& t)
{
    startGameMessage_ =
        tr("Press %1\nto start game","").arg(t);
    shipKilledMessage_ =
        tr("Ship Destroyed\nPress %1", "").arg(t);
    gameOverMessage_ =
        tr("GAME OVER\nPress %1\nfor new game.","").arg(t);
}

/*!
  \internal

  Display a message telling the user how to start the game
  so he doesn't sit there feeling stupid.
 */
void KAsteroidsView::reportStartGame()
{
    showText(startGameMessage_,Qt::yellow);
}

/*!
  \internal

  Display a message indicating the ship has been destroyed,
  as if it wasn't obvious from all the bits and pieces flying
  around.
 */
void KAsteroidsView::reportShipKilled()
{
    showText(shipKilledMessage_,Qt::yellow);
}

/*!
  \internal

  Display a message indicating the game is over.
 */
void KAsteroidsView::reportGameOver()
{
    showText(gameOverMessage_,false);
}

/*!
  \internal

  This is where most of the game processing happens. It is
  called for each timer event. All the objects on the screen
  are aged and moved, and the important collisions are detected
  and interpreted.

  First the scene's advance() function is called to advance the
  animation of each of the game's objects.
 */
void
KAsteroidsView::timerEvent(QTimerEvent* )
{
    if (!started_) {
        reportStartGame();
        started_ = true;
    }
    scene_->advance();

    processMissiles();
    processFragments();

    while (!exhaust_.isEmpty())
        delete exhaust_.takeFirst();

    processShip();
    processPowerups();

    if (textSprite_->isVisible()) {
        if (textAboveScreen()) {
            textSprite_->hide();
        }
        else {
            textSprite_->moveBy(0,textDy_);
        }
        QRectF br = textSprite_->boundingRect();
        if (br.y() > (scene_->height() - br.height())/2)
            textDy_ = 0;
    }

    if (vitalsChanged_ && !(timerEventCount_ % 10)) {
        emit updateVitals();
        vitalsChanged_ = false;
    }

    timerEventCount_++;
}

/*!
  If there aren't too many powerups of the specifield \a type,
  create a powerup of that \a type and return a pointer to it.
  Otherwise return null.
 */
KPowerup*
KAsteroidsView::createPowerup(int type)
{
    if (KPowerup::quotaFilled(type))
        return 0;
    return new KPowerup(&animation_[type],scene_,type);
}

/*!
  \internal

  Destroy \a rock because it was either hit by a missile
  fired by the ship, or it was hit by the ship itself while
  the ship's shield was up.

  If \a rock is a large rock, it is broken into medium
  size rock fragments. If \a rock is a medium size rock, it
  is broken into small rock fragments. If \a rock is small
  already, it is simply removed from the screen.

  An appropriate rockHit signal is emitted so the game score
  can be updated.

  Additionally, a powerup might be created as a consequence
  of destroying the rock.
 */
void KAsteroidsView::destroyRock(MyAnimation* oldRock)
{
    rocks_.removeAll(oldRock); // destroyed at exit.
    if (oldRock->type() == ID_ROCK_SMALL) {
        KPowerup* new_pup = 0;
        switch (randInt(30)) {
          case 2:
              new_pup = createPowerup(ID_ENERGY_POWERUP);
              break;
          case 10:
              new_pup = createPowerup(ID_TELEPORT_POWERUP);
              break;
          case 15:
              new_pup = createPowerup(ID_BRAKE_POWERUP);
              break;
          case 20:
              new_pup = createPowerup(ID_SHIELD_POWERUP);
              break;
          case 24:
              new_pup = createPowerup(ID_SHOOT_POWERUP);
              break;
        }

        if (new_pup) {
            double r = (0.5 - randDouble()) * 4.0;
            new_pup->setPos(oldRock->x(),oldRock->y());
            new_pup->setVelocity(oldRock->velocityX()+r,oldRock->velocityY()+r);
            new_pup->show();
            powerups_.append(new_pup);
        }
    }

    /*
      Break large rocks into medium rocks and medium rocks
      into small rocks.
     */
    int newRockType = 0;
    if (oldRock->type() == ID_ROCK_LARGE) {
        newRockType = ID_ROCK_MEDIUM;
        emit rockHit(0);
    }
    else if (oldRock->type() == ID_ROCK_MEDIUM) {
        newRockType = ID_ROCK_SMALL;
        emit rockHit(1);
    }
    else if (oldRock->type() == ID_ROCK_SMALL) {
        emit rockHit(2);
    }
    if (newRockType) {
        double x_multiplier[4] = { 1.0, 1.0, -1.0, -1.0 };
        double y_multiplier[4] = { -1.0, 1.0, -1.0, 1.0 };

        double dx = oldRock->velocityX();
        double dy = oldRock->velocityY();

        double maxRockSpeed = ROCK_SPEED_MULTIPLIER * rockSpeed_;
        if (dx > maxRockSpeed)
            dx = maxRockSpeed;
        else if (dx < -maxRockSpeed)
            dx = -maxRockSpeed;
        if (dy > maxRockSpeed)
            dy = maxRockSpeed;
        else if (dy < -maxRockSpeed)
            dy = -maxRockSpeed;

        MyAnimation* newRock;

        /*
          When the old rock explodes, we create four new, smaller
          rocks in its place. If the old rock is a large one, create
          four medium size rocks. If the old rock is a medium one,
          create four small ones. If the old rock is already small,
          we don't create anything. We don't even get into this loop
          if the old rock is small.
         */
        for (int i = 0; i < 4; i++) {
            double r = rockSpeed_/2 - (randDouble() * rockSpeed_);
            if (oldRock->type() == ID_ROCK_LARGE) {
                newRock = new KRock(&animation_[ID_ROCK_MEDIUM],
                                    scene_,
                                    ID_ROCK_MEDIUM,
                                    randInt(2),
                                    randInt(2) ? -1 : 1);
            }
            else {
                newRock = new KRock(&animation_[ID_ROCK_SMALL],
                                    scene_,
                                    ID_ROCK_SMALL,
                                    randInt(2),
                                    randInt(2) ? -1 : 1);
            }

            /*
              Each new rock is given an initial position which
              is offset from the old rock's last position by the
              width of one quadrant of the old rock's bounding box.
              Each of the new rocks is positioned in a different
              quadrant of the old rock's bounding box.
             */
            qreal quadrant = newRock->boundingRect().width()/4;
            newRock->setPos(oldRock->x() + (x_multiplier[i] * quadrant),
                            oldRock->y() + (y_multiplier[i] * quadrant));
            newRock->setVelocity(dx + (x_multiplier[i] * rockSpeed_) + r,
                                 dy + (y_multiplier[i] * rockSpeed_) + r);
            newRock->setImage(randInt(ROCK_IMAGE_COUNT));
            newRock->show();
            rocks_.append(newRock);
        }
    }
    delete oldRock;
    if (rocks_.count() == 0)
        emit rocksRemoved();
}

/*!
  \internal

  Reduce the ship's power level by \a reduction.
 */
void KAsteroidsView::reducePower(int reduction)
{
    shipPowerLevel_ -= reduction;
    if (shipPowerLevel_ <= 0) {
        shipPowerLevel_ = 0;
        stopEngine();
        dropShield();
        shieldStrength_ = 0;
    }
    vitalsChanged_ = true;
}

/*!
  \internal

  I don't think the exhaust is working at the moment.
  I will look into it later.
 */
void
KAsteroidsView::addExhaust(double x,
                           double y,
                           double dx,
                           double dy,
                           int count)
{
    for (int i = 0; i < count; i++) {
        KExhaust* e = new KExhaust(&animation_[ID_EXHAUST], scene_);
        e->setPos(x + 2 - randDouble()*4, y + 2 - randDouble()*4);
        e->setVelocity(dx, dy);
        e->show();
        exhaust_.append(e);
    }
}

/*!
  \internal

  Process all the ship fragments that were created when the
  ship exploded due to a collision with a rock.
 */
void
KAsteroidsView::processFragments()
{
    if (ship_fragments_.isEmpty())
        return;
    for (int i=0; i<ship_fragments_.size(); ++i) {
        KFragment* fragment = ship_fragments_.at(i);
        if (fragment->isExpired()) {
            ship_fragments_.removeAll(fragment);
            delete fragment;
        }
        else {
            fragment->incrementAge();
            fragment->advanceImage();
        }
    }
}

/*!
  \internal

  Process all fired missiles that haven't petered out yet.
  Each fired missile's age is increment. It's age is then
  tested. If the missile is too old, ie if it has reached
  its maximum range, remove it from the screen and from
  the list and delete it.

  For each fired missile that had not passed its maximum
  range, check to see if it has hit a rock. If it has hit
  a rock, call destroyRock() to break up the rock and randomly
  add a powerup symbol to the screen.

  When a rock is hit, it explodes into smaller rocks, and
  the original rock is removed from the screen and from
  the list and is deleted.
 */
void
KAsteroidsView::processMissiles()
{
    KMissile* missile;
    QList<KMissile*>::iterator mi = missiles_.begin();

    // if a missile has hit a rock, remove missile and break rock into smaller
    // rocks or remove completely.

    while (mi != missiles_.end()) {
        missile = (*mi);
        missile->incrementAge();

        if (missile->isExpired()) {
            mi = missiles_.erase(mi);
            delete missile;
            continue;
        }

        //wrapSprite(missile);

        QList<QGraphicsItem*> hits = missile->collidingItems();
        QList<QGraphicsItem*>::iterator hi;
        for (hi=hits.begin(); hi!=hits.end(); ++hi) {
            if (((*hi)->type() >= ID_ROCK_LARGE) &&
                ((*hi)->type() <= ID_ROCK_SMALL)) {
                incrementRocksHit();
                destroyRock((MyAnimation*)(*hi));
                missiles_.erase(mi);
                delete missile;
                return;
            }
        }
        ++mi;
    }
}

/*
  \internal

  This is where the ship is updated. This function is called
  once for each timer period.
 */
void
KAsteroidsView::processShip()
{
    if (!ship_->isVisible())
        return;

    if (rotatingSlowly_)
        rotatingSlowly_--;

    if (rotateShipLeft_) {
        shipAngleIndex_ -= rotatingSlowly_ ? 1 : rotationRate_;
        if (shipAngleIndex_ < 0)
            shipAngleIndex_ += SHIP_STEPS;
    }

    if (rotateShipRight_) {
        shipAngleIndex_ += rotatingSlowly_ ? 1 : rotationRate_;
        if (shipAngleIndex_ >= SHIP_STEPS)
            shipAngleIndex_ -= SHIP_STEPS;
    }

    double angle = shipAngleIndex_ * PI_X_2 / SHIP_STEPS;
    double cosangle = cos(angle);
    double sinangle = sin(angle);

    if (shipIsBraking()) {
        vitalsChanged_ = true;
        stopEngine();
        stopShipRotation();
        if (fabs(shipDx_) < 2.5 && fabs(shipDy_) < 2.5) {
            shipDx_ = 0.0;
            shipDy_ = 0.0;
            ship_->setVelocity(shipDx_,shipDy_);
            releaseBrakes();
        }
        else {
            double motionAngle = atan2(-shipDy_, -shipDx_);
            if (angle > M_PI)
                angle -= PI_X_2;
            double angleDiff = angle - motionAngle;
            if (angleDiff > M_PI)
                angleDiff = PI_X_2 - angleDiff;
            else if (angleDiff < -M_PI)
                angleDiff = PI_X_2 + angleDiff;
            double fdiff = fabs(angleDiff);
            if (fdiff > 0.08) {
                if (angleDiff > 0)
                    rotateShipLeft_ = true;
                else if (angleDiff < 0)
                    rotateShipRight_ = true;
                if (fdiff > 0.6)
                    rotationRate_ = brakeForce() + 1;
                else if (fdiff > 0.4)
                    rotationRate_ = 2;
                else
                    rotationRate_ = 1;

                if (rotationRate_ > 5)
                    rotationRate_ = 5;
            }
            else if (fabs(shipDx_) > 1 || fabs(shipDy_) > 1) {
                engineIsOn_ = true;
                // we'll make braking a bit faster
                shipDx_ += cosangle/6 * (brakeForce() - 1);
                shipDy_ += sinangle/6 * (brakeForce() - 1);
                reducePower(BRAKE_ON_COST);
                addExhaust(ship_->x() + 10 - cosangle*11,
                           ship_->y() + 10 - sinangle*11,
                           shipDx_-cosangle, shipDy_-sinangle,
                           brakeForce()+1);
            }
        }
    }

    if (engineIsOn()) {
        // The ship has a terminal velocity, but trying to go faster
        // still uses fuel (can go faster diagonally - don't care).
        double thrustx = cosangle/8;
        double thrusty = sinangle/8;
        if (fabs(shipDx_ + thrustx) < MAX_SHIP_SPEED)
            shipDx_ += thrustx;
        if (fabs(shipDy_ + thrusty) < MAX_SHIP_SPEED)
            shipDy_ += thrusty;
        ship_->setVelocity(shipDx_,shipDy_);
        reducePower(1);
        vitalsChanged_ = true;
        addExhaust(ship_->x() + 10 - cosangle*10,
                   ship_->y() + 10 - sinangle*10,
                   shipDx_-cosangle, shipDy_-sinangle, 3);
    }

    ship_->setImage(shipAngleIndex_ >> 1);

    if (shipIsShooting()) {
#ifndef QTOPIA_PHONE
        int maxMissiles = firePower_ + 2;
#else
        int maxMissiles = firePower_ + 1;
#endif
        if (shipCanShoot() && ((int)missiles_.count() < maxMissiles)) {
            KMissile* missile = new KMissile(&animation_[ID_MISSILE],scene_);
#ifdef QTOPIA_PHONE
            missile->setMaximumAge(12);
#endif
            missile->setPos(11 + ship_->x() + cosangle * 11,
                            11 + ship_->y() + sinangle * 11);
            missile->setVelocity(shipDx_ + cosangle * MISSILE_SPEED,
                                 shipDy_ + sinangle * MISSILE_SPEED);
            missile->show();
            missiles_.append(missile);
            emit missileFired();
            incrementShotsFired();
            reducePower(1);
            vitalsChanged_ = true;

            // delay firing next missile.
            delayShooting(5);
        }

        decrementNextShotDelay();
    }

    if (teleportShip_) {
        int ra = rand() % 10;
        if(ra == 0)
            ra += rand() % 20;
        int xra = ra * 60 + ((rand() % 20) * (rand() % 20));
        int yra = ra * 50 - ((rand() % 20) * (rand() % 20));
        ship_->setPos(xra, yra);
        teleportShip_ = false;
        if (teleportCount_ > 0) {
            --teleportCount_;
            vitalsChanged_ = true;
        }
    }

    if (shieldIsUp_) {
        shield_->show();
        static int sf = 0;
        sf++;

        if (sf % 2)
            shield_->advanceImage();
        shield_->setPos(ship_->x() - 5, ship_->y() - 5);

        QList<QGraphicsItem*> hits = shield_->collidingItems();
        QList<QGraphicsItem*>::Iterator it;

        for (it=hits.begin(); it!=hits.end(); ++it) {
            int factor = 0;
            bool shieldHitRock = false;
            switch ((*it)->type()) {
            case ID_ROCK_LARGE:
                factor = 3;
                shieldHitRock = true;
                if (shieldStrength_ < 3)
                    shieldIsUp_ = false;
                break;

            case ID_ROCK_MEDIUM:
                factor = 2;
                shieldHitRock = true;
                if (shieldStrength_ < 2)
                    shieldIsUp_ = false;
                break;

            case ID_ROCK_SMALL:
                factor = 1;
                shieldHitRock = true;
                if (shieldStrength_ < 1)
                    shieldIsUp_ = false;
                break;

            case ID_ENERGY_POWERUP:
                ((KPowerup*)(*it))->expire();
                break;
            case ID_TELEPORT_POWERUP:
                ((KPowerup*)(*it))->expire();
                break;
            case ID_BRAKE_POWERUP:
                ((KPowerup*)(*it))->expire();
                break;
            case ID_SHIELD_POWERUP:
                ((KPowerup*)(*it))->expire();
                break;
            case ID_SHOOT_POWERUP:
                ((KPowerup*)(*it))->expire();
                break;
            default:
                factor = 0;
            }
            if (shieldHitRock) {
                destroyRock((MyAnimation*)(*it));
                // the more shields we have the less costly
                reducePower(factor * (SHIELD_HIT_COST-shieldStrength_*2));
                vitalsChanged_ = true;
            }
            if (!shieldIsUp_) {
                // shield failed
                break;
            }
        }
    }

    if (!shieldIsUp_) {
        shield_->hide();
        QList<QGraphicsItem*> hits = ship_->collidingItems();
        QList<QGraphicsItem*>::Iterator it;
        for (it = hits.begin(); it != hits.end(); ++it) {
            KPowerup* pup = 0;
            switch ((*it)->type()) {
                case ID_ROCK_LARGE:
                case ID_ROCK_MEDIUM:
                case ID_ROCK_SMALL:
                {
                    KFragment* f;
                    for (int i = 0; i < 4; i++) {
                        f = new KFragment(&animation_[ID_FRAGMENT],scene_);
                        f->setPos(ship_->x() + 5 - randDouble() * 10,
                                  ship_->y() + 5 - randDouble() * 10);
                        f->setImage(randInt(FRAG_IMAGE_COUNT));
                        f->setVelocity(1-randDouble()*2,1-randDouble()*2);
                        f->setMaximumAge(60 + randInt(60));
                        f->show();
                        ship_fragments_.append(f);
                    }
                    ship_->hide();
                    shield_->hide();
                    emit shipKilled();
                    vitalsChanged_ = true;
                    return;
                }
                case ID_ENERGY_POWERUP:
                    shipPowerLevel_ += 15;
                    if (shipPowerLevel_ > MAX_SHIP_POWER_LEVEL)
                        shipPowerLevel_ = MAX_SHIP_POWER_LEVEL;
                    ((KPowerup*)(*it))->expire();
                    vitalsChanged_ = true;
                    break;
                case ID_TELEPORT_POWERUP:
                    pup = (KPowerup*)(*it);
                    teleportCount_++;
                    pup->expire();
                    vitalsChanged_ = true;
                    break;
                case ID_BRAKE_POWERUP:
                    pup = (KPowerup*)(*it);
                    incrementBrakeForce();
                    pup->expire();
                    vitalsChanged_ = true;
                    break;
                case ID_SHIELD_POWERUP:
                    pup = (KPowerup*)(*it);
                    if (shieldStrength_ < MAX_SHIELD_STRENGTH)
                        shieldStrength_++;
                    pup->expire();
                    vitalsChanged_ = true;
                    break;
                case ID_SHOOT_POWERUP:
                    pup = (KPowerup*)(*it);
                    if (firePower_ < MAX_FIREPOWER)
                        firePower_++;
                    pup->expire();
                    vitalsChanged_ = true;
                    break;
            }
        }
    }
}

/*!
  \internal

  The ship often hits special symbols representing packets
  of increased capabilities. These hits are processed here.
  They include increased ship power, increased fire power,
  increased shield strength, and increased braking force
  to slow the ship. There is also a symbol representing
  the ability to teleport the ship to a safe location, but
  this kind of hit has been disabled in the Qtopia version.
  I don't know why. Maybe the orignal author couldn't get
  it to work?

  If the ship hits a powerup, remove the powerup from the
  screen and award the value to the player.

  If the shield is up and the power up hits the shield,
  sorry, the player doesn't get the powerup value.

  If a missile hits a powerup and the option allowing
  missiles to destroy powerups has been enabled, destroy
  the powerup.
 */
void KAsteroidsView::processPowerups()
{
    if (powerups_.isEmpty())
        return;

    KPowerup* pup = 0;
    QList<KPowerup*>::iterator pi = powerups_.begin();

    while (pi != powerups_.end()) {
        pup = (*pi);
        pup->incrementAge();

        if(pup->isExpired()) {
            pi = powerups_.erase(pi);
            delete pup;
            continue;
        }

        //wrapSprite(pup);
        ++pi;
    }
}

/*!
  \internal

  Generate and return a random double.
 */
double KAsteroidsView::randDouble()
{
    int v = rand();
    return (double)v / (double)RAND_MAX;
}

/*!
  \internal

  Generate and return a random integer in the specified
  \a range.
 */
int KAsteroidsView::randInt(int range)
{
    return rand() % range;
}

/*!
  \internal

  Turn on engine. While the engine is on, velocity increases.
 */
void KAsteroidsView::startEngine()
{
    engineIsOn_ = (shipPowerLevel_ > 0);
}

/*!
  \internal

  Turn off engine. The ship doesn't stop when you turn off
  the engine. You just coast.
 */
void KAsteroidsView::stopEngine()
{
    engineIsOn_ = false;
}

/*!
  \internal

  Returns 1 if both the shield strength and power level are
  greater than 0. Otherwise returns 0.
 */
int KAsteroidsView::checksum() const
{
    if (shieldStrength() > 0) {
        if (shipPowerLevel() > 0) {
            return 1;
        }
    }
    return 0;
}

/*!
  If any teleports are available, enable teleporting
  for the next timer interrup when the ship is processed.
 */
void
KAsteroidsView::teleport()
{
    if (teleportCount_)
        teleportShip_ = true;
}
