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

#ifndef TREEWIDGETEDITOR_H
#define TREEWIDGETEDITOR_H

#include "ui_treewidgeteditor.h"

class QTreeWidget;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TreeWidgetEditor: public QDialog
{
    Q_OBJECT
public:
    TreeWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~TreeWidgetEditor();

    void fillContentsFromTreeWidget(QTreeWidget *treeWidget);

    void fillTreeWidgetFromContents(QTreeWidget *treeWidget);

private slots:
    void on_newItemButton_clicked();
    void on_newSubItemButton_clicked();
    void on_deleteItemButton_clicked();
    void on_moveItemUpButton_clicked();
    void on_moveItemDownButton_clicked();
    void on_moveItemRightButton_clicked();
    void on_moveItemLeftButton_clicked();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current,
            QTreeWidgetItem *previous);
    void on_treeWidget_itemChanged(QTreeWidgetItem *current);

    void on_itemTextLineEdit_textEdited(const QString &text);
    void on_previewPixmapItemButton_clicked();
    void on_deletePixmapItemButton_clicked();

    void on_listWidget_currentRowChanged(int currentRow);
    void on_listWidget_itemChanged(QListWidgetItem *item);

    void on_newColumnButton_clicked();
    void on_renameColumnButton_clicked();
    void on_deleteColumnButton_clicked();
    void on_moveColumnUpButton_clicked();
    void on_moveColumnDownButton_clicked();

    void on_previewPixmapColumnButton_clicked();
    void on_deletePixmapColumnButton_clicked();
private:
    void copyContents(QTreeWidget *sourceWidget, QTreeWidget *destWidget);
    void copyContents(QTreeWidgetItem *sourceItem, QTreeWidgetItem *destItem);
    void updateEditor();
    void moveColumnsLeft(int fromColumn, int toColumn);
    void moveColumnsRight(int fromColumn, int toColumn);

    Ui::TreeWidgetEditor ui;
    QDesignerFormWindowInterface *m_form;
    bool m_updating;
};

}  // namespace qdesigner_internal

#endif // TREEWIDGETEDITOR_H
