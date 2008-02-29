/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "stdinputwidgets.h"
#include "calculator.h"

#include <qwhatsthis.h>

extern Engine *systemEngine;

// TYPE 1 BASE

Type1BaseInputWidget::Type1BaseInputWidget( QWidget* parent,  const char* name, WFlags fl )
: QWidget( parent, name, fl )
{
    QWhatsThis::add( this, tr("Click to enter digit or operator") );
    Type1BaseInputWidgetLayout = new QGridLayout( this );
    Type1BaseInputWidgetLayout->setMargin( 0 );

    QFont big(font());
#ifndef QTOPIA_PHONE
    big.setPointSize(QMIN(big.pointSize()*2,18));
#else
    big.setPointSize(QMIN(big.pointSize()*2,16));
#endif

    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);
    setSizePolicy(mySizePolicy);

    // First column
    PBBS = new QToolButton( this, "PBBS" );
    PBBS->setSizePolicy(mySizePolicy);
    PBBS->setFont(big);
    PBBS->setText( tr("<BS") );
    Type1BaseInputWidgetLayout->addWidget( PBBS, 0, 0 );
    
    PB7 = new QToolButton( this, "PB7" );
    PB7->setSizePolicy(mySizePolicy);
    PB7->setFont(big);
    PB7->setText( tr( "7" ) );
    Type1BaseInputWidgetLayout->addWidget( PB7, 1, 0 );

    PB4 = new QToolButton( this, "PB4" );
    PB4->setSizePolicy(mySizePolicy);
    PB4->setFont(big);
    PB4->setText( tr( "4" ) );
    Type1BaseInputWidgetLayout->addWidget( PB4, 2, 0 );

    PB1 = new QToolButton( this, "PB1" );
    PB1->setSizePolicy(mySizePolicy);
    PB1->setFont(big);
    PB1->setText( tr( "1" ) );
    Type1BaseInputWidgetLayout->addWidget( PB1, 3, 0 );

    PB0 = new QToolButton( this, "PB0" );
    PB0->setSizePolicy(mySizePolicy);
    PB0->setFont(big);
    PB0->setText( tr( "0" ) );
    Type1BaseInputWidgetLayout->addWidget( PB0, 4, 0 );

    // Second column
    PBDiv = new QToolButton( this, "PBDiv" );
    PBDiv->setSizePolicy(mySizePolicy);
    PBDiv->setFont(big);
    PBDiv->setText(QChar(0xF7));
    Type1BaseInputWidgetLayout->addWidget( PBDiv, 0, 1 );
    
    PB8 = new QToolButton( this, "PB8" );
    PB8->setSizePolicy(mySizePolicy);
    PB8->setFont(big);
    PB8->setText( tr( "8" ) );
    Type1BaseInputWidgetLayout->addWidget( PB8, 1, 1 );

    PB5 = new QToolButton( this, "PB5" );
    PB5->setSizePolicy(mySizePolicy);
    PB5->setFont(big);
    PB5->setText( tr( "5" ) );
    Type1BaseInputWidgetLayout->addWidget( PB5, 2, 1 );

    PB2 = new QToolButton( this, "PB2" );
    PB2->setSizePolicy(mySizePolicy);
    PB2->setFont(big);
    PB2->setText( tr( "2" ) );
    Type1BaseInputWidgetLayout->addWidget( PB2, 3, 1 );

    // Thrid column
    PBTimes = new QToolButton( this, "PBTimes" );
    PBTimes->setSizePolicy(mySizePolicy);
    PBTimes->setFont(big);
    PBTimes->setText( tr( "x", "times" ) );
    Type1BaseInputWidgetLayout->addWidget( PBTimes, 0, 2 );
    
    PB9 = new QToolButton( this, "PB9" );
    PB9->setSizePolicy(mySizePolicy);
    PB9->setFont(big);
    PB9->setText( tr( "9" ) );
    Type1BaseInputWidgetLayout->addWidget( PB9, 1, 2 );

    PB6 = new QToolButton( this, "PB6" );
    PB6->setSizePolicy(mySizePolicy);
    PB6->setFont(big);
    PB6->setText( tr( "6" ) );
    Type1BaseInputWidgetLayout->addWidget( PB6, 2, 2 );

    PB3 = new QToolButton( this, "PB3" );
    PB3->setSizePolicy(mySizePolicy);
    PB3->setFont(big);
    PB3->setText( tr( "3" ) );
    Type1BaseInputWidgetLayout->addWidget( PB3, 3, 2 );
    
    QToolButton* PBNegate = new QToolButton( this, "PBNegate" );
    PBNegate->setFont(big);
    PBNegate->setSizePolicy(mySizePolicy);
    PBNegate->setText( tr("+/-") );
    Type1BaseInputWidgetLayout->addWidget( PBNegate, 4, 2 );

    // Forth column
    PBMinus = new QToolButton( this, "PBMinus" );
    PBMinus->setSizePolicy(mySizePolicy);
    PBMinus->setFont(big);
    PBMinus->setText( tr( "-" ) );
    Type1BaseInputWidgetLayout->addWidget( PBMinus, 0, 3 );
    
    PBPlus = new QToolButton( this, "PBPlus" );
    PBPlus->setSizePolicy(mySizePolicy);
    PBPlus->setFont(big);
    PBPlus->setText( tr( "+" ) );
    Type1BaseInputWidgetLayout->addMultiCellWidget( PBPlus, 1, 2, 3, 3 );
    
    PBEval = new QToolButton( this, "PBEval" );
    PBEval->setSizePolicy(mySizePolicy);
    PBEval->setFont(big);
    PBEval->setText( tr( "=" ) );
    Type1BaseInputWidgetLayout->addMultiCellWidget( PBEval, 3, 4, 3, 3 );

#ifndef QTOPIA_PHONE
    PB0->setFocusPolicy(TabFocus);
    PB1->setFocusPolicy(TabFocus);
    PB2->setFocusPolicy(TabFocus);
    PB3->setFocusPolicy(TabFocus);
    PB4->setFocusPolicy(TabFocus);
    PB5->setFocusPolicy(TabFocus);
    PB6->setFocusPolicy(TabFocus);
    PB7->setFocusPolicy(TabFocus);
    PB8->setFocusPolicy(TabFocus);
    PB9->setFocusPolicy(TabFocus);
    PBEval->setFocusPolicy(TabFocus);
    PBDiv->setFocusPolicy(TabFocus);
    PBPlus->setFocusPolicy(TabFocus);
    PBMinus->setFocusPolicy(TabFocus);
    PBTimes->setFocusPolicy(TabFocus);
    PBNegate->setFocusPolicy(TabFocus);
    PBBS->setFocusPolicy(TabFocus);

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

void Type1BaseInputWidget::evalClicked() {
        systemEngine->evaluate();
}

void Type1BaseInputWidget::val1Clicked ()
{
        systemEngine->push('1');
}

void Type1BaseInputWidget::val2Clicked ()
{
        systemEngine->push('2');
}
void Type1BaseInputWidget::val3Clicked ()
{
        systemEngine->push('3');
}

void Type1BaseInputWidget::val4Clicked ()
{
        systemEngine->push('4');
}

void Type1BaseInputWidget::val5Clicked ()
{
        systemEngine->push('5');
}

void Type1BaseInputWidget::val6Clicked ()
{
        systemEngine->push('6');
}

void Type1BaseInputWidget::val7Clicked ()
{
        systemEngine->push('7');
}

void Type1BaseInputWidget::val8Clicked ()
{
        systemEngine->push('8');
}

void Type1BaseInputWidget::val9Clicked ()
{
        systemEngine->push('9');
}

void Type1BaseInputWidget::val0Clicked ()
{
        systemEngine->push('0');
}

void Type1BaseInputWidget::addClicked () {
        systemEngine->pushInstruction("Add"); // No tr
}

void Type1BaseInputWidget::subClicked () {
        systemEngine->pushInstruction("Subtract"); // No tr
}

void Type1BaseInputWidget::mulClicked () {
        systemEngine->pushInstruction("Multiply"); // No tr
}

void Type1BaseInputWidget::divClicked () {
        systemEngine->pushInstruction("Divide"); // No tr
}

void Type1BaseInputWidget::negClicked () {
        systemEngine->pushInstruction("Negate"); // No tr
}

void Type1BaseInputWidget::bsClicked () {
        systemEngine->delChar();
}

// TYPE 1 DECIMAL

Type1DecimalInputWidget::Type1DecimalInputWidget( QWidget* parent,  const char* name, WFlags fl )
: Type1BaseInputWidget( parent, name, fl )
{
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);
    QFont big(font());
#ifndef QTOPIA_PHONE
    big.setPointSize(QMIN(big.pointSize()*2,18));
#else
    big.setPointSize(QMIN(big.pointSize()*2,16));
#endif

    PBDecimal = new QToolButton( this, "PBDecimal" );
    PBDecimal->setSizePolicy(mySizePolicy);
    PBDecimal->setText( tr( ".", "decimal point" ) );
#ifndef QTOPIA_PHONE
    PBDecimal->setFocusPolicy(TabFocus);
#endif
    PBDecimal->setFont(big);
    Type1BaseInputWidgetLayout->addWidget( PBDecimal, 4, 1);
    connect (PBDecimal, SIGNAL(clicked()), this, SLOT(decimalClicked()));

    Type1BaseInputWidgetLayout->setSpacing(3);
}

void Type1DecimalInputWidget::decimalClicked () {
    systemEngine->push('.');
}

// TYPE 1 FRACTION
Type1FractionInputWidget::Type1FractionInputWidget( QWidget* parent,  const char* name, WFlags fl )
: Type1BaseInputWidget( parent, name, fl )
{
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);
    QFont big(font());
    big.setPointSize(QMIN(big.pointSize()*2,18));

    PBFraction = new QToolButton( this, "PBFraction" );
    PBFraction->setSizePolicy(mySizePolicy);
    PBFraction->setFocusPolicy(TabFocus);
    PBFraction->setFont(big);
    PBFraction->setText( tr( "/" , "seperator numerator - denominator") );
    Type1BaseInputWidgetLayout->addWidget( PBFraction, 4, 1);
    connect (PBFraction, SIGNAL(clicked()), this, SLOT(fractionClicked()));

    Type1BaseInputWidgetLayout->setSpacing(3);
}

void Type1FractionInputWidget::fractionClicked () {
    systemEngine->push('/');
}

// TYPE 2

Type2InputWidget::Type2InputWidget( QWidget* parent,  const char* name, WFlags fl )
: QWidget( parent, name, fl )
{
    QWhatsThis::add( this, tr("Click to enter digit or operator") );
    QSizePolicy mySizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred,FALSE);
    Type2InputWidgetLayout = new QGridLayout( this );
    Type2InputWidgetLayout->setSpacing( 0 );
    Type2InputWidgetLayout->setMargin( 0 );

    PB9 = new QToolButton( this, "PB9" );
    PB9->setSizePolicy(mySizePolicy);
    PB9->setText( tr( "9" ) );

    Type2InputWidgetLayout->addWidget( PB9, 1, 3 );

    PB6 = new QToolButton( this, "PB6" );
    PB6->setSizePolicy(mySizePolicy);
    PB6->setText( tr( "6" ) );

    Type2InputWidgetLayout->addWidget( PB6, 2, 3 );

    PB3 = new QToolButton( this, "PB3" );
    PB3->setSizePolicy(mySizePolicy);
    PB3->setText( tr( "3" ) );

    Type2InputWidgetLayout->addWidget( PB3, 3, 3 );

    PB7 = new QToolButton( this, "PB7" );
    PB7->setSizePolicy(mySizePolicy);
    PB7->setText( tr( "7" ) );

    Type2InputWidgetLayout->addMultiCellWidget( PB7, 1, 1, 0, 1 );

    PB4 = new QToolButton( this, "PB4" );
    PB4->setSizePolicy(mySizePolicy);
    PB4->setText( tr( "4" ) );

    Type2InputWidgetLayout->addMultiCellWidget( PB4, 2, 2, 0, 1 );

    PBEval = new QToolButton( this, "PBEval" );
    PBEval->setText( tr( "=" ) );

    Type2InputWidgetLayout->addWidget( PBEval, 4, 3 );

    PB8 = new QToolButton( this, "PB8" );
    PB8->setSizePolicy(mySizePolicy);
    PB8->setText( tr( "8" ) );

    Type2InputWidgetLayout->addWidget( PB8, 1, 2 );

    PB2 = new QToolButton( this, "PB2" );
    PB2->setSizePolicy(mySizePolicy);
    PB2->setText( tr( "2" ) );

    Type2InputWidgetLayout->addWidget( PB2, 3, 2 );

    PB1 = new QToolButton( this, "PB1" );
    PB1->setSizePolicy(mySizePolicy);
    PB1->setText( tr( "1" ) );

    Type2InputWidgetLayout->addMultiCellWidget( PB1, 3, 3, 0, 1 );

    PB5 = new QToolButton( this, "PB5" );
    PB5->setSizePolicy(mySizePolicy);
    PB5->setText( tr( "5" ) );

    Type2InputWidgetLayout->addWidget( PB5, 2, 2 );

    PBDiv = new QToolButton( this, "PBDiv" );
    PBDiv->setSizePolicy(mySizePolicy);
    PBDiv->setText( tr( "/","division" ) );
    Type2InputWidgetLayout->addWidget( PBDiv, 3, 0 );

    PBTimes = new QToolButton( this, "PBTimes" );
    PBTimes->setSizePolicy(mySizePolicy);
    PBTimes->setText( tr( "x" ) );
    Type2InputWidgetLayout->addWidget( PBTimes, 3, 1 );

    PBMinus = new QToolButton( this, "PBMinus" );
    PBMinus->setSizePolicy(mySizePolicy);
    PBMinus->setText( tr( "-" ) );

    Type2InputWidgetLayout->addWidget( PBMinus, 3, 2 );

    PBPlus = new QToolButton( this, "PBPlus" );
    PBPlus->setSizePolicy(mySizePolicy);
    PBPlus->setText( tr( "+" ) );

    Type2InputWidgetLayout->addMultiCellWidget( PBPlus, 3, 4, 4, 4 );

    PB0 = new QToolButton( this, "PB0" );
    PB0->setSizePolicy(mySizePolicy);
    PB0->setText( tr( "0" ) );

    Type2InputWidgetLayout->addWidget( PB0, 4, 0 );

    PBDecimal = new QToolButton( this, "PBDecimal" );
    PBDecimal->setSizePolicy(mySizePolicy);
    PBDecimal->setText( tr( ".", "decimal point" ) );
    PBDecimal->setFocusPolicy(TabFocus);

    Type2InputWidgetLayout->addMultiCellWidget( PBDecimal, 4, 4, 1, 2 );

    PBDiv->setText(QChar(0xF7));

    connect (PBDecimal, SIGNAL(clicked()), this, SLOT(decimalClicked()));
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
    connect (PBPlus, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect (PBMinus, SIGNAL(clicked()), this, SLOT(subClicked()));
    connect (PBTimes, SIGNAL(clicked()), this, SLOT(mulClicked()));
    connect (PBDiv, SIGNAL(clicked()), this, SLOT(divClicked()));
    connect (PBEval, SIGNAL(clicked()), this, SLOT(evalClicked()));

    setTabOrder(PB7,PB8);
    setTabOrder(PB8,PB9);
    setTabOrder(PB9,PBPlus);
    setTabOrder(PBPlus,PB4);
    setTabOrder(PB4,PB5);
    setTabOrder(PB5,PB6);
    setTabOrder(PB6,PBMinus);
    setTabOrder(PBMinus,PB1);
    setTabOrder(PB1,PB2);
    setTabOrder(PB2,PB3);
    setTabOrder(PB3,PBTimes);
    setTabOrder(PBTimes,PB0);
    setTabOrder(PB0,PBEval);
    setTabOrder(PBEval,PBDiv);

    QFont big(font());
    big.setPointSize(QMIN(big.pointSize()*2,18));
    PBDecimal->setFont(big);
    PB0->setFont(big);
    PB1->setFont(big);
    PB2->setFont(big);
    PB3->setFont(big);
    PB4->setFont(big);
    PB5->setFont(big);
    PB6->setFont(big);
    PB7->setFont(big);
    PB8->setFont(big);
    PB9->setFont(big);
    PBPlus->setFont(big);
    PBMinus->setFont(big);
    PBTimes->setFont(big);
    PBDiv->setFont(big);
    PBEval->setFont(big);
}

void Type2InputWidget::evalClicked() {
        systemEngine->evaluate();
}

void Type2InputWidget::addClicked ()
{
        systemEngine->pushInstruction("Add"); // No tr
}

void Type2InputWidget::subClicked ()
{
        systemEngine->pushInstruction("Subtract"); // No tr
}

void Type2InputWidget::mulClicked ()
{
        systemEngine->pushInstruction("Multiply"); // No tr
}

void Type2InputWidget::divClicked ()
{
        systemEngine->pushInstruction("Divide"); // No tr
}

void Type2InputWidget::decimalClicked ()
{
        systemEngine->push('.');
}

void Type2InputWidget::val1Clicked ()
{
        systemEngine->push('1');
}

void Type2InputWidget::val2Clicked ()
{
        systemEngine->push('2');
}
void Type2InputWidget::val3Clicked ()
{
        systemEngine->push('3');
}

void Type2InputWidget::val4Clicked ()
{
        systemEngine->push('4');
}

void Type2InputWidget::val5Clicked ()
{
        systemEngine->push('5');
}

void Type2InputWidget::val6Clicked ()
{
        systemEngine->push('6');
}

void Type2InputWidget::val7Clicked ()
{
        systemEngine->push('7');
}

void Type2InputWidget::val8Clicked ()
{
        systemEngine->push('8');
}

void Type2InputWidget::val9Clicked ()
{
        systemEngine->push('9');
}

void Type2InputWidget::val0Clicked ()
{
        systemEngine->push('0');
}



