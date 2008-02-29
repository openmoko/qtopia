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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef PROMOTETOCUSTOMWIDGETDIALOG_H
#define PROMOTETOCUSTOMWIDGETDIALOG_H

#include <QtGui/QDialog>
#include <QtCore/QPair>

class QDesignerWidgetDataBaseInterface;

namespace qdesigner_internal {

namespace Ui {
    class PromoteToCustomWidgetDialog;
} // namespace Ui

class PromoteToCustomWidgetDialog : public QDialog
{
    Q_OBJECT
public:
    PromoteToCustomWidgetDialog(QDesignerWidgetDataBaseInterface *db,
                                const QString &base_class_name,
                                QWidget *parent = 0);
    virtual ~PromoteToCustomWidgetDialog();

    virtual void accept();
    QString includeFile() const;
    QString customClassName() const;

private slots:
    void checkInputs();
    void setIncludeForClass(const QString &name);

private:
    Ui::PromoteToCustomWidgetDialog *ui;
    bool m_automatic_include;
    QDesignerWidgetDataBaseInterface *m_db;
    typedef QPair<QString, QString> PromotedWidgetInfo;
    typedef QList<PromotedWidgetInfo> PromotedWidgetInfoList;
    PromotedWidgetInfoList m_promoted_list;
    QString m_base_class_name;
};

} // namespace qdesigner_internal

#endif // PROMOTETOCUSTOMWIDGETDIALOG_H
