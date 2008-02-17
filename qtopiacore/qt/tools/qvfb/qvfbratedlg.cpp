/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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

QVFbRateDialog::QVFbRateDialog(int rate, QWidget *parent)
    : QDialog(parent)
{
    oldRate = rate;

    QVBoxLayout *tl = new QVBoxLayout(this);
    tl->setMargin(5);

    QLabel *label = new QLabel("Target frame rate:", this);
    tl->addWidget(label);

    QHBoxLayout *hl = new QHBoxLayout();
    tl->addItem(hl);
    rateSlider = new QSlider(Qt::Horizontal);
    rateSlider->setMinimum(1);
    rateSlider->setMaximum(100);
    rateSlider->setPageStep(10);
    rateSlider->setValue(rate);
    hl->addWidget(rateSlider);
    connect(rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateChanged(int)));
    rateLabel = new QLabel(QString("%1fps").arg(rate), this);
    hl->addWidget(rateLabel);

    hl = new QHBoxLayout();
    tl->addItem(hl);
    QPushButton *pb = new QPushButton("OK", this);
    connect(pb, SIGNAL(clicked()), this, SLOT(ok()));
    hl->addWidget(pb);
    pb = new QPushButton("Cancel", this);
    connect(pb, SIGNAL(clicked()), this, SLOT(cancel()));
    hl->addWidget(pb);
}

void QVFbRateDialog::rateChanged(int r)
{
    if (rateSlider->value() != r)
	rateSlider->setValue(r);
    rateLabel->setText(QString("%1fps").arg(r));
    emit updateRate(r);
}

void QVFbRateDialog::cancel()
{
    rateChanged(oldRate);
    reject();
}

void QVFbRateDialog::ok()
{
    oldRate = rateSlider->value();
    accept();
}
