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

#include "translatedialog.h"

TranslateDialog::TranslateDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);
    connect(m_ui.findNxt, SIGNAL(clicked()), this, SLOT(emitFindNext()));
    connect(m_ui.translate, SIGNAL(clicked()), this, SLOT(emitTranslateAndFindNext()));
    connect(m_ui.translateAll, SIGNAL(clicked()), this, SLOT(emitTranslateAll()));
    connect(m_ui.ledFindWhat, SIGNAL(textChanged(const QString &)), this, SLOT(verifyText(const QString &)));

    m_ui.ledFindWhat->setFocus();
    
    // disable all buttons except 'cancel'
    verifyText(QLatin1String(""));
}

void TranslateDialog::verifyText(const QString &text)
{
    bool canFind = !text.isEmpty();
    m_ui.findNxt->setEnabled(canFind);
    m_ui.translate->setEnabled(canFind);
    m_ui.translateAll->setEnabled(canFind);
}

void TranslateDialog::emitFindNext()
{
    translateAndFindNext_helper(Skip);
}

void TranslateDialog::emitTranslateAndFindNext()
{
    translateAndFindNext_helper(Translate);
}

void TranslateDialog::emitTranslateAll()
{
    translateAndFindNext_helper(TranslateAll);
}

void TranslateDialog::translateAndFindNext_helper(int mode)
{
    bool markFinished = m_ui.ckMarkFinished->isChecked() ? true : false;
    int matchOption = m_ui.ckMatchCase->isChecked() ? MatchCase : 0;

    emit translateAndFindNext(m_ui.ledFindWhat->text(), m_ui.ledTranslateTo->text(), matchOption, 
        mode, markFinished);
}

void TranslateDialog::setFindWhat(const QString &str)
{
    m_ui.ledFindWhat->setText(str);
}
