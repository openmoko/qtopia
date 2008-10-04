/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef NEWFORM_H
#define NEWFORM_H

#include "ui_newform.h"

#include <QtGui/QDialog>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QIODevice;
class QTreeWidgetItem;

class QDesignerWorkbench;

class NewForm: public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(NewForm)

public:
    NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget, const QString &fileName = QString());
    virtual ~NewForm();

    QDesignerWorkbench *workbench() const;

private slots:
    void on_buttonBox_clicked(QAbstractButton *btn);
    void on_treeWidget_itemActivated(QTreeWidgetItem *item);
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *);
    void on_treeWidget_itemPressed(QTreeWidgetItem *item);
    void recentFileChosen();

private:
    QPixmap formPreviewPixmap(const QString &fileName) const;
    QPixmap formPreviewPixmap(QIODevice &file, const QString &workingDir = QString()) const;
    QPixmap formPreviewPixmap(const QTreeWidgetItem *item);

    void loadFrom(const QString &path, bool resourceFile, const QString &uiExtension,
                  const QString &selectedItem, QTreeWidgetItem *&selectedItemFound);
    void loadFrom(const QString &title, const QStringList &nameList,
                  const QString &selectedItem, QTreeWidgetItem *&selectedItemFound);

private:
    bool openTemplate(const QTreeWidgetItem *item, QString *errorMessage);

    typedef QMap<const QTreeWidgetItem *, QPixmap> ItemPixmapMap;
    ItemPixmapMap m_itemPixmapMap;

    QDesignerWorkbench *m_workbench;
    Ui::NewForm ui;
    QPushButton *m_createButton;
    QPushButton *m_recentButton;
    QString m_fileName;
    QTreeWidgetItem *m_currentItem;
    QTreeWidgetItem *m_acceptedItem;
};

QT_END_NAMESPACE

#endif // NEWFORM_H
