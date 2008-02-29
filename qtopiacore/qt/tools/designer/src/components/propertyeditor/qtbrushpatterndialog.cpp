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

/*
TRANSLATOR qdesigner_internal::QtBrushPatternDialog
*/

#include "qtbrushpatterndialog.h"
#include "ui_qtbrushpatterndialog.h"

using namespace qdesigner_internal;

namespace qdesigner_internal {

class QtBrushPatternDialogPrivate
{
    QtBrushPatternDialog *q_ptr;
    Q_DECLARE_PUBLIC(QtBrushPatternDialog)
public:
    Ui::QtBrushPatternDialog m_ui;
};

}

QtBrushPatternDialog::QtBrushPatternDialog(QWidget *parent)
    : QDialog(parent)
{
    d_ptr = new QtBrushPatternDialogPrivate();
    d_ptr->q_ptr = this;
    d_ptr->m_ui.setupUi(this);
}

QtBrushPatternDialog::~QtBrushPatternDialog()
{
    delete d_ptr;
}

void QtBrushPatternDialog::setBrush(const QBrush &brush)
{
    d_ptr->m_ui.brushPatternEditor->setBrush(brush);
}

QBrush QtBrushPatternDialog::brush() const
{
    return d_ptr->m_ui.brushPatternEditor->brush();
}
