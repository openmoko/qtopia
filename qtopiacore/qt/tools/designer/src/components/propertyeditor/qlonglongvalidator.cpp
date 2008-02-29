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

#include "qlonglongvalidator.h"

using namespace qdesigner_internal;

QLongLongValidator::QLongLongValidator(QObject * parent)
    : QValidator(parent),
      b(Q_UINT64_C(0x8000000000000000)), t(Q_UINT64_C(0x7FFFFFFFFFFFFFFF))
{
}

QLongLongValidator::QLongLongValidator(qlonglong minimum, qlonglong maximum,
                              QObject * parent)
    : QValidator(parent), b(minimum), t(maximum)
{
}

QLongLongValidator::~QLongLongValidator()
{
    // nothing
}

QValidator::State QLongLongValidator::validate(QString & input, int &) const
{
    if (input.contains(' '))
        return Invalid;
    if (input.isEmpty() || (b < 0 && input == "-"))
        return Intermediate;
    bool ok;
    qlonglong entered = input.toLongLong(&ok);
    if (!ok || (entered < 0 && b >= 0)) {
        return Invalid;
    } else if (entered >= b && entered <= t) {
        return Acceptable;
    } else {
        if (entered >= 0)
            return (entered > t) ? Invalid : Intermediate;
        else
            return (entered < b) ? Invalid : Intermediate;
    }
}

void QLongLongValidator::setRange(qlonglong bottom, qlonglong top)
{
    b = bottom;
    t = top;
}

void QLongLongValidator::setBottom(qlonglong bottom)
{
    setRange(bottom, top());
}

void QLongLongValidator::setTop(qlonglong top)
{
    setRange(bottom(), top);
}
