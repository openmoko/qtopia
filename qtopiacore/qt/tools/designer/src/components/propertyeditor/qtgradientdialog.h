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

#ifndef QTGRADIENTDIALOG_H
#define QTGRADIENTDIALOG_H

#include <QDialog>

namespace qdesigner_internal {

class QtGradientDialog : public QDialog
{
    Q_OBJECT
public:
    QtGradientDialog(QWidget *parent = 0);
    ~QtGradientDialog();

    void setGradient(const QGradient &gradient);
    QGradient gradient() const;

private:
    class QtGradientDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtGradientDialog)
    Q_DISABLE_COPY(QtGradientDialog)
};

}

#endif
