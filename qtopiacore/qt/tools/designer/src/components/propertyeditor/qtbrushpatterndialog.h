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

#ifndef QTBRUSHPATTERNDIALOG_H
#define QTBRUSHPATTERNDIALOG_H

#include <QDialog>

namespace qdesigner_internal {

class QtBrushPatternDialog : public QDialog
{
    Q_OBJECT
public:
    QtBrushPatternDialog(QWidget *parent = 0);
    ~QtBrushPatternDialog();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

private:
    class QtBrushPatternDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushPatternDialog)
    Q_DISABLE_COPY(QtBrushPatternDialog)
};

}

#endif
