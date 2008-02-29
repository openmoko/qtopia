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
#include <qtopiaapplication.h>


#ifndef QTOPIA_PHONE
#include <QList>
#include <QSettings>

#ifdef ENABLE_SCIENCE
#include "interfaces/advanced.h"
#endif
#ifdef ENABLE_FRACTION
#include "interfaces/fraction.h"
#endif
#ifdef ENABLE_CONVERSION
#include "interfaces/conversion.h"
#endif

#else
#include <QAction>
#include <QMenu>
#include "interfaces/phone.h"
#endif

#include <QKeyEvent>
#include <QDebug>

#include "interfaces/simple.h"
#include "calculator.h"
#include "engine.h"

Engine *systemEngine;

// Calculator class
Calculator::Calculator( QWidget * p, Qt::WFlags fl) : QWidget (p, fl)
{
    setObjectName("Calculator");

    QtopiaApplication::setInputMethodHint(this, QtopiaApplication::AlwaysOff);
    setWindowTitle(tr("Calculator", "application header"));
    systemEngine = new Engine();
    QVBoxLayout *calculatorLayout = new QVBoxLayout(this);
    calculatorLayout->setSizeConstraint(QLayout::SetNoConstraint);
    calculatorLayout->setSpacing( 0 );
    calculatorLayout->setMargin( 0 );

    LCD = new MyLcdDisplay();
    LCD->setFrameShape( QFrame::NoFrame );
    LCD->setLineWidth( 0 );
    calculatorLayout->addWidget(LCD);

    systemEngine->setDisplay(LCD);

#ifndef QT_NO_CLIPBOARD
    cb = qApp->clipboard();
#endif

    connect(systemEngine,SIGNAL(stackChanged()),
        LCD,SLOT(readStack()));

    // Load plugins
#ifndef QTOPIA_PHONE
    modeBox = new QComboBox(this);
    calculatorLayout->addWidget(modeBox);
    pluginStackedWidget = new QStackedWidget();
    connect (modeBox, SIGNAL(activated(int)), pluginStackedWidget, SLOT(setCurrentIndex(int)));
    calculatorLayout->addWidget(pluginStackedWidget);

    // add & sort user interfaces
    QList<CalcUserInterface*> uiList;
    uiList.append(new FormSimple());
#ifdef ENABLE_FRACTION
    uiList.append(new FormFraction());
#endif
#ifdef ENABLE_SCIENCE
    uiList.append(new FormAdvanced());
#endif
#ifdef ENABLE_CONVERSION
    uiList.append(new FormConversion());
#endif

    QStringList uiNames;
    foreach(CalcUserInterface* ui, uiList) {
        uiNames.append(ui->interfaceName());
    }
    uiNames.sort();

    foreach(QString uiName, uiNames) {
        for (int i=0; i<uiList.size(); i++) {
            if (uiList[i]->interfaceName() == uiName) {
                pluginStackedWidget->addWidget(uiList.takeAt(i));
                modeBox->addItem(uiName);
            }
        }
    }

    QSettings config("Trolltech","calculator"); // No tr
    config.beginGroup("View"); // No tr
    QString simpleUIName = qApp->translate("FormSimple", "Simple");
    QString uiName = config.value("lastView", simpleUIName).toString();
    int lastViewIndex = uiNames.indexOf(uiName);
    if (lastViewIndex == -1) // language has changed in the meantime
        lastViewIndex = uiNames.indexOf(simpleUIName);
    modeBox->setCurrentIndex(2/*lastViewIndex*/);
    pluginStackedWidget->setCurrentIndex(lastViewIndex);
#else
    si = 0;
    if (!Qtopia::mousePreferred()) {
        si = new FormPhone(LCD);
        QtopiaApplication::setInputMethodHint(si, QtopiaApplication::AlwaysOff);
        connect(si, SIGNAL( close() ), this, SLOT( close() ) );
        calculatorLayout->addWidget(si);
        si->setFocus( );
        LCD->setFocusPolicy( Qt::NoFocus );
    } else {
        calculatorLayout->addWidget(new FormSimple());
    }

    QMenu * cmenu = QSoftMenuBar::menuFor(this);
    QAction * a_copy = new QAction(  QIcon( ":icon/copy" ),
            tr( "Copy" ), this);
    a_copy->setWhatsThis( tr("Copy the last result.") );
    connect( a_copy, SIGNAL( triggered() ), this, SLOT( copy() ) );


    if (!Qtopia::mousePreferred()) {
        QAction * a_clear = new QAction(  QIcon( ":icon/clearall" ),
                tr( "Clear All" ), this );
        connect( a_clear, SIGNAL( triggered() ), si, SLOT( clearAll() ) );
        cmenu->addAction(a_clear);
    }
    cmenu->addAction(a_copy);
#endif // !QTOPIA_PHONE
}

Calculator::~Calculator()
{
#ifndef QTOPIA_PHONE
    if (modeBox->count() > 0 ) {
        QSettings config("Trolltech","calculator"); // No tr
        config.beginGroup("View"); // No tr
        config.setValue("lastView", modeBox->currentText() ); // No tr
    }

#endif
    if ( LCD )
        delete LCD;
    if ( systemEngine )
        delete systemEngine;
}

void Calculator::keyPressEvent(QKeyEvent *e) {
    int key = e->key();

#ifndef QT_NO_CLIPBOARD
    if (e->modifiers() & Qt::ControlModifier) {
        switch (key) {
            case Qt::Key_C:
                copy();
                break;
            case Qt::Key_V:
                paste();
                break;
            case Qt::Key_X:
                cut();
                break;
        }
        e->accept();
    }
#endif

    if (Qtopia::mousePreferred()) {
        switch (key) {
            // backspace
            case Qt::Key_Backtab:
            case Qt::Key_Backspace:
            case Qt::Key_Delete:
                    systemEngine->delChar();
                break;
            // evaluate
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Equal:
                systemEngine->evaluate();           //void paste();
    //   void cut();
                break;
            // basic mathematical keys
            case Qt::Key_Plus:
                systemEngine->pushInstruction("Add"); // No tr
                break;
            case Qt::Key_Asterisk:
                systemEngine->pushInstruction("Multiply"); // No tr
                break;
            case Qt::Key_Minus:
                systemEngine->pushInstruction("Subtract"); // No tr
                break;
            case Qt::Key_Slash:
                systemEngine->pushInstruction("Divide"); // No tr
                break;
            case Qt::Key_ParenLeft:
                systemEngine->openBrace();
                break;
            case Qt::Key_ParenRight:
                systemEngine->closeBrace();
                break;
            default:
                QString text = e->text();
                if ( !text.isEmpty() ) {
                    QChar qc = e->text().at(0);
                    if ( qc.isPrint() && !qc.isSpace() ) {
                        systemEngine->push(qc.toLatin1());
                        e->accept();
                        return;
                    }
                }
                QWidget::keyPressEvent(e);
       }
    }
    e->ignore();
}

void Calculator::copy() {
    cb->setText(systemEngine->getDisplay());
}

void Calculator::cut() {
    copy();
    systemEngine->softReset();
}

void Calculator::paste() {
    QString t = cb->text();
    if (!t.isEmpty()) {
        for (int i=0; i<t.length(); i++) {
            switch (t[i].toLatin1()) {
                case '=':
                    systemEngine->evaluate();
                    break;
                case '+':
                    systemEngine->pushInstruction("Add"); // No tr
                    break;
                case '*':
                    systemEngine->pushInstruction("Multiply"); // No tr
                    break;
                case '-':
                    systemEngine->pushInstruction("Subtract"); // No tr
                    break;
                case '/':
                    systemEngine->pushInstruction("Divide"); // No tr
                    break;
                case '(':
                    systemEngine->openBrace();
                    break;
                case ')':
                    systemEngine->closeBrace();
                    break;
                default:
                    if ( t[i].isPrint() && !t[i].isSpace() ) {
                        systemEngine->push(t[i].toLatin1());
                    }
            }
        }
    }
}

void Calculator::showEvent(QShowEvent * e)
{
    systemEngine->memoryReset();
    systemEngine->dualReset();
    QtopiaApplication::hideInputMethod();
    QWidget::showEvent( e );
}
