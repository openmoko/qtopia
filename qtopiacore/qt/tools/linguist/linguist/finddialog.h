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

#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "ui_finddialog.h"
#include <QDialog>

class FindDialog : public QDialog, public Ui::FindDialog
{
    Q_OBJECT
public:
    enum {SourceText = 0x1, Translations = 0x2, Comments = 0x4};

    FindDialog(QWidget *parent = 0);

signals:
    void findNext(const QString& text, int where, bool matchCase);

private slots:
    void emitFindNext();
    void verifyText(const QString &);
};

#endif
