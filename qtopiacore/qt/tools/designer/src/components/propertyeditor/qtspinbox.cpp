/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qtspinbox.h"

#include "qdebug.h"

using namespace qdesigner_internal;

QtSpinBox::QtSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
}

void QtSpinBox::stepBy(int steps)
{
    QSpinBox::stepBy(steps);
    emit editingFinished();
}

QtDoubleSpinBox::QtDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{
}

void QtDoubleSpinBox::stepBy(int steps)
{
    QDoubleSpinBox::stepBy(steps);
    emit editingFinished();
}

void QtDoubleSpinBox::fixup(QString &input) const
{
    QDoubleSpinBox::fixup(input);
    double val = valueFromText(input);
    input = textFromValue(val);
}

