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

#ifndef QTSPINBOX_H
#define QTSPINBOX_H

#include <QSpinBox>

namespace qdesigner_internal {

class QtSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    QtSpinBox(QWidget *parent = 0);

    void stepBy(int steps);
};

class QtDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    QtDoubleSpinBox(QWidget *parent = 0);
    void fixup(QString &input) const;

    void stepBy(int steps);
};

}

#endif
