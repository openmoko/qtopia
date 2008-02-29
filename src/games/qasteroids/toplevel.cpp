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

#include "toplevel.h"
#include "ledmeter.h"

#include <qtopiaapplication.h>
#include <qdevicebuttonmanager.h>
#include <qsoftmenubar.h>

#include <QMenu>
#include <QKeyEvent>
#include <qlabel.h>
#include <qlayout.h>
#include <qlcdnumber.h>
#include <qpushbutton.h>
#include <QDesktopWidget>

struct GameLevelStruct
{
    int    nrocks_;
    double rockSpeed_;
};

#define MAX_GAME_LEVELS      16

#ifndef QTOPIA_PHONE

GameLevelStruct gameLevels[MAX_GAME_LEVELS] =
{
    { 1, 0.4 },
    { 1, 0.6 },
    { 2, 0.5 },
    { 2, 0.7 },
    { 2, 0.8 },
    { 3, 0.6 },
    { 3, 0.7 },
    { 3, 0.8 },
    { 4, 0.6 },
    { 4, 0.7 },
    { 4, 0.8 },
    { 5, 0.7 },
    { 5, 0.8 },
    { 5, 0.9 },
    { 5, 1.0 }
};

#else

GameLevelStruct gameLevels[MAX_GAME_LEVELS] =
{
    { 1, 0.2 },
    { 1, 0.3 },
    { 1, 0.25 },
    { 1, 0.35 },
    { 2, 0.4 },
    { 2, 0.3 },
    { 2, 0.35 },
    { 2, 0.4 },
    { 3, 0.3 },
    { 3, 0.35 },
    { 3, 0.4 },
    { 3, 0.35 },
    { 4, 0.4 },
    { 4, 0.45 },
    { 4, 0.5 }
};

#endif

#if 0

/*
  These had been used for the QTOPIA_PHONE case,
  but they are too slow on the GreenPhone. mws
 */
GameLevelStruct gameLevels[MAX_GAME_LEVELS] =
{
    { 1, 0.1 },
    { 1, 0.2 },
    { 1, 0.3 },
    { 1, 0.4 },
    { 2, 0.1 },
    { 2, 0.2 },
    { 2, 0.3 },
    { 2, 0.4 },
    { 3, 0.1 },
    { 3, 0.2 },
    { 3, 0.3 },
    { 3, 0.4 },
    { 4, 0.1 },
    { 4, 0.2 },
    { 4, 0.3 }
};

#endif

class RowWidget : public QWidget
{
    Q_OBJECT

  public:
    RowWidget(QWidget* parent);

  protected:
    void paintEvent(QPaintEvent* event);
};

RowWidget::RowWidget(QWidget* parent)
    : QWidget(parent)
{
    setPalette(parent->palette());
    setBackgroundRole(QPalette::Window);
}

void RowWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(),Qt::black);
}

/*!
  \internal
  \class KAstTopLevel

  \brief Class KAstTopLevel provides the implementation of
         the main window for the Asteroids game.

  It constructs all the widgets and lays them out. The main
  widget it constructs is the single instance of
  \l {KAsteroidsView} {KAsteroidsView}.

 */

/*!
  The standard constructor creates and lays out all the
  widgets. There is a top row widget for displaying data
  values, a bottom row widget for displaying data values,
  and a large view widget in the middle where the game is
  animated and played.
 */
KAstTopLevel::KAstTopLevel(QWidget* parent, Qt::WFlags fl)
    : QMainWindow(parent,fl),
    shipDestroyed(":sound/qasteroids/shipdestroyed"),
    rockDestroyed(":sound/qasteroids/rockdestroyed"),
    missileFired(":sound/qasteroids/missilefired")
{
    QtopiaApplication::grabKeyboard();

#ifdef QTOPIA_PHONE
    contextMenu_ = 0;
#endif

    setWindowTitle(tr("Asteroids"));
    QPalette p = buildPalette();
    QApplication::setPalette(p);
    QWidget* mainWin = new QWidget(this);
    setCentralWidget(mainWin);
    mainWin->setPalette(p);

    /*
      Give that "main" widget a vertical box layout. To the
      vertical layout, add a top horizontal layout, the game
      view graphics widget, and a bottom horizontal layout.
     */
    QVBoxLayout* vb = new QVBoxLayout(mainWin);
    vb->setSpacing(0);
    vb->setMargin(0);
    vb->addWidget(buildTopRow(mainWin));
    view_ = buildAsteroidsView(mainWin);
    QSizePolicy policy( QSizePolicy::MinimumExpanding,
                        QSizePolicy::MinimumExpanding );
    view_->setSizePolicy( policy );
    vb->addWidget(view_,10);
    vb->addWidget(buildBottomRow(mainWin));

    shipCount_ = 3;

    actions_.insert(Qt::Key_0,Launch);
    actions_.insert(Qt::Key_Up,Thrust);
    actions_.insert(Qt::Key_Left,RotateLeft);
    actions_.insert(Qt::Key_Right,RotateRight);
    actions_.insert(Qt::Key_Down,Brake);

#ifndef QTOPIA_PHONE
    actions_.insert(Qt::Key_Enter,Shoot);
    actions_.insert(Qt::Key_Space,Shoot);
    actions_.insert(Qt::Key_Z,Teleport);
    actions_.insert(Qt::Key_P,Pause);
#else
    actions_.insert(Qt::Key_NumberSign,Teleport);
    actions_.insert(Qt::Key_Asterisk,Pause);
    actions_.insert(Qt::Key_Select,Shoot);
    actions_.insert(Qt::Key_Context1,Shield);
    QSoftMenuBar::setLabel(this,
                           Qt::Key_0,
                           "qasteroids/ship/ship0000",
                           tr("Launch"));
    contextMenu_ = QSoftMenuBar::menuFor(this);
#endif

#ifndef QTOPIA_PHONE
    const QValueList<QDeviceButton>& buttons =
        QDeviceButtonManager::instance().buttons();
    actions_.insert(buttons[0].keycode(),Launch);
    actions_.insert(buttons[1].keycode(),Shield);
    QString s = buttons[0].userText();
#else
    QString s = tr("zero (0)");
#endif

    view_->constructMessages(s);
    setFocusPolicy(Qt::StrongFocus);
}

/*!
  The destructor has nothing to do.
 */
KAstTopLevel::~KAstTopLevel()
{
    // nothing.
}

/*!
  Construct the palette for the asteroids game. This can
  probably be improved.
 */
QPalette KAstTopLevel::buildPalette()
{
    //    QColorGroup grp(Qt::darkGreen, Qt::black, QColor(128, 128, 128),
    //            QColor(64, 64, 64), Qt::black, Qt::darkGreen, Qt::black);
    //    QPalette pal(grp, grp, grp);

    // The stuff below is meant to replace the commented stuff above.
    // I don't know if it is right. (mws)
    QBrush foreground(Qt::darkGreen);
    QBrush background(Qt::black);
    QBrush light(QColor(128,128,128));
    QBrush dark(QColor(64,64,64));
    QBrush mid(Qt::black);
    QBrush text(Qt::darkGreen);
    QBrush base(Qt::black);
    QPalette p(foreground,background,light,dark,mid,text,text,base,background);
    return p;
}

/*!
  Build a widget containing the top row of widgets for the
  asteroids game, and return a pointer to it. The top row
  widget contains several widgets laid out horizontally.
  Use the palette from the \a parent. Each widget we add
  becomes a child of the \a parent.
 */
QWidget*
KAstTopLevel::buildTopRow(QWidget* parent)
{
    QWidget* w = new RowWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(w);
    layout->setSpacing(0);
    layout->setMargin(0);

    QPalette palette = parent->palette();
    QFont labelFont(QApplication::font());
    labelFont.setPointSize(5);
    QLabel* label = 0;

    int layoutSpace = qApp->desktop()->width() > 200 ? 5 : 3;
    layout->addSpacing(layoutSpace);

    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/powerups/teleport"));
    label->setFixedWidth(16);
    label->setPalette(palette);
    layout->addWidget(label);

#if 0
    //label = new QLabel(tr(" T"),parent);
    label->setFont(labelFont);
    label->setFixedWidth(label->sizeHint().width());
    label->setPalette(palette);
    layout->addWidget(label);
#endif

    teleportsLCD_ = new QLCDNumber(1,parent);
    teleportsLCD_->setFrameStyle(QFrame::NoFrame);
    teleportsLCD_->setSegmentStyle(QLCDNumber::Flat);
    teleportsLCD_->setPalette(palette);
    teleportsLCD_->setFixedHeight(16);
    layout->addWidget(teleportsLCD_);
    layout->addStretch(1);

    /*
      Add an LCD widget to show the current game score. Give
      it five digits, because some players are very good.
     */
    /*
    label = new QLabel(tr("Score"),parent);
    label->setFont(labelFont);
    label->setPalette(palette);
    layout->addWidget(label);
    */
    scoreLCD_ = new QLCDNumber(5,parent);
    scoreLCD_->setFrameStyle(QFrame::NoFrame);
    scoreLCD_->setSegmentStyle(QLCDNumber::Flat);
    scoreLCD_->setFixedHeight(16);
    scoreLCD_->setPalette(palette);
    layout->addWidget(scoreLCD_);
    layout->addStretch(1);

    /*
      Add a label widget to for the game level LCD. The label
      text is "Level." Then add the game level LCD widget, with
      two digits. It can only go to 16.
     */
    label = new QLabel(tr("Lvl", "short for level"),parent);
    label->setFont(labelFont);
    label->setPalette(palette);
    layout->addWidget(label);
    levelLCD_ = new QLCDNumber(2,parent);
    levelLCD_->setFrameStyle(QFrame::NoFrame);
    levelLCD_->setSegmentStyle(QLCDNumber::Flat);
    levelLCD_->setFixedHeight(16);
    levelLCD_->setPalette(palette);
    layout->addWidget(levelLCD_);
    layout->addStretch(1);


    /*
      Add a label widget for the ship count LCD. For phones,
      set the label to a pixmap of the ship. For everyone
      else, set the label text to "Ships." Then add the ship
      count LCD with a single digit. The maximum number of
      ships is 3, i think.
     */
#ifdef QTOPIA_PHONE
    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/ship/ship0000"));
#else
    label = new QLabel(tr("Ships"),parent);
#endif
    label->setFont(labelFont);
    label->setPalette(palette);
    layout->addWidget(label);
    shipsLCD_ = new QLCDNumber(1,parent);
    shipsLCD_->setFrameStyle(QFrame::NoFrame);
    shipsLCD_->setSegmentStyle(QLCDNumber::Flat);
    shipsLCD_->setFixedHeight(16);
    shipsLCD_->setPalette(palette);
    layout->addWidget(shipsLCD_);

    return w;
}

/*!
  Build the asteroids game view widget as a child of the
  \a parent. Connect all its signals to slots in the top
  level class instance.
 */
KAsteroidsView* KAstTopLevel::buildAsteroidsView(QWidget* parent)
{
    KAsteroidsView* v = new KAsteroidsView(parent);
    connect(v,SIGNAL(shipKilled()),SLOT(slotShipKilled()));
    connect(v,SIGNAL(missileFired()),SLOT(slotMissileFired()));
    connect(v,SIGNAL(rockHit(int)),SLOT(slotRockHit(int)));
    connect(v,SIGNAL(rocksRemoved()),SLOT(slotRocksRemoved()));
    connect(v,SIGNAL(updateVitals()),SLOT(slotUpdateVitals()));
    return v;
}

/*!
  Build a widget containing the bottom row of widgets for
  the asteroids game, and return a pointer to it. The bottom
  row widget contains several widgets laid out horizontally.
  Use the palette from the \a parent. Each widget we add
  becomes a child of the \a parent.
 */
QWidget*
KAstTopLevel::buildBottomRow(QWidget* parent)
{
    QWidget* w = new RowWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(w);
    layout->setSpacing(0);
    layout->setMargin(0);

    QPalette palette = parent->palette();
    QFont labelFont(QApplication::font());
    labelFont.setPointSize(6);
    QLabel* label = 0;

    int layoutSpace = qApp->desktop()->width() > 200 ? 5 : 3;
    layout->addSpacing(layoutSpace);

    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/powerups/brake"));
    label->setFixedWidth(16);
    label->setPalette(palette);
    layout->addWidget(label);

    brakesLCD_ = new QLCDNumber(1,parent);
    brakesLCD_->setFrameStyle(QFrame::NoFrame);
    brakesLCD_->setSegmentStyle(QLCDNumber::Flat);
    brakesLCD_->setPalette(palette);
    brakesLCD_->setFixedHeight(16);
    layout->addWidget(brakesLCD_);

    layout->addSpacing(layoutSpace);

    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/powerups/shield"));
    label->setFixedWidth(16);
    label->setPalette(palette);
    layout->addWidget(label);

    shieldLCD_ = new QLCDNumber(1,parent);
    shieldLCD_->setFrameStyle(QFrame::NoFrame);
    shieldLCD_->setSegmentStyle(QLCDNumber::Flat);
    shieldLCD_->setPalette(palette);
    shieldLCD_->setFixedHeight(16);
    layout->addWidget(shieldLCD_);

    layout->addSpacing(layoutSpace);

    label = new QLabel(parent);
    label->setPixmap(QPixmap(":image/powerups/shoot"));
    label->setFixedWidth(16);
    label->setPalette(palette);
    layout->addWidget(label);

    shootLCD_ = new QLCDNumber(1,parent);
    shootLCD_->setFrameStyle(QFrame::NoFrame);
    shootLCD_->setSegmentStyle(QLCDNumber::Flat);
    shootLCD_->setPalette(palette);
    shootLCD_->setFixedHeight(16);
    layout->addWidget(shootLCD_);
    layout->addStretch(1);

    /*
    label = new QLabel(tr("Fuel"),parent);
    label->setFont(labelFont);
    label->setFixedWidth(label->sizeHint().width() + 5);
    label->setPalette(palette);
    layout->addWidget(label);
    */
    powerMeter_ = new KALedMeter(parent);
    powerMeter_->setFrameStyle(QFrame::Box | QFrame::Plain);
    powerMeter_->setMaxRawValue(MAX_SHIP_POWER_LEVEL);
    powerMeter_->setMeterLevels(qApp->desktop()->width() > 200 ? 15 : 10);
    powerMeter_->setPalette(palette);
    powerMeter_->setFixedSize(qApp->desktop()->width() > 200 ? 40 : 40, 12);
    layout->addWidget(powerMeter_);
    return w;
}

/*!
  If the event is for a key not used by the game, ignore it,
  ie allow it to be consumed higher up the food chain. If it
  is a key used by the game, but it is an auto repeat key,
  accept the event but don't actually do anything with it.

  So this function can both ignore and accept an event, and
  they both mean the event is consumed. If this function
  returns true, it means the event has been consumed and the
  caller should do nothing with it, not even accept it or
  ignore it.
 */
bool
KAstTopLevel::eventConsumed(QKeyEvent* e) const
{
    if (!actions_.contains(e->key())) {
        e->ignore();
        return true;
    }
    if (e->isAutoRepeat()) {
        e->accept();
        return true;
    }
    return false;
}

/*!
  Process each key press event. Key presses that are meant
  for the game are accepted whether they are acted on or not.
 */
void
KAstTopLevel::keyPressEvent(QKeyEvent* event)
{
    if (eventConsumed(event))
        return;

    keyIsPressed_ = true;

    Action a = actions_[ event->key() ];

    switch (a) {
        case RotateLeft:
            view_->rotateShipLeft(true);
            break;

        case RotateRight:
            view_->rotateShipRight(true);
            break;

        case Thrust:
            view_->startEngine();
            break;

        case Shoot:
            view_->startShooting();
            break;

        case Shield:
            view_->raiseShield();
            break;

        case Teleport:
            view_->teleport();
            break;

        case Brake:
            view_->startBraking();
            break;

        default:
            event->ignore();
            return;
    }
    event->accept();
}

/*!
  Process each key release event. Key presses that are meant
  for the game are accepted whether they are acted on or not.
 */
void
KAstTopLevel::keyReleaseEvent(QKeyEvent* event)
{
    if (eventConsumed(event))
        return;

    Action a = actions_[ event->key() ];

    switch (a) {
        case RotateLeft:
            view_->rotateShipLeft(false);
            break;

        case RotateRight:
            view_->rotateShipRight(false);
            break;

        case Thrust:
            view_->stopEngine();
            break;

        case Brake:
            view_->stopBraking();
            break;

        case Shield:
            break;

        case Teleport:
            break;

        case Shoot:
#ifndef QTOPIA_PHONE
            view_->stopShooting();
            break;
#else
            if (!view_->gameOver() && !waitForNewShip_) {
                view_->stopShooting();
                break;
            }
            break;
            // fall through intended
#endif
        case Launch:
            if (keyIsPressed_ && view_->gameOver()) {
                slotNewGame();
            }
            else if (keyIsPressed_ && waitForNewShip_) {
                view_->newShip();
                waitForNewShip_ = false;
                view_->hideText();
#ifdef QTOPIA_PHONE
                QSoftMenuBar::setLabel(this,
                                       Qt::Key_Select,
                                       "qasteroids/powerups/shoot",
                                       tr("Shoot"));
                updateContext1();
#endif
            }
            else {
                event->ignore();
                keyIsPressed_ = false;
                return;
            }
            break;

        case NewGame:
            slotNewGame();
            break;
        case Pause:
            {
                view_->pause(true);
                QMessageBox::information(this,
                                         tr("KAsteroids is paused"),
                                         tr("Paused"));
                view_->pause(false);
            }
            break;
        default:
            event->ignore();
            keyIsPressed_ = false;
            return;
    }

    keyIsPressed_ = false;
    event->accept();
}

/*!
  Handles the Show event \a e. Calls the base class's
  showEvent() function with \a e and resumes the game,
  ie takes the game out of its paused state. Gives the
  game the keyboard input focus.
 */
void KAstTopLevel::showEvent(QShowEvent* e)
{
    QMainWindow::showEvent(e);
    view_->pause(false);
    setFocus();
}

/*!
  Handles the Hide event \a e. Calls the base class's
  hideEvent() function with \a e and puts the game into
  its paused state.
 */
void KAstTopLevel::hideEvent(QHideEvent* e)
{
    QMainWindow::hideEvent(e);
    view_->pause(true);
}

/*!
  Takes the game out of its paused state and gives the
  game the keyboard input focus.
 */
void KAstTopLevel::focusInEvent(QFocusEvent* )
{
    view_->pause(false);
    setFocus();
}

/*!
  Puts the game into its paused state.
 */
void KAstTopLevel::focusOutEvent(QFocusEvent* )
{
    view_->pause(true);
}

/*!
  This slot function is called to start a new game. It isn't
  connected to any signals, however. I don't know why.
 */
void KAstTopLevel::slotNewGame()
{
#ifdef QTOPIA_PHONE
    delete contextMenu_;
    contextMenu_ = 0;
    QSoftMenuBar::setLabel(this,
                           Qt::Key_Select,
                           "qasteroids/powerups/shoot",
                           tr("Shoot"));
#endif
    shipCount_ = 3;
    score_ = 0;
    scoreLCD_->display(0);
    currentLevel_ = 0;
    levelLCD_->display(currentLevel_+1);
    shipsLCD_->display(shipCount_);
    view_->newGame();
    view_->setRockSpeed(gameLevels[0].rockSpeed_);
    view_->addRocks(gameLevels[0].nrocks_);
    view_->newShip();
    waitForNewShip_ = false;
    view_->hideText();
}

/*!
  Plays a sound indicating a missile was fired.
 */
void KAstTopLevel::slotMissileFired()
{
    missileFired.play();
}

/*!
  Decrements the ships remaining count and displays the new
  count. Plays a sound indicating a ship was destroyed.

  If any ships remain, the ship killed message is reported,
  and the flag to wait for a new ship is set. Otherwise, the
  game over message is reported, and the left button of the
  context menu is cleared.
 */
void KAstTopLevel::slotShipKilled()
{
    shipCount_--;
    shipsLCD_->display(shipCount_);

    shipDestroyed.play();

    if (shipCount_ > 0) {
        waitForNewShip_ = true;
        view_->reportShipKilled();
    }
    else {
        view_->endGame();
        doStats();
    }
#ifdef QTOPIA_PHONE
    QSoftMenuBar::setLabel(this,
                           Qt::Key_0,
                           "qasteroids/ship/ship0000",
                           tr("Launch"));
    QSoftMenuBar::clearLabel(this,Qt::Key_Context1);
    if (!contextMenu_)
        contextMenu_ = QSoftMenuBar::menuFor(this);
#endif
}

/*!
  This slot is called whenever the rockHit signal is emitted because
  a missile has struck a rock. \a size specifies the size of the rock
  that was hit by the missile.

  This slot increments the score by the appropriate amount, displays
  the score, and plays the rock destroyed sound.
 */
void KAstTopLevel::slotRockHit(int size)
{
    switch (size) {
        case 0:
            score_ += 10;
            break;

        case 1:
            score_ += 20;
            break;

        default:
            score_ += 40;
      }

    rockDestroyed.play();

    scoreLCD_->display(score_);
}

/*!
  This slot is called to advance to the next game level, once
  all the rocks at the current level have been destroyed. The
  game level count is incremented if we haven't already reached
  the maximum. The new rock speed is set, and then the required
  number of rocks for the new level are created. The game level
  LCD is updated to show the new level.
 */
void KAstTopLevel::slotRocksRemoved()
{
    currentLevel_++;

    if (currentLevel_ >= MAX_GAME_LEVELS)
        currentLevel_ = MAX_GAME_LEVELS - 1;

    view_->setRockSpeed(gameLevels[currentLevel_-1].rockSpeed_);
    view_->addRocks(gameLevels[currentLevel_-1].nrocks_);

    levelLCD_->display(currentLevel_+1);
}

/*!
  This function is called when the game is over. It computes
  a rock shooting efficiency number but does nothing with it.
  It reports the game over message.
 */
void KAstTopLevel::doStats()
{
    QString r("0.00");
    if (view_->shotsFired()) {
        double d = (double)view_->rocksHit() / view_->shotsFired() * 100.0;
        r = QString::number(d,'g',2);
    }

    view_->reportGameOver();
}

/*!
  This slot function updates all the LCD widgets when the
  updateVitals() signal is emitted.
 */
void KAstTopLevel::slotUpdateVitals()
{
    brakesLCD_->display(view_->brakeForce());
    shieldLCD_->display(view_->shieldStrength());
    shootLCD_->display(view_->firePower());
    teleportsLCD_->display(view_->teleportCount());
    powerMeter_->setValue(view_->shipPowerLevel());

#ifdef QTOPIA_PHONE
    updateContext1();
#endif
}

#ifdef QTOPIA_PHONE
/*!
  This function only exists in the Qtopis Phone Edition. It
  is called by the slot function slotUpdateVitals() to set
  or clear the label for the leftmost button of the three
  button bar at the bottom of the phone screen.

  If it sets a label for the leftmost button, the label it
  sets is a png for the ship's shield.
 */
void KAstTopLevel::updateContext1()
{
    static int lastChecksum = -1;

    int checksum = view_->checksum();

    if (checksum != lastChecksum) {
        if (checksum) {
            if(contextMenu_) {
                delete contextMenu_;
                contextMenu_ = 0;
            }
            QSoftMenuBar::setLabel(this,
                                   Qt::Key_Context1,
                                   "qasteroids/powerups/shield",
                                   tr("Shield"));
        }
        else {
            QSoftMenuBar::clearLabel(this,Qt::Key_Context1);
            if(waitForNewShip_) {
                if(!contextMenu_)
                    contextMenu_ = QSoftMenuBar::menuFor(this);
            }
            else {
                if(contextMenu_) {
                    delete contextMenu_;
                    contextMenu_ = 0;
                }
            }
        }
        lastChecksum = checksum;
    }
}
#endif

#include <toplevel.moc>
