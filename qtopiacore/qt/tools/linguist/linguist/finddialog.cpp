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

/*  TRANSLATOR FindDialog

    Choose Edit|Find from the menu bar or press Ctrl+F to pop up the
    Find dialog
*/

#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    
    findNxt->setEnabled(false);

    connect(findNxt, SIGNAL(clicked()), this, SLOT(emitFindNext()));
    connect(led, SIGNAL(textChanged(const QString &)), this, SLOT(verifyText(const QString &)));
    
    led->setFocus();
}

void FindDialog::verifyText(const QString &text)
{
    findNxt->setEnabled(!text.isEmpty());
}

void FindDialog::emitFindNext()
{
    int where;
    if (sourceText != 0)
        where = (sourceText->isChecked() ? SourceText : 0) |
                (translations->isChecked() ? Translations : 0) |
                (comments->isChecked() ? Comments : 0);
    else
        where = Translations;
    emit findNext(led->text(), where, matchCase->isChecked());
}
