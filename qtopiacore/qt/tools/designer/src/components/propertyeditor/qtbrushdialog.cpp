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

/*
TRNASLATOR qdesigner_internal::QtBrushDialog
*/

#include "qtbrushdialog.h"
#include "ui_qtbrushdialog.h"

#include "qdebug.h"

using namespace qdesigner_internal;

namespace qdesigner_internal {

class QtBrushDialogPrivate
{
    QtBrushDialog *q_ptr;
    Q_DECLARE_PUBLIC(QtBrushDialog)
public:
    Ui::QtBrushDialog m_ui;
};

}

QtBrushDialog::QtBrushDialog(QWidget *parent)
    : QDialog(parent)
{
    d_ptr = new QtBrushDialogPrivate();
    d_ptr->q_ptr = this;
    d_ptr->m_ui.setupUi(this);

    connect(d_ptr->m_ui.brushEditor, SIGNAL(textureChooserActivated(QWidget *, const QBrush &)),
            this, SIGNAL(textureChooserActivated(QWidget *, const QBrush &)));
}

QtBrushDialog::~QtBrushDialog()
{
    delete d_ptr;
}

void QtBrushDialog::setBrush(const QBrush &brush)
{
    d_ptr->m_ui.brushEditor->setBrush(brush);
}

QBrush QtBrushDialog::brush() const
{
    return d_ptr->m_ui.brushEditor->brush();
}

void QtBrushDialog::setBrushManager(QDesignerBrushManagerInterface *manager)
{
    d_ptr->m_ui.brushEditor->setBrushManager(manager);
}

#include "moc_qtbrushdialog.cpp"
