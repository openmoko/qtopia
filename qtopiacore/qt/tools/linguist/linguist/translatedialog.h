/****************************************************************************
**
** Copyright (C) 2006-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#ifndef TRANSLATEDIALOG_H
#define TRANSLATEDIALOG_H

#include "ui_translatedialog.h"
#include <QDialog>

class TranslateDialog : public QDialog
{
    Q_OBJECT
public:
    enum {
        Skip,
        Translate,
        TranslateAll
    };

    enum {
        MatchCase = 0x01
    };

    TranslateDialog(QWidget *parent = 0);
    void setFindWhat(const QString &str);
signals:
    void translateAndFindNext(const QString& findWhat, const QString &translateTo, int matchOption, int mode, bool markFinished);

private slots:
    void emitFindNext();
    void emitTranslateAndFindNext();
    void emitTranslateAll();
    void verifyText(const QString &text);

private:
    void translateAndFindNext_helper(int mode);
    Ui::TranslateDialog m_ui;
};

#endif  //TRANSLATEDIALOG_H

