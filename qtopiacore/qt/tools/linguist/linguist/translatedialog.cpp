/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include "translatedialog.h"

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE
