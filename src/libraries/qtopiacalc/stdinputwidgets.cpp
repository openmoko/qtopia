/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include "stdinputwidgets.h"

// TYPE 1 BASE

Type1BaseInputWidget::Type1BaseInputWidget( QWidget* parent,  const char* name, WFlags fl )
: QWidget( parent, name, fl )
{
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, sizePolicy().hasHeightForWidth() ) );
    Type1BaseInputWidgetLayout = new QGridLayout( this );
    Type1BaseInputWidgetLayout->setSpacing( 0 );
    Type1BaseInputWidgetLayout->setMargin( 0 );
    QFont big(font());
    big.setPointSize(big.pointSize()*2);

    // Column one
    PB7 = new QToolButton( this, "PB7" );
    PB7->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB7->sizePolicy().hasHeightForWidth() ) );
    PB7->setFont(big);
    PB7->setText( tr( "7" ) );
    Type1BaseInputWidgetLayout->addWidget( PB7, 1, 0 );

    PB4 = new QToolButton( this, "PB4" );
    PB4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB4->sizePolicy().hasHeightForWidth() ) );
    PB4->setFont(big);
    PB4->setText( tr( "4" ) );
    Type1BaseInputWidgetLayout->addWidget( PB4, 2, 0 );

    PB1 = new QToolButton( this, "PB1" );
    PB1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB1->sizePolicy().hasHeightForWidth() ) );
    PB1->setFont(big);
    PB1->setText( tr( "1" ) );
    Type1BaseInputWidgetLayout->addWidget( PB1, 3, 0 );

    PB0 = new QToolButton( this, "PB0" );
    PB0->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB0->sizePolicy().hasHeightForWidth() ) );
    PB0->setFont(big);
    PB0->setText( tr( "0" ) );
    Type1BaseInputWidgetLayout->addWidget( PB0, 4, 0 );


    // Column two
    PB8 = new QToolButton( this, "PB8" );
    PB8->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB8->sizePolicy().hasHeightForWidth() ) );
    PB8->setFont(big);
    PB8->setText( tr( "8" ) );
    Type1BaseInputWidgetLayout->addWidget( PB8, 1, 1 );

    PB5 = new QToolButton( this, "PB5" );
    PB5->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB5->sizePolicy().hasHeightForWidth() ) );
    PB5->setFont(big);
    PB5->setText( tr( "5" ) );
    Type1BaseInputWidgetLayout->addWidget( PB5, 2, 1 );

    PB2 = new QToolButton( this, "PB2" );
    PB2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB2->sizePolicy().hasHeightForWidth() ) );
    PB2->setFont(big);
    PB2->setText( tr( "2" ) );
    Type1BaseInputWidgetLayout->addWidget( PB2, 3, 1 );


    // Column three
    PB9 = new QToolButton( this, "PB9" );
    PB9->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB9->sizePolicy().hasHeightForWidth() ) );
    PB9->setFont(big);
    PB9->setText( tr( "9" ) );
    Type1BaseInputWidgetLayout->addWidget( PB9, 1, 2 );

    PB6 = new QToolButton( this, "PB6" );
    PB6->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB6->sizePolicy().hasHeightForWidth() ) );
    PB6->setFont(big);
    PB6->setText( tr( "6" ) );
    Type1BaseInputWidgetLayout->addWidget( PB6, 2, 2 );

    PB3 = new QToolButton( this, "PB3" );
    PB3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB3->sizePolicy().hasHeightForWidth() ) );
    PB3->setFont(big);
    PB3->setText( tr( "3" ) );
    Type1BaseInputWidgetLayout->addWidget( PB3, 3, 2 );

    PBEval = new QToolButton( this, "PBEval" );
    PBEval->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBEval->sizePolicy().hasHeightForWidth() ) );
    PBEval->setFont(big);
    PBEval->setText( tr( "=" ) );
    Type1BaseInputWidgetLayout->addWidget( PBEval, 4, 2 );


    // Column four
    PBPlus = new QToolButton( this, "PBPlus" );
    PBPlus->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBPlus->sizePolicy().hasHeightForWidth() ) );
    PBPlus->setFont(big);
    PBPlus->setText( tr( "+" ) );
    Type1BaseInputWidgetLayout->addWidget( PBPlus, 1, 3 );

    PBMinus = new QToolButton( this, "PBMinus" );
    PBMinus->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBMinus->sizePolicy().hasHeightForWidth() ) );
    PBMinus->setFont(big);
    PBMinus->setText( tr( "-" ) );
    Type1BaseInputWidgetLayout->addWidget( PBMinus, 2, 3 );

    PBTimes = new QToolButton( this, "PBTimes" );
    PBTimes->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBTimes->sizePolicy().hasHeightForWidth() ) );
    PBTimes->setFont(big);
    PBTimes->setText( tr( "x" ) );
    Type1BaseInputWidgetLayout->addWidget( PBTimes, 3, 3 );

    PBDiv = new QToolButton( this, "PBDiv" );
    PBDiv->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBDiv->sizePolicy().hasHeightForWidth() ) );
    PBDiv->setFont(big);
    PBDiv->setText( tr( "/" ) );
    Type1BaseInputWidgetLayout->addWidget( PBDiv, 4, 3 );


    // Connect
    PBDiv->setText(QChar(0xF7));
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
}

void Type1BaseInputWidget::evalClicked() {
        sys.evaluate();
}

void Type1BaseInputWidget::val1Clicked ()
{
        sys.pushChar('1');
}

void Type1BaseInputWidget::val2Clicked ()
{
        sys.pushChar('2');
}
void Type1BaseInputWidget::val3Clicked ()
{
        sys.pushChar('3');
}

void Type1BaseInputWidget::val4Clicked ()
{
        sys.pushChar('4');
}

void Type1BaseInputWidget::val5Clicked ()
{
        sys.pushChar('5');
}

void Type1BaseInputWidget::val6Clicked ()
{
        sys.pushChar('6');
}

void Type1BaseInputWidget::val7Clicked ()
{
        sys.pushChar('7');
}

void Type1BaseInputWidget::val8Clicked ()
{
        sys.pushChar('8');
}

void Type1BaseInputWidget::val9Clicked ()
{
        sys.pushChar('9');
}

void Type1BaseInputWidget::val0Clicked ()
{
        sys.pushChar('0');
}

// TYPE 1 DECIMAL

Type1DecimalInputWidget::Type1DecimalInputWidget( QWidget* parent,  const char* name, WFlags fl )
: Type1BaseInputWidget( parent, name, fl )
{
    PBDecimal = new QToolButton( this, "PBDecimal" );
    PBDecimal->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBDecimal->sizePolicy().hasHeightForWidth() ) );
    QFont big(font());
    big.setPointSize(big.pointSize()*2);
    PBDecimal->setFont(big);
    PBDecimal->setText( tr( "." ) );
    Type1BaseInputWidgetLayout->addWidget( PBDecimal, 4, 1);
    connect (PBDecimal, SIGNAL(clicked()), this, SLOT(decimalClicked()));
    connect (PBPlus, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect (PBMinus, SIGNAL(clicked()), this, SLOT(subClicked()));
    connect (PBTimes, SIGNAL(clicked()), this, SLOT(mulClicked()));
    connect (PBDiv, SIGNAL(clicked()), this, SLOT(divClicked()));
}

void Type1DecimalInputWidget::decimalClicked () {
    sys.pushChar('.');
}
void Type1DecimalInputWidget::addClicked () {
        sys.pushInstruction(new AddDoubleDouble());
}
void Type1DecimalInputWidget::subClicked () {
        sys.pushInstruction(new SubtractDoubleDouble());
}
void Type1DecimalInputWidget::mulClicked () {
        sys.pushInstruction(new MultiplyDoubleDouble());
}
void Type1DecimalInputWidget::divClicked () {
        sys.pushInstruction(new DivideDoubleDouble());
}


// TYPE 1 FRACTION

Type1FractionInputWidget::Type1FractionInputWidget( QWidget* parent,  const char* name, WFlags fl )
: Type1BaseInputWidget( parent, name, fl )
{

    PBFraction = new QToolButton( this, "PBFraction" );
    PBFraction->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBFraction->sizePolicy().hasHeightForWidth() ) );
    QFont big(font());
    big.setPointSize(big.pointSize()*2);
    PBFraction->setFont(big);
    PBFraction->setText( tr( "/" ) );
    Type1BaseInputWidgetLayout->addWidget( PBFraction, 4, 1);
    connect (PBFraction, SIGNAL(clicked()), this, SLOT(fractionClicked()));
    connect (PBPlus, SIGNAL(clicked()), this, SLOT(addClicked()));
    connect (PBMinus, SIGNAL(clicked()), this, SLOT(subClicked()));
    connect (PBTimes, SIGNAL(clicked()), this, SLOT(mulClicked()));
    connect (PBDiv, SIGNAL(clicked()), this, SLOT(divClicked()));
}

void Type1FractionInputWidget::fractionClicked () {
    sys.pushChar('/');
}
void Type1FractionInputWidget::addClicked () {
        sys.pushInstruction(new AddFractionFraction());
}
void Type1FractionInputWidget::subClicked () {
        sys.pushInstruction(new SubtractFractionFraction());
}
void Type1FractionInputWidget::mulClicked () {
        sys.pushInstruction(new MultiplyFractionFraction());
}
void Type1FractionInputWidget::divClicked () {
        sys.pushInstruction(new DivideFractionFraction());
}

// TYPE 2

Type2InputWidget::Type2InputWidget( QWidget* parent,  const char* name, WFlags fl )
: QWidget( parent, name, fl )
{
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, sizePolicy().hasHeightForWidth() ) );
    Type2InputWidgetLayout = new QGridLayout( this );
    Type2InputWidgetLayout->setSpacing( 0 );
    Type2InputWidgetLayout->setMargin( 0 );

    PB9 = new QToolButton( this, "PB9" );
    PB9->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB9->sizePolicy().hasHeightForWidth() ) );
    PB9->setText( tr( "9" ) );

    Type2InputWidgetLayout->addWidget( PB9, 1, 3 );

    PB6 = new QToolButton( this, "PB6" );
    PB6->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB6->sizePolicy().hasHeightForWidth() ) );
    PB6->setText( tr( "6" ) );

    Type2InputWidgetLayout->addWidget( PB6, 2, 3 );

    PB3 = new QToolButton( this, "PB3" );
    PB3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB3->sizePolicy().hasHeightForWidth() ) );
    PB3->setText( tr( "3" ) );

    Type2InputWidgetLayout->addWidget( PB3, 3, 3 );

    PB7 = new QToolButton( this, "PB7" );
    PB7->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB7->sizePolicy().hasHeightForWidth() ) );
    PB7->setText( tr( "7" ) );

    Type2InputWidgetLayout->addMultiCellWidget( PB7, 1, 1, 0, 1 );

    PB4 = new QToolButton( this, "PB4" );
    PB4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB4->sizePolicy().hasHeightForWidth() ) );
    PB4->setText( tr( "4" ) );

    Type2InputWidgetLayout->addMultiCellWidget( PB4, 2, 2, 0, 1 );

    PBEval = new QToolButton( this, "PBEval" );
    PBEval->setText( tr( "=" ) );

    Type2InputWidgetLayout->addWidget( PBEval, 4, 3 );

    PB8 = new QToolButton( this, "PB8" );
    PB8->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB8->sizePolicy().hasHeightForWidth() ) );
    PB8->setText( tr( "8" ) );

    Type2InputWidgetLayout->addWidget( PB8, 1, 2 );

    PB2 = new QToolButton( this, "PB2" );
    PB2->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB2->sizePolicy().hasHeightForWidth() ) );
    PB2->setText( tr( "2" ) );

    Type2InputWidgetLayout->addWidget( PB2, 3, 2 );

    PB1 = new QToolButton( this, "PB1" );
    PB1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB1->sizePolicy().hasHeightForWidth() ) );
    PB1->setText( tr( "1" ) );

    Type2InputWidgetLayout->addMultiCellWidget( PB1, 3, 3, 0, 1 );

    PB5 = new QToolButton( this, "PB5" );
    PB5->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB5->sizePolicy().hasHeightForWidth() ) );
    PB5->setText( tr( "5" ) );

    Type2InputWidgetLayout->addWidget( PB5, 2, 2 );

    PBDiv = new QToolButton( this, "PBDiv" );
    PBDiv->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBDiv->sizePolicy().hasHeightForWidth() ) );
    PBDiv->setText( tr( "/" ) );
    Type2InputWidgetLayout->addWidget( PBDiv, 3, 0 );

    PBTimes = new QToolButton( this, "PBTimes" );
    PBTimes->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBTimes->sizePolicy().hasHeightForWidth() ) );
    PBTimes->setText( tr( "x" ) );
    Type2InputWidgetLayout->addWidget( PBTimes, 3, 1 );

    PBMinus = new QToolButton( this, "PBMinus" );
    PBMinus->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBMinus->sizePolicy().hasHeightForWidth() ) );
    PBMinus->setText( tr( "-" ) );

    Type2InputWidgetLayout->addWidget( PBMinus, 3, 2 );

    PBPlus = new QToolButton( this, "PBPlus" );
    PBPlus->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBPlus->sizePolicy().hasHeightForWidth() ) );
    PBPlus->setText( tr( "+" ) );

    Type2InputWidgetLayout->addMultiCellWidget( PBPlus, 3, 4, 4, 4 );

    PB0 = new QToolButton( this, "PB0" );
    PB0->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PB0->sizePolicy().hasHeightForWidth() ) );
    PB0->setText( tr( "0" ) );

    Type2InputWidgetLayout->addWidget( PB0, 4, 0 );

    PBDecimal = new QToolButton( this, "PBDecimal" );
    PBDecimal->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, PBDecimal->sizePolicy().hasHeightForWidth() ) );
    PBDecimal->setText( tr( "." ) );

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

    QFont big(font());
    big.setPointSize(big.pointSize()*2);
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
        sys.evaluate();
}

void Type2InputWidget::addClicked ()
{
        sys.pushInstruction(new AddDoubleDouble());
}

void Type2InputWidget::subClicked ()
{
        sys.pushInstruction(new SubtractDoubleDouble());
}

void Type2InputWidget::mulClicked ()
{
        sys.pushInstruction(new MultiplyDoubleDouble());
}

void Type2InputWidget::divClicked ()
{
        sys.pushInstruction(new DivideDoubleDouble());
}

void Type2InputWidget::decimalClicked ()
{
        sys.pushChar('.');
}

void Type2InputWidget::val1Clicked ()
{
        sys.pushChar('1');
}

void Type2InputWidget::val2Clicked ()
{
        sys.pushChar('2');
}
void Type2InputWidget::val3Clicked ()
{
        sys.pushChar('3');
}

void Type2InputWidget::val4Clicked ()
{
        sys.pushChar('4');
}

void Type2InputWidget::val5Clicked ()
{
        sys.pushChar('5');
}

void Type2InputWidget::val6Clicked ()
{
        sys.pushChar('6');
}

void Type2InputWidget::val7Clicked ()
{
        sys.pushChar('7');
}

void Type2InputWidget::val8Clicked ()
{
        sys.pushChar('8');
}

void Type2InputWidget::val9Clicked ()
{
        sys.pushChar('9');
}

void Type2InputWidget::val0Clicked ()
{
        sys.pushChar('0');
}



