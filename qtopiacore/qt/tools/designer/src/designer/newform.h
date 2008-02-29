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

#ifndef NEWFORM_H
#define NEWFORM_H

#include "ui_newform.h"

#include <QDialog>

class QDesignerWorkbench;

class NewForm: public QDialog
{
    Q_OBJECT
public:
    NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget);
    virtual ~NewForm();

    QDesignerWorkbench *workbench() const;

private slots:
    void on_buttonBox_clicked(QAbstractButton *btn);
    void on_treeWidget_itemActivated(QTreeWidgetItem *item);
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *);
    void on_treeWidget_itemPressed(QTreeWidgetItem *item);

private:
    QIcon formPreviewIcon(const QString &fileName);
    void loadFrom(const QString &path, bool resourceFile);

private:
    QDesignerWorkbench *m_workbench;
    Ui::NewForm ui;
    QPushButton *createButton;
};

#endif // NEWFORM_H
