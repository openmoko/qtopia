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

#ifndef NEWACTIONDIALOG_P_H
#define NEWACTIONDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "ui_newactiondialog.h"

namespace qdesigner_internal {

class ActionEditor;

class NewActionDialog: public QDialog
{
    Q_OBJECT
public:
    NewActionDialog(ActionEditor *parent);
    virtual ~NewActionDialog();

    QString actionText() const;
    QString actionName() const;
    QIcon actionIcon() const;

    void setActionData(const QString &text, const QString &name, const QIcon &icon);

private slots:
    void accept();
    void on_editActionText_textEdited(const QString &text);
    void on_editObjectName_textEdited(const QString &text);
    void on_iconButton_clicked();
    void on_removeIconButton_clicked();

private:
    ActionEditor *m_actionEditor;
    Ui::NewActionDialog ui;
    bool m_auto_update_object_name;

    void updateButtons();
};

} // namespace qdesigner_internal

#endif // NEWACTIONDIALOG_P_H
