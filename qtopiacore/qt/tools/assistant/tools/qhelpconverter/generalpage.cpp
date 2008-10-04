/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include <QtGui/QMessageBox>
#include "generalpage.h"

QT_BEGIN_NAMESPACE

GeneralPage::GeneralPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("General Settings"));
    setSubTitle(tr("Specify the namespace and the virtual "
        "folder for the documentation."));

    m_ui.setupUi(this);
    connect(m_ui.namespaceLineEdit, SIGNAL(textChanged(const QString&)),
        this, SIGNAL(completeChanged()));
    connect(m_ui.folderLineEdit, SIGNAL(textChanged(const QString&)),
        this, SIGNAL(completeChanged()));

    m_ui.namespaceLineEdit->setText(QLatin1String("mycompany.com"));
    m_ui.folderLineEdit->setText(QLatin1String("product_1.0"));

    registerField(QLatin1String("namespaceName"), m_ui.namespaceLineEdit);
    registerField(QLatin1String("virtualFolder"), m_ui.folderLineEdit);
}

bool GeneralPage::isComplete() const
{
    if (m_ui.namespaceLineEdit->text().isEmpty()
        || m_ui.folderLineEdit->text().isEmpty())
        return false;
    return true;
}

bool GeneralPage::validatePage()
{
    QString s = m_ui.namespaceLineEdit->text();
    if (s.contains(QLatin1Char('/')) || s.contains(QLatin1Char('\\'))) {
        QMessageBox::critical(this, tr("Namespace Error"),
            tr("The namespace contains some invalid characters."));
        return false;
    }
    s = m_ui.folderLineEdit->text();
    if (s.contains(QLatin1Char('/')) || s.contains(QLatin1Char('\\'))) {
        QMessageBox::critical(this, tr("Virtual Folder Error"),
            tr("The virtual folder contains some invalid characters."));
        return false;
    }
    return true;
}

QT_END_NAMESPACE
