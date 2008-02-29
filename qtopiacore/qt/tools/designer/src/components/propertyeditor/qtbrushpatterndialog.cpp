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
