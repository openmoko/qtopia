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

#ifndef TABLEWIDGETEDITOR_H
#define TABLEWIDGETEDITOR_H

#include "ui_tablewidgeteditor.h"

class QTableWidget;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class TableWidgetEditor: public QDialog
{
    Q_OBJECT
public:
    TableWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~TableWidgetEditor();

    void fillContentsFromTableWidget(QTableWidget *tableWidget);

    void fillTableWidgetFromContents(QTableWidget *tableWidget);

private slots:

    void on_tableWidget_currentCellChanged(int currentRow, int currnetCol, int, int);
    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_itemTextLineEdit_textEdited(const QString &text);
    void on_previewPixmapItemButton_clicked();
    void on_deletePixmapItemButton_clicked();

    void on_columnsListWidget_currentRowChanged(int col);
    void on_columnsListWidget_itemChanged(QListWidgetItem *item);

    void on_newColumnButton_clicked();
    void on_renameColumnButton_clicked();
    void on_deleteColumnButton_clicked();
    void on_moveColumnUpButton_clicked();
    void on_moveColumnDownButton_clicked();

    void on_previewPixmapColumnButton_clicked();
    void on_deletePixmapColumnButton_clicked();

    void on_rowsListWidget_currentRowChanged(int row);
    void on_rowsListWidget_itemChanged(QListWidgetItem *item);

    void on_newRowButton_clicked();
    void on_renameRowButton_clicked();
    void on_deleteRowButton_clicked();
    void on_moveRowUpButton_clicked();
    void on_moveRowDownButton_clicked();

    void on_previewPixmapRowButton_clicked();
    void on_deletePixmapRowButton_clicked();
private:
    void copyContents(QTableWidget *sourceWidget, QTableWidget *destWidget);
    void updateEditor();
    void moveColumnsLeft(int fromColumn, int toColumn);
    void moveColumnsRight(int fromColumn, int toColumn);
    void moveRowsUp(int fromRow, int toRow);
    void moveRowsDown(int fromRow, int toRow);

    Ui::TableWidgetEditor ui;
    QDesignerFormWindowInterface *m_form;
    bool m_updating;
};

}  // namespace qdesigner_internal

#endif // TABLEWIDGETEDITOR_H
