/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qvfbratedlg.h"
#include <QLayout>
#include <QLabel>
#include <qslider.h>
#include <QPushButton>

QVFbRateDialog::QVFbRateDialog( int rate, QWidget *parent )
    : QDialog( parent )
{
    oldRate = rate;

    QVBoxLayout *tl = new QVBoxLayout( this );
    tl->setMargin( 5 );

    QLabel *label = new QLabel( "Target frame rate:", this );
    tl->addWidget( label );

    QHBoxLayout *hl = new QHBoxLayout();
    tl->addItem(hl);
    rateSlider = new QSlider(Qt::Horizontal);
    rateSlider->setMinimum(1);
    rateSlider->setMaximum(100);
    rateSlider->setPageStep(10);
    rateSlider->setValue(rate);
    hl->addWidget( rateSlider );
    connect( rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateChanged(int)) );
    rateLabel = new QLabel( QString( "%1fps" ).arg(rate), this );
    hl->addWidget( rateLabel );

    hl = new QHBoxLayout();
    tl->addItem(hl);
    QPushButton *pb = new QPushButton( "OK", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(ok()) );
    hl->addWidget( pb );
    pb = new QPushButton( "Cancel", this );
    connect( pb, SIGNAL(clicked()), this, SLOT(cancel()) );
    hl->addWidget( pb );
}

void QVFbRateDialog::rateChanged( int r )
{
    if ( rateSlider->value() != r )
	rateSlider->setValue( r );
    rateLabel->setText( QString( "%1fps" ).arg(r) );
    emit updateRate(r);
}

void QVFbRateDialog::cancel()
{
    rateChanged( oldRate );
    reject();
}

void QVFbRateDialog::ok()
{
    oldRate = rateSlider->value();
    accept();
}
