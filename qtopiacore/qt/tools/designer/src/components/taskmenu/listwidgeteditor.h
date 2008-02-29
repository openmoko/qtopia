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

#ifndef LISTWIDGETEDITOR_H
#define LISTWIDGETEDITOR_H

#include "ui_listwidgeteditor.h"

class QListWidget;
class QComboBox;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class ListWidgetEditor: public QDialog
{
    Q_OBJECT
public:
    ListWidgetEditor(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~ListWidgetEditor();

    void fillContentsFromListWidget(QListWidget *listWidget);
    void fillContentsFromComboBox(QComboBox *comboBox);

    int count() const;
    QIcon icon(int row) const;
    QString text(int row) const;

private slots:
    void on_newItemButton_clicked();
    void on_deleteItemButton_clicked();
    void on_moveItemUpButton_clicked();
    void on_moveItemDownButton_clicked();
    void on_listWidget_currentRowChanged(int currentRow);
    void on_listWidget_itemChanged(QListWidgetItem *item);
    void on_itemTextLineEdit_textEdited(const QString &text);
    void on_previewPixmapItemButton_clicked();
    void on_deletePixmapItemButton_clicked();

private:
    void updateEditor();
    Ui::ListWidgetEditor ui;
    QDesignerFormWindowInterface *m_form;
};

}  // namespace qdesigner_internal

#endif // LISTWIDGETEDITOR_H
