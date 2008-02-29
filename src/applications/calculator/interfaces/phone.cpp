/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "../engine.h"
#include "../phoneinstruction.h"
#include "phone.h"


#include <qsoftmenubar.h>
#include <qtopiaapplication.h>

#include <QSignalMapper>
#include <QShortcut>
#include <QKeyEvent>

static const int KEY_HOLD_TIME = 300;

FormPhone::FormPhone(QWidget *parent)
    : CalcUserInterface(parent)
{
    lastInstruction = 0;

    displayedState = drNone;
    connect(systemEngine,SIGNAL(dualResetStateChanged(ResetState)),
        this,SLOT(changeResetButtonText(ResetState)));

    lockEvaluation = firstNumber = true;
    backpressed = false;
    setFocusPolicy(Qt::TabFocus);

#if defined(QTOPIA_PHONE)
    QtopiaApplication::setInputMethodHint( this, QtopiaApplication::Number );
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
#endif

    // DPad shortcuts
    QVector<int> keyIdents(13);
    keyIdents.append(Qt::Key_Up);
    keyIdents.append(Qt::Key_Plus);
    keyIdents.append(Qt::Key_Down);
    keyIdents.append(Qt::Key_Minus);
    keyIdents.append(Qt::Key_Right);
    keyIdents.append(Qt::Key_Left);
    keyIdents.append(Qt::Key_Slash);
    keyIdents.append(Qt::Key_Select);
    keyIdents.append(Qt::Key_NumberSign);
    keyIdents.append(Qt::Key_Asterisk);
    keyIdents.append(Qt::Key_Enter);
    keyIdents.append(Qt::Key_Return);
    keyIdents.append(Qt::Key_Equal);

    signalMapper = new QSignalMapper(this);
    QShortcut * shortcut = 0;
    foreach (int ident, keyIdents) {
        shortcut = new QShortcut(QKeySequence(ident), this);
        connect(shortcut, SIGNAL(activated()), signalMapper, SLOT(map()));
        signalMapper->setMapping(shortcut, ident);
    }

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(shortcutClicked(int)));

    Instruction *da = new iPhoneDoubleFactory();
    systemEngine->registerInstruction(da);
    da = new iEvaluateLine();
    systemEngine->registerInstruction(da);
    da = new iPhoneDoubleCopy();
    systemEngine->registerInstruction(da);
    da = new iPhoneAddDoubleDouble();
    systemEngine->registerInstruction(da);
    da = new iPhoneMultiplyDoubleDouble();
    systemEngine->registerInstruction(da);
    da = new iPhoneSubtractDoubleDouble();
    systemEngine->registerInstruction(da);
    da = new iPhoneDivideDoubleDouble();
    systemEngine->registerInstruction(da);

}

void FormPhone::shortcutClicked(int keyIdent)
{
    switch(keyIdent) {
        case Qt::Key_Up :
        case Qt::Key_Plus:
            plus();
            break;
        case Qt::Key_Down:
        case Qt::Key_Minus:
            minus();
            break;
        case Qt::Key_Right:
            times();
            break;
        case Qt::Key_Left:
        case Qt::Key_Slash:
            div();
            break;
        case Qt::Key_Select:
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Equal:
            eval(); break;
        case Qt::Key_NumberSign:
            nextInstruction(); break;
        case Qt::Key_Asterisk:
            dec();
            break;
    }
}

void FormPhone::showEvent ( QShowEvent *e ) {
    systemEngine->setAccType("Double"); // No tr

    QWidget::showEvent(e);
}

void FormPhone::plus(){
    systemEngine->pushInstruction("Add");
    lastInstruction=1;
    firstNumber = false;
#if defined(QTOPIA_PHONE)
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    if ( systemEngine->error() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
#endif
    lockEvaluation = true;
}
void FormPhone::minus(){
    systemEngine->pushInstruction("Subtract");
    lastInstruction=2;
    firstNumber = false;
#if defined(QTOPIA_PHONE)
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    if ( systemEngine->error() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
#endif
    lockEvaluation = true;
}
void FormPhone::times(){
    systemEngine->pushInstruction("Multiply");
    lastInstruction=3;
    firstNumber = false;
#if defined(QTOPIA_PHONE)
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    if ( systemEngine->error() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
#endif
    lockEvaluation = true;
}
void FormPhone::div(){
    systemEngine->pushInstruction("Divide");
    lastInstruction=0;
    firstNumber = false;
#if defined(QTOPIA_PHONE)
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    if ( systemEngine->error() )
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
#endif
    lockEvaluation = true;
}
void FormPhone::eval(){
    if (
#if defined(QTOPIA_PHONE)
            !lockEvaluation
#else
            true
#endif
    ) {
        systemEngine->evaluate();
        firstNumber = true;
#if defined(QTOPIA_PHONE)
        QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
        if ( systemEngine->error() )
            QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
#endif
        lockEvaluation = true;
    }
}

void FormPhone::dec() {
    systemEngine->push('.');
}
void FormPhone::nextInstruction(){
    switch (lastInstruction){
        case 0:
            plus();
            break;
        case 1:
            minus();
            break;
        case 2:
            times();
            break;
        case 3:
            div();
            break;
    }
}

void FormPhone::changeResetButtonText ( ResetState drs ) {
    displayedState = drs;
    if (drs == drNone || drs == drHard) {
        firstNumber = true;
#if defined(QTOPIA_PHONE)
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
#endif
    } else {
#if defined(QTOPIA_PHONE)
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::BackSpace);
#endif
    }
}

void FormPhone::keyReleaseEvent(QKeyEvent *e){
    if ((e->key() == Qt::Key_Back || e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace )
            && !e->isAutoRepeat()
            && backpressed) {
        backpressed = false;
        if ( tid_hold ) {
            killTimer(tid_hold);
            tid_hold = 0;
            systemEngine->delChar();
#if defined(QTOPIA_PHONE)
            int numDataOps = systemEngine->numOps();
            if ((numDataOps % 2 == 1)) {
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
                lockEvaluation = true;
                if (numDataOps == 1)
                    firstNumber = true;
            } else {
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Ok);
                lockEvaluation = false;
            }
#endif

        } else {
            if (e->key() == Qt::Key_Back)
            close();
        }
    }
#if defined(QTOPIA_PHONE)
    switch (e->key())
    {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_Asterisk:
            if (systemEngine->numOps()%2 == 0)
                firstNumber = false;
            if (!firstNumber) {
                QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::Ok);
                lockEvaluation = false;
            }
    }
#endif

    e->accept();

}

void FormPhone::keyPressEvent(QKeyEvent *e) {
    if (e->key() != Qt::Key_NumberSign){
        lastInstruction = 0;
    }
    switch(e->key()) {
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            {
                QChar qc = e->text().at(0);
                if ( qc.isPrint() && !qc.isSpace() )
                    systemEngine->push(qc.toLatin1());
            }
            break;
        case Qt::Key_Back:
        case Qt::Key_No:
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            if ( displayedState == drNone || displayedState == drHard ) {
                if ( systemEngine->error() )
                    clearAll();
                else
                    if (e->key() == Qt::Key_Back || e->key() == Qt::Key_No)
                        close();
            } else if (!e->isAutoRepeat()){
                tid_hold = startTimer(KEY_HOLD_TIME);
                backpressed = true;
            }
            break;
        default:
            e->ignore();
            //qDebug() << "Key not handled: " << e->key();
            break;
    }
}

void FormPhone::clearAll(){
    if (displayedState == drSoft) {
        systemEngine->softReset();
        systemEngine->hardReset();
    }
    else
        systemEngine->hardReset();
#ifdef QTOPIA_PHONE
    if( !Qtopia::mousePreferred() )
        setEditFocus(true);
#endif

#if defined(QTOPIA_PHONE)
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);
    lockEvaluation = true;
#endif
}

void FormPhone::timerEvent(QTimerEvent *e){
#ifdef QTOPIA_PHONE
   if (e->timerId() == tid_hold) {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QSoftMenuBar::Back);
        killTimer(tid_hold);
        tid_hold = 0;
   }
#endif
}

