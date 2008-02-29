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
TRANSLATOR qdesigner_internal::QtGradientDialog
*/

#include "qtgradientdialog.h"
#include "ui_qtgradientdialog.h"
#include <QPushButton>

using namespace qdesigner_internal;

namespace qdesigner_internal {

class QtGradientDialogPrivate
{
    QtGradientDialog *q_ptr;
    Q_DECLARE_PUBLIC(QtGradientDialog)
public:
    Ui::QtGradientDialog m_ui;
};

}

QtGradientDialog::QtGradientDialog(QWidget *parent)
    : QDialog(parent)
{
    d_ptr = new QtGradientDialogPrivate();
    d_ptr->q_ptr = this;
    d_ptr->m_ui.setupUi(this);
    QPushButton *button = d_ptr->m_ui.buttonBox->button(QDialogButtonBox::Ok);
    if (button)
        button->setAutoDefault(false);
    button = d_ptr->m_ui.buttonBox->button(QDialogButtonBox::Cancel);
    if (button)
        button->setAutoDefault(false);
}

QtGradientDialog::~QtGradientDialog()
{
    delete d_ptr;
}

void QtGradientDialog::setGradient(const QGradient &gradient)
{
    d_ptr->m_ui.gradientEditor->setGradient(gradient);
}

QGradient QtGradientDialog::gradient() const
{
    return d_ptr->m_ui.gradientEditor->gradient();
}
