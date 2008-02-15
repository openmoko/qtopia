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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ORDERDIALOG_P_H
#define ORDERDIALOG_P_H

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

#include <QDialog>
#include "ui_orderdialog.h"

class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class OrderDialog: public QDialog
{
    Q_OBJECT
public:
    OrderDialog(QDesignerFormWindowInterface *form, QWidget *parent);
    virtual ~OrderDialog();

    void setPageList(QList<QWidget*> *pages);

private slots:
    void accept();
    void on_upButton_clicked();
    void on_downButton_clicked();
    void on_pageList_currentRowChanged(int row);

private:
    Ui::OrderDialog ui;
    QDesignerFormWindowInterface *m_form;
    QList<QWidget*> *m_pages;
};

}  // namespace qdesigner_internal

#endif // ORDERDIALOG_P_H
