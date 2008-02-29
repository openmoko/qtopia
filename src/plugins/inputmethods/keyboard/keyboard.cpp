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

#include "keyboard.h"
#include <QVariant>
#include <qwindowsystem_qws.h>
#include <QAction>

#include <QDebug>
#include <qtopialog.h>

KeyboardConfig::~KeyboardConfig()
{
}

Keyboard::Keyboard(QWidget* parent, Qt::WFlags f) : QWSInputMethod()
{
    // The keyboard frame is meaningless after the Keyboard IM is destroyed,
    // so keep control of it by never parenting KeyboardFrame;
    // This should also help keep the keyboard on top of other widgets.
    Q_UNUSED(parent);
    keyboardFrame = new KeyboardFrame(0, f);
    connect(keyboardFrame, SIGNAL(needsPositionConfirmation()), this, SLOT(checkMicroFocus()));
};

Keyboard::~Keyboard()
{
    if(keyboardFrame){
        delete keyboardFrame;
        keyboardFrame = 0; // not strictly necessary;
    }
};

QWidget* Keyboard::frame(){
    return keyboardFrame;
};

void Keyboard::checkMicroFocus()
{
    qwsServer->sendIMQuery ( Qt::ImMicroFocus );
}

void Keyboard::queryResponse ( int property, const QVariant & result )
{
    if(property==Qt::ImMicroFocus){
        QRect resultRect = result.toRect();
        if(resultRect.isValid()){
            microX = resultRect.x();
            microY = resultRect.y();
            if(keyboardFrame->obscures(QPoint(microX, microY)))
                keyboardFrame->swapPosition();
        }
        //microFocusPending = false;
        //if (showPending) show();
    }

};

void Keyboard::resetState()
{
    if(keyboardFrame) keyboardFrame->resetState();
};

void Keyboard::swapPosition(bool){
    keyboardFrame->swapPosition();
};

void Keyboard::updateHandler(int type)
{

    switch(type){
        case QWSInputMethod::Update:
            break;
        case QWSInputMethod::FocusIn:
            break;
        case QWSInputMethod::FocusOut:
            break;
        case QWSInputMethod::Reset:
            break;
        case QWSInputMethod::Destroyed:
            break;
    };
};

void Keyboard::menuActionActivated(int v)
{
    qLog(Input) << "void Keyboard::menuItemActivated("<<v<<")";
    switch(v){
        case SwapPosition:
            qLog(Input) << "Keyboard received and recognized menu action";
            keyboardFrame->swapPosition();
            break;
        case ShowKeyboard:
            qLog(Input) << "Keyboard received activation from show menu action";
            // this will cause stateChanged to be emitted when the frame is shown
            frame()->show();
            break;
        case HideKeyboard:
            qLog(Input) << "Keyboard received activation from hide menu action";
            // this will cause stateChanged to be emitted when the frame hides
            frame()->hide();
            break;
        case Unknown:
        default:
            qLog(Input) << "Keyboard IM recieved unrecognised menu action";
    };
}
