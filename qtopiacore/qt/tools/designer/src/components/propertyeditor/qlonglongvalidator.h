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

#ifndef QLONGLONGVALIDATOR_H
#define QLONGLONGVALIDATOR_H

#include <QValidator>

namespace qdesigner_internal {

class QLongLongValidator : public QValidator
{
    Q_OBJECT
    Q_PROPERTY(qlonglong bottom READ bottom WRITE setBottom)
    Q_PROPERTY(qlonglong top READ top WRITE setTop)

public:
    explicit QLongLongValidator(QObject * parent);
    QLongLongValidator(qlonglong bottom, qlonglong top, QObject * parent);
    ~QLongLongValidator();

    QValidator::State validate(QString &, int &) const;

    void setBottom(qlonglong);
    void setTop(qlonglong);
    virtual void setRange(qlonglong bottom, qlonglong top);

    qlonglong bottom() const { return b; }
    qlonglong top() const { return t; }

private:
    Q_DISABLE_COPY(QLongLongValidator)

    qlonglong b;
    qlonglong t;
};

}  // namespace qdesigner_internal

#endif // QLONGLONGVALIDATOR_H
