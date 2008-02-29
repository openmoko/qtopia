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

#include "sprites.h"
#include <QGraphicsScene>

#define ENERGY_POWERUP_QUOTA 2
#define TELEPORT_POWERUP_QUOTA 2
#define BRAKE_POWERUP_QUOTA 2
#define SHIELD_POWERUP_QUOTA 2
#define SHOOT_POWERUP_QUOTA 2

/*! \class MyAnimation
  \brief The MyAnimation class is the base class for all the
         animated elements that appear on the screen in the
         asteroids game.

  The MyAnimation class inherits QGraphicsPixmapItem. It
  contains a pointer to a list of pixmaps, which are used
  to draw the item. the list represents an animation. The
  list can contain a single pixmap for items that are not
  animated. For example, the powerups are not animated,
  but the rocks and ship are animated.

  The state of a MyAnimation includes the index into the
  list of pixmaps of the current image being displayed.
  It also includes the x and y components of the item's
  velocity on the screen, which is used for computing its
  next position.
*/

/*!
  The constructor requires a pointer to the list of pixmaps
  used to animate the item on the screen. The list can have
  only one image in it, but it must have at least one.

  The \a scene must also be provided, and it is passed to
  the QGraphicsPixmapItem constructor.
 */
MyAnimation::MyAnimation(QList<QPixmap>* animation, QGraphicsScene* scene)
    : QGraphicsPixmapItem(0,scene),
      images_(animation),
      velocity_x_(0.0),
      velocity_y_(0.0)
{
    resetImage();
}

/*!
  The destructor is virtual, but it does nothing.
  An instance of MyAnimation does not own its list
  of images.
 */
MyAnimation::~MyAnimation()
{
    // nothing.
}

/*! \fn void MyAnimation::incrementAge()
  This virtual function does nothing in the base class.
  In subclasses of items that age and die, it increments
  the item's current age by one unit of time or distance,
  however the subclass's age is measured.

  For example, powerups live a certain amount of time,
  while missiles travel a certain distance. It doesn't
  make any difference in the implementation, but there
  it is.
 */

/*! \fn bool MyAnimation::isExpired() const
  In this base class, this function returns false,
  but in subclasses, it returns true if the item has
  past its use-by date, ie when its current age is
  greater than or equal to its maximum allowed age.
 */

/*! \fn void MyAnimation::setMaximumAge(int )
  This virtual function does nothing in this base class.
  In subclasses for items that can age, it sets the
  item's maximum allowed age to the value provided.
 */

/*! \fn int MyAnimation::currentImage() const
  Returns the index in the list of pixmaps of the
  pixmap currently being displayed.
 */

/*! \fn int MyAnimation::imageCount() const
  Returns the number of images in the animation list.
  Always greater than or equal to 1.
 */

/*!
  This function sets the animation's current image
  to the one specified by \a index.
 */
void MyAnimation::setImage(int index)
{
    current_image_ = index % images_->size();
    setPixmap(images_->at(current_image_));
}

/*!
  This function sets the animation's current image
  to the first one in its image list.
 */
void MyAnimation::resetImage()
{
    current_image_ = 0;
    setPixmap(images_->at(0));
}

/*!
 */
void MyAnimation::advanceImage()
{
    if (images_->size() == 1)
        return;
    current_image_ = (current_image_+1) % images_->size();
    setPixmap(images_->at(current_image_));
}

/*! \fn void MyAnimation::setVelocity(qreal vx, qreal vy)
  Sets the item's x and why component velocities to \a vx
  and \a vy. These are used to compute each successive
  position on the screen for the animated item.
 */

/*! \fn qreal MyAnimation::velocityX() const
  Returns the x component of the item's velocity.
 */

/*! \fn qreal MyAnimation::velocityY() const
  Returns the y component of the item's velocity.
 */

/*!
  This function is called by QGraphicsScene::advance().
  It is called twice for this item at each animation step,
  first with \a phase = 0, and then with \a phase = 1.

  ie, QGraphicsScene::advance() first calls each item's
  advance() function as item->advance(0), then it calls
  each item's advance() function as item->advance(1).

  Phase 0 is meant to tell each item to get ready to
  move itself. Phase 1 is when the actual move is meant
  to occur.

  This implementation only uses phase 1 at the moment.
 */
void
MyAnimation::advance(int phase)
{
    if (phase == 1) {
        advanceImage();
        if ((velocity_x_ != 0.0) || (velocity_y_ != 0.0)) {
            moveBy(velocity_x_,velocity_y_);
            wrap();
        }
    }
}

/*!
  \internal

  When a symbol goes off the edge of the screen, this function
  puts it back on the screen somewhere else.
 */
void
MyAnimation::wrap()
{
    int tmp_x = int(x() + boundingRect().width() / 2);
    int tmp_y = int(y() + boundingRect().height() / 2);

    QGraphicsScene* the_scene = scene();
    if (tmp_x > the_scene->width())
        setPos(x() - the_scene->width(),y());
    else if (tmp_x < 0)
        setPos(the_scene->width() + x(),y());

    if (tmp_y > the_scene->height())
        setPos(x(),y() - the_scene->height());
    else if (tmp_y < 0)
        setPos(x(),the_scene->height() + y());
}

/*! \class KMissile
  \brief Class KMissile contains the missile pixmap.

  It inherits MyAnimation but it isn't animated, because
  there is nothing to animate. Its pixmap list contains
  only one image, which just looks like a yellow dot on
  the screen. It doesn't look at all like a missile.

  A missile, once fired, travels in a straight line until
  it reaches its maximum range. Then it dies. To implement
  this, the internel state of the class contains a current
  age value and a maximum age value. The current age is
  imcremented each time the animation is advanced.
 */


/*!
  Sets the current image for this rock to the next image
  in the animation sequence.
 */
void KRock::advanceImage()
{
    if (cskip-- <= 0) {
        setImage((currentImage()+step+imageCount()) % imageCount());
        cskip = qAbs(skip);
    }
}

/*!
  Constructs a powerup of type \a t. Increments the count
  of active powerups of type \a t.
 */
KPowerup::KPowerup(QList<QPixmap>* s, QGraphicsScene* c, int t)
    : MyAnimation(s,c),
      currentAge_(0),
      type_(t)
{
    incrementCount();
}

/*!
  Clear the powerup counts. Each powerup count
  keeps track of how many of that type of powerup
  are currently on the screen.
 */
void
KPowerup::clearCounts()
{
    KPowerup::energy_powerups_ = 0;
    KPowerup::teleport_powerups_ = 0;
    KPowerup::brake_powerups_ = 0;
    KPowerup::shield_powerups_ = 0;
    KPowerup::shoot_powerups_ = 0;
}

int KPowerup::energy_powerups_ = 0;
int KPowerup::teleport_powerups_ = 0;
int KPowerup::brake_powerups_ = 0;
int KPowerup::shield_powerups_ = 0;
int KPowerup::shoot_powerups_ = 0;

/*!
  A static function that returns true if the quota of
  powerups of this \a type are already on the screen.
  Otherwise returns false.
 */
bool
KPowerup::quotaFilled(int type)
{
    switch (type) {
        case ID_ENERGY_POWERUP:
            return (energy_powerups_ >= ENERGY_POWERUP_QUOTA);
        case ID_TELEPORT_POWERUP:
            return (teleport_powerups_ >= TELEPORT_POWERUP_QUOTA);
        case ID_BRAKE_POWERUP:
            return (brake_powerups_ >= BRAKE_POWERUP_QUOTA);
        case ID_SHIELD_POWERUP:
            return (shield_powerups_ >= SHIELD_POWERUP_QUOTA);
        case ID_SHOOT_POWERUP:
            return (shoot_powerups_ >= SHOOT_POWERUP_QUOTA);
        default:
            break;
    }
    return false;
}

/*!
  This powerup was just created. Increment the count of
  active powerups of this type.
 */
void
KPowerup::incrementCount()
{
    switch (type_) {
        case ID_ENERGY_POWERUP:
            ++energy_powerups_;
            break;
        case ID_TELEPORT_POWERUP:
            ++teleport_powerups_;
            break;
        case ID_BRAKE_POWERUP:
            ++brake_powerups_;
            break;
        case ID_SHIELD_POWERUP:
            ++shield_powerups_;
            break;
        case ID_SHOOT_POWERUP:
            ++shoot_powerups_;
            break;
        default:
            break;
    }
}

/*!
  This powerup is being destroyed. Decrement the count of
  active powerups of this type.
 */
void
KPowerup::decrementCount()
{
    switch (type_) {
        case ID_ENERGY_POWERUP:
            if (energy_powerups_ > 0)
                --energy_powerups_;
            break;
        case ID_TELEPORT_POWERUP:
            if (teleport_powerups_ > 0)
                --teleport_powerups_;
            break;
        case ID_BRAKE_POWERUP:
            if (brake_powerups_ > 0)
                --brake_powerups_;
            break;
        case ID_SHIELD_POWERUP:
            if (shield_powerups_ > 0)
                --shield_powerups_;
            break;
        case ID_SHOOT_POWERUP:
            if (shoot_powerups_ > 0)
                --shoot_powerups_;
            break;
        default:
            break;
    }
}
