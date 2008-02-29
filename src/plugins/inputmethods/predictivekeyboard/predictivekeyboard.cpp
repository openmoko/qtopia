/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "predictivekeyboard.h"
#include "keyboard.h"
#include <QVariant>
#include <qwindowsystem_qws.h>
#include <QAction>
#include <QtopiaApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <qtopialog.h>

PredictiveKeyboard::PredictiveKeyboard(QWidget* parent) 
: QWSInputMethod(), mKeyboard(0), mActive(0)
{
    // The predictivekeyboard frame is meaningless after the 
    // PredictiveKeyboard IM is destroyed, so keep control of it by never 
    // parenting PredictiveKeyboardWidget;  This should also help keep the 
    // predictivekeyboard on top of other widgets.
    Q_UNUSED(parent);

    QObject::connect(qwsServer, SIGNAL(windowEvent(QWSWindow *, QWSServer::WindowEvent)), this, SLOT(windowEvent(QWSWindow *, QWSServer::WindowEvent)));
};

PredictiveKeyboard::~PredictiveKeyboard()

{
    if(mKeyboard) {
        delete mKeyboard;
        mKeyboard = 0;
    }
};

static KeyboardWidget::Config createKeyboardConfig()
{
    KeyboardWidget::Config config;

    int swidth = QApplication::desktop()->width();
    int sheight = QApplication::desktop()->width();

    config.minimumStrokeMotionPerPeriod = 50;
    config.strokeMotionPeriod = 200;
    config.maximumClickStutter = swidth / 12;
    config.maximumClickTime = 400;
    config.minimumStrokeLength = 0.3f;
    config.minimumStrokeDirectionRatio = 2.0f;
    config.selectCircleDiameter = swidth / 4;
    config.selectCircleOffset = -swidth / 4;
    config.boardChangeTime = 400;
    config.keySize.setWidth(QApplication::desktop()->width());
    config.keySize.setHeight(config.keySize.height() / 4);
    config.keyMargin = swidth / 10;
    config.bottomMargin = sheight / 24;
    config.trieFile = Qtopia::qtopiaDir() + "/etc/words.qtrie";
    config.maxGuesses = 5;
    config.optionsWindowHeight = -1;
    config.optionWordSpacing = swidth / 24;
    config.reallyNoMoveSensitivity = swidth / 48;
    config.moveSensitivity = config.maximumClickStutter;
    config.excludeDistance = (swidth * 10) / 48;

    QSettings cfg("Trolltech", "PredictiveKeyboard");
    cfg.beginGroup("Settings");

    config.minimumStrokeMotionPerPeriod = 
        cfg.value("MinimumStrokeMotionPerPeriod", 50).toInt();
    config.strokeMotionPeriod = 
        cfg.value("StrokeMotionPeriod", 200).toInt();
    config.maximumClickStutter = 
        cfg.value("MaximumClickStutter", config.maximumClickStutter).toInt();
    config.maximumClickTime = 
        cfg.value("MaximumClickTime", 400).toInt();
    config.minimumStrokeLength = 
        cfg.value("MinimumStrokeLength", 0.3f).toInt();
    config.minimumStrokeDirectionRatio = 
        cfg.value("MinimumStrokeDirectionRatio", 2.0f).toDouble();
    config.selectCircleDiameter = 
        cfg.value("SelectCircleDiameter", config.selectCircleDiameter).toInt();
    config.selectCircleOffset = 
        cfg.value("SelectCircleOffset", config.selectCircleOffset).toInt();
    config.boardChangeTime = 
        cfg.value("BoardChangeTime", 400).toInt();
    config.keySize.setWidth(cfg.value("KeySizeWidth", config.keySize.width()).toInt());
    if(!cfg.contains("KeySizeHeight")) {
        config.keySize.setHeight(config.keySize.width() / 3);
    } else {
        config.keySize.setHeight(cfg.value("KeySizeHeight", config.keySize.height()).toInt());
    }
    config.keyMargin = 
        cfg.value("KeyMargin", config.keyMargin).toInt();
    config.bottomMargin = 
        cfg.value("BottomMargin", config.bottomMargin).toInt();
    config.trieFile = 
        cfg.value("TrieFile", config.trieFile).toString();
    config.maxGuesses = 
        cfg.value("MaxGuesses", 5).toInt();
    config.optionWordSpacing = 
        cfg.value("OptionWordSpacing", config.optionWordSpacing).toInt();
    config.optionsWindowHeight = 
        cfg.value("OptionsWindowHeight", -1).toInt();
    config.reallyNoMoveSensitivity = 
        cfg.value("ReallyNoMoveSensitivity", config.reallyNoMoveSensitivity).toInt();
    config.moveSensitivity = 
        cfg.value("MoveSensitivity", config.moveSensitivity).toInt();
    config.excludeDistance = 
        cfg.value("ExcludeDistance", config.excludeDistance).toInt();

    return config;
}


QWidget* PredictiveKeyboard::widget(QWidget*)
{
    if(!mKeyboard) {
        mKeyboard = new KeyboardWidget(createKeyboardConfig(), 0);
        mKeyboard->addBoard(QStringList() << "QWERTYUIOP" << "ASDFGHJKL" << "ZXCVBNM", KeyboardWidget::UpperCase);
        mKeyboard->addBoard(QStringList() << "qwertyuiop" << "asdfghjkl" << "zxcvbnm", KeyboardWidget::LowerCase);
        mKeyboard->addBoard(QStringList() << "12345" << "67890", KeyboardWidget::Numeric);
        mKeyboard->addBoard(QStringList() << "^#@!$()*&%" << "|,.;:'?\\`" << (QString("[]+=-/~\"_") + QChar(0x21b5)), KeyboardWidget::NonAlphabet);

        QObject::connect(mKeyboard, SIGNAL(preedit(QString)), this, SLOT(preedit(QString)));
        QObject::connect(mKeyboard, SIGNAL(commit(QString)), this, SLOT(submitWord(QString)));
        QObject::connect(mKeyboard, SIGNAL(commit(QString)), this, SLOT(checkMicroFocus()));
        QObject::connect(mKeyboard, SIGNAL(backspace()), this, SLOT(erase()));
        KeyboardWidget::instantiatePopupScreen();
    }

    return mKeyboard;
};

void PredictiveKeyboard::checkMicroFocus()
{
    qwsServer->sendIMQuery ( Qt::ImMicroFocus );
}

void PredictiveKeyboard::queryResponse ( int property, const QVariant & result )
{
    if(property == Qt::ImMicroFocus) {
        QRect r = result.toRect();
        QPoint p = r.center();

        if(mActive) {
            QPoint bp = mActive->requestedRegion().boundingRect().topLeft();
            p += bp;
        }

        mKeyboard->setAcceptDest(p);
    }
};

void PredictiveKeyboard::resetState()
{
    if(mKeyboard) 
        mKeyboard->reset();
};

void PredictiveKeyboard::windowEvent(QWSWindow *w, QWSServer::WindowEvent e)
{
    if(e == QWSServer::Active)
        mActive = w;
}

void PredictiveKeyboard::updateHandler(int type)
{

    switch(type){
        case QWSInputMethod::Update:
            break;
        case QWSInputMethod::FocusIn:
            break;
        case QWSInputMethod::FocusOut:
            break;
        case QWSInputMethod::Reset:
            resetState();
            break;
        case QWSInputMethod::Destroyed:
            break;
    };
};


void PredictiveKeyboard::erase() 
{
    // TODO: Find out how to do this properly.  Sending a key event seems to
    // have fewer unexpected consequences, but doesn't seem consistent with
    // the QIMEvents that represent all the rest of the output.
    // Sending the commit string works most of the time.
    
//    QWSInputMethod::sendCommitString (QString(), -1, 1); // TODO - fix case where no text to left of cursor (currently deletes character on right)

    //qDebug() << int(QChar(Qt::Key_Backspace).unicode()); // shows 3 
    // 8 for unicode is from pkim. It doesn't match any source I could find.
    QWSServer::sendKeyEvent (8, Qt::Key_Backspace, 0, true, false); 
    QWSServer::sendKeyEvent (8, Qt::Key_Backspace, 0, false, false); 
}

void PredictiveKeyboard::submitWord(QString word) 
{
    QWSInputMethod::sendCommitString (word);
}

void PredictiveKeyboard::preedit(QString text) 
{
    Q_UNUSED(text);
//  TODO: Get rid of the preedit flicker (on mousepress).  
//  Until then, no preedit will look better.
    QWSInputMethod::sendPreeditString (text, text.length());
}

bool PredictiveKeyboard::filter ( int unicode, int keycode, int modifiers, bool isPress, bool autoRepeat )
{
    return mKeyboard && mKeyboard->filter ( unicode, keycode, modifiers, isPress, autoRepeat );

    Q_UNUSED(unicode);
    Q_UNUSED(modifiers);
    Q_UNUSED(modifiers);
    Q_UNUSED(autoRepeat);

    //Select is  Qt::Key_Select
    if (!isPress || !mKeyboard->hasText())
        return false;
    
    //Handle backspace
    if(keycode == Qt::Key_Back) {
        mKeyboard->doBackspace();
        return true;
    }

    //Handle Select Key
    if(keycode == Qt::Key_Select) {
        mKeyboard->acceptWord();
        return true;
    }

    return false;
}

