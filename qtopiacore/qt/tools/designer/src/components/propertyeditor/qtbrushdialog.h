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

#ifndef QTBRUSHDIALOG_H
#define QTBRUSHDIALOG_H

#include <QDialog>

class QDesignerBrushManagerInterface;

namespace qdesigner_internal {

class QtBrushDialog : public QDialog
{
    Q_OBJECT
public:
    QtBrushDialog(QWidget *parent = 0);
    ~QtBrushDialog();

    void setBrush(const QBrush &brush);
    QBrush brush() const;

    void setBrushManager(QDesignerBrushManagerInterface *manager);

signals:
    void textureChooserActivated(QWidget *parent, const QBrush &initialBrush);
private:
    class QtBrushDialogPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QtBrushDialog)
    Q_DISABLE_COPY(QtBrushDialog)
};

}

#endif
