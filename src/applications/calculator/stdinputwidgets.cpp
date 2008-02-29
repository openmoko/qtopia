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
#include "stdinputwidgets.h"
#include "calculator.h"

#include <QWhatsThis>

extern Engine *systemEngine;

// TYPE 1 BASE

InputWidget::InputWidget( QWidget* parent,  Qt::WFlags fl )
: QWidget( parent, fl )
{
    setObjectName("Type1BaseInputWidget");
    setWhatsThis( tr("Click to enter digit or operator") );
    InputWidgetLayout = new QGridLayout( this );
    InputWidgetLayout->setMargin( 3 );
    InputWidgetLayout->setSpacing( 3 );

    QFont big(font());
    big.setPointSize(qMin(big.pointSize()*2,16));

    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    setSizePolicy(mySizePolicy);

    // First column
    PBBS = new QToolButton( this );
    PBBS->setSizePolicy(mySizePolicy);
    PBBS->setFont(big);
    PBBS->setText( tr("<BS") );
    InputWidgetLayout->addWidget( PBBS, 0, 0 );

    PB7 = new QToolButton( this );
    PB7->setSizePolicy(mySizePolicy);
    PB7->setFont(big);
    PB7->setText( tr( "7" ) );
    InputWidgetLayout->addWidget( PB7, 1, 0 );

    PB4 = new QToolButton( this );
    PB4->setSizePolicy(mySizePolicy);
    PB4->setFont(big);
    PB4->setText( tr( "4" ) );
    InputWidgetLayout->addWidget( PB4, 2, 0 );

    PB1 = new QToolButton( this );
    PB1->setSizePolicy(mySizePolicy);
    PB1->setFont(big);
    PB1->setText( tr( "1" ) );
    InputWidgetLayout->addWidget( PB1, 3, 0 );

    PB0 = new QToolButton( this );
    PB0->setSizePolicy(mySizePolicy);
    PB0->setFont(big);
    PB0->setText( tr( "0" ) );
    InputWidgetLayout->addWidget( PB0, 4, 0 );

    // Second column
    PBDiv = new QToolButton( this );
    PBDiv->setSizePolicy(mySizePolicy);
    PBDiv->setFont(big);
    PBDiv->setText(QString(QChar(0xF7)));
    InputWidgetLayout->addWidget( PBDiv, 0, 1 );

    PB8 = new QToolButton( this );
    PB8->setSizePolicy(mySizePolicy);
    PB8->setFont(big);
    PB8->setText( tr( "8" ) );
    InputWidgetLayout->addWidget( PB8, 1, 1 );

    PB5 = new QToolButton( this );
    PB5->setSizePolicy(mySizePolicy);
    PB5->setFont(big);
    PB5->setText( tr( "5" ) );
    InputWidgetLayout->addWidget( PB5, 2, 1 );

    PB2 = new QToolButton( this );
    PB2->setSizePolicy(mySizePolicy);
    PB2->setFont(big);
    PB2->setText( tr( "2" ) );
    InputWidgetLayout->addWidget( PB2, 3, 1 );

    // Thrid column
    PBTimes = new QToolButton( this );
    PBTimes->setSizePolicy(mySizePolicy);
    PBTimes->setFont(big);
    PBTimes->setText( tr( "x", "times" ) );
    InputWidgetLayout->addWidget( PBTimes, 0, 2 );

    PB9 = new QToolButton( this );
    PB9->setSizePolicy(mySizePolicy);
    PB9->setFont(big);
    PB9->setText( tr( "9" ) );
    InputWidgetLayout->addWidget( PB9, 1, 2 );

    PB6 = new QToolButton( this );
    PB6->setSizePolicy(mySizePolicy);
    PB6->setFont(big);
    PB6->setText( tr( "6" ) );
    InputWidgetLayout->addWidget( PB6, 2, 2 );

    PB3 = new QToolButton( this );
    PB3->setSizePolicy(mySizePolicy);
    PB3->setFont(big);
    PB3->setText( tr( "3" ) );
    InputWidgetLayout->addWidget( PB3, 3, 2 );

    QToolButton* PBNegate = new QToolButton( this );
    PBNegate->setFont(big);
    PBNegate->setSizePolicy(mySizePolicy);
    PBNegate->setText( tr("+/-") );
    InputWidgetLayout->addWidget( PBNegate, 4, 2 );

    // Forth column
    PBMinus = new QToolButton( this );
    PBMinus->setSizePolicy(mySizePolicy);
    PBMinus->setFont(big);
    PBMinus->setText( tr( "-" ) );
    InputWidgetLayout->addWidget( PBMinus, 0, 3 );

    PBPlus = new QToolButton( this );
    PBPlus->setSizePolicy(mySizePolicy);
    PBPlus->setFont(big);
    PBPlus->setText( tr( "+" ) );
    InputWidgetLayout->addWidget( PBPlus, 1, 3, 2, 1 );

    PBEval = new QToolButton( this );
    PBEval->setSizePolicy(mySizePolicy);
    PBEval->setFont(big);
    PBEval->setText( tr( "=" ) );
    InputWidgetLayout->addWidget( PBEval, 3, 3, 2, 1 );

#ifdef QTOPIA_UNPORTED
    PB0->setFocusPolicy(Qt::TabFocus);
    PB1->setFocusPolicy(Qt::TabFocus);
    PB2->setFocusPolicy(Qt::TabFocus);
    PB3->setFocusPolicy(Qt::TabFocus);
    PB4->setFocusPolicy(Qt::TabFocus);
    PB5->setFocusPolicy(Qt::TabFocus);
    PB6->setFocusPolicy(Qt::TabFocus);
    PB7->setFocusPolicy(Qt::TabFocus);
    PB8->setFocusPolicy(Qt::TabFocus);
    PB9->setFocusPolicy(Qt::TabFocus);
    PBEval->setFocusPolicy(Qt::TabFocus);
    PBDiv->setFocusPolicy(Qt::TabFocus);
    PBPlus->setFocusPolicy(Qt::TabFocus);
    PBMinus->setFocusPolicy(Qt::TabFocus);
    PBTimes->setFocusPolicy(Qt::TabFocus);
    PBNegate->setFocusPolicy(Qt::TabFocus);
    PBBS->setFocusPolicy(Qt::TabFocus);

    setTabOrder(PB0,PB1);
    setTabOrder(PB1,PB2);
    setTabOrder(PB2,PB3);
    setTabOrder(PB3,PB4);
    setTabOrder(PB4,PB5);
    setTabOrder(PB5,PB6);
    setTabOrder(PB6,PB7);
    setTabOrder(PB7,PB8);
    setTabOrder(PB8,PB9);
    setTabOrder(PB9,PBBS);
    setTabOrder(PBBS,PBDiv);
    setTabOrder(PBDiv,PBTimes);
    setTabOrder(PBTimes,PBMinus);
    setTabOrder(PBMinus,PBPlus);
    setTabOrder(PBPlus,PBEval);
    setTabOrder(PBEval,PBNegate);
#endif

    // Connect
    connect (PB0, SIGNAL(clicked()), this, SLOT(val0Clicked()));
    connect (PB1, SIGNAL(clicked()), this, SLOT(val1Clicked()));
    connect (PB2, SIGNAL(clicked()), this, SLOT(val2Clicked()));
    connect (PB3, SIGNAL(clicked()), this, SLOT(val3Clicked()));
    connect (PB4, SIGNAL(clicked()), this, SLOT(val4Clicked()));
    connect (PB5, SIGNAL(clicked()), this, SLOT(val5Clicked()));
    connect (PB6, SIGNAL(clicked()), this, SLOT(val6Clicked()));
    connect (PB7, SIGNAL(clicked()), this, SLOT(val7Clicked()));
    connect (PB8, SIGNAL(clicked()), this, SLOT(val8Clicked()));
    connect (PB9, SIGNAL(clicked()), this, SLOT(val9Clicked()));
    connect (PBEval, SIGNAL(clicked()), this, SLOT(evalClicked()));
    connect (PBPlus, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect (PBMinus, SIGNAL(clicked()), this, SLOT(subClicked()));
    connect (PBTimes, SIGNAL(clicked()), this, SLOT(mulClicked()));
    connect (PBDiv, SIGNAL(clicked()), this, SLOT(divClicked()));
    connect (PBNegate, SIGNAL(clicked()), this, SLOT(negClicked()));
    connect (PBBS, SIGNAL(clicked()), this, SLOT(bsClicked()));
}

void InputWidget::evalClicked() {
        systemEngine->evaluate();
}

void InputWidget::val1Clicked ()
{
        systemEngine->push('1');
}

void InputWidget::val2Clicked ()
{
        systemEngine->push('2');
}
void InputWidget::val3Clicked ()
{
        systemEngine->push('3');
}

void InputWidget::val4Clicked ()
{
        systemEngine->push('4');
}

void InputWidget::val5Clicked ()
{
        systemEngine->push('5');
}

void InputWidget::val6Clicked ()
{
        systemEngine->push('6');
}

void InputWidget::val7Clicked ()
{
        systemEngine->push('7');
}

void InputWidget::val8Clicked ()
{
        systemEngine->push('8');
}

void InputWidget::val9Clicked ()
{
        systemEngine->push('9');
}

void InputWidget::val0Clicked ()
{
        systemEngine->push('0');
}

void InputWidget::addClicked () {
        systemEngine->pushInstruction("Add"); // No tr
}

void InputWidget::subClicked () {
        systemEngine->pushInstruction("Subtract"); // No tr
}

void InputWidget::mulClicked () {
        systemEngine->pushInstruction("Multiply"); // No tr
}

void InputWidget::divClicked () {
        systemEngine->pushInstruction("Divide"); // No tr
}

void InputWidget::negClicked () {
        systemEngine->pushInstruction("Negate"); // No tr
}

void InputWidget::bsClicked () {
        systemEngine->delChar();
}

// DECIMAL input widget

DecimalInputWidget::DecimalInputWidget( QWidget* parent, Qt::WFlags fl )
: InputWidget( parent, fl )
{
    setObjectName("DecimalInputWidget");
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    QFont big(font());
    big.setPointSize(qMin(big.pointSize()*2,16));

    PBDecimal = new QToolButton( this );
    PBDecimal->setSizePolicy(mySizePolicy);
    PBDecimal->setText( tr( ".", "decimal point" ) );
#ifdef QTOPIA_UNPORTED
    PBDecimal->setFocusPolicy(Qt::TabFocus);
#endif
    PBDecimal->setFont(big);
    InputWidgetLayout->addWidget( PBDecimal, 4, 1);
    connect (PBDecimal, SIGNAL(clicked()), this, SLOT(decimalClicked()));
}

void DecimalInputWidget::decimalClicked () {
    systemEngine->push('.');
}

// TYPE 1 FRACTION
FractionInputWidget::FractionInputWidget( QWidget* parent, Qt::WFlags fl )
: InputWidget( parent, fl )
{
    setObjectName("FractionInputWidget");
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    QFont big(font());
    big.setPointSize(qMin(big.pointSize()*2,18));

    PBFraction = new QToolButton( this );
    PBFraction->setSizePolicy(mySizePolicy);
    PBFraction->setFocusPolicy(Qt::TabFocus);
    PBFraction->setFont(big);
    PBFraction->setText( tr( "/" , "separator numerator - denominator") );
    InputWidgetLayout->addWidget( PBFraction, 4, 1);
    connect (PBFraction, SIGNAL(clicked()), this, SLOT(fractionClicked()));
}

void FractionInputWidget::fractionClicked () {
    systemEngine->push('/');
}

