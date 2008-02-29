/****************************************************************************
**
** Copyright (C) 2006-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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

