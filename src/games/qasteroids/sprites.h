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

#ifndef __SPRITES_H__
#define __SPRITES_H__

#include <QGraphicsPixmapItem>

#define ID_Base             QGraphicsItem::UserType
#define ID_ROCK_LARGE       ID_Base + 1024
#define ID_ROCK_MEDIUM      ID_Base + 1025
#define ID_ROCK_SMALL       ID_Base + 1026
#define ID_MISSILE          ID_Base + 1030
#define ID_FRAGMENT         ID_Base + 1040
#define ID_EXHAUST          ID_Base + 1041
#define ID_ENERGY_POWERUP   ID_Base + 1310
#define ID_TELEPORT_POWERUP ID_Base + 1311
#define ID_BRAKE_POWERUP    ID_Base + 1312
#define ID_SHIELD_POWERUP   ID_Base + 1313
#define ID_SHOOT_POWERUP    ID_Base + 1314
#define ID_SHIP             ID_Base + 1350
#define ID_SHIELD           ID_Base + 1351

#if 0
#define ID_ROCK_LARGE           1024
#define ID_ROCK_MEDIUM          1025
#define ID_ROCK_SMALL           1026

#define ID_MISSILE              1030

#define ID_FRAGMENT             1040
#define ID_EXHAUST              1041

#define ID_ENERGY_POWERUP       1310
#define ID_TELEPORT_POWERUP     1311
#define ID_BRAKE_POWERUP        1312
#define ID_SHIELD_POWERUP       1313
#define ID_SHOOT_POWERUP        1314

#define ID_SHIP                 1350
#define ID_SHIELD               1351
#endif

#define MAX_SHIELD_AGE          350
#define MAX_POWERUP_AGE         500
#define EXPIRED_POWERUP         MAX_POWERUP_AGE+1
#define MAX_MISSILE_AGE         25

class MyAnimation : public QGraphicsPixmapItem
{
 public:
    MyAnimation(QList<QPixmap>* animation, QGraphicsScene* scene);
    virtual ~MyAnimation();
    virtual void incrementAge() { }
    virtual bool isExpired() const { return false; }
    virtual void setMaximumAge(int ) { }
    virtual void advance(int phase);
    virtual void advanceImage();

    int currentImage() const { return current_image_; }
    int imageCount() const { return images_->size(); }
    void setImage(int index);
    void resetImage();
    void setVelocity(qreal vx, qreal vy) {
        velocity_x_ = vx;
        velocity_y_ = vy;
    }
    qreal velocityX() const { return velocity_x_; }
    qreal velocityY() const { return velocity_y_; }
    void wrap();

 private:
    int current_image_;
    QList<QPixmap>* images_;
    qreal velocity_x_;
    qreal velocity_y_;
};

class KMissile : public MyAnimation
{
 public:
    KMissile(QList<QPixmap>* s, QGraphicsScene* c)
        : MyAnimation(s,c)
        { currentAge_ = 0; maximumAge_ = MAX_MISSILE_AGE; }

    virtual int type() const { return ID_MISSILE; }
    virtual void incrementAge() { currentAge_++; }
    virtual bool isExpired() const { return currentAge_ > maximumAge_; }
    virtual void setMaximumAge(int age) { maximumAge_ = age; }

 private:
    int currentAge_;
    int maximumAge_;
};

class KFragment : public MyAnimation
{
 public:
    KFragment(QList<QPixmap>* s, QGraphicsScene* c)
        : MyAnimation(s,c) {  maximumAge_ = 7; }

    virtual int type() const {  return ID_FRAGMENT; }
    virtual void incrementAge() { maximumAge_--; }
    virtual bool isExpired() const { return maximumAge_ <= 0; }
    virtual void setMaximumAge( int age ) { maximumAge_ = age; }

 private:
    int maximumAge_;
};

class KExhaust : public MyAnimation
{
 public:
    KExhaust( QList<QPixmap> *s, QGraphicsScene *c )
        : MyAnimation( s, c )
        {  maximumAge_ = 1; }

    virtual int type() const {  return ID_EXHAUST; }
    virtual void incrementAge() { maximumAge_--; }
    virtual bool isExpired() const { return maximumAge_ <= 0; }
    virtual void setMaximumAge(int age) { maximumAge_ = age; }

 private:
    int maximumAge_;
};

class KPowerup : public MyAnimation
{
 public:
    KPowerup(QList<QPixmap>* s, QGraphicsScene* c, int t);
    ~KPowerup() { decrementCount(); }

  virtual int type() const { return type_; }
  virtual void incrementAge() { currentAge_++; }
  virtual bool isExpired() const { return currentAge_ > MAX_POWERUP_AGE; }

  void expire() { currentAge_ = EXPIRED_POWERUP; }

  static void clearCounts();
  static bool quotaFilled(int type);

 private:
  void incrementCount();
  void decrementCount();

 protected:
  static int    energy_powerups_;
  static int    teleport_powerups_;
  static int    brake_powerups_;
  static int    shield_powerups_;
  static int    shoot_powerups_;

 protected:
  int           currentAge_;
  int           type_;
};

class KRock : public MyAnimation
{
 public:
    KRock (QList<QPixmap>* s, QGraphicsScene* c, int t, int sk, int st)
        : MyAnimation(s,c)
        { type_ = t; skip = cskip = sk; step = st; }

    virtual void advanceImage();
    virtual int type() const { return type_; }

 private:
    int type_;
    int skip;
    int cskip;
    int step;
};

class KShield : public MyAnimation
{
 public:
  KShield(QList<QPixmap>* s, QGraphicsScene* c)
      : MyAnimation(s,c) { }

  virtual int type() const { return ID_SHIELD; }
};

#endif
