/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
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

